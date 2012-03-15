#include "system.h"
#include "thread.h"
#include "console.h"

//The current thread, externally declared ..
extern volatile thread_t *current_thread;

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
extern fs_node_t *current_node;
static void syscall_system(const char *name, int argc, char **argv)
{
    fs_node_t *node = finddir_fs(current_node, (char*)name);
    if(node)
        system(node, argc, argv);
}

void syscall_execve_prototype()
{
    fs_node_t *node = finddir_fs(current_node, (char*)CurrentThread()->name);
    if(!node)
        exit();
    char *argv[] = { "test", NULL };
    exec(node, 1, argv);
    exit();
}
static void syscall_execve(const char *name)
{
    /*thread_t *th = CreateThread(name, (uint)syscall_execve_prototype, STATE_RUNNABLE);
    while(th->state != STATE_DEAD);*/
    int ret = fork();
    if(ret == 0)
    {
        kprint("I am the child.\n");
        exit();
    }
    kprint("I am the parent! ( child pid: %i )\n", ret);
}

static void *syscalls[] =
{
	//Printing operations
	&kputs,					//kputs - prints a string to the screen
	&kputc,					//kputc - prints a character to the screen
    &kbd_get_string,        //kbd_get_string - reads a string to a buffer
	
	&GetPID,				//GetPID - gets the current pid
	
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
	&syscall_exit, //Exit current thread ..
    
    //Thread operations
    &syscall_system,        //Launch INTO a new process
    &fork,                  //Fork .. no need for an explanation
    &syscall_execve,        //Execve - create a new thread with a different executable
    &GetThreadByName,
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
	
    //Set the current threads syscall registers
    current_thread->syscall_registers = regs;

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
	//In case the thread changed it's stack ( fork .. )
    regs = current_thread->syscall_registers;
   // kprint("Returning %i, syscall %i\n", ret, regs->eax);
    regs->eax = ret;
}
