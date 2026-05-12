[bits 64]

file_load_va: equ 0x4000 * 40
page_size:	equ 0x4000

; mef - Minimal Executable Format
db  0x7f, 'm', 'e', 'f'		; magic - ფიქსირებული რიცხვი
dd	file_load_va			; mem_laod	- ფაილის ჩატვირთვის მისამართი
dd	file_end				; file_size	- ფაილის ზომა
db	2						; cpu_type
db	1						; os_type
db	1						; sect_count - სექციების რაოდენობა
db	0						; კოდის სექციის ინდექსი

; კოდის სექციის ჰედერი
dd	page_size				; file_offset
dd	code_size				; file_size
dd	page_size				; mem_offset
dd	page_size				; mem_size
dd	0b00000101				; protect

times page_size - ($ - $$) db 0

; კოდის სექცია
code_start:
	mov rax, 60
	mov rdi, 0
	syscall

code_size: equ $ - code_start

times page_size - ($ - code_start) db 0

file_end:
