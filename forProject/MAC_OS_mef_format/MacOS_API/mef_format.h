#ifndef MEF_FORMAT_H
#define MEF_FORMAT_H
//
#ifdef __KERNEL__
#include <linux/types.h>
#define MEF_PACKED __packed
#else
#include <stdint.h>
#define MEF_PACKED __attribute__((packed))
#endif
//
// magic
#define MEF_HDR_MAGIC               0x66656D7Fu
// cpu_type
#define MEF_HDR_CPU_TYPE_UNKNOWN    0x00
#define MEF_HDR_CPU_TYPE_X86        0x01
#define MEF_HDR_CPU_TYPE_X86_64     0x02
#define MEF_HDR_CPU_TYPE_AARCH      0x03
#define MEF_HDR_CPU_TYPE_AARCH_64   0x04
// os_type
#define MEF_HDR_OS_TYPE_UNIVERSAL   0x00
#define MEF_HDR_OS_TYPE_LINUX       0x01
#define MEF_HDR_OS_TYPE_WINDOWS     0x02
#define MEF_HDR_OS_TYPE_MAC_OS      0x03
//
// პლატფორმის ავტომატური განსაზღვრა
#if defined(__x86_64__)
#define MEF_CPU_TYPE  MEF_HDR_CPU_TYPE_X86_64
#elif defined(__aarch64__)
#define MEF_CPU_TYPE  MEF_HDR_CPU_TYPE_AARCH_64
#else
#error "Unsupported architecture"
#endif

#if defined(__linux__)
#define MEF_OS_TYPE   MEF_HDR_OS_TYPE_LINUX
#define MEF_PAGE_SIZE 4096
#elif defined(__APPLE__)
#define MEF_OS_TYPE   MEF_HDR_OS_TYPE_MAC_OS
#define MEF_PAGE_SIZE 16384
#else
#error "Unsupported operating system"
#endif
//
typedef struct MEF_PACKED {
    uint32_t    magic;
    uint32_t    vm_load;
    uint32_t    vm_size;
    uint8_t     cpu_type;
    uint8_t     os_type;
    uint8_t     sect_count;
    uint8_t     code_index;
    uint8_t     reserved;
} mef_hdr_t, *pmef_hdr_t;
//
typedef struct MEF_PACKED {
    uint32_t    offset;
    uint32_t    size;
    uint8_t     protect;
} mef_sect_t, *pmef_sect_t;
//
#endif