#include "thread.h"
#include "list.h"
#include "memory.h"
#include "paging.h"
#include "cio.h"
#include "event.h"

extern page_directory_t *current_directory;

list_t *thread_ready;
list_t *thread_wait;
list_t *thread_dead;

volatile thread_t *current_thread;
uint next_pid = 1;

void start_multithreading(uint esp)
{
	asm volatile("cli");	
	thread_ready = list_create();
	thread_wait = list_create();
	thread_dead = list_create();
	
	//Create the initial task
	current_thread = (thread_t*)kmalloc(sizeof(thread_t));
	memset((byte*)current_thread, 0, sizeof(thread_t));
	strcpy(current_thread->name, "kernel");
	current_thread->state = STATE_RUNNABLE;
	current_thread->pid = next_pid++;
	current_thread->base.stack = esp;
	current_thread->esp0 = (uint)kmalloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
    set_kernel_stack(current_thread->esp0);
	current_thread->page_directory = current_directory;
	current_thread->signal_queue = list_create();
	
	asm volatile("sti");
}

thread_t *CreateThread(const char *name, uint eip, uint state)
{	
	asm volatile("cli");
	thread_t *new = (thread_t*)kmalloc(sizeof(thread_t));
	memset(new, 0, sizeof(thread_t)); 
	
	new->ebp = 0;
	new->eip = eip;
	new->base.stack = (uint)kmalloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
	new->esp = new->base.stack;
	new->esp0 = (uint)kmalloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
	
	new->page_directory = clone_directory(current_directory);
	
	new->pid = next_pid++;
	strcpy(new->name, name);
	new->signal_queue = list_create();
	
	new->state = state;
	switch(new->state)
	{
		case STATE_RUNNABLE:
		make_thread_ready(new);
		break;
		case STATE_WAITING:
		list_insert(thread_wait, (void*)new);
		break;
	}
	
	asm volatile("sti");
	return new;
}

thread_t *GetThread(uint pid)
{
	if(current_thread->pid == pid)
		return (thread_t*)current_thread;
		
	foreach(item_, thread_ready)
	{
		thread_t *th = (thread_t*)item_->value;
		if(th->pid == pid)
			return th;
	}
	
	//Reached here -> we've not found anything at all.
	foreach(item, thread_wait)
	{
		thread_t *th = (thread_t*)item->value;
		if(th->pid == pid)
			return th;
	}
		
	return 0;
}
thread_t *GetThreadByName(const char *name)
{
	if(strcmp(current_thread->name, name))
		return (thread_t*)current_thread;
	
	foreach(item_, thread_ready)
	{
		thread_t *th = (thread_t*)item_->value;
		if(strcmp(th->name, name))
			return th;
	}
	
	//Reached here -> we've not found anything at all.
	foreach(item, thread_wait)
	{
		thread_t *th = (thread_t*)item->value;
		if(strcmp(th->name, name))
			return th;
	}
		
	return 0;
}

int fork()
{
	asm volatile("cli");
	//The pointers to store
	uint ebp, eip, esp;
	uint magic = 0xDEADBEEF;
	
	//Save the parent, cause we fork -> the parent continues the work..
	thread_t *parent = (thread_t*)current_thread;
	
	//Copy the current page directory
	page_directory_t *dir = clone_directory(current_directory);
	
	//Allocate memory for the new thread -> the child
	thread_t *new = (thread_t*)kmalloc(sizeof(thread_t));
	memset((byte*)new, 0, sizeof(thread_t));
	new->pid = next_pid++;
	new->state = STATE_RUNNABLE;
	new->base.stack = (uint)kmalloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
	new->esp0 = (uint)kmalloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
	new->page_directory = dir;
	new->signal_queue = list_copy(current_thread->signal_queue);
	new->parent = (thread_t*)current_thread;
	
	//Read the current intstruction pointer
	eip = read_eip();
	if(current_thread == parent)
	{
		//Check the magic
		if(magic != 0xDEADBEEF)
		{
			kprint("Bad fork() magic, current pid %i (parent)\n", current_thread->pid);
			asm volatile("cli");
			for(;;);
		}
		//Fetch base pointer and stack pointer (current)
		asm volatile("mov %%esp, %0" : "=r"(esp));
		asm volatile("mov %%ebp, %0" : "=r"(ebp));
		//Calculate the new bp and sp
		if(current_thread->base.stack > new->base.stack)
		{
			new->esp = esp - (current_thread->base.stack - new->base.stack);
			new->ebp = ebp - (current_thread->base.stack - new->base.stack);
		}
		else
		{
			new->esp = esp + (new->base.stack - current_thread->base.stack);
			new->ebp = ebp - (current_thread->base.stack - new->base.stack);
		}
		//Copy the stack
		memcpy((byte*)(new->base.stack - KERNEL_STACK_SIZE), (byte*)(current_thread->base.stack - KERNEL_STACK_SIZE), KERNEL_STACK_SIZE);
        
        //Systemcall?
        if(current_thread->syscall_registers != 0)
        {
            uint o_stack = ((uint)current_thread->base.stack - KERNEL_STACK_SIZE);
            uint n_stack = ((uint)new->base.stack - KERNEL_STACK_SIZE);
            uint offset = ((uint)current_thread->syscall_registers - o_stack);
            new->syscall_registers = (registers_t*)(n_stack + offset);
        }
		//set the eip
		new->eip = eip;
		
		//Insert into queue
		make_thread_ready(new);
		
		asm volatile("sti");
		return new->pid;
	}
	else
	{
		//Check the magic
		if(magic != 0xDEADBEEF)
		{
			kprint("Bad fork() magic, current thread %s(%i) (child)\n", current_thread->name, current_thread->pid);
			exit();
		}
		return 0;
	}
}

thread_t *get_next_thread()
{
	//A packet has been sent, or an event triggered.
	//check the wait que
	if(thread_wait->length > 0)
	{
		foreach(item, thread_wait)
		{
			thread_t *n = (thread_t*)item->value;
			if(!n)
				continue;
			if(n->state == STATE_RUNNABLE || !QueueIsEmpty(n))
			{
				notify_event(EVENT_THREAD_WAKE, (void*)n);
				n->state = STATE_RUNNABLE;
				list_delete(thread_wait, item);
				return n;
				break;
			}
		}
	}
	
	if(thread_ready->length == 0)
		return (thread_t*)current_thread;
	
		
	node_t *np = list_dequeue(thread_ready);
	thread_t *ret = (thread_t*)np->value;
	free((void*)np);
	if(ret->state != STATE_RUNNABLE)
	{
		if(ret->state == STATE_DEAD)
		{
			notify_event(EVENT_THREAD_DIE, (void*)ret);
			list_insert(thread_dead, (void*)ret);
		}
		if(ret->state == STATE_WAITING)
		{
			notify_event(EVENT_THREAD_WAIT, (void*)ret);
			list_insert(thread_wait, (void*)ret);
		}
			
		return get_next_thread();
	}
	return ret;
}
thread_t *next_dead_thread()
{
	node_t *np = list_dequeue(thread_dead);
	if(!np)
		return 0;
	thread_t *ret = (thread_t*)np->value;
	free(np);
	return ret;
}
void free_dead_threads()
{
	while(thread_dead->head != 0)
	{
		thread_t *th = next_dead_thread();
		if(strcmp(th->name, "kernel"))
			continue;
		if(th)
		{
			free((void*)(th->base.stack - KERNEL_STACK_SIZE));
			free((void*)(th->esp0 - KERNEL_STACK_SIZE));
			free(th->page_directory);
			
			list_destroy(th->signal_queue);
			list_clean(th->signal_queue);
			free(th->signal_queue);
			
			free(th);
		}
	}
}
void schedule()
{
	if(!current_thread)
		return;
		
	uint esp, ebp, eip;
	asm volatile("mov %%esp, %0" : "=r"(esp));
	asm volatile("mov %%ebp, %0" : "=r"(ebp));
	
	eip = read_eip();
	if(eip == 0x10000) 
		return; //execute tasks here later on
		
	current_thread->eip = eip;
	current_thread->esp = esp;
	current_thread->ebp = ebp;
	
	thread_t *last = (thread_t*)current_thread;
	thread_t *tmp = get_next_thread();
	if(tmp == last)
        return;
        
	make_thread_ready((thread_t*)current_thread);
	//Free the dead threads running
	free_dead_threads();

	_switch(tmp);
}
void _switch(thread_t *new)
{
	uint esp, ebp, eip;	
	asm volatile("cli"); //Just in case..
	current_thread = new; //Set the appropriate var
	current_directory = new->page_directory;
	
	esp = current_thread->esp;
	ebp = current_thread->ebp;
	eip = current_thread->eip;
	
	notify_event(EVENT_THREAD_SWITCH, (void*)current_thread);
	set_kernel_stack(current_thread->esp0);
	
	//perform the jump
	asm volatile(
		"mov %0, %%ebx\n"
		"mov %1, %%esp\n"
		"mov %2, %%ebp\n"
		"mov %3, %%cr3\n"
		"mov $0x10000, %%eax\n"
		"sti\n"
		"jmp *%%ebx"
		: : "r"(eip), "r"(esp), "r" (ebp), "r"(current_directory->physicalAddr) : "%ebx", "%esp", "%eax");
}

void kill_thread(thread_t *th)
{
	th->state = STATE_DEAD;
	list_insert(thread_dead, (void*)th);
}

int GetPID()
{
	return current_thread->pid;
}
inline thread_t *CurrentThread()
{
	return (thread_t*)current_thread;
}
void make_thread_ready(thread_t *th)
{
	list_insert(thread_ready, (void*)th);
}
void thread_set_state(thread_t *th, uint state)
{
	th->state = state;
}
void exit()
{
	asm volatile("cli");
	//Remove it from the current que and set the status to DEAD
	thread_set_state(CurrentThread(), STATE_DEAD);
	asm volatile("sti");
	for(;;); //Never return
}

uint get_thread_count()
{
	return thread_ready->length + thread_wait->length + 1;
}
