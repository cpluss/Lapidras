#ifndef TASK_H
#define TASK_H
#include "types.h"
#include "signal.h"
#include "paging.h"
#include "cio.h"

#define MAX_THREADS 256
#define ORIG_TIMESLICE 2

typedef struct thread
{
	uint id;		//Thread id
	uint esp0;		//Ring 0 stack
	uint esp3;		//Usermode stack
	
	uint state : 1; //Alive or dead thread?
	uint executed : 1; //Executed?
	uint idle : 1; //We should not switch to this thread, it's idle ..
	uint waiting : 1; //If we wait for a signal or not
	
	uint timeslice; //Current thread timeslice
	uint orig_timeslice;
	
	virtual_console_t *console;
	uint t_keep : 1;
	
	page_directory_t *page_directory;
	
	//There will be four priorities available
		//0 - The interactive processes ( gui for an example )
		//1 - Lesser interactive ( applications such as text editor etc )
		//2 - The non interactive ( compiler for an example )
		//3 - The absolutely no interactive constantly running processes .. ( dunno )
	uint priority;
	
	byte name[32];	//Thread name identifier
	
	signal_queue_t signal_queue; //The signal queue, to recieve messages from other threads / services
	
	void (*thread_c)();	//Thread entry..
} thread_t;

typedef struct threadpool
{
	thread_t *thread[MAX_THREADS];
	uint pid_count;
	uint thread_count;
	uint alive_threads;
	
	uint dead_threads[MAX_THREADS];
	uint got_dead_thread;
	
	uint priority;
} threadpool_t;

//Createthread routine - to create a new thread
thread_t *CreateThread(const char *name, void (*thread)(), uint priority, uint idle, uint visible, uint cons);
//Fork the current process.
//A great way for a process to change its priority ;)
int fork(uint priority);

thread_t *GetThread(uint id);
thread_t *GetThreadByName(const char *name);
thread_t *CurrentThread();

//Get PID
uint GetPID();
uint GotMT();

//Yield the current process ..
void set_sched();
void set_interactive();

void set_mt(uint state); //1 - on, 0 - disabled

uint get_thread_count();

//exits the current process
void exit();

void WaitForSignal();
signal_t *LatestSignal();

void init_threading();

//Kill the thread
void KillThread(thread_t *th);
#endif
