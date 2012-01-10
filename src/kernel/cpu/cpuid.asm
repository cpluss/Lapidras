[global get_cpuid]
;function get_cpuid
;purpose: get the cpuid information in [esp + 4] ( first argument ), if [esp + 4] = 0
;         then provide an extra argument [esp + 8] to store the vendor string within'
;returns: the cpuid information
get_cpuid:
    ;first check if cpuid is supported or not
    call check_cpuid
    cmp eax, 0 		;not supported
    je .not_supported
    
    mov eax, [esp + 4]	;the cpuid offset
    
    ;do we extract the vendor string?
    cmp eax, 0
    je .vendor_string
    
    ;maybe the signature?
    cmp eax, 1
    je .signature
    
    ;the features
    cmp eax, 0x101
    je .features
    
    ;apic id
    cmp eax, 0x102
    je .apicid
    
    .apicid:
	mov eax, 1
	cpuid
	
	;the contents is in the highest byte of ebx
	shr ebx, 16		;ebx >> 16 -> bh contains the APIC id now
	xor eax, eax		;clear eax, eax = 0
	mov al, bh
	ret
    
    .features:
	mov eax, 1
	cpuid
	mov eax, edx
	ret
	
    .signature:
	;returns the processor signature within' eax
	mov eax, 1
	cpuid			;issue cpuid instruction
	ret
	
    .vendor_string:
	mov edi, [esp + 8]	;pointer to our storage string
	
	;the string is stored within' ebx, edx and ecx. In that order.
	push ebx
	push edx
	push ecx
	
	mov eax, 0	;the offset of the vendor string
	cpuid		;issue the cpuid request
	
	;first off copy the ebx content
	;mov eax, 4	;we copy four characters, one per byte
	.copy_ebx:
	    mov byte [edi], bl	;the first char
	    inc edi		;increment pointer
	    mov byte [edi], bh	;the second char 
	    inc edi
	    
	    shr ebx, 16		;ebx >> 16
	    mov byte [edi], bl
	    inc edi
	    mov byte [edi], bh
	    inc edi		;the same as above using the new bytes
	.copy_edx:
	    ;use the same technique as above
	    mov byte [edi], dl
	    inc edi
	    mov byte [edi], dh
	    inc edi
	    
	    shr edx, 16		;edx >> 16
	    mov byte [edi], dl
	    inc edi
	    mov byte [edi], dh
	    inc edi
	.copy_ecx:
	    ;the same again
	    mov byte [edi], cl
	    inc edi
	    mov byte [edi], ch
	    inc edi
	    
	    shr ecx, 16		;ecx >> 16
	    mov byte [edi], cl
	    inc edi
	    mov byte [edi], ch
	    inc edi
	 ;restore registers
	 pop ecx
	 pop edx
	 pop ebx
	 
	 mov eax, 1 ;success
	 ret
    
    .not_supported:
	mov eax, -1	;return -1 if cpuid isn't supported
	ret
    .return:
	ret

;function check_cpuid
;purpose: checks if cpuid command is supported
;returns: eax = 1 if it is supported, 0 if it is not
check_cpuid:
    ;method:
    ;you're able to use the cpuid instruction if you're able to write
    ;to the ID bit (0x200000) in eflags, which only is modifiable when
    ;cpuid instruction is supported
    push ecx		;save ecx
    pushfd 		;store eflags to the stack
    pop eax		;pop eflags to eax
    mov ecx, eax	;ecx = eax = eflags
    xor eax, 0x200000	;flip the ID bit
    push eax		;store eax to stack
    popfd		;restore eflags from eax
    ;test if the bit flipped or not
    pushfd		;push eflags to the stack
    pop eax		;pop eflags to eax
    xor eax, ecx	;mask changed bits
    shr eax, 21		;move bit 21 to bit 0
    and eax, 1		;mask the rest of the bits
    push ecx		;restore original eflags
    popfd		;pop eflags
    pop ecx		;restore ecx
    ret