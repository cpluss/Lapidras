#ifndef TYPES_H
#define TYPES_H

typedef unsigned char byte;
typedef unsigned int  uint;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned long long uint64_t;

#include "console.h"
#define PANIC(msg) cprint("PANIC at %s:%s - %s", __FILE__, __LINE__, msg);

#endif
