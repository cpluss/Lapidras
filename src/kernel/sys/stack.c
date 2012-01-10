#include "memory.h"
#include "paging.h"
extern page_directory_t *current_directory;

void move_stack(void *initial_esp, void *new_stack_start, uint size)
{
	uint i;
	//Allocate the page tables for the stack
	for(i = (uint)new_stack_start; i >= ((uint)new_stack_start - size); i -= 0x1000)
		alloc_frame(get_page(i, 1, current_directory), 0, 1);
		
	uint pd_addr;
	//flush the TLB
	asm volatile("mov %%cr3, %0" : "=r"(pd_addr));
	asm volatile("mov %0, %%cr3" : : "r"(pd_addr));
	
	uint old_stack_pointer, old_base_pointer;
	asm volatile("mov %%esp, %0" : "=r"(old_stack_pointer));
	asm volatile("mov %%ebp, %0" : "=r"(old_base_pointer));
	
	uint offset = (uint)new_stack_start - (uint)initial_esp;
	uint new_stack_pointer = old_stack_pointer + offset;
	uint new_base_pointer = old_base_pointer + offset;
	
	//Copy the stack
	memcpy((void*)new_stack_pointer, (void*)old_stack_pointer, initial_esp - old_stack_pointer);
	
	//Backtrace through original stack, copy values
	for(i = (uint)new_stack_start; i >= ((uint)new_stack_start - size); i -= 4)
	{
		uint tmp = *(uint*)i;
		if((old_stack_pointer < tmp) && (tmp < (uint)initial_esp))
		{
			tmp = tmp + offset;
			uint *tmp2 = (uint*)i;
			*tmp2 = tmp;
		}
	}
	
	//Change stacks
	asm volatile("mov %0, %%esp" : : "r"(new_stack_pointer));
	asm volatile("mov %0, %%ebp" : : "r"(new_base_pointer));
}

void copy_stack(void *initial_esp, void *new_stack_start, uint size, uint *esp, uint *ebp)
{
	//Assume that new_stack_start is page aligned
	uint old_stack_pointer, old_base_pointer;
	asm volatile("mov %%esp, %0" : "=r"(old_stack_pointer));
	asm volatile("mov %%ebp, %0" : "=r"(old_base_pointer));
	
	uint offset;//(uint)new_stack_start - (uint)initial_esp;
	offset = (uint)initial_esp - (uint)new_stack_start;
	uint new_stack_pointer = old_stack_pointer - offset;
	uint new_base_pointer = old_base_pointer - offset;
	
	//Copy the stack
	memcpy((void*)new_stack_pointer, (void*)old_stack_pointer, initial_esp - old_stack_pointer);
	int i;
	//Backtrace through original stack, copy values
	for(i = (uint)new_stack_start; i >= ((uint)new_stack_start - size); i -= 4)
	{
		uint tmp = *(uint*)i;
		if((old_stack_pointer < tmp) && (tmp < (uint)initial_esp))
		{
			tmp = tmp + offset;
			uint *tmp2 = (uint*)i;
			*tmp2 = tmp;
		}
	}	
	
	kprint("NSP : 0x%x\nOSP : 0x%x\n", (uint)new_stack_pointer, (uint)old_stack_pointer);
	kprint("NSS : 0x%x\nINESP: 0x%x\n", (uint)new_stack_start, (uint)initial_esp);
	kprint("OFF: 0x%x\n", offset);
	*esp = (uint)new_stack_pointer;
	*ebp = (uint)new_base_pointer;
}
