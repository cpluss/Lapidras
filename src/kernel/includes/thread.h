#ifndef THREAD_H
#define THREAD_H
#include "types.h"
#include "cio.h"
#include "signal.h"
#include "paging.h"
#include "list.h"

#define ORIG_TIMESLICE 		2
#define KERNEL_STACK_SIZE 	0x1000

#define DEAD 		0
#define RUNNABLE 	1
#define WAITING 	2
#define SLEEPING	3

typedef struct thread
{
	uint id;			//Thread id	
	uint esp;			//stack
	uint esp0;			//Ring 0 stack
	uint esp_loc;		//Base location of the stack
	uint user_esp;		//User stack
	
	uint ebp;			//Base pointer
	uint eip;			//Instruction pointer
	
	uint state;			//State of the thread
	
	page_directory_t *page_directory;
	
	uint timeslice; //Current thread timeslice	
	virtual_console_t *console;	//The output console for the thread - stdout
	
	//There will be four priorities available
		//0 - The interactive processes ( module-gui for an example )
		//1 - Lesser interactive ( applications such as text editor etc )
		//2 - The non interactive ( compiler for an example )
		//3 - The absolutely no interactive constantly running processes .. ( dunno )
	uint priority;
	byte name[32];	//Thread name identifier
	list_t *signal_queue; //The signal queue, to recieve messages from other threads / services
	
	struct thread *next; //The next in the list
} thread_t;

//Start the multithreading
void start_multithreading(uint esp);

void _switch(thread_t *next);
void schedule();

//thread_t *CreateThread(const char *name, uint thread, uint priority, uint state);
thread_t *CreateThread(const char *name, uint eip, uint priority, uint state);
thread_t *GetThread(uint pid);
thread_t *GetThreadByName(const char *name);

int fork();				  //fork the process
//int _fork(uint priority); //fork the process to a given priority

void WaitForThread(uint pid);

void thread_set_state(thread_t *th, uint state);
void thread_set_priority(uint priority);
inline thread_t *CurrentThread();
void make_thread_ready(thread_t *th);

void set_interactive();

void exit(); //Exits the current thread, and makes it dead

int GetPID();

#endif
