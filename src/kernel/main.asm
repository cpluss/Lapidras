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
	
section .bss
stack:
	resb 0x1000
_stack_end:
