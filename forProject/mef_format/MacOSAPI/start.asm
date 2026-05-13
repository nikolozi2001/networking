// macOS ARM64 syscall-ები
// სისტემური გამოძახება: svc #0x80
// ნომერი: x16
// არგუმენტები: x0, x1, x2, x3, x4, x5
// შედეგი: x0 (carry set = error)

.global _start

.extern _mef_loader

.global _sys_write
.global _sys_open
.global _sys_lseek
.global _sys_read
.global _sys_close
.global _sys_mmap
.global _sys_munmap
.global _sys_exit

.text
.align 2

_start:
    // macOS dyld argc/argv რეგისტრებში გადმოგვცემს (x0=argc, x1=argv)
    bl      _mef_loader
    bl      _sys_exit

_sys_write:
    mov     x16, #4
    svc     #0x80
    b.cs    _syscall_err
    ret

_sys_open:
    mov     x16, #5
    svc     #0x80
    b.cs    _syscall_err
    ret

_sys_lseek:
    mov     x16, #199
    svc     #0x80
    b.cs    _syscall_err
    ret

_sys_read:
    mov     x16, #3
    svc     #0x80
    b.cs    _syscall_err
    ret

_sys_close:
    mov     x16, #6
    svc     #0x80
    b.cs    _syscall_err
    ret

_sys_mmap:
    mov     x16, #197
    svc     #0x80
    b.cs    _syscall_err_mmap
    ret

_sys_munmap:
    mov     x16, #73
    svc     #0x80
    b.cs    _syscall_err
    ret

_sys_exit:
    mov     x16, #1
    svc     #0x80

_syscall_err:
    neg     x0, x0
    ret

_syscall_err_mmap:
    mov     x0, #-1
    ret
