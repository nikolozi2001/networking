#include <stdint.h>
#include "mef_format.h"

#define PAGE_SIZE   4096
//
#define PROT_READ   0x1
#define PROT_WRITE  0x2
#define PROT_EXEC   0x4
//
#define MAP_PRIVATE 0x02
#define MAP_FIXED   0x10

// syscall-ების დეკლარაცია
extern long     sys_write(int fd, const void *buf, uint64_t count);
extern long     sys_open(const char *path, int flags, int mode);
extern long     sys_lseek(int fd, long offset, int whence);
extern long     sys_read(int fd, void *buf, uint64_t count);
extern int      sys_close(int fd);
extern void*    sys_mmap(void *addr, uint64_t len, int prot, int flags, int fd, uint64_t offset);
extern int      sys_munmap(void *addr, uint64_t len);
extern void     sys_exit(int code);
//
void print_hex(uint64_t val) {
    char buf[20] = "0x000000000000000\n";
    for(int i = 15; i >= 2; i--) {
        int d = val & 0xF;
        buf[i] = d < 10 ? '0' + d : 'a' + d - 10;
        val >>= 4;
    }
    sys_write(2, buf, 19);
}
//
int mef_loader(int argc, char **argv)
{
    // ფაილის გახსნა
    int fd = sys_open(argv[1], 0, 0);
    if(fd < 0) {
        return 1;
    }

    // ფაილის ზომის დადგენა
    long size = sys_lseek(fd, 0, 2);   // SEEK_END = 2
    if(size < 0){
        sys_close(fd);
        return 2;
    }
    // დაბრუნება დასაწყისში
    sys_lseek(fd, 0, 0);
    // ზომა უნდა იყოს 4096-ის ჯერადი
    if(size % PAGE_SIZE != 0) {
        sys_close(fd);
        return 3;
    }

    // მინიმუმ 2 გვერდი უნდა იყოს
    if(size < 2 * PAGE_SIZE) {
        sys_close(fd);
        return 4;
    }

    // ფაილის ჰედერის წაკითხვა
    mef_hdr_t hdr;
    if(sys_read(fd, &hdr, sizeof(mef_hdr_t)) != sizeof(mef_hdr_t)) {
        sys_close(fd);
        return 5;
    }
    // ჰედერის ვალიდაცია
    if(hdr.magic != MEF_HDR_MAGIC) {
        sys_close(fd);
        return 6;
    }
    if(hdr.cpu_type != MEF_HDR_CPU_TYPE_X86_64) {
        sys_write(2, "შეცდომა: პროგრამა განკუთვნილია სხვა პროცესორისთვის!\n", 143);
        sys_close(fd);
        return 7;
    }
    if(hdr.os_type != MEF_HDR_OS_TYPE_LINUX) {
        sys_write(2, "შეცდომა: პროგრამა განკუთვნილია სხვა სისტემისთვის!\n", 137);
        sys_close(fd);
        return 8;
    }
    // სექციების წაკითხვა
    pmef_hdr_t p_mv_hdr = (pmef_hdr_t)sys_mmap((void *)((uint64_t)hdr.vm_load * PAGE_SIZE), PAGE_SIZE, 
        PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_FIXED, fd, 0);
    if(p_mv_hdr == (void*)-1) {
        sys_write(2, "შეცდომა: ჰედერის ჩატვირთვა ვერ მოხერხდა!\n", 110);
        sys_close(fd);
        return 9;
    }
    //
    pmef_sect_t p_sect = (pmef_sect_t)(p_mv_hdr + 1);
    for(int i = 0; i < hdr.sect_count; i++) {
        int prot = 0;
        prot = prot + (p_sect[i].protect & PROT_READ ? PROT_READ : 0) + 
               (p_sect[i].protect & PROT_WRITE ? PROT_WRITE : 0) + 
               (p_sect[i].protect & PROT_EXEC ? PROT_EXEC : 0);
        void *addr = sys_mmap((void *)((uint64_t)p_mv_hdr + (uint64_t)p_sect[i].offset * PAGE_SIZE), (uint64_t)p_sect[i].size * PAGE_SIZE, 
            prot, MAP_PRIVATE | MAP_FIXED, fd, (uint64_t)p_sect[i].offset * PAGE_SIZE);
        if(addr == (void*)-1) {
            sys_write(2, "შეცდომა: სექციის ჩატვირთვა ვერ მოხერხდა!\n", 110);
            for(int j = 0; j < i; j++) {
                sys_munmap((void *)((uint64_t)p_mv_hdr + (uint64_t)p_sect[j].offset * PAGE_SIZE), (uint64_t)p_sect[j].size * PAGE_SIZE);
            }
            sys_munmap(p_mv_hdr, PAGE_SIZE);
            sys_close(fd);
            return 10;
        }
    }
    // ფაილის დახურვა
    sys_close(fd);
    // entry point-ის გამოთვლა და გადასვლა
    void (*entry)(void) = (void (*)(void))((uint64_t)p_mv_hdr + (uint64_t)p_sect[hdr.code_index].offset * PAGE_SIZE);
    // გადასვლა entry point-ზე
    entry();
    //
    return 0;
}
