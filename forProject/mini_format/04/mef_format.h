#ifndef _ME_FORMAT_H_
#define _ME_FORMAT_H_
//
#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif
// magic
#define MEF_HDR_MAGIC               0x66656D7F
// cpu_type
#define MEF_HDR_CPU_TYPE_UNKNOWN    0x00
#define MEF_HDR_CPU_TYPE_X86        0x01
#define MEF_HDR_CPU_TYPE_X86_64     0x02
#define MEF_HDR_CPU_TYPE_AARCH      0x03
#define MEF_HDR_CPU_TYPE_AARCH_64   0x04
//
#define MEF_HDR_OS_TYPE_UNKNOWN     0x00
#define MEF_HDR_OS_TYPE_LINUX       0x01
#define MEF_HDR_OS_TYPE_WINDOWS     0x02
#define MEF_HDR_OS_TYPE_MAC_OS      0x03
//
typedef struct __attribute__((packed)){
    uint32_t    magic;
    uint32_t    mem_laod;
    uint32_t    file_size;
    uint8_t     cpu_type;
    uint8_t     os_type;
    uint8_t     sect_count;
    uint8_t     code_index;
}mef_hdr, *pmef_hdr;
//
#define SEC_HDR_PROTECT_READABLE    0b00000001  // PROT_READ  = 0x1
#define SEC_HDR_PROTECT_WRITEBLE    0b00000010  // PROT_WRITE = 0x2
#define SEC_HDR_PROTECT_EXECUTABLE  0b00000100  // PROT_EXEC  = 0x4
//
typedef struct __attribute__((packed)){
    uint32_t    file_offset;
    uint32_t    file_size;
    uint32_t    memory_offset;
    uint32_t    memory_size;
    uint32_t    protect;
}sec_hdr, *psec_hdr;
//
#endif
//