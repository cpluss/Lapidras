[extern schedule]		;inside of task.c
[global irq0]
irq01:
	cli
	
	pusha			;push general registers onto stack
	push ds
	push es
	push fs
	push gs
	
	mov eax, 0x10	;set kernel context
	mov ds, eax
	mov es, eax
	mov fs, eax
	mov gs, eax
	
	;ack IRQ
	mov al, 0x20
	out 0x20, al
	
	call schedule	;call the schedule routine to switch task
	
	pop gs
	pop fs
	pop es
	pop ds
	popa			;pop general registers onto stack
	
	sti
	iret
	
[global swtch]
;switch the current task
;arguments: thread_t *new, thread_t *old
swtch:
	;the stack got the return eip on its way in ..
	push ebp		;push the base pointer
	mov ebp, esp	;fetch the esp
	
	pushfd			;push current flags̈́
	cli				;disable interrupts
	
	;push general purpose registers
	push eax
	push ebx
	push ecx
	push edx
	push esi
	push edi
	
	;push ds, es, fs, gs
	xor eax, eax	;clear out eax
	mov ax, ds
	push eax
	mov ax, es
	push eax
	mov ax, fs
	push eax
	mov ax, gs
	push eax
	
	;perform the context switch
	;save current context
	mov ebx, [ebp + 12] 	;pointer to the thread
	mov eax, cr3			;copy current page directory
	mov [ebx + 12], eax		;into the old process structure
	
	mov eax, esp			;copy esp (stack pointer)
	mov [ebx + 4], eax		;into the old process structure
	
	xor eax, eax			;clean out eax contents .. xor ;)
	mov ax, ss				;copy ss (stack segment)
	mov [ebx + 20], ax		;into the structure
	
	;restore new context
	mov ebx, [ebp + 8]		;pointer to new thread
	
	mov ax, [ebx + 20]		;get ss
	mov ss, ax				;set ss
	
	mov eax, [ebx + 4]		;get esp
	mov esp, eax			;set esp
	
	mov eax, [ebx + 12]		;get page directory
	mov cr3, eax			;set page directory
	
	;restore the segment pointers
	;gs, fs, es, ds
	pop eax
	mov gs, ax
	pop eax
	mov fs, ax
	pop eax
	mov es, ax
	pop eax
	mov ds, ax
	
	;pop general purpose registers
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	
	popfd					;restore last flags
	pop ebp					;pop ebp to restore that one as well, popped first. See above.
	sti						;enable interrupts again
	ret
