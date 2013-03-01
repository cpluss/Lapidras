#include "console.h"
#include "debug.h"
#include "args.h"
#include "string.h"

puts_t puts_caller;
putc_t putc_caller;
clear_t clear_caller;

void console_early_init()
{
    puts_caller = NULL;
    putc_caller = NULL;
    clear_caller = NULL;
    
    debug_init();
}

void register_console_putc(putc_t putc)
{
    putc_caller = putc;
}
void register_console_puts(puts_t puts)
{
    puts_caller = puts;
}
void register_console_clear(clear_t clear)
{
    clear_caller = clear;
}

void cclear()
{
    if(clear_caller != NULL)
        clear_caller();
}
void cputc(char c)
{
    if(putc_caller != NULL)
        putc_caller(c);
    else
        *(byte*)0xb8080 = '@';
}
void cputs(const char *s)
{
    if(puts_caller != NULL)
        puts_caller(s);
}

void cprint(const char *s, ...)
{
    int i;
    
    va_list ap;
    va_start(ap, s);
    
    for(i = 0; i < strlen(s); i++)
    {
        if(s[i] != '%')
        {
            cputc(s[i]);
            continue;
        }
        
        i++;
        char t = s[i];
        switch(t)
        {
            case 'c':
            {
                char c = va_arg(ap, int);
                cputc(c);
            }break;
            case 's':
            {
                char *s = va_arg(ap, char*);
                cputs(s);
            }break;
            case 'i':
            {
                int num = va_arg(ap, int);
                if(num == 0)
                {
                    cputc('0');
                    continue;
                }
                
                char c[32];
                int j = 0;
                while(num > 0)
                {
                    c[j] = '0' + (num % 10);
                    num /= 10;
                    j++;
                }
                c[j] = 0;
                
                while(j > 0)
                    cputc(c[--j]);
                
            }break;
            case 'x':
            {
                int num = va_arg(ap, int);
                const char *hexarray = "0123456789ABCDEF";
                
                int j;
                for(j = 28; j > -1; j -= 4)
                    cputc(hexarray[(num >> j) & 0xF]);
            }break;  
        }
    }
}


