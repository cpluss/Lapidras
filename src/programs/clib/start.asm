[bits 32]
[global _start]
[extern main]
_start:
	add esp, 4
	call main	;This is already at the stack ..
	
	mov eax, 12
	int 0x70	;Exit ..
	.wait:
		jmp .wait
