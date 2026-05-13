[bits 64]

page_size:		equ	4096
vm_load:		equ 40

; mef - Minimal Executable Format
db  0x7f, 'm', 'e', 'f'		; magic - ფიქსირებული რიცხვი
dd	vm_load					; vm_laod - ფურცლის მისამართი სადაც უნდა ჩაიტვირთოს ფაილი
dd	2						; vm_size - რამდენი ფურცელია საჭირო ფაილის ჩასატვირთად
db	2						; cpu_type - პროცესორის არქიტექტურა
db	1						; os_type - ოპერაციული სისტემა
db	1						; sect_count - სექციების რაოდენობა
db	0						; code_index - კოდის სექციის ინდექსი
db	0						; tbl_count - ცხრილების რაოდენობა

; კოდის სექციის ჰედერი
dd	1						; offset მერემდენე ფურცლიდან იწყება სექცია, vm_load + vm_offset
dd	1						; size რამდენი ფურცლისგან შედგება სექცია
db	0b00000101				; protect

times page_size - ($ - $$) db 0

; კოდის სექცია
code_start:
	mov rax, 60
	mov rdi, 0
	syscall

times page_size - ($ - code_start) db 0
