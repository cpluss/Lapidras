[global gdt_flush]
;function gdt_flush
;purpose: replaces the gdt with the pointer inside [esp + 4]
;returns: nothing
gdt_flush:
    mov eax, [esp + 4]		;move the new pointer of our gdt to eax
    lgdt [eax]			;load the new gdt
    
    mov ax, 0x10		;0x10 is the offset in the gdt to our data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush		;0x08 is the offset to our code segment: far jump!
    .flush:
	ret

[global idt_flush]
;function idt_flush
;purpose: replaces the idt with the pointer inside [esp + 4]
;returns: nothing
idt_flush:
    mov eax, [esp + 4]		;move the new pointer of our idt to eax
    lidt [eax]			;load the new idt
    ret
    
[global tss_flush]
;function tss_flush
;purpose: flushes the current tss and re-initializes it
;returns: nothing
tss_flush:
	mov ax, 0x2B		;index of TSS structure, 
	ltr ax				;load 0x2b to the tss ( task state register )
	ret
    
[global tlb_flush]
;function tlb_fluhs
;purpose: flush the TLB
;returns: nothing
tlb_flush:
	mov eax, cr3
	mov cr3, eax
	ret
    
[global copy_page_physical]
;function copy_page_physical
;purpose: copy a page by it's physical address
;returns: just copies directly, returns nothing
copy_page_physical:
    push ebx			;__cdecl function
    pushf			;push eflags
    
    cli				;disable interrupts
    
    mov ebx, [esp + 12]		;source address
    mov ecx, [esp + 16] 	;destination address
    
    ;disable paging
    mov edx, cr0
    and edx, 0x7fffffff
    mov cr0, edx
    
    ;set the counter
    mov edx, 1024
    
    .loop:
	mov eax, [ebx]		;get a word at the source address
	mov [ecx], eax		;store it at the destination
	add ebx, 4		;increment source pointer
	add ecx, 4		;increment destination pointer
	
	dec edx			;decrement pointer
	jnz .loop		;jump if not zero to .loop
	
    ;enable paging again
    mov edx, cr0
    or edx, 0x80000000
    mov cr0, edx
    
    popf			;pop eflags
    pop ebx			;pop ebx
    ret
    
[global set_page_directory]
;function set_page_directory
;purpose: sets the current page directory
;returns: nothing
set_page_directory:
    push ebx			;push ebx
    pushf			;push eflags
    
    push edx
    push ecx
    cli				;no interruptions here !
    
    mov edx, [esp + 4]		;new page directory
    
    ;disable paging
    mov ecx, cr0
    and ecx, 0x7fffffff
    mov cr0, ecx
    
    ;set the directory
    mov ecx, 0
    mov cr3, ecx
    mov cr3, edx
    
    ;enable paging again
    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx
    
    pop ecx
    pop edx
    
    popf
    pop ebx
    ret
    
    
[global read_eip]
;function read_eip
;purpose: get current eip ( instruction pointer )
;returns: current eip, instruction pointer
read_eip:
    pop eax
    jmp eax
