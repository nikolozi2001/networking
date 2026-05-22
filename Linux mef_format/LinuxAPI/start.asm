[bits 64]

global _start

extern mef_loader

; syscall-ების export
global sys_write
global sys_open
global sys_fstat
global sys_lseek
global sys_read
global sys_close
global sys_mmap
global sys_munmap
global sys_exit

section .text

_start:
    ; argc და argv OS-ის stack-იდან
    mov     rdi, [rsp]      ; argc
    lea     rsi, [rsp+8]    ; argv
    call    mef_loader

    ; mef_loader-ის დაბრუნების შემთხვევაში
    mov     rdi, rax        ; mef_loader-ის დაბრუნების მნიშვნელობა
    call    sys_exit

; ----------------------------------------------------------------
; long sys_write(int fd, const void *buf, size_t count)
; ----------------------------------------------------------------
sys_write:
    mov     rax, 1              ; write
    syscall
    ret

; ----------------------------------------------------------------
; long sys_open(const char *path, int flags, int mode)
; ----------------------------------------------------------------
sys_open:
    mov     rax, 2              ; open
    syscall
    ret

sys_fstat:
    mov     rax, 5              ; stat
    syscall
    ret

; ----------------------------------------------------------------
; long sys_lseek(int fd, long offset, int whence)
; ----------------------------------------------------------------
sys_lseek:
    mov     rax, 8              ; lseek
    syscall
    ret

; ----------------------------------------------------------------
; long sys_read(int fd, void *buf, size_t count)
; ----------------------------------------------------------------
sys_read:
    mov     rax, 0              ; read
    syscall
    ret

; ----------------------------------------------------------------
; int sys_close(int fd)
; ----------------------------------------------------------------
sys_close:
    mov     rax, 3              ; close
    syscall
    ret

; ----------------------------------------------------------------
; void* sys_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
; სისტემური გამოძახებისთვის მე-4 არგუმენტი r10-შია და არა rcx-ში
; ----------------------------------------------------------------
sys_mmap:
    mov     rax, 9              ; mmap
    mov     r10, rcx            ; flags: rcx -> r10
    syscall
    ret

; ----------------------------------------------------------------
; int sys_munmap(void *addr, size_t len)
; ----------------------------------------------------------------
sys_munmap:
    mov     rax, 11             ; munmap
    syscall
    ret

; ----------------------------------------------------------------
; void sys_exit(int code)
; ----------------------------------------------------------------
sys_exit:
    mov     rax, 60             ; exit
    syscall