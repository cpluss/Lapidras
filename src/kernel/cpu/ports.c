#include "ports.h"

void memcpy(unsigned char *dest, unsigned char *source, int len)
{
	const unsigned char *sp = (const unsigned char *)source;
    unsigned char *dp = (unsigned char *)dest;
    for(; len != 0; len--) *dp++ = *sp++;
}
void memset(unsigned char *dest, unsigned char value, int len)
{
	unsigned char *temp = (unsigned char *)dest;
    for ( ; len != 0; len--) *temp++ = value;
}
int memcmp(unsigned char *p1, unsigned char *p2, int n)
{
	const unsigned char *s1, *s2;
  
	s1 = p1;
	s2 = p2;	
	while(n-- > 0)
	{
		if(*s1 != *s2)
		return *s1 - *s2;
		s1++, s2++;
	}

  return 0;
}

void outb(unsigned short port, unsigned char value)
{
	asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

unsigned char inb(unsigned short port)
{
	unsigned char ret;
	asm volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

void outw(unsigned short port, unsigned short value)
{
	asm volatile("outw %1, %0" : : "dN"(port), "a"(value));
}
unsigned short inw(unsigned short port)
{
	unsigned short ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

unsigned long inl(unsigned short port)
{
	unsigned long ret;
	asm volatile("inl %1, %0" : "=a"(ret) : "dN"(port));
	return ret;
}
void outl(unsigned short port, unsigned long value)
{
	asm volatile("outl %1, %0" : : "dN"(port), "a"(value));
}

void insl(int port, void *addr, int cnt)
{
	asm volatile("cld; rep insl" : "=D"(addr), "=c"(cnt) : "d"(port), "0"(addr), "1"(cnt) : "memory", "cc");
}
void outsl(int port, const void *addr, int cnt)
{
	asm volatile("cld; rep outsl" : "=S"(addr), "=c"(cnt) : "d"(port), "0"(addr), "1"(cnt) : "cc");
}

void insw(unsigned short port, void *addr, int cnt)
{
	asm volatile("rep; insw" : "+D"(addr), "+c"(cnt) : "d"(port) : "memory");
}
void outsw(unsigned short port, void *addr, int cnt)
{
	asm volatile("rep; outsw" : "+D"(addr), "+c"(cnt) : "d"(port) : "memory");
}
