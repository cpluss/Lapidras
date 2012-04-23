#include "system.h"
#include "thread.h"
#include "event.h"
#include "keymap.h"

extern virtual_console_t *current_visible_console;
extern keymap_t dvorak_keymap, qwerty_keymap;
keymap_t *current_keymap;

typedef struct
{
	char *buffer;
	uint ptr;
	char *name;
	thread_t *th;
} kbd_listener_t;
list_t *listeners;

static void begin_listen(char *buffer)
{
	kbd_listener_t *l = (kbd_listener_t*)kmalloc(sizeof(kbd_listener_t));
	l->buffer = buffer;
	l->th = CurrentThread();
	l->name = l->th->name;
	l->ptr = 0;
	
	list_insert(listeners, (void*)l);
}
static void stop_listen()
{
	foreach(item, listeners)
	{
		if(strcmp(CurrentThread()->name, ((kbd_listener_t*)item->value)->name))
		{
			kbd_listener_t *l = (kbd_listener_t*)item->value;
			list_delete(listeners, item);
			free(item);
			free(l);
			
			break;
		}
	}
}

void disable_kbd()
{
	byte tmp = inb(0x61);
	outb(0x61, tmp | 0x80);
}
void enable_kbd()
{
	byte tmp = inb(0x61);
	outb(0x61, tmp & 0x7F);
}

void kbd_get_string(char *b)
{
	set_backspace_point(); //Prevent the user of erasing last message

	//Begin to listen
	begin_listen(b);
	//Go into the waiting state.
	thread_set_state(CurrentThread(), STATE_WAITING);
	while(CurrentThread()->state == STATE_WAITING); //Wait for the string to complete
	stop_listen();
	
	remove_backspace_point();
}

int shift, alt, ctrl;
#define KBD_SHFT 0x100
#define KBD_CTRL 0x200
#define KBD_ALT  0x400
int state = 0;
static void kbd_handler(registers_t *regs)
{
	byte scancode = inb(0x60);
		
	//Modkeys
	switch(scancode)
	{
		case 0x2A: //Shift in
		case 0x36:
			//shift = 1;
            state |= KBD_SHFT;
			break;
		case 0xAA: //Shift out
		case 0xB6:
			state &= ~(KBD_SHFT);
			break;
			
		case 0x1D: //Ctrl in
			state |= KBD_CTRL;
			break;
		case 0x9D: //Ctrl out
			state &= ~(KBD_CTRL);
			break;
		
		case 0x38: //Alt in
			state |= KBD_ALT;
			break;
		case 0xB8: //Alt out
			state &= ~(KBD_ALT);
			break;
	}
	
	byte c = current_keymap->map[scancode];
	if(scancode & 0x80)
		return;
	
	if((state & KBD_SHFT) && (c > 0x60 && c < 0x7A))
		c = current_keymap->map_shft[scancode];
	else if(state & KBD_ALT)
		c = current_keymap->map_alt[scancode];
	kputc_v(current_visible_console, c);
    uint p = state | c;
	notify_event(EVENT_KBD_CHAR, (void*)&p);
	
	if(listeners->length > 0)
	{
		foreach(item, listeners)
		{
			kbd_listener_t *l = (kbd_listener_t*)item->value;
			
			//all other printable characters is above space ( 0x20 )
			if(c == 0x08 || c >= 0x20)
			{
				if(c == 0x08 && l->ptr > 0)
				{
					l->ptr--;
					l->buffer[l->ptr] = ' ';
				}
				else
				{
					l->buffer[l->ptr] = c;
					l->ptr++;
				}
			}
			else if(c == 0x0D) //enter -> we're done reading..
			{
				l->buffer[l->ptr] = 0x00; //string terminator
				kputc('\n');
				
				//Wake the thread from its sleep
				thread_set_state(l->th, STATE_RUNNABLE);
			}
		}	
	}
	
	outb(0x20, 0x20);
}

void set_dvorak()
{
	current_keymap = &dvorak_keymap;
}
void set_qwerty()
{
	current_keymap = &qwerty_keymap;
}

void init_kbd()
{
	asm volatile("cli"); 	//No interrupts now...
	register_interrupt_handler(33, &kbd_handler);
	//set_dvorak(); //by default ;)
	set_qwerty();
	
	listeners = list_create();
	
	//clear the keyboard buffer
	while(inb(0x64) & 0x01)
		inb(0x60);
	
	//enable it
	disable_kbd();
	enable_kbd();
	asm volatile("sti");
}
