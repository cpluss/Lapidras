#ifndef KEYMAP_H
#define KEYMAP_H
#include "types.h"

typedef struct keymap
{
	char *map;
	char *map_alt;
	char *map_shft;
	char *map_ctrl;
} keymap_t;

#endif
