#include "system.h"
#include "thread.h"
#include "cio.h"

//The syscall handler
void syscall_handler(registers_t *regs);

#define NUM_SYSCALLS 22
//All available syscalls right now
uint num_syscalls = NUM_SYSCALLS;

static void syscall_exit()
{
    asm volatile("sti");
    exit();
    for(;;); //Just incase
}

static void *syscalls[] =
{
	//Printing operations
	&kputs,					//kputs - prints a string to the screen
	&kputc,					//kputc - prints a character to the screen
	
	//Thread operations
	&SendSignal,			//SendSignal - sends a signal
	&GetPID,				//GetPID - gets the current pid
	&GetThreadByName,		//GetThreadByName - gets a thread by name
	&CreateThread,			//CreateThread - creates a new thread
	&CurrentThread,			//CurrentThread - gets the current thread
	
	//IO - Operations
	&fopen,					//fopen - opens a file handle
	&fclose, 				//closes a file handle
	&fread,					//reads from a file handle
	&fwrite,				//writes to a file handle
	&ftell_size,			//tell the size of this file handle
	
	//Memory operations
	&alloc,					//allocate memory
	&free,					//free memory
	
	//Get the current system tick
	&gettick,
	
	//User input
	&register_event,		//read a string from the user
	&unregister_event,
	
	&WaitForSignal,
	&LatestSignal,
	&GetThread,
	
	&kbd_get_string,
    &syscall_exit //Exit current thread ..
};

void init_syscalls()
{
	//Register syscall handler
	register_interrupt_handler(112, &syscall_handler);
}

void syscall_handler(registers_t *regs)
{
	//eax is the syscall number
	//check if the range is valid
	if(regs->eax >= num_syscalls)
		return;
		
	//get the syscall function
	void *location = syscalls[regs->eax];
	
	//we have no idea what the parameters is for every
	//syscall, use all parameters available
	int ret;
	asm volatile("\
		push %1; \
		push %2; \
		push %3; \
		push %4; \
		push %5; \
		sti; \
		call *%6; \
		cli; \
		pop %%ebx; \
		pop %%ebx; \
		pop %%ebx; \
		pop %%ebx; \
		pop %%ebx;"
			: "=a"(ret) : "r"(regs->edi), "r"(regs->esi), "r"(regs->edx), "r"(regs->ecx), "r"(regs->ebx), "r"(location));
	regs->eax = ret;
}
