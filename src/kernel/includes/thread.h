#ifndef THREAD_H
#define THREAD_H
#include "types.h"
#include "isr.h"
#include "list.h"
#include "paging.h"
#include "signal.h"

#define KERNEL_STACK_SIZE 	0x1000
#define STATE_DEAD 		0
#define STATE_RUNNABLE 	1
#define STATE_WAITING 	2
#define STATE_SLEEPING	3

typedef struct image
{
	uint size;
	uint stack;
	uint user_stack;
	uint start;
	uint entry;
} image_t;

typedef struct thread
{
	uint pid;
	uint esp;
	uint esp0;
	uint ebp;
	uint eip;
	
    uint state;

	page_directory_t *page_directory;
	
	image_t base;
	char name[16];
	list_t *signal_queue;
	//uint state;
	
    registers_t *syscall_registers;
	struct thread *parent;
} thread_t;

//Start the multithreading
void start_multithreading(uint esp);

void _switch(thread_t *next);
void schedule();

thread_t *CreateThread(const char *name, uint eip, uint state);
thread_t *GetThread(uint pid);
thread_t *GetThreadByName(const char *name);

int fork();

void thread_set_state(thread_t *th, uint state);
inline thread_t *CurrentThread();
void make_thread_ready(thread_t *th);

void kill_thread(thread_t *th);
void exit();

int GetPID();

#endif
