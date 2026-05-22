#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "mef_format.h"

int main(void)
{
    uint8_t pages[2 * MEF_PAGE_SIZE];
    memset(pages, 0, sizeof(pages));

    // --- page 0: header + section table ---
    mef_hdr_t *hdr = (mef_hdr_t *)pages;
    hdr->magic      = MEF_HDR_MAGIC;
    hdr->vm_load    = 0x140000;     // load address = 0x140000 * 16384 = 0x500000000
    hdr->vm_size    = 2;            // 2 pages total
    hdr->cpu_type   = MEF_CPU_TYPE;
    hdr->os_type    = MEF_OS_TYPE;
    hdr->sect_count = 1;
    hdr->code_index = 0;
    hdr->reserved   = 0;

    pmef_sect_t sect = (pmef_sect_t)(hdr + 1);
    sect[0].offset  = 1;            // page 1
    sect[0].size    = 1;            // 1 page
    sect[0].protect = 0x5;          // PROT_READ | PROT_EXEC

    // --- page 1: ARM64 code ---
    uint8_t *code = pages + MEF_PAGE_SIZE;

    // message at offset +64 in the code page
    const char *msg = "Hello from MEF!\n";
    size_t msg_len = 16;
    memcpy(code + 64, msg, msg_len);

    // ARM64 instructions:
    //   adr  x1, .+64       -> load address of message string
    //   mov  x2, #16         -> length
    //   mov  x0, #1          -> fd = stdout
    //   mov  x16, #4         -> write syscall
    //   svc  #0x80
    //   mov  x0, #0          -> exit code 0
    //   mov  x16, #1         -> exit syscall
    //   svc  #0x80

    uint32_t *insn = (uint32_t *)code;
    insn[0] = 0x10000201;  // adr x1, #64  (PC + 64 = message)
    insn[1] = 0xD2800202;  // mov x2, #16
    insn[2] = 0xD2800020;  // mov x0, #1
    insn[3] = 0xD2800090;  // mov x16, #4
    insn[4] = 0xD4001001;  // svc #0x80
    insn[5] = 0xD2800000;  // mov x0, #0
    insn[6] = 0xD2800030;  // mov x16, #1
    insn[7] = 0xD4001001;  // svc #0x80

    FILE *fp = fopen("test.mef", "wb");
    if (!fp) {
        perror("fopen");
        return 1;
    }
    fwrite(pages, 1, sizeof(pages), fp);
    fclose(fp);

    printf("Generated test.mef (2 pages, %zu bytes)\n", sizeof(pages));
    return 0;
}
