#ifndef SIGNAL_H
#define SIGNAL_H
#include "types.h"

#define SIG_KBD_CHAR 0x01

struct thread;
typedef struct signal
{
	char message[128];
	int id;
	
	int from_pid;
	int to_pid;
} signal_t;

signal_t *GetSignal(struct thread *th);
void SendSignal(signal_t *s);

signal_t *LatestSignal();
void WaitForSignal();

signal_t ComposeSignal(byte *message, uint from, uint to);

void FastSignal(byte *message, const char *name);
void FastSignalP(byte *message, int pid);

byte QueueIsEmpty();

#endif
