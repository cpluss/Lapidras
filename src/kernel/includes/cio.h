#ifndef CIO_H
#define CIO_H
#include "types.h"

/* Colors for the console.. */
#define C_BLACK 			0x00
#define C_BLUE 				0x01
#define C_GREEN				0x02
#define C_CYAN				0x03
#define C_RED				0x04
#define C_MAGENTA			0x05
#define C_BROWN				0x06
#define C_LIGHT_GRAY		0x07
#define C_DARK_GRAY			0x08
#define C_LIGHT_BLUE		0x09
#define C_LIGHT_GREEN		0x0A
#define C_LIGHT_CYAN		0x0B
#define C_LIGHT_RED			0x0C
#define C_LIGHT_MAGENTA		0x0D
#define C_LIGHT_BROWN		0x0E
#define C_WHITE				0x0F

#define VIDMEM_BUFFER 		0xb8000

typedef struct virtual_console
{
	char *console_buffer;
	char *old_console_buffer;
	int x, y, bx, by;
	
	byte visible;
} virtual_console_t;
#define CONSOLE_BUFFER_SIZE (24*160)
#define CONSOLE_BUFFER_START 81

//scroll console window
void scroll();
//move cursor position
void move_cursor();
void set_cursor(int x, int y);
int get_y();
int get_x();
//clear screen
void clear_screen();

//to control character input : backspace
void set_backspace_point();
void remove_backspace_point();

//set color
void ksetcolor(byte b, byte f);
void ksetforeground(byte f);
void ksetbackground(byte b);
void ksetdefaultcolor();
void kchangedefaultcolor(byte b, byte f);

void set_console(virtual_console_t *console, uint takeover);

void kset_status(char *str);

//print functions
void kprint(char *s, ...);
void kputs(char *s);
void kputc(char c);

void init_video_console();

#endif
