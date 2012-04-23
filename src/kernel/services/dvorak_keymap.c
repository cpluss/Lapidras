#include "keymap.h"

/*default keymap - dvorak*/
byte dvorak_map[128] =
{
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
	'9', '0', '-', '=', 0x08,	/* Backspace */
	0x09,			/* Tab */
	0, 0, 0, 'p',	/* 19 */
	'y', 'f', 'g', 'c', 'r', 'l', ',', '*',
	0x0D,	/* Enter key */
	0,			/* 29   - Control */
	'a', 'o', 'e', 'u', 'i', 'd', 'h', 't', 'n', 's',	/* 39 */
	'-', '\'',   0x0E,		/* Left shift */
	'\'', '.', 'q', 'j', 'k', 'x', 'b',			/* 49 */
	'm', 'w', 'v', 'z',   0x0E,				/* Right shift */
	'*',
	0x0F,	/* Alt */
	' ',	/* Space bar */
	0x10,	/* Caps lock */
	0xF1,	/* 59 - F1 key ... > */
	0xF2,   0xF3,   0xF4,   0xF5,   0xF6,   0xF7,   0xF8,   0xF9,
	0xFA,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0x0E,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0x20,	/* Left Arrow */
	0,
	0x22,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0x24,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */
};

char dvorak_alt_map[128] = 
{
		0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
	'9', '0', '-', '=', 0x08,	/* Backspace */
	0x09,			/* Tab */
	'\'', '|', ':', ']',	/* 19 */
	'$', '"', '?', '&', '<', '>', 0, 0,
	0x0D,	/* Enter key */
	0,			/* 29   - Control */
	';', '/', '(', ')', '|', '#', '^', '#', '"', '~',	/* 39 */
	'`', '*',   0x0E,		/* Left shift */
	'\'', ':', '=', '@', '!', '\\', '%',			/* 49 */
	'`', 0, 0, 0,   0x0E,				/* Right shift */
	'*',
	0x0F,	/* Alt */
	' ',	/* Space bar */
	0x10,	/* Caps lock */
	0xF1,	/* 59 - F1 key ... > */
	0xF2,   0xF3,   0xF4,   0xF5,   0xF6,   0xF7,   0xF8,   0xF9,
	0xFA,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0x0E,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0x20,	/* Left Arrow */
	0,
	0x22,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0x24,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */
};

byte dvorak_shft_map[128] =
{
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
	'9', '0', '-', '=', 0x08,	/* Backspace */
	0x09,			/* Tab */
	0, 0, 0, 'P',	/* 19 */
	'Y', 'F', 'G', 'C', 'R', 'L', '^', '*',
	0x0D,	/* Enter key */
	0,			/* 29   - Control */
	'A', 'O', 'E', 'U', 'I', 'D', 'H', 'T', 'N', 'S',	/* 39 */
	'-', '\'',   0x0E,		/* Left shift */
	'>', 0, 'Q', 'J', 'K', 'X', 'B',			/* 49 */
	'M', 'W', 'V', 'Z',   0x0E,				/* Right shift */
	'*',
	0x0F,	/* Alt */
	' ',	/* Space bar */
	0x10,	/* Caps lock */
	0xF1,	/* 59 - F1 key ... > */
	0xF2,   0xF3,   0xF4,   0xF5,   0xF6,   0xF7,   0xF8,   0xF9,
	0xFA,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0x0E,	/* Up Arrow */
	0,	/* Page Up */
	'_',
	0x20,	/* Left Arrow */
	0,
	0x22,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0x24,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */
};

keymap_t dvorak_keymap = { dvorak_map, dvorak_alt_map, dvorak_shft_map, dvorak_map };
