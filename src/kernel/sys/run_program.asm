[global launch_program]
;launch_program(ptr, get_x(), get_y());
;Function launch_program
;purpose: sets the appropriate registers and pushes arguments to the stack
;returns: no return point, you jump to the new address
launch_program:
	;clear them
	xor eax, eax
	xor edx, edx
	;get the coordinates
	mov eax, [esp + 8] 		;x coordinate
	mov edx, [esp + 12] 	;y coordinate
	
	mov ecx, [esp + 4] ;the jump to point
	;push ebx
	jmp ecx
