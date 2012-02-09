#include "thread.h"
#include "memory.h"
#include "list.h"

signal_t *LatestSignal()
{
	return GetSignal(CurrentThread());
}
void WaitForSignal()
{
	//Wait this current thread for a single signal
	asm volatile("cli");	//No interrupt atm !
	thread_set_state(CurrentThread(), STATE_WAITING);
	//Wait infinitely
	asm volatile("sti"); //Enable them again as these changes have been made
	while(QueueIsEmpty(CurrentThread()));
	//The user can now pop the latest one
}

signal_t *GetSignal(thread_t *th)
{
	node_t *n = list_pop(th->signal_queue);
	if(!n)
		return 0;
	signal_t *ret = (signal_t*)n->value;
	free(n);
	return ret;
}
void SendSignal(signal_t *s)
{
	asm volatile("cli");
	thread_t *to = GetThread(s->to_pid);
	if(!to)
	{
		kprint("Could not send signal to %i\n", s->to_pid);
		return;
	}
		
	list_insert(to->signal_queue, (void*)s);
	
	asm volatile("sti");
}

void FastSignal(byte *message, const char *name)
{
	asm volatile("cli");
	thread_t *th = GetThreadByName(name);
	if(!th)
	{
		kprint("Could not send '%s' to %s\n", message, name);
		return;
	}
	
	signal_t *signal = (signal_t*)kmalloc(sizeof(signal_t));
	memset((byte*)signal, 0, sizeof(signal_t));
	strcpy(signal->message, message);
	signal->to_pid = th->pid;
	signal->from_pid = GetPID();
		
	list_insert(th->signal_queue, (void*)signal);
	
	asm volatile("sti");
}
void FastSignalP(byte *message, int pid)
{
	asm volatile("cli");
	thread_t *th = GetThread(pid);
	if(!th)
	{
		kprint("Could not send '%s' to %i\n", message, pid);
		return;
	}
	
	signal_t *signal = (signal_t*)kmalloc(sizeof(signal_t));
	memset((byte*)signal, 0, sizeof(signal_t));
	strcpy(signal->message, message);
	signal->to_pid = th->pid;
	signal->from_pid = GetPID();
		
	list_insert(th->signal_queue, (void*)signal);
	
	asm volatile("sti");
}

signal_t ComposeSignal(byte *message, uint from, uint to)
{
	signal_t sig;
	strcpy(sig.message, message);
	sig.from_pid = from;
	sig.to_pid = to;
	return sig;
}

byte QueueIsEmpty(thread_t *th)
{
	return (th->signal_queue->length == 0);
}
