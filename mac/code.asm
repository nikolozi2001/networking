[bits 64]

; --- კონსტანტები ---
MH_MAGIC_64 equ 0xfeedfacf
CPU_TYPE_ARM64 equ 0x0100000c
CPU_SUBTYPE_ARM64_ALL equ 0x00000000
MH_EXECUTE equ 0x2
LC_SEGMENT_64 equ 0x19

; --- Mach-O Header ---
dd MH_MAGIC_64           ; magic
dd CPU_TYPE_ARM64        ; cputype
dd CPU_SUBTYPE_ARM64_ALL ; cpusubtype
dd MH_EXECUTE            ; filetype
dd 2                     ; ncmds (ორი Load Command: __TEXT და __DATA)
dd load_commands_end - load_commands_start ; sizeofcmds
dd 0x00200001            ; flags (MH_NOUNDEFS | MH_DYLDLINK)
dd 0                     ; reserved

load_commands_start:

; --- Segment 1: __TEXT (Code) ---
segment_text_start:
  dd LC_SEGMENT_64       ; cmd
  dd 72                  ; cmdsize
  db "__TEXT", 0, 0, 0, 0, 0, 0, 0, 0, 0 ; segname (16 bytes)
  dq 0x100000000         ; vmaddr
  dq 0x4000              ; vmsize
  dq 0                   ; fileoff
  dq 0x4000              ; filesize
  dd 7                   ; maxprot (rwx)
  dd 5                   ; initprot (r-x)
  dd 0                   ; nsects
  dd 0                   ; flags

; --- Segment 2: __DATA (Data) ---
segment_data_start:
  dd LC_SEGMENT_64       ; cmd
  dd 72                  ; cmdsize
  db "__DATA", 0, 0, 0, 0, 0, 0, 0, 0, 0 ; segname
  dq 0x100004000         ; vmaddr
  dq 0x4000              ; vmsize
  dq 0x4000              ; fileoff (იწყება ფაილის მეორე გვერდიდან)
  dq 0x4000              ; filesize
  dd 7                   ; maxprot (rwx)
  dd 3                   ; initprot (rw-)
  dd 0                   ; nsects
  dd 0                   ; flags

load_commands_end:

; --- კოდის ნაწილი ---
align 4096, db 0         ; Mach-O მოითხოვს გვერდებად დაყოფას (4096 ბაიტი)
entry_point:
    mov x0, 1            ; stdout
    adr x1, message      ; ARM64-ზე შეფარდებითი მისამართი
    mov x2, 14           ; სიგრძე
    mov x16, 4           ; write syscall
    svc 0x80

    mov x0, 0            ; exit code
    mov x16, 1           ; exit syscall
    svc 0x80

; --- მონაცემების ნაწილი ---
align 4096, db 0
message: db "Hello, Mach-O!", 10
