#ifndef PORTS_H
#define PORTS_H

/*
typedef byte unsigned char;
typedef unsigned int uint32;
typedef unsigned short uint16;*/

void memcpy(unsigned char *dest, unsigned char *source, int len);
void memset(unsigned char *dest, unsigned char value, int len);

void outb(unsigned short port, unsigned char value);
unsigned char inb(unsigned short port);

void outw(unsigned short port, unsigned short value);
unsigned short inw(unsigned short port);

unsigned long inl(unsigned short port);
void outl(unsigned short port, unsigned long value);

void insl(int port, void *addr, int cnt);
void outsl(int port, const void *addr, int cnt);

void insw(unsigned short port, void *addr, int cnt);

#endif
