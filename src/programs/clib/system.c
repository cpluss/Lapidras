#include "system.h"
#include "string.h"

void puts(char *buffer)
{
	int ret;
    asm volatile("int $112" : "=a"(ret) : "0"(0), "b"((int)buffer));
}
void putc(char c)
{
	int ret;
    asm volatile("int $112" : "=a"(ret) : "0"(1), "b"((int)c));
}
void gets(char *buffer)
{
	int ret;
    asm volatile("int $112" : "=a"(ret) : "0"(2), "b"((int)buffer));
}

int GetPID()
{
    int ret;
    asm volatile("int $112" : "=a"(ret) : "0"(3));
    return ret;
}

int fopen(const char *name)
{
    int ret;
    asm volatile("int $112" : "=a"(ret) : "0"(4), "b"((int)((char*)name)));
    return ret;
}
void fclose(int n)
{
	int ret;
    asm volatile("int $112" : "=a"(ret) : "0"(5), "b"(n));
}
int fread(byte *buffer, uint size, uint n, int handle)
{
    int ret;
    asm volatile("int $112" : "=a"(ret) : "0"(6), "b"((int)buffer), "c"(size), "d"(n), "S"(handle));
    return ret;
}
//fwrite - 7
int ftell_size(int handle)
{
    int ret;
    asm volatile("int $112" : "=a"(ret) : "0"(8), "b"(handle));
    return ret;
}

void *malloc(int sz)
{
    int ret;
    asm volatile("int $112" : "=a"(ret) : "0"(9), "b"(sz));
    return (void*)ret;
}
void free(void *p)
{
	int ret;
    asm volatile("int $112" : "=a"(ret) : "0"(10), "b"((int)p));
}

int gettick()
{
    int ret;
    asm volatile("int $112" : "=a"(ret) : "0"(11));
    return ret;
}

void exit()
{
	int ret;
    asm volatile("int $112" : "=a"(ret) : "0"(12));
}

void execve(const char *path)
{
    int ret;
    asm volatile("int $112" : "=a"(ret) : "0"(15), "b"((int)path));
}


uint GetThread(const char *name)
{
    int ret;
    asm volatile("int $112" : "=a"(ret) : "0"(16), "b"((int)name));
    return ret;
}

void printf(char *s, ...)
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
					char c = va_arg(ap, int);
					putc(c);
				}
				case 's':
				{
					char *str = va_arg(ap, char*);
					for(j = 0; j < strlen(str); j++)
						putc(str[j]);
				}break;
				case 'i':
				{
					int num = va_arg(ap, int);
					if(num == 0)
						putc('0');
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
							putc(c[j--]);
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
						putc(hexarray[(num >> j) & 0xF]);
					}
				}break;
			}
		}
		else
			putc(s[i]);
	}
	
	va_end(ap);
}

