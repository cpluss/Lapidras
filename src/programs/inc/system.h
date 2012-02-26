#ifndef SYSCALL_DEFN_H
#define SYSCALL_DEFN_H
//from types.h ( kernel source )
typedef unsigned char byte;
typedef unsigned int  uint;
typedef unsigned short ushort;
typedef unsigned long ulong;

#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)
#define va_copy(d,s)  __builtin_va_copy(d,s)
typedef __builtin_va_list va_list;

extern void printf(char *s, ...);
extern void puts(char *s);
extern void putc(char c);
extern void gets(char *s);

extern int GetPID();

extern int fopen(const char *name);
extern void fclose(int handle);
extern int fread(byte *buffer, uint sz, uint n, int handle);
extern int ftell_size(int handle);

extern void *malloc(int sz);
extern void free(void *p);

extern int gettick();

extern void exit();
extern void exec(const char *path);
extern void execve(const char *path);
extern uint GetThread(const char *name);

#endif
