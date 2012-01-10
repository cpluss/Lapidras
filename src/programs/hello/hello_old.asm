bits 32
org 0x400000

section .text
align 4
_startpoint:
	;set a new stack
	mov esp, _second_stack_end
	push ebx ;store the return point
	
	;the launcher provides this information for us
	mov byte [x], al
	mov byte [y], dl
	
	mov esi, hello
	call print
	
	;store the new x, and y value in ecx + ebx
	movzx ecx, byte [x]	;ecx = new x value
	movzx edx, byte [y]	;ebx = new y value	
	mov eax, 0x12345	;dummy value to trick them all! ;)
	pop ebx
	jmp ebx ;jump back
_stop:
	jmp _stop
	
print:
	push eax
	push ecx
	push edx
	push ebx
	.dochar:
		call putc
		mov eax, [esi]		;string char to al
		lea esi, [esi + 1]	;next char
		cmp al, 0
		jne .dochar
		add byte [y], 1
		mov byte [x], 0
	.done:
		pop ebx
		pop edx
		pop ecx
		pop eax
		ret
		
;function putc
;puts char inside eax to screen
putc:
	mov ah, 0x07 		;attribute - grey on black
	mov ecx, eax		;save char / attribute
	movzx eax, byte [y]
	mov edx, 160
	mul edx				;y offset = y * 160
	movzx ebx, byte [x]
	shl ebx, 1			;x offset = x << 1
	
	mov edi, 0xb8000	;start of video memory
	add edi, eax		;y offset
	add edi, ebx		;x offset
	
	mov eax, ecx 		;restore char/attribute
	mov word [edi], ax
	add byte [x], 1		;advance to the right
	ret

section .data
hello db 'Hello there!', 0
hello_bin db 'HELLO.BIN!', 0

x db 0
y db 0	

section .bss
_second_stack:
	resb 0x1000		;0x1000 = 1 * 1024 = 1024 bytes
_second_stack_end:
