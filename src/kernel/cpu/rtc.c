#include "ports.h"
#include "isr.h"
#include "cio.h"
#include "version.h"

int time = 0, t_tick = 0;
extern virtual_console_t *current_visible_console;

static void rtc_int(registers_t *regs)
{
	t_tick++;
	
	//The RTC get called each 1000ms by default.
	if(time <= 0 && current_visible_console != 0)
	{
		//poll the bsy bit
		byte status = 0x80;
		while(status & 0x80)
		{
			outb(0x70, 10);
			status = inb(0x71);
		}
		
		//get the current second
		outb(0x70, 0);
		byte sec_hex = inb(0x71);
		byte sec = ((sec_hex & 0xF0) >> 1) + ((sec_hex & 0xF0) >> 3) + (sec_hex & 0xf); //optimised hex formula found on the net
		
		//get the minute
		outb(0x70, 2);
		byte min_hex = inb(0x71);
		byte min = ((min_hex & 0xF0) >> 1) + ((min_hex & 0xF0) >> 3) + (min_hex & 0xf);
		
		//get the hour
		outb(0x70, 4);
		byte hour_hex = inb(0x71);
		byte hour = ((hour_hex & 0xF0) >> 1) + ((hour_hex & 0xF0) >> 3) + (hour_hex & 0xf);
		
		//save x and y value
		int x = get_x(), y = get_y();
		ksetforeground(C_WHITE);
		
		//Lapidras version
		set_cursor(0, 0);
		kprint_v(current_visible_console, "Lapidras v%s", VERSION);
		
		//Clock position
		set_cursor(72, 0);
		kprint_v(current_visible_console, "%s%i:%s%i:%s%i", (hour < 10) ? "0" : "", hour, (min < 10) ? "0" : "", min, (sec < 10) ? "0" : "", sec);
		
		//reset the cursor position and color
		ksetdefaultcolor();
		set_cursor(x, y);
		move_cursor();
		
		time = 500; //reset the counter
	}
	time--;
	
	//vital code, to make user the interrupt is called upon again
	outb(0x70, 0x0C);
	inb(0x71);
}

void wait(uint ms)
{
	uint old = t_tick;
	while(t_tick < (old + ms));
}

void init_rtc()
{
	asm volatile("cli"); // we can not be disturbed!
	//register the rtc irq to irq8
	register_interrupt_handler(IRQ8, &rtc_int);
	
	//enable the rtc
	outb(0x70, 0x0B); //0x0B - register B ( regB )
	char prev = inb(0x71); //read current value of regB
	outb(0x70, 0x0B); //index got reset last read operation.
	outb(0x71, prev | 0x40); //or the value with 0x40 ( 100b )
}
