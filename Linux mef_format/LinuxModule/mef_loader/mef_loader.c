#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/binfmts.h>
#include <linux/ptrace.h>
#include <asm/ptrace.h>
#include <linux/slab.h>

#include "mef_format.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("გიორგი მოდაბაძე");
MODULE_DESCRIPTION("MEF(Minimal Execute Format) ფაილების ჩამტვირთავი");
MODULE_VERSION("0.1");

static int mef_load_binary(struct linux_binprm *bprm)
{
    //
    pmef_hdr_t p_hdr = (pmef_hdr_t)bprm->buf;
    // ვამოწმებთ ჰედერის ვალიდურობას.
    if(p_hdr->magic != MEF_HDR_MAGIC) {
        printk(KERN_ERR "mef magic არასწორია\n");
        return -ENOEXEC;
    }
    // ვამოწმებთ ფაილის ზომას ჰედერის მითითებულ ზომასთან.
    if(i_size_read(file_inode(bprm->file)) != p_hdr->vm_size * PAGE_SIZE) {
        printk(KERN_ERR "mef ფაილის ზომა არასწორია\n");
        return -ENOEXEC;
    }
    // ვამოწმებთ სექციების რაოდენობას, ნული არ უნდა იყოს.
    if (p_hdr->sect_count == 0) {
        printk(KERN_ERR "mef: სექციების რაოდენობა არ შეიძლება იყოს 0-ლი\n");
        return -ENOEXEC;
    }
    // სექციების რაოდენობა არ უნდა აღემატებოდეს 127-ს.
    if (p_hdr->sect_count > 127) {
        printk(KERN_ERR "mef: სექციების რაოდენობა არ უნდა აღემატებოდეს 128-ს\n");
        return -ENOEXEC;
    }
    // გამოვყოფთ მეხსიერებას ჰედერისა და სექციების ინფორმაციისთვის.
    p_hdr = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if(!p_hdr) {
        printk(KERN_ERR "mef: მეხსიერების გამოყოფა ვერ მოხერხდა\n");
        return -ENOMEM;
    }
    // მთელი ჰედერი გადმოგვაქვს მეხსიერებაში.
    loff_t pos = 0;
    if (kernel_read(bprm->file, p_hdr, PAGE_SIZE, &pos) < 0) {
        printk(KERN_ERR "mef: ჰედერის წაკითხვა ვერ მოხერხდა\n");
        kfree(p_hdr);
        return -EIO;
    }
    //
    printk(KERN_INFO "mef: vm_load=%u vm_size(pages)=%u sect_count=%u code_index=%u\n",
           p_hdr->vm_load, p_hdr->vm_size, p_hdr->sect_count, p_hdr->code_index);
    pmef_sect_t p_sect = (pmef_sect_t)(p_hdr + 1);
    // ბირთვს ვატყობინებთ ახალი პროცესის დაწყებას.
    int res = begin_new_exec(bprm);
    if(res) {
        printk(KERN_ERR "begin_new_exec ჩავარდა: %d\n", res);
        kfree(p_hdr);
        return res;
    }
    // ვამზადებთ პროცესის მეხსიერებას ახალი პროგრამისთვის.
    setup_new_exec(bprm);
    // ჰედერის ვირტუალურ მეხსიერებაში ჩატვირთვა.
    uint64_t mv_hdr = vm_mmap( bprm->file, p_hdr->vm_load * PAGE_SIZE, p_hdr->vm_size * PAGE_SIZE,
        PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, 0);
    if (IS_ERR_VALUE(mv_hdr)) {
        printk(KERN_ERR "mef: ჰედერი ჩაიტვირთა ვირტუალურ მეხსიერებაში ვერ მოხერხდა\n");
        kfree(p_hdr);
        return -ENOMEM;
    }
    // სექციების ჩატვირთვა ვირტუალურ მეხსიერებაში.
    for(uint8_t i = 0; i < p_hdr->sect_count; i++) {
        uint64_t prot = 0;
        if(p_sect[i].protect & 0x01) {
            prot |= PROT_READ;
        }
        if(p_sect[i].protect & 0x02) {
            prot |= PROT_WRITE;
        }
        if(p_sect[i].protect & 0x04) {
            prot |= PROT_EXEC;
        }
        // ვატვირთავთ სექციას ვირტუალურ მეხსიერებაში.
        uint64_t vm_sec = vm_mmap( bprm->file, (p_hdr->vm_load + p_sect[i].offset) * PAGE_SIZE,
            p_sect[i].size * PAGE_SIZE,  prot, MAP_PRIVATE | MAP_FIXED, p_sect[i].offset * PAGE_SIZE);
        if (IS_ERR_VALUE(vm_sec)) {
            printk(KERN_ERR "mef: სექცია %u ვერ ჩაიტვირთა ვირტუალურ მეხსიერებაში\n", i);
            // უკვე გამოყოფილი ვირტუალური მეხსიერების გათავისუფლება.
            for(uint8_t j = 0; j < i; j++) {
                vm_munmap((p_hdr->vm_load + p_sect[j].offset) * PAGE_SIZE, p_sect[j].size * PAGE_SIZE);
            }
            // ჰედერის ვირტუალური მეხსიერების გათავისუფლება.
            vm_munmap((p_hdr->vm_load * PAGE_SIZE), p_hdr->vm_size * PAGE_SIZE);
            // გამოყოფილი მეხსიერების გათავისუფლება.
            kfree(p_hdr);
            return -ENOMEM;
        }
    }
    // სექციების ჩატვირთვის შემდეგ ვითვლით კოდის სექსიის ჩატვირთვის მისამართს.
    uint64_t entry_point = (p_hdr->vm_load + p_sect[p_hdr->code_index].offset) * PAGE_SIZE;
    // გამოყოფილი მეხსიერების გათავისუფლება.
    kfree(p_hdr);
    // ვამზადებთ პროცესის რეგისტრებს კოდის სექციის მისამართით და ვიწყებთ პროცესის შესრულებას.
    struct pt_regs *regs = current_pt_regs();
    // ვასრულებთ პროცესის დაწყების შემდგომ საჭირო მოქმედებებს.
    finalize_exec(bprm);
    // ვუშვებთ პროცესის პირველად ნაკადს შესრულებაზე.
    start_thread(regs, entry_point, bprm->p);
    // ვასრულებთ mef_loader-ის მუშაობას წარმატებით.
    return 0;
}
// 
static struct linux_binfmt mef_binfmt = {
    .module      = THIS_MODULE,     // მიმდინარე მოდული
    .load_binary = mef_load_binary, // ბინარული ფორმატის ჩატვირთვის ფუნქცია
};

static int __init mef_init(void)
{
    // ვაკეთებთ ახალი ბინარული ფორმატის ჩამტვირთავის რეგისტრაციას.
    register_binfmt(&mef_binfmt);
	printk(KERN_INFO "mef: ჩამტვირთავი ჩაიტვირთა\n");
	return 0;
}

static void __exit mef_exit(void)
{
    // ბინარული ფორმატის ჩამტვირთავის რეგისტრაციის გაუქმება.
    unregister_binfmt(&mef_binfmt);
	printk(KERN_INFO "mef: ჩამტვირთავი ამოიტვირთა\n");
}

module_init(mef_init);
module_exit(mef_exit);
