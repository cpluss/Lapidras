#include "debug.h"
#include "args.h"
#include "ports.h"
#include "string.h"
#include "console.h"

//Video buffer start in memory
#define VIDMEM 0xb8000
//Current cursor settings
int x, y;

static inline void scroll()
{
    if(y < 25) //At the bottom?
        return;
    
    memcpy((byte*)VIDMEM, (byte*)(VIDMEM + 160), 24*160);
    x = 0;
    y = 24;
}
static inline void move_cursor()
{
    ushort loc = (y * 80 + x);
    outb(0x3D4, 14); //setting high cursor byte
    outb(0x3D5, (loc >> 8));
    outb(0x3D4, 15); //setting the low cursor byte
    outb(0x3D5, loc);
}

static void debug_putc(char c)
{
    switch(c)
    {
        case 0x08: //Backspace
            x--;
            if(x < 0)
            {
                x = 79;
                y -= (y == 0) ? 0 : 1;
            }
            break;
        case 0x09: //Tab
            x = (x + 8) & ~7;
            break;
        case '\r': //Carriage return
            x = 0;
            break;
        case '\n': //Newline
            x = 0;
            y++;
            break;
            
        default: //Print it out otherwise
            {
                if(c < ' ')
                    return;
                
                *(ushort*)(VIDMEM + (x << 1) + 160*y) = (c | (0x07 << 8));
                x++;
            
                if(x >= 80) //newline again
                {
                    y++;
                    x = 0;
                }
            }
    }
    
    scroll();
    move_cursor();
}

static void debug_puts(const char *s)
{
    int i;
    for(i = 0; i < strlen(s); i++)
        debug_putc(s[i]);
}

static void debug_clear()
{
    for(y = 0; y < 25; y++)
        for(x = 0; x < 80; x++)
            *(ushort*)(VIDMEM + (x << 1) + 160*y) = (0x20 | (0x07 << 8));
    
    x = 0;
    y = 0;
}

void debug_init()
{
    x = 0, y = 0;
    
    register_console_putc(&debug_putc);
    register_console_puts(&debug_puts);
    register_console_clear(&debug_clear);
}








