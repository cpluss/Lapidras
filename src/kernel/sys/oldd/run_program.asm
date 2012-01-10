[global launch_program]
;launch_program(ptr, argc, argv[]);
;Function launch_program
;purpose: sets the appropriate registers and pushes arguments to the stack
;returns: no return point, you jump to the new address
launch_program:
	push edx
	push eax
	push ecx
	
	;get the coordinates
	mov edx, [esp + 8] 		;argc
	mov eax, [esp + 12] 	;argv
	
	mov ecx, [esp + 4] ;the jump to point
	
	push eax
	push edx
	call ecx
	add esp, 8 ;cleanup
	
	pop ecx
	pop eax
	pop edx
	ret
	
