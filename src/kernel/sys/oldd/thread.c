#include "thread.h"
#include "memory.h"
#include "string.h"
#include "paging.h"

//extern declarations
extern page_directory_t *current_directory, *kernel_directory;

//The threadpools
threadpool_t thread_pool[3]; //We got one threadpool for each priority
uint current_priority = 0;

//we have to have a idle thread, in priority 3
thread_t *idle_thread;

uint proc_yield = 0;
uint check_priority = 0;

int current_pid = -1;
thread_t *current_thread;
int pid_count = 0;
uint total_thread_count = 0;

//Is multitasking enabled or not?
byte enabled = 0;

inline void AddThreadToThreadPool(thread_t *th, uint priority)
{
	threadpool_t *threadpool = &thread_pool[priority];
	//Find a free spot
	/*if(threadpool->got_dead_thread)
	{
		int i, a = 0, b = 0;
		for(i = 0; i < MAX_THREADS; i++)
		{
			if(threadpool->dead_threads[i] != -1 && a == 0)
			{
				//dead_threads[i] is the new pid
				int id = threadpool->dead_threads[i];
				th->id = id;
				
				//free last thread memory to replace with ours
				if(!threadpool->thread[id]->t_keep)
				{
					free(threadpool->thread[id]->console->console_buffer);
					free(threadpool->thread[id]->console);
				}
				free(threadpool->thread[id]);
				
				threadpool->thread[id] = th;
				th->id = id;
				a = 1; //Assigned
			}
			else if(threadpool->dead_threads[i] != -1 && a == 1)
			{
				b = 1;
				break;
			}
		}
		if(b == 0)
			threadpool->got_dead_thread = 0;
	}
	else
	{*/
		threadpool->thread[threadpool->pid_count] = th;
		threadpool->pid_count++;
	//}
	//Increase the thread count of the threadpool
	threadpool->thread_count++;
	threadpool->alive_threads++; //never to be decreased
	th->priority = priority;
	th->id = pid_count;// - 1;
	pid_count++;
	total_thread_count++;
}
thread_t *CreateThread(const char *name, void (*thread)(), uint priority, uint idle, uint visible, uint cons)
{
	if(priority < 0 || priority > 3)
		return 0;
	
	asm volatile("cli"); //We don't want to be interrupted here ..	
	uint *stack;
	thread_t *th = (thread_t*)alloc(sizeof(thread_t));
	memset((byte*)th, 0, sizeof(thread_t));
	th->esp0 = (uint)alloc(0x1000) + 0x1000; //Allocate a 4kb stack
	
	stack = (uint*)th->esp0;
	*stack-- = 0x202;		//eflags
	*stack-- = 0x08;		//cs
	*stack-- = (uint)thread;//eip
	
	//pusha
	*stack-- = 0;           //edi
    *stack-- = 0;           //esi
    *stack-- = 0;           //ebp
	*stack-- = 0;           //null
	*stack-- = 0;           //ebx
	*stack-- = 0;           //edx
	*stack-- = 0;           //ecx
	*stack-- = 0;           //eax
	
	//data segments
	*stack-- = 0x10;		//ds
	*stack-- = 0x10;		//es
	*stack-- = 0x10;		//fs
	*stack = 0x10;			//gs
	
	th->state = 1;
	th->timeslice = ORIG_TIMESLICE;
	th->orig_timeslice = ORIG_TIMESLICE;
	th->esp0 = (uint)stack;
	th->thread_c = thread;
	strcpy((char*)th->name, (char*)name);
	memset(&th->signal_queue, 0, sizeof(signal_queue_t));
	th->idle = idle;
	
	if(cons)
	{
		//create a virtual console for the process
		th->console = (virtual_console_t*)alloc(sizeof(virtual_console_t));
		memset(th->console, 0, sizeof(virtual_console_t));
		th->console->console_buffer = (char*)alloc(CONSOLE_BUFFER_SIZE);
		memset(th->console->console_buffer, 0, CONSOLE_BUFFER_SIZE);
		th->console->visible = visible;
		if(visible)
		{
			th->console->y = 1;
			th->console->x = 0;
		}
		th->t_keep = 1;
	}
	th->t_keep = 0;
	
	AddThreadToThreadPool(th, priority);
	
	page_directory_t *dir = clone_directory(current_directory);
	th->page_directory = dir;
	
	//In case this is above the current priority
	set_interactive();

	asm volatile("sti");
}
int fork(uint priority)
{
	asm volatile("cli"); //No interrupts..
	uint pid = GetPID();
	uint esp, *stack;
	
	//Create the clone
	thread_t *clone = (thread_t*)kmalloc(sizeof(thread_t));
	memcpy((byte*)clone, (byte*)CurrentThread(), sizeof(thread_t));
	clone->priority = priority;
	
	//Assign it a new page directory
	page_directory_t *dir = clone_directory(current_directory);
	clone->page_directory = dir;
	
	AddThreadToThreadPool(clone, priority);
	
	uint eip = read_eip();
	
	if(pid == GetPID())
	{		
		kprint("EIP: %x\n", eip);
		//set the stack
		asm volatile("mov %%esp, %0" : "=r"(esp));
		clone->esp0 = esp;
		//Manipulate the stack.
		asm volatile("mov %0, %%cr3" : : "r"(dir->physicalAddr));
		stack = (uint*)clone->esp0;
		*stack-- = 0x202;		//eflags
		*stack-- = 0x08;		//cs
		*stack-- = (uint)eip;	//eip
		
		//pusha
		*stack-- = 0;           //edi
		*stack-- = 0;           //esi
		*stack-- = 0;           //ebp
		*stack-- = 0;           //null
		*stack-- = 0;           //ebx
		*stack-- = 0;           //edx
		*stack-- = 0;           //ecx
		*stack-- = 0;           //eax
		
		//data segments
		*stack-- = 0x10;		//ds
		*stack-- = 0x10;		//es
		*stack-- = 0x10;		//fs
		*stack = 0x10;			//gs
		asm volatile("mov %0, %%cr3" : : "r"(current_directory->physicalAddr));
		clone->esp0 = (uint)stack;
		
		asm volatile("sti");
		return clone->id;
	}
	else
		return 0;
}
void print_number(uint n)
{
	kprint("N: %x\n", n);
}

void an_idle_thread()
{
	for(;;);
}
void init_threading()
{
	//Initialize each threadpool
	int i;
	for(i = 0; i < 3; i++)
	{
		//Allocate a new thread pool
		threadpool_t *threadpool = &thread_pool[i];
		
		threadpool->priority = i;
		int j;
		for(j = 0; j < MAX_THREADS; j++)
			threadpool->dead_threads[j] = -1;
			
		//Assign it.
		thread_pool[i] = *threadpool;
	}
	
	enabled = 1;
}

thread_t *GetThread(uint id)
{
	//for each priority group search
	int i, j;
	for(i = 0; i < 3; i++)
	{
		threadpool_t *threadpool = &thread_pool[i];
		for(j = 0; j <= threadpool->thread_count; j++)
		{
			if(threadpool->thread[j]->id == id)
				return threadpool->thread[j];
		}
	}
	return 0;
}
thread_t *GetThreadByName(const char *name)
{
	//for each priority group search
	int i, j;
	for(i = 0; i < 3; i++)
	{
		threadpool_t *threadpool = &thread_pool[i];
		if(threadpool->thread_count <= 0)
			continue;
		for(j = 0; j < threadpool->thread_count; j++)
		{
			if(strcmp(threadpool->thread[j]->name, (char*)name))
				return threadpool->thread[j];
		}
	}
	return 0;
}
thread_t *CurrentThread()
{
	return current_thread;
}
uint GetPID()
{
	return current_pid;
}
uint GotMT()
{
	return enabled;
}
//Yield the current process ..
void set_sched()
{
	proc_yield = 1;
}
void set_interactive()
{
	check_priority = 1;
}

void set_mt(uint state)
{
	enabled = state;
}

uint get_thread_count()
{
	return total_thread_count;
}

void WaitForSignal()
{
	//Wait this current thread for a single signal
	asm volatile("cli");	//No interrupt atm !
	thread_t *current = CurrentThread();
	current->idle = 1;
	current->waiting = 1;
	set_interactive();
	//Wait infinitely
	asm volatile("sti"); //Enable them again as these changes have been made
	while(QueueIsEmpty(&current->signal_queue));
	//The user can now pop the latest one
}
signal_t *LatestSignal()
{
	return GetSignal(CurrentThread());
}

void exit()
{
	asm volatile("cli");
	thread_t *current = CurrentThread();
	current->state = 0;
	asm volatile("sti");
}

static void ExecuteThread(threadpool_t *threadpool, thread_t *th)
{
	//Now store it
	threadpool->got_dead_thread = 1;
	th->t_keep = 0;
	int i;
	for(i = 0; i < MAX_THREADS; i++)
	{
		if(threadpool->dead_threads[i] == -1)
		{
			threadpool->dead_threads[i] = th->id;
			threadpool->alive_threads--;
			break;
		}
	}
	
	total_thread_count--;
	//Executed ..
	th->executed = 1;
}
static byte CheckPriority(int pri)
{
	threadpool_t *threadpool = &thread_pool[pri];
	int i;
	for(i = 0; i < threadpool->thread_count; i++)
	{
		thread_t *th = threadpool->thread[i];
		if(!th->idle)
			return 1;
		if(th->idle && th->waiting && !QueueIsEmpty(&th->signal_queue))
			return 1;
	}
	
	return 0;
}
//Scheduling function -> switch to next task
extern uint tick;
uint slice = 0;
uint schedule(uint context)
{
	//Increase current tick count
	tick++;
	
	//Not enough threads to switch ..
	if(enabled == 0)
		return context;
	if(current_thread->timeslice > 0 && !proc_yield && !check_priority && (uint)current_thread != 0)
	{
		current_thread->timeslice--;
		return context;
	}
	if(proc_yield)
		proc_yield = 0;
	if(check_priority || current_thread->id == idle_thread->id)
	{
		//Check if any with higher priority not is idle anymore, or got new messages
		int i;
		for(i = 1; i < 3; i++)
		{
			if(CheckPriority(i))
			{
				current_priority = i;
				current_pid = 0;
				break;
			}
		}
		check_priority = 0;
	}
	
	threadpool_t *threadpool = &thread_pool[current_priority];
	if(threadpool->thread_count <= 0)
	{
		while(threadpool->thread_count <= 0)
		{
			current_priority++;
			threadpool = &thread_pool[current_priority];
			if(current_priority >= 3)
				break;
		}
		if(threadpool->thread_count <= 0)
			return context;
		current_pid = 0;
	}
	
	//Save the current context
	if(current_thread != 0)
		current_thread->esp0 = context;
	
	//Is it only one process in this priority?
	if(threadpool->thread_count == 1 && !threadpool->thread[0]->idle)
	{	
		//If it's not set, set it ..
		if((uint)current_thread != (uint)threadpool->thread[0])
		{
			current_thread = threadpool->thread[0];
			current_pid = 0;
			current_thread->timeslice = current_thread->orig_timeslice;
			return current_thread->esp0;
		}
		
		if(current_thread->idle)
			set_interactive();
			
		return context;
	}
	
	//fetch a new thread
	int f = 0;	
	int tries = 0;
	int i;
	for(i = current_pid + 1;tries < 2 && !f; i++)
	{
		if(current_pid >= threadpool->thread_count)
		{
			current_pid = -1;
			i = 0;
			tries++;
		}
		
		thread_t *th = threadpool->thread[i];			
		if(th->waiting && th->idle && !QueueIsEmpty(&th->signal_queue))
		{
			th->idle = 0;
			current_pid = i;
			f = 1;
			break;
		}
		if(th->idle == 0 && th->state == 1)
		{
			current_pid = i;
			f = 1;
			break;
		}
		if(th->state == 0 && !th->executed)
			ExecuteThread(threadpool, th);
	}	
	
	//No new one found
	if(!f)
	{
		if(current_priority < 3)
		{
			current_priority++;
			set_interactive(); //perform a switch next interrupt ..
		}
		else
		{
			//none found at ALL!
			set_interactive();
			return idle_thread->esp0;
		}
		
		return context;
	}
	
	if(current_pid >= threadpool->thread_count)
	{
		current_pid = -1;
		return context; //Don't perform the actual switch
	}
	
	uint last_id = current_thread->id;
	//Switch to a new thread
	current_thread = threadpool->thread[current_pid];
	current_thread->timeslice = current_thread->orig_timeslice;
	
	//Now switch the page directory
	asm volatile("mov %0, %%cr3" : : "r"(current_thread->page_directory->physicalAddr));
	//switch to the appropriate console
	set_console(current_thread->console, 0);

	return current_thread->esp0;
}
