#include "event.h"
#include "list.h"
#include "cio.h"

list_t *events;

void init_events()
{
	events = list_create();
}

void register_event(ushort type, evt_t handler)
{
	event_t *ev = (event_t*)kmalloc(sizeof(event_t));
	memset((byte*)ev, 0, sizeof(event_t));
	ev->type = type;
	ev->handler = handler;
	list_insert(events, (void*)ev);
}

void unregister_event(ushort type, evt_t handler)
{
	foreach(item, events)
	{
		event_t *ev = (event_t*)item->value;
		if(ev->type == type && ev->handler == handler)
		{
			list_delete(events, item);
			free(item);
			free(ev);
			break;
		}
	}
}

void notify_event(ushort type, void *param)
{
#if 0
	kputc('[');
	ksetforeground(C_GREEN);
	kprint("events");
	ksetdefaultcolor();
	kprint("]:%x raised\n");
#endif
	foreach(item, events)
	{
		event_t *ev = (event_t*)item->value;
		if(ev->type == type)
			ev->handler(param);
	}
}
