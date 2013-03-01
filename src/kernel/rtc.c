#include "isr.h"

volatile uint tick = 0, freq;

//RTC - Real Time Clock
static void rtc_tick(registers_t *regs)
{
	tick++;
	
	outb(0x70, 0x0C);
	inb(0x71);
}

void delayms(int ms)
{
	if(tick != 0)
	{
		int ct = tick;
		while(tick < (ct + ms));
	}
	else
	{
		//wait tick .. tick .. tick ..
		
	}	
}

void rtc_init()
{
	freq = 1000; //The default frequency of the RTC is each millisecond.
	tick = 1;
	
	register_irq_handler(8, &rtc_tick);
	
	//enable the rtc
	outb(0x70, 0x0B); //0x0B - register B ( regB )
	char prev = inb(0x71); //read current value of regB
	outb(0x70, 0x0B); //index got reset last read operation.
	outb(0x71, prev | 0x40); //or the value with 0x40 ( 100b )
}
