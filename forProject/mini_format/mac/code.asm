.section __TEXT,__text,regular,pure_instructions
.globl _start
.p2align 2
_start:
    mov x0, #1
    adrp x1, message@PAGE
    add x1, x1, message@PAGEOFF
    mov x2, #14
    mov x16, #4
    svc #0x80

    mov x0, #0
    mov x16, #1
    svc #0x80

.section __TEXT,__cstring,cstring_literals
message:
    .ascii "Hello, Mach-O!"
