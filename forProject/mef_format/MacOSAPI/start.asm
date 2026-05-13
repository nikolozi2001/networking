; macOS ARM64 syscall-ები
; სისტემური გამოძახება: svc #0x80
; ნომერი: x16
; არგუმენტები: x0, x1, x2, x3, x4, x5
; შედეგი: x0

global _start

extern _mef_loader

; syscall-ების export
global _sys_write
global _sys_open
global _sys_lseek
global _sys_read
global _sys_close
global _sys_mmap
global _sys_munmap
global _sys_exit

section .text

_start:
    ; argc და argv OS-ის stack-იდან
    ldr     x0, [sp]        ; argc
    add     x1, sp, #8      ; argv
    bl      _mef_loader

    ; mef_loader-ის დაბრუნების შემთხვევაში
    mov     x0, x0          ; return code
    bl      _sys_exit

; ----------------------------------------------------------------
; long sys_write(int fd, const void *buf, size_t count)
; ----------------------------------------------------------------
_sys_write:
    mov     x16, #4         ; write
    svc     #0x80
    ret

; ----------------------------------------------------------------
; long sys_open(const char *path, int flags, int mode)
; ----------------------------------------------------------------
_sys_open:
    mov     x16, #5         ; open
    svc     #0x80
    ret

; ----------------------------------------------------------------
; long sys_lseek(int fd, long offset, int whence)
; ----------------------------------------------------------------
_sys_lseek:
    mov     x16, #199       ; lseek
    svc     #0x80
    ret

; ----------------------------------------------------------------
; long sys_read(int fd, void *buf, size_t count)
; ----------------------------------------------------------------
_sys_read:
    mov     x16, #3         ; read
    svc     #0x80
    ret

; ----------------------------------------------------------------
; int sys_close(int fd)
; ----------------------------------------------------------------
_sys_close:
    mov     x16, #6         ; close
    svc     #0x80
    ret

; ----------------------------------------------------------------
; void* sys_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
; ----------------------------------------------------------------
_sys_mmap:
    mov     x16, #197       ; mmap
    svc     #0x80
    ret

; ----------------------------------------------------------------
; int sys_munmap(void *addr, size_t len)
; ----------------------------------------------------------------
_sys_munmap:
    mov     x16, #73        ; munmap
    svc     #0x80
    ret

; ----------------------------------------------------------------
; void sys_exit(int code)
; ----------------------------------------------------------------
_sys_exit:
    mov     x16, #1         ; exit
    svc     #0x80
