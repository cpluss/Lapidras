#include "system.h"
#include "string.h"

int kputs(char *s)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(0), "b"((int)s));
	return a;
}
int kputc(char c)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(1), "b"((int)c));
}

int SendSignal(signal_t *s)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(2), "b"((int)s));
	return a;
}
signal_t *LatestSignal()
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(5));
	return (signal_t*)a;
}
void WaitForSignal()
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(17));
}

int GetPID()
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(3));
	return a;
}

thread_t *GetThreadByName(const char *name)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(4), "b"((int)name));
	return (thread_t*)a;
}
thread_t *GetThread(uint id)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(19), "b"(id));
	return (thread_t*)a;
}

thread_t *CreateThread(const char *name, void (*thread)(), uint priority, uint idle)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(5), "b"((int)name), "c"((int)thread), "d"(priority), "S"(idle));
	return (thread_t*)a;
}

thread_t *CurrentThread()
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(6));
	return (thread_t*)a;
}

int QueueIsEmpty(thread_t *th)
{
    int a;
    asm volatile("int $112" : "=a"(a) : "0"(22), "b"((int)th));
    return a;
}

int fopen(const char *path)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(7), "b"((int)path));
	return a;
}
void fclose(int handle)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(8), "b"(handle));
}

int fread(byte *buffer, uint size, uint n, int handle)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(9), "b"((int)buffer), "c"((int)size), "d"((int)n), "S"(handle));
	return a;
}
int fwrite(byte *buffer, uint size, uint n, int handle)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(10), "b"((int)buffer), "c"(size), "d"(n), "S"(handle));
	return a;
}
int ftell_size(int handle)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(11), "b"(handle));
	return a;
}

void *kmalloc(int sz)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(12), "b"(sz));
	return (void*)a;
}
void kfree(void *memory)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(13), "b"((int)memory));
}

void kprint(char *s, ...)
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
					kputc(c);
				}
				case 's':
				{
					char *str = va_arg(ap, char*);
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

void register_event(ushort type, evt_t handler)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(15), "b"((int)type), "c"((int)handler));
}
void unregister_event(ushort type, evt_t handler)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(16), "b"((int)type), "c"((int)handler));
}

char *buf;
void kbd_handle_read(void *param)
{
	char c = *(char*)param;
	kprint("Character '%c' read.\n", c);
}

void kread(char *buffer)
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(20), "b"((int)buffer));
}

void exit()
{
	int a;
	asm volatile("int $112" : "=a"(a) : "0"(21));
}
