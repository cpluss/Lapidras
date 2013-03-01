#include "ports.h"

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

unsigned int xchg(volatile unsigned int *addr, unsigned int newval)
{
  unsigned int result;
  asm volatile("lock; xchgl %0, %1" :
               "+m" (*addr), "=a" (result) :
               "1" (newval) :
               "cc");
  return result;
}
