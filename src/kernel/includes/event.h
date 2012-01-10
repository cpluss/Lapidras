#ifndef EVENT_H
#define EVENT_H
#include "types.h"

#define EVENT_THREAD_DIE		0x0FF
#define EVENT_THREAD_WAIT		0x100
#define EVENT_THREAD_CREATE		0x101
#define EVENT_THREAD_FORK		0x102
#define EVENT_THREAD_SWITCH		0x103
#define EVENT_THREAD_CORRUPT	0x104
#define EVENT_THREAD_WAKE		0x105

#define EVENT_MEM_ALLOC			0x200
#define EVENT_MEM_FREE			0x201

#define EVENT_KBD_CHAR			0x300

//typedef void (*isr_t)(registers_t*);
typedef void (*evt_t)(void *param);

typedef struct event
{
	evt_t handler;
	ushort type;
} event_t;

void init_events();

void register_event(ushort type, evt_t handler);
void unregister_event(ushort type, evt_t handler);

void notify_event(ushort type, void *param);

#endif
