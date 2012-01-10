bits 32
org 0x400000

section .text
align 4
_startpoint:
	mov ebx, hello
	int 0x70
		
	ret
_stop:
	jmp _stop
	
section .data
hello db 'Hello there!', 0xA, 0
