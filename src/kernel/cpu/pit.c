#include "isr.h"

uint tick = 0;
int gettick()
{
	return tick;
}

static void pit_callback(registers_t *regs)
{
	tick++;
	schedule();
}

void init_pit(uint frequency)
{
    //register our callback
    register_interrupt_handler(IRQ0, &pit_callback);
    
    //we have to divide the frequency with the pit's input clock
    //to get the desired frequency we're after
    uint divisor = 1193180 / frequency;
    
    //send command byte
    outb(0x43, 0x36);
    
    //send it byte-wise
    //low bits
    byte low = (byte)(divisor & 0xFF);
    byte high = (byte)((divisor >> 8) & 0xFF);
    
    //send frequency divisor
    outb(0x40, low);
    outb(0x40, high);
}
