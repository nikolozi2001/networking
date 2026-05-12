[bits 64]

; M2-ზე გვერდის ზომა არის 16KB
page_size:      equ 0x4000 
; macOS-ზე exit syscall არის 0x2000001
syscall_exit:   equ 0x2000001

; Mach-O Header-ის მინიმალური სტრუქტურა macOS-ისთვის
dd 0xfeedfacf          ; magic (MH_MAGIC_64)
dd 16777228            ; cputype (ARM64-ისთვის macOS-ზე ხშირად გამოიყენება CPU_TYPE_ARM64)
dd 0                   ; cpusubtype
dd 2                   ; filetype (MH_EXECUTE)
dd 1                   ; ncmds (ბრძანებების რაოდენობა)
dd load_cmd_size       ; sizeofcmds
dd 0x00000001          ; flags
dd 0                   ; reserved

; Load Command (LC_SEGMENT_64)
load_cmd_start:
dd 0x19                ; cmd (LC_SEGMENT_64)
dd 72                  ; cmdsize
db "__TEXT", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ; segname
dq 0                   ; vmaddr
dq page_size * 2       ; vmsize
dq 0                   ; fileoff
dq page_size * 2       ; filesize
dd 7                   ; maxprot (rwx)
dd 5                   ; initprot (r-x)
dd 0                   ; nsects
dd 0                   ; flags
load_cmd_size: equ $ - load_cmd_start

; ვავსებთ პირველ გვერდს (Header Page)
times page_size - ($ - $$) db 0

; კოდის სექცია (მეორე გვერდი)
code_start:
    mov rax, syscall_exit
    mov rdi, 0          ; exit code
    syscall

code_size: equ $ - code_start

; ვავსებთ კოდის გვერდს 16KB-მდე
times page_size - ($ - code_start) db 0

file_end: