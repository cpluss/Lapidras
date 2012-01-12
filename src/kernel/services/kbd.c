#include "system.h"
#include "thread.h"
#include "event.h"

extern virtual_console_t *current_visible_console;

/*default keymap - dvorak*/
byte dvorak_keymap_ascii[128] =
{
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
	'9', '0', '-', '=', 0x08,	/* Backspace */
	0x09,			/* Tab */
	0, 0, 0, 'p',	/* 19 */
	'y', 'f', 'g', 'c', 'r', 'l', ',', '*',
	0x0D,	/* Enter key */
	0,			/* 29   - Control */
	'a', 'o', 'e', 'u', 'i', 'd', 'h', 't', 'n', 's',	/* 39 */
	'-', '\'',   0x0E,		/* Left shift */
	'\'', '.', 'q', 'j', 'k', 'x', 'b',			/* 49 */
	'm', 'w', 'v', 'z',   0x0E,				/* Right shift */
	'*',
	0x0F,	/* Alt */
	' ',	/* Space bar */
	0x10,	/* Caps lock */
	0xF1,	/* 59 - F1 key ... > */
	0xF2,   0xF3,   0xF4,   0xF5,   0xF6,   0xF7,   0xF8,   0xF9,
	0xFA,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0x0E,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0x20,	/* Left Arrow */
	0,
	0x22,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0x24,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */
};
char second_map[128] = 
{
		0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
	'9', '0', '-', '=', 0x08,	/* Backspace */
	0x09,			/* Tab */
	'\'', '|', ':', ']',	/* 19 */
	'$', '"', '?', '&', '<', '>', 0, 0,
	0x0D,	/* Enter key */
	0,			/* 29   - Control */
	';', '/', '(', ')', '|', '#', '^', '#', '"', '~',	/* 39 */
	'`', '*',   0x0E,		/* Left shift */
	'\'', ':', '=', '@', '!', '\\', '%',			/* 49 */
	'`', 0, 0, 0,   0x0E,				/* Right shift */
	'*',
	0x0F,	/* Alt */
	' ',	/* Space bar */
	0x10,	/* Caps lock */
	0xF1,	/* 59 - F1 key ... > */
	0xF2,   0xF3,   0xF4,   0xF5,   0xF6,   0xF7,   0xF8,   0xF9,
	0xFA,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0x0E,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0x20,	/* Left Arrow */
	0,
	0x22,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0x24,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */
};

char qwerty_keymap_ascii[128] =
{
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
	'9', '0', '-', '=', '\b',	/* Backspace */
	'\t',			/* Tab */
	'q', 'w', 'e', 'r',	/* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0x0d,	/* Enter key */
	0,			/* 29   - Control */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
	'\'', '`',   0x0E,		/* Left shift */
	'\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
	'm', ',', '.', '/',   0x0E,				/* Right shift */
	'*',
	0x0F,	/* Alt */
	' ',	/* Space bar */
	0,	/* Caps lock */
	0xF1,	/* 59 - F1 key ... > */
	0xF2,   0xF3,   0xF4,   0xF5,   0xF6,   0xF7,   0xF8,   0xF9,
	0xFA,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0,	/* Left Arrow */
	0,
	0,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0xFB,	/* F11 Key */
	0xFC,	/* F12 Key */
	0,	/* All other keys are undefined */
};

typedef struct
{
	char *buffer;
	uint ptr;
	char *name;
	thread_t *th;
} kbd_listener_t;
char *current_keymap;
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
	thread_set_state(CurrentThread(), WAITING);
	while(CurrentThread()->state == WAITING); //Wait for the string to complete
	stop_listen();
	
	remove_backspace_point();
}

int shift, alt, ctrl;
static void kbd_handler(registers_t *regs)
{
	byte scancode = inb(0x60);
		
	//Modkeys
	switch(scancode)
	{
		case 0x2A: //Shift in
		case 0x36:
			shift = 1;
			break;
		case 0xAA: //Shift out
		case 0xB6:
			shift = 0;
			break;
			
		case 0x1D: //Ctrl in
			ctrl = 1;
			break;
		case 0x9D: //Ctrl out
			ctrl = 0;
			break;
		
		case 0x38: //Alt in
			alt = 1;
			break;
		case 0xB8: //Alt out
			alt = 0;
			break;
	}
	
	byte c = current_keymap[scancode];
	if(scancode & 0x80)
		return;
	
	if((shift == 1) && (c > 0x60 && c < 0x7A))
		c -= 0x20;
	if(alt)
		c = second_map[scancode];
	kputc_v(current_visible_console, c);
	notify_event(EVENT_KBD_CHAR, (void*)&c);
	
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
				thread_set_state(l->th, RUNNABLE);
			}
		}	
	}
	
	outb(0x20, 0x20);
}

void set_dvorak()
{
	current_keymap = dvorak_keymap_ascii;
}
void set_qwerty()
{
	current_keymap = qwerty_keymap_ascii;
}

void init_kbd()
{
	asm volatile("cli"); 	//No interrupts now...
	register_interrupt_handler(33, &kbd_handler);
	set_dvorak(); //by default ;)
	
	listeners = list_create();
	
	//clear the keyboard buffer
	while(inb(0x64) & 0x01)
		inb(0x60);
	
	//enable it
	disable_kbd();
	enable_kbd();
	asm volatile("sti");
}
