[BITS 32]
global _start
extern kmain

; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN equ  1<<0                   ; align loaded modules on page boundaries
MEMINFO     equ  1<<1                   ; provide memory map
FLAGS       equ  MODULEALIGN | MEMINFO  ; this is the Multiboot 'flag' field
MAGIC       equ    0x1BADB002           ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)        ; checksum required
 
section .mbHeader
align 4
MultiBootHeader:
   dd MAGIC
   dd FLAGS
   dd CHECKSUM

section .text
align 4
_start:
	mov esp, _stack_end ;setup stack - 32kB
	
	push esp
	push ebx ;multiboot info structure
	call kmain
	
stop:
	jmp stop
	
global gousermode
gousermode:
    cli
    pop edx
    
    xor eax, eax
    mov ax, 0x23    ;Set the usermode context
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push 0x23       ;SS
    
    mov eax, esp
    add eax, 12     ;Correct the stack
    push eax        ;Stack
    pushfd          ;eflags
    pop eax         ;pop flags
    or eax, 0x200   ;Enable interrupts ...
    push eax        ;Push flags again
    
    push 0x1b       ;CS 
    push edx        ;EIP
    iret

global gousermode_loc
gousermode_loc:
    cli
    pop edx
    pop edx			;Second time to get the right
    
    xor eax, eax
    mov ax, 0x23    ;Set the usermode context
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push 0x23       ;SS
    
    mov eax, esp
    ;add eax, 12     ;Correct the stack
    push eax        ;Stack
    pushfd          ;eflags
    pop eax         ;pop flags
    or eax, 0x200   ;Enable interrupts ...
    push eax        ;Push flags again
    
    push 0x1b       ;CS 
    push edx        ;EIP
    iret

section .bss
stack:
	resb 0x1000
_stack_end:
