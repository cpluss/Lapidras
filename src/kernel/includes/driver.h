#ifndef DRIVER_H
#define DRIVER_H
#include "types.h"

#define DRIVER_TYPE_FILESYSTEM      0x00
#define DRIVER_TYPE_SERVICE         0x01
#define DRIVER_TYPE_NETWORK         0x02

typedef struct driver
{
    char name[32];
    int type;
} driver_t;

#endif
