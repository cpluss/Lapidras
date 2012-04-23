#include "keymap.h"

char qwerty_map[128] =
{
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
	'9', '0', '-', '=', '\b',	/* Backspace */
	'\t',			/* Tab */
	'q', 'w', 'e', 'r',	/* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0x0d,	/* Enter key */
	0,			/* 29   - Control */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
	'\'', '`',   0x0E,		/* Left shift */
	'\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
	'm', ',', '.', '/',   0x0E,				/* Right shift */
	'*',
	0x0F,	/* Alt */
	' ',	/* Space bar */
	0,	/* Caps lock */
	0xF1,	/* 59 - F1 key ... > */
	0xF2,   0xF3,   0xF4,   0xF5,   0xF6,   0xF7,   0xF8,   0xF9,
	0xFA,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0,	/* Left Arrow */
	0,
	0,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0xFB,	/* F11 Key */
	0xFC,	/* F12 Key */
	0,	/* All other keys are undefined */
};
char qwerty_shft_map[128] =
{
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
	'9', '0', '-', '=', '\b',	/* Backspace */
	'\t',			/* Tab */
	'Q', 'W', 'E', 'R',	/* 19 */
	'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 0x0d,	/* Enter key */
	0,			/* 29   - Control */
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',	/* 39 */
	'\'', '`',   0x0E,		/* Left shift */
	'\\', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
	'M', ';', ':', '/',   0x0E,				/* Right shift */
	'*',
	0x0F,	/* Alt */
	' ',	/* Space bar */
	0,	/* Caps lock */
	0xF1,	/* 59 - F1 key ... > */
	0xF2,   0xF3,   0xF4,   0xF5,   0xF6,   0xF7,   0xF8,   0xF9,
	0xFA,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0,	/* Left Arrow */
	0,
	0,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0xFB,	/* F11 Key */
	0xFC,	/* F12 Key */
	0,	/* All other keys are undefined */
};

keymap_t qwerty_keymap = { qwerty_map, qwerty_map, qwerty_shft_map, qwerty_map };
