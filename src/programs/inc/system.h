#ifndef SYSCALL_DEFN_H
#define SYSCALL_DEFN_H
//from types.h ( kernel source )
typedef unsigned char byte;
typedef unsigned int  uint;
typedef unsigned short ushort;
typedef unsigned long ulong;

#define EVENT_THREAD_DIE		0x0FF
#define EVENT_THREAD_WAIT		0x100
#define EVENT_THREAD_SWITCH		0x103
#define EVENT_THREAD_WAKE		0x105

#define EVENT_KBD_CHAR			0x300
typedef void (*evt_t)(void *param);

typedef struct signal
{
	char message[128];
	int priority;
	int priority_from;
	int id;
	
	int from_pid;
	int to_pid;
} signal_t;

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
	
	uint *console;
	
	//page_directory_t *page_directory;
	uint *page_directory;
	
	//There will be four priorities available
		//0 - The interactive processes ( gui for an example )
		//1 - Lesser interactive ( applications such as text editor etc )
		//2 - The non interactive ( compiler for an example )
		//3 - The absolutely no interactive constantly running processes .. ( dunno )
	uint priority;
	
	byte name[32];	//Thread name identifier
	
	uint *signal_queue; //The signal queue, to recieve messages from other threads / services
	
	void (*thread_c)();	//Thread entry..
} thread_t;

extern int kputs(char *s);
extern void kprint(char *s, ...);

extern int SendSignal(signal_t *s);
extern int GetPID();
extern thread_t *GetThreadByName(const char *name);
extern thread_t *GetThread(uint pid);
extern thread_t *CreateThread(const char *name, void (*thread)(), uint priority, uint idle);
extern thread_t *CurrentThread();
extern void WaitForSignal();
extern signal_t *LatestSignal();
extern void FastSignal(byte *message, const char *name);
extern int QueueIsEmpty(thread_t *th);

extern int fopen(const char *name);
extern void fclose(int handle);
extern int fread(byte *buffer, uint size, uint n, int handle);
extern int fwrite(byte *buffer, uint size, uint n, int handle);
extern int ftell_size(int handle);

extern void *kmalloc(int sz);
extern void kfree(void *memory);

extern void register_event(ushort type, evt_t handler);
extern void unregister_event(ushort type, evt_t handler);

#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)
#define va_copy(d,s)  __builtin_va_copy(d,s)
typedef __builtin_va_list va_list;

extern void wait(int ms);

#endif
