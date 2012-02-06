#include "thread.h"
#include "list.h"
#include "memory.h"
#include "cio.h"
#include "event.h"

extern page_directory_t *current_directory;
extern virtual_console_t *current_console;

volatile thread_t *current_thread;
thread_t *idle_thread;
uint next_pid = 1;
uint current_priority = 0;
uint enabled = 0;
uint total_thread_count = 0;
void set_mt(uint state)
{
	enabled = state;
}

list_t *thread_ready;
list_t *thread_wait;
list_t *thread_dead;
list_t *thread_sleep;

void idle_func()
{
	for(;;);
}
void start_multithreading(uint esp)
{
	asm volatile("cli");	
	thread_ready = list_create();
	thread_wait = list_create();
	thread_dead = list_create();
	thread_sleep = list_create();
	
    move_stack((void*)esp, (void*)0xE0000000, 0x2000);

	//Create the initial task
	current_thread = (thread_t*)kmalloc(sizeof(thread_t));
	memset((byte*)current_thread, 0, sizeof(thread_t));
	strcpy(current_thread->name, "kernel");
	current_thread->priority = 0;
	current_thread->state = RUNNABLE;
	current_thread->id = next_pid++;
	current_thread->esp_loc = 0xE0000000; 
	current_thread->esp0 = (uint)kmalloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
	current_thread->page_directory = current_directory;
	current_thread->console = current_console;
	current_thread->signal_queue = list_create();
	
	set_mt(1);
	
	asm volatile("sti");
}

thread_t *CreateThread(const char *name, uint eip, uint priority, uint state)
{	
	asm volatile("cli");
	thread_t *new = (thread_t*)kmalloc(sizeof(thread_t));
	memset(new, 0, sizeof(thread_t)); 
	
	new->ebp = 0;
	new->eip = eip;
	new->esp_loc = (uint)kmalloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
	new->esp = new->esp_loc;
	new->esp0 = (uint)kmalloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
	new->page_directory = clone_directory(current_directory);
	
	new->id = next_pid++;
	strcpy(new->name, name);
	new->signal_queue = list_create();
	
	/*new->console = (virtual_console_t*)kmalloc(sizeof(virtual_console_t));
	memset((byte*)new->console, 0, sizeof(virtual_console_t));
	new->console->console_buffer = (char*)kmalloc(CONSOLE_BUFFER_SIZE);
	new->console->y = 1;*/
	
	total_thread_count++;
	
	new->state = state;
	switch(new->state)
	{
		case RUNNABLE:
		make_thread_ready(new);
		break;
		case WAITING:
		list_insert(thread_wait, (void*)new);
		break;
	}
	
	asm volatile("sti");
	return new;
}

thread_t *GetThread(uint pid)
{
	if(current_thread->id == pid)
		return (thread_t*)current_thread;
	
	foreach(item, thread_ready)
	{
		thread_t *th = (thread_t*)item->value;
		if(th->id == pid)
			return th;
	}
	
	//Reached here -> we've not found anything at all.
	foreach(item_, thread_wait)
	{
		thread_t *th = (thread_t*)item_->value;
		if(th->id == pid)
			return th;
	}
		
	return 0;
}
thread_t *GetThreadByName(const char *name)
{
	if(strcmp(current_thread->name, name))
		return (thread_t*)current_thread;
		
	foreach(item, thread_ready)
	{
		thread_t *th = (thread_t*)item->value;
		if(strcmp(th->name, name))
			return th;
	}
	
	//Reached here -> we've not found anything at all.
	foreach(item_, thread_wait)
	{
		thread_t *th = (thread_t*)item_->value;
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
	new->id = next_pid++;
	new->priority = current_thread->priority;
	new->state = RUNNABLE;
	new->console = current_thread->console;
	new->esp_loc = (uint)kmalloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
	new->esp0 = (uint)kmalloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
	new->page_directory = dir;
	new->signal_queue = list_copy(current_thread->signal_queue);
	new->parent = (thread_t*)current_thread;
	
	total_thread_count++;
	make_thread_ready(new);
	
	//Read the current intstruction pointer
	eip = read_eip();
	if(current_thread == parent)
	{
		//Check the magic
		if(magic != 0xDEADBEEF)
		{
			kprint("Bad fork() magic, current pid %i\n", current_thread->id);
			asm volatile("cli");
			for(;;);
		}
		//Fetch base pointer and stack pointer (current)
		asm volatile("mov %%esp, %0" : "=r"(esp));
		asm volatile("mov %%ebp, %0" : "=r"(ebp));
		//Calculate the new bp and sp
		if(current_thread->esp_loc > new->esp_loc)
		{
			new->esp = esp - (current_thread->esp_loc - new->esp_loc);
			new->ebp = ebp - (current_thread->esp_loc - new->esp_loc);
		}
		else
		{
			new->esp = esp + (new->esp_loc - current_thread->esp_loc);
			new->ebp = ebp - (current_thread->esp_loc - new->esp_loc);
		}
		//Copy the stack
		memcpy((byte*)(new->esp_loc - KERNEL_STACK_SIZE), (byte*)(current_thread->esp_loc - KERNEL_STACK_SIZE), KERNEL_STACK_SIZE);
		//set the eip
		new->eip = eip;
		
		asm volatile("sti");
		return new->id;
	}
	else
	{
		//Check the magic
		if(magic != 0xDEADBEEF)
		{
			kprint("Bad fork() magic, current pid %i\n", current_thread->id);
			asm volatile("cli");
			for(;;);
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
			if(n->state == RUNNABLE || !QueueIsEmpty(n))
			{
				notify_event(EVENT_THREAD_WAKE, (void*)n);
				n->state = RUNNABLE;
				list_delete(thread_wait, item);
				return n;
				break;
			}
		}
	}
	
	//In case this is the only running thread in a que
	if(thread_ready->length == 0)
		return (thread_t*)current_thread;
		
	node_t *np = list_dequeue(thread_ready);
	if(!np) //no thread in this que
	{
		if(thread_ready->length == 0)
			return (thread_t*)current_thread; 
		return get_next_thread();
	}
	
	thread_t *ret = (thread_t*)np->value;
	if(!ret) //do as same as above
	{
		notify_event(EVENT_THREAD_CORRUPT, (void*)np);
		if(thread_ready->length == 0)
			return (thread_t*)current_thread; //Don't perform a switch in case we reach the final..
		
		return get_next_thread();
	}
    
    if(valid_magic((void*)np))
	    free((void*)np);
	    
	if(ret->state != RUNNABLE)
	{
		if(ret->state == DEAD || ret->state == SLEEPING)
		{
			notify_event(EVENT_THREAD_DIE, (void*)ret);
			list_insert(thread_dead, (void*)ret);
		}
		if(ret->state == WAITING)
		{
			notify_event(EVENT_THREAD_WAIT, (void*)ret);
			list_insert(thread_wait, (void*)ret);
		}
			
		if(thread_ready->length == 0)
			return (thread_t*)current_thread;
			
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
    if(valid_magic((void*)np))
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
			if(th->state == SLEEPING)
			{
				list_insert(thread_sleep, (void*)th);
				continue;
			}
			free((void*)(th->esp_loc - KERNEL_STACK_SIZE));
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
	if(next_pid <= 2 || !enabled)
		return;
//	if(current_thread->timeslice > 0)
//		current_thread->timeslice--;
	
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
	if(current_thread->state != RUNNABLE)
	{
		if(current_thread->state == WAITING)
		{
			notify_event(EVENT_THREAD_WAIT, (void*)current_thread);
			list_insert(thread_wait, (thread_t*)current_thread);
		}
		else if(current_thread->state == DEAD || current_thread->state == SLEEPING)
		{
			notify_event(EVENT_THREAD_DIE, (void*)current_thread);
			list_insert(thread_dead, (thread_t*)current_thread);
		}
		
	}	
	if(current_thread->state == RUNNABLE)
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
	
	//set_console(current_thread->console, 0);
	//kprint("%s", current_thread->name);
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
	if(th->state == SLEEPING)
	{
		foreach(item, thread_sleep)
		{
			thread_t *t = (thread_t*)item->value;
			if(t->id == th->id)
			{
				list_delete(thread_sleep, item);
				free(item);
				break;
			}
		}
	}
	th->state = DEAD;
	list_insert(thread_dead, (void*)th);
}

int GetPID()
{
	return current_thread->id;
}
inline thread_t *CurrentThread()
{
	return (thread_t*)current_thread;
}
void make_thread_ready(thread_t *th)
{
	th->timeslice = ORIG_TIMESLICE;
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
	thread_set_state(CurrentThread(), DEAD);
	asm volatile("sti");
	for(;;); //Never return
}

uint get_thread_count()
{
	return thread_ready->length + thread_wait->length + 1;
}
