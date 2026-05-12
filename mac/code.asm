; =====================================================
; Mach-O ARM64 Hello World (macOS / Apple Silicon)
; =====================================================
;
; Mach-O Header (32 bytes):
;   dd 0xFEEDFACF         ; magic (MH_MAGIC_64)
;   dd 0x0100000C         ; cputype (CPU_TYPE_ARM64)
;   dd 0x00000000         ; cpusubtype (CPU_SUBTYPE_ARM64_ALL)
;   dd 0x2                ; filetype (MH_EXECUTE)
;   dd ncmds              ; ncmds
;   dd sizeofcmds         ; sizeofcmds
;   dd 0x00200085         ; flags (MH_NOUNDEFS|MH_DYLDLINK|MH_PIE)
;   dd 0                  ; reserved
;
; Load Commands:
;   LC_SEGMENT_64 __PAGEZERO  vmaddr=0, vmsize=0x100000000
;   LC_SEGMENT_64 __TEXT       code (r-x), fileoff=0
;   LC_SEGMENT_64 __DATA       data (rw-)
;   LC_SEGMENT_64 __LINKEDIT   symbols, code signature
;   LC_LOAD_DYLINKER           /usr/lib/dyld
;   LC_LOAD_DYLIB              libSystem.B.dylib
;   LC_MAIN                    entry point offset
;   LC_BUILD_VERSION           macOS 14.0+
;   LC_CODE_SIGNATURE          ad-hoc signing
;
; ARM64 macOS Syscalls:
;   write = 4   (x16=4, x0=fd, x1=buf, x2=len, svc #0x80)
;   exit  = 1   (x16=1, x0=code, svc #0x80)
; =====================================================

.globl _main
.p2align 2

_main:
    mov x0, #1            ; stdout
    adr x1, message       ; შეფარდებითი მისამართი
    mov x2, #15           ; სიგრძე (14 სიმბოლო + newline)
    mov x16, #4           ; write syscall
    svc #0x80

    mov x0, #0            ; exit code
    mov x16, #1           ; exit syscall
    svc #0x80

message:
    .ascii "Hello, Mach-O!\n"
