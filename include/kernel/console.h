#ifndef CONSOLE_H
#define CONSOLE_H
#include "types.h"

typedef void (*puts_t)(const char*);
typedef void (*putc_t)(char);
typedef void (*clear_t)();

void console_early_init();

void register_console_putc(putc_t c);
void register_console_puts(puts_t s);
void register_console_clear(clear_t c);

void cputc(char);
void cputs(const char *);
void cprint(const char *, ...);

void cclear();

#endif
