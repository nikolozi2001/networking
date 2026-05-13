#include <stdint.h>
#include "mef_format.h"

#define PAGE_SIZE   4096

#define PROT_READ   0x1
#define PROT_WRITE  0x2
#define PROT_EXEC   0x4

#define MAP_PRIVATE 0x02
#define MAP_ANON    0x1000
#define MAP_JIT     0x0800

extern long     sys_write(int fd, const void *buf, uint64_t count);
extern long     sys_open(const char *path, int flags, int mode);
extern long     sys_lseek(int fd, long offset, int whence);
extern long     sys_read(int fd, void *buf, uint64_t count);
extern int      sys_close(int fd);
extern void*    sys_mmap(void *addr, uint64_t len, int prot, int flags, int fd, uint64_t offset);
extern int      sys_munmap(void *addr, uint64_t len);
extern void     sys_exit(int code);
extern void     pthread_jit_write_protect_np(int enabled);
extern void     sys_icache_invalidate(void *start, uint64_t size);

static void memcpy_simple(void *dst, const void *src, uint64_t n) {
    uint8_t *d = dst;
    const uint8_t *s = src;
    while(n--) *d++ = *s++;
}

int mef_loader(int argc, char **argv)
{
    if(argc < 2) return 1;

    int fd = sys_open(argv[1], 0, 0);
    if(fd < 0) return 1;

    long size = sys_lseek(fd, 0, 2);
    if(size < 0) { sys_close(fd); return 2; }
    sys_lseek(fd, 0, 0);

    if(size % PAGE_SIZE != 0) { sys_close(fd); return 3; }
    if(size < 2 * PAGE_SIZE) { sys_close(fd); return 4; }

    // ჰედერის წაკითხვა
    mef_hdr_t hdr;
    if(sys_read(fd, &hdr, sizeof(mef_hdr_t)) != sizeof(mef_hdr_t)) {
        sys_close(fd); return 5;
    }
    if(hdr.magic != MEF_HDR_MAGIC) { sys_close(fd); return 6; }
    if(hdr.cpu_type != MEF_HDR_CPU_TYPE_AARCH_64) {
        sys_write(2, "err: wrong cpu\n", 15);
        sys_close(fd); return 7;
    }
    if(hdr.os_type != MEF_HDR_OS_TYPE_MAC_OS) {
        sys_write(2, "err: wrong os\n", 14);
        sys_close(fd); return 8;
    }

    // სექციების ცხრილის წაკითხვა
    mef_sect_t sects[hdr.sect_count];
    if(sys_read(fd, sects, sizeof(mef_sect_t) * hdr.sect_count) !=
       (long)(sizeof(mef_sect_t) * hdr.sect_count)) {
        sys_close(fd); return 5;
    }

    // JIT მეხსიერების გამოყოფა
    uint64_t total = (uint64_t)hdr.vm_size * PAGE_SIZE;
    void *base = sys_mmap(0, total, PROT_READ | PROT_WRITE | PROT_EXEC,
                          MAP_PRIVATE | MAP_ANON | MAP_JIT, -1, 0);
    if(base == (void*)-1) {
        sys_write(2, "err: mmap jit\n", 14);
        sys_close(fd); return 9;
    }

    // დროებითი ბუფერი ფაილის წასაკითხად
    void *tmp = sys_mmap(0, total, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANON, -1, 0);
    if(tmp == (void*)-1) {
        sys_munmap(base, total);
        sys_close(fd); return 9;
    }

    // JIT write mode
    pthread_jit_write_protect_np(0);

    // სექციების ჩატვირთვა: ფაილი → tmp → JIT
    for(int i = 0; i < hdr.sect_count; i++) {
        uint64_t off  = (uint64_t)sects[i].offset * PAGE_SIZE;
        uint64_t sz   = (uint64_t)sects[i].size * PAGE_SIZE;

        sys_lseek(fd, (long)off, 0);
        long rd = sys_read(fd, (void *)((uint64_t)tmp + off), sz);
        if(rd != (long)sz) {
            sys_write(2, "err: read sect\n", 15);
            sys_munmap(tmp, total);
            sys_munmap(base, total);
            sys_close(fd); return 10;
        }
        memcpy_simple((void *)((uint64_t)base + off),
                      (void *)((uint64_t)tmp + off), sz);
    }
    sys_munmap(tmp, total);
    sys_close(fd);

    // icache invalidate + JIT exec mode
    sys_icache_invalidate(base, total);
    pthread_jit_write_protect_np(1);

    // entry point-ზე გადასვლა
    uint64_t entry_off = (uint64_t)sects[hdr.code_index].offset * PAGE_SIZE;
    void (*entry)(void) = (void (*)(void))((uint64_t)base + entry_off);
    entry();

    sys_munmap(base, total);
    return 0;
}
