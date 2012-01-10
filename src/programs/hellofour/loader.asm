[bits 32]
[global _start]
[extern pmain]

section .startPoint
_start:
	jmp _startpoint

section .text
align 4
_startpoint:
	call pmain
	ret
