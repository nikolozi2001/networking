// Linux kernel module-ის სტანდარტული header — module_init, module_exit და სხვა
#include <linux/module.h>
// binary format-ის მხარდაჭერა — linux_binfmt, linux_binprm სტრუქტურები
#include <linux/binfmts.h>
// ფაილური სისტემის ოპერაციები
#include <linux/fs.h>
// მეხსიერების რუკირება
#include <linux/mman.h>
//
#include <linux/ptrace.h>
#include <asm/ptrace.h>
// ჩვენი ფორმატის სტრუქტურები და კონსტანტები
#include "mef_format.h"

static int mef_load_binary(struct linux_binprm *bprm) {
    int retval;
    uint32_t i;
    loff_t pos;

    // bprm->buf შეიცავს ფაილის დასაწყისს — cast-ავთ ჩვენს სტრუქტურაზე
    pmef_hdr p_hdr = (pmef_hdr)bprm->buf;

    // magic შემოწმება
    if (p_hdr->magic != MEF_HDR_MAGIC) {
        pr_err("error: magic!\n");
        return -ENOEXEC;
    }

    // ფაილის ზომის შემოწმება
    long long file_size = i_size_read(file_inode(bprm->file));
    if (file_size <= 0) {
        pr_err("error: get file size!\n");
        return -EINVAL;
    }

    // file_size შედარება — uint32_t vs long long
    if ((long long)p_hdr->file_size != file_size) {
        pr_err("error: file size!\n");
        return -EINVAL;
    }

    // ჰედერი kernel space-ში წავიკითხოთ — stack-ზე
    mef_hdr hdr_buf;
    pos = 0;
    if (kernel_read(bprm->file, &hdr_buf, sizeof(mef_hdr), &pos) != sizeof(mef_hdr)) {
        pr_err("error: kernel_read mef_hdr!\n");
        return -EIO;
    }

    // სექციების ჰედერები kernel space-ში — kvmalloc
    sec_hdr *sec_buf = kvmalloc(hdr_buf.sect_count * sizeof(sec_hdr), GFP_KERNEL);
    if (!sec_buf) {
        pr_err("error: kvmalloc sec_buf!\n");
        return -ENOMEM;
    }

    if (kernel_read(bprm->file, sec_buf, hdr_buf.sect_count * sizeof(sec_hdr), &pos)
            != hdr_buf.sect_count * sizeof(sec_hdr)) {
        pr_err("error: kernel_read sec_buf!\n");
        kvfree(sec_buf);
        return -EIO;
    }

    // kernel-ს ვაცნობებთ რომ ახალი პროცესი იწყება
    retval = begin_new_exec(bprm);
    if (retval < 0) {
        pr_err("error: begin_new_exec!\n");
        kvfree(sec_buf);
        return retval;
    }

    // სექციების ჩატვირთვა — hdr_buf და sec_buf kernel space-შია
    for (i = 0; i < hdr_buf.sect_count; i++) {
        unsigned long mapped = vm_mmap(bprm->file,
                                        hdr_buf.mem_laod + sec_buf[i].memory_offset,
                                        sec_buf[i].file_size,
                                        sec_buf[i].protect,
                                        MAP_PRIVATE | MAP_FIXED,
                                        sec_buf[i].file_offset);
        if (IS_ERR_VALUE(mapped)) {
            pr_err("MEF: სექცია %u-ის რუკირების შეცდომა\n", i);
            for (uint32_t j = 0; j < i; j++) {
                vm_munmap(hdr_buf.mem_laod + sec_buf[j].memory_offset,
                          sec_buf[j].file_size);
            }
            kvfree(sec_buf);
            return -ENOMEM;
        }
    }

    // entry point = mem_laod + code სექციის memory_offset
    unsigned long entry_point = hdr_buf.mem_laod +
                                sec_buf[hdr_buf.code_index].memory_offset;

    kvfree(sec_buf);

    // entry point-ის დაყენება
    struct pt_regs *regs = current_pt_regs();
    start_thread(regs, entry_point, bprm->p);

    return 0;
}

static struct linux_binfmt mef_binfmt = {
    .module      = THIS_MODULE,
    .load_binary = mef_load_binary,
};

static int __init mef_init(void) {
    register_binfmt(&mef_binfmt);
    pr_info("MEF format loaded\n");
    return 0;
}

static void __exit mef_exit(void) {
    unregister_binfmt(&mef_binfmt);
    pr_info("MEF format unloaded\n");
}

module_init(mef_init);
module_exit(mef_exit);
MODULE_LICENSE("GPL");
