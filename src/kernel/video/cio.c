#include "cio.h"
#include "ports.h"
#include "args.h"
#include "thread.h"

//local variables
int _cx, _cy = 1;
int _bx, _by;
byte color = 0x9F;
byte background = 0x00, foreground = 0x07;
byte d_background = 0x00, d_foreground = 0x07;

virtual_console_t *current_console;
virtual_console_t *video_console;
virtual_console_t *current_visible_console;

void ksetdefaultcolor()
{
	background = d_background;
	foreground = d_foreground;
	ksetcolor(background, foreground);
}
void kchangedefaultcolor(byte b, byte f)
{
	d_background = b * 0x10;
	d_foreground = f;
}
void ksetcolor(byte b, byte f)
{
	color = (b  | f );
}
void ksetbackground(byte b)
{
	background = b * 0x10;
	color = (background | foreground);
}
void ksetforeground(byte f)
{
	foreground = f;
	color = (background | f);
}

void scroll()
{
	if(_cy < 24) //at the bottom yet?
		return;
	
	int _ry = 1;
	int x, y;
	for(y = 1; y <= 23; y++)
	{
		for(x = 0; x < 80; x++)
		{
			_ry = y + 1;
			unsigned char *loc = (unsigned char*)(current_console->console_buffer + (x << 1) + (y * 160));
			unsigned char *where = (unsigned char*)(current_console->console_buffer + (x << 1) + (_ry * 160));
			
			loc[0] = where[0];
			loc[1] = where[1];
		}
	}
	
	_cy = 23;
	_cx = 0;
	move_cursor();
}

void move_cursor()
{
	unsigned short location = (_cy * 80 + _cx);
	
	outb(0x3D4, 14); //tell vga we are setting the high cursor byte
	outb(0x3D5, (location >> 8)); //send it
	
	outb(0x3D4, 15); //tell vga we are setting low cursor byte
	outb(0x3D5, location);
}
void set_cursor(int x, int y)
{
	_cx = x;
	_cy = y;
}
int get_x()
{
	return _cx;
}
int get_y()
{
	return _cy;
}

void set_backspace_point()
{
	_bx = _cx;
	_by = _cy;
}
void remove_backspace_point()
{
	_bx = -1;
	_by = -1;
}

void set_console(virtual_console_t *console, uint takeover)
{
	if(current_console == console)
		return;
	
	//save the last console
	asm volatile("cli");
	//Copy current contents
	if(takeover && current_console->old_console_buffer != (char*)VIDMEM_BUFFER)
	{
		//Copy the buffer to the visible layer
		memcpy(current_visible_console->old_console_buffer, (char*)VIDMEM_BUFFER, CONSOLE_BUFFER_SIZE);
		current_visible_console->console_buffer = current_visible_console->old_console_buffer;
	}
	current_console->x = _cx;
	current_console->y = _cy;
	current_console->bx = _bx;
	current_console->by = _by;
	
	//Switch to the new console
	current_console = console;
	if(takeover)
	{
		//We don't want to lose the pointer if the text is visible
		current_console->old_console_buffer = current_console->console_buffer;
		//clear the video memory
		memset((char*)(VIDMEM_BUFFER + CONSOLE_BUFFER_START), 0, CONSOLE_BUFFER_SIZE);
		//copy the contents to the visual buffer
		memcpy((char*)VIDMEM_BUFFER, current_console->console_buffer, CONSOLE_BUFFER_SIZE);
		current_console->console_buffer = (char*)VIDMEM_BUFFER; //now switch! ;)
		
		current_visible_console = current_console;
	}
	_cx = current_console->x;
	_cy = current_console->y;
	_bx = current_console->bx;
	_by = current_console->by;
	
	move_cursor();
	
	asm volatile("sti");
}

void kputc_v(virtual_console_t *console, byte c)
{
	//backspace
	if(c == 0x08)
	{
		//did we reach the point we should not reach?
		if(_bx != -1 && _by != -1)
			if(_cx == _bx && _cy == _by)
				return;
			
		_cx -= 1;
		if(_cx < 0)
		{
			_cx = 79;
			_cy--;
		}
		
		int _x = (_cx << 1); //multiply by two
		int _y = (_cy * 160); //each row ;)
		unsigned char *where = (unsigned char*)(console->console_buffer + _x + _y);
		where[0] = 0x20;
		where[1] = color;
	}
	else if(c == 0x09) //tab - implement later
	{
		_cx = (_cx + 8) & ~(8 - 1);
	}
	else if(c == '\r') //carriage return
	{
		_cx = 0;
	}
	else if(c == '\n') //newline
	{
		_cx = 0;
		_cy++;
	}
	else if(c >= ' ') //printable?
	{
		int _x = (_cx << 1); //multiply by two
		int _y = (_cy * 160); //each row ;)
		unsigned char *where = (unsigned char*)(console->console_buffer + _x + _y);
		where[0] = c;
		where[1] = color;
		
		_cx++;
	}
	
	if(_cx >= 80) //newline, again..
	{
		_cx = 0;
		_cy++;
	}
	
	current_console->x = _cx;
	current_console->y = _cy;
	
	scroll();
	move_cursor();
}
void kputc(byte c)
{
	kputc_v(current_console, c);
	move_cursor();
}
void kputs(byte *s)
{
	int i;
	for(i = 0; i < strlen(s); i++)
		kputc(s[i]);
}

void clear_screen()
{
	int _x, _y;
	unsigned char *where;
	for(_cy = 1; _cy < 25; _cy++)
	{
		for(_cx = 0; _cx < 80; _cx++)
		{
			_x = _cx << 1;
			_y = _cy * 160;
			where = (unsigned char*)(current_console->console_buffer + _y + _x);
			
			where[0] = 0x20;
			where[1] = color;
		}
	}
	
	_cy = 1;
	_cx = 0;
}

void kprint_v(virtual_console_t *console, byte *s, ...)
{
	va_list ap;
	int i, n, j = 0;
			
	va_start(ap, s);
	
	for(i = 0; i < strlen(s); i++)
	{
		if(s[i] == '%')
		{
			i++;
			char type = s[i];
			switch(type)
			{
				case 'c':
				{
					byte c = va_arg(ap, int);
					kputc_v(console, c);
				}
				case 's':
				{
					byte *str = va_arg(ap, byte*);
					for(j = 0; j < strlen(str); j++)
						kputc(str[j]);
				}break;
				case 'i':
				{
					int num = va_arg(ap, int);
					if(num == 0)
						kputc_v(console, '0');
					else
					{
						byte c[32];
						j = 0;
						while(num > 0)
						{
							c[j] = '0' + (num % 10);
							num /= 10;
							j++;
						}
						c[j] = 0; //null terminator
						
						while(j >= 0)
							kputc_v(console, c[j--]);
					}
				}break;
				case 'x':
				{
					int num = va_arg(ap, int);
					const char *hexarray = "0123456789ABCDEF";
					
					byte f = 0;
					for(j = 28; j > -1; j -= 4)
					{
						char o = hexarray[(num >> j) & 0xF];
						//if(o != '0')
						//	f = 1;
						//if(f == 0 && o == '0')
						//	continue;
						kputc_v(console, hexarray[(num >> j) & 0xF]);
					}
				}break;
			}
		}
		else
			kputc_v(console, s[i]);
	}
	
	va_end(ap);
}
void kprint(byte *s, ...)
{
	va_list ap;
	int i, n, j = 0;
			
	va_start(ap, s);
	
	for(i = 0; i < strlen(s); i++)
	{
		if(s[i] == '%')
		{
			i++;
			byte type = s[i];
			switch(type)
			{
				case 'c':
				{
					byte c = va_arg(ap, int);
					kputc(c);
				}
				case 's':
				{
					byte *str = va_arg(ap, byte*);
					for(j = 0; j < strlen(str); j++)
						kputc(str[j]);
				}break;
				case 'i':
				{
					int num = va_arg(ap, int);
					if(num == 0)
						kputc('0');
					else
					{
						char c[32];
						j = 0;
						while(num > 0)
						{
							c[j] = '0' + (num % 10);
							num /= 10;
							j++;
						}
						c[j] = 0; //null terminator
						
						while(j >= 0)
							kputc(c[j--]);
					}
				}break;
				case 'x':
				{
					int num = va_arg(ap, int);
					const char *hexarray = "0123456789ABCDEF";
					
					byte f = 0;
					for(j = 28; j > -1; j -= 4)
					{
						char o = hexarray[(num >> j) & 0xF];
						//if(o != '0')
						//	f = 1;
						//if(f == 0 && o == '0')
						//	continue;
						kputc(hexarray[(num >> j) & 0xF]);
					}
				}break;
			}
		}
		else
			kputc(s[i]);
	}
	
	va_end(ap);
}

void kset_status(char *str)
{
	//save the old x and y value as a reference
	int olx, oly;
	olx = _cx;
	oly = _cy;
	
	//the top status bar uses white on blue color.
	set_cursor(0, 0);
	//ksetbackground(C_BLUE);
	ksetforeground(C_WHITE);
	
	kprint(str);
	int i;
	for(i = strlen(str); i < 81; i++)
		kputc(' ');
	
	//restore the old values
	ksetdefaultcolor();
	set_cursor(olx, oly);
	move_cursor();
}

void init_video_console()
{
	video_console = (virtual_console_t*)kmalloc(sizeof(virtual_console_t));
	memset((byte*)video_console, 0, sizeof(virtual_console_t));
	video_console->console_buffer = (char*)VIDMEM_BUFFER;
	video_console->old_console_buffer = (char*)VIDMEM_BUFFER;
	current_console = video_console;
	current_visible_console = video_console;
}
