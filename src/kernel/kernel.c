#include "system.h"

uint memory_size;
int kmain(multiboot_t *mb, uint esp)
{
    // Initialize the debug console
	console_early_init();
    cclear();
    
    uint memory_size = (mb->mem_lower + mb->mem_upper) * 1000;
    cprint("Detected %ikB of RAM.\n", memory_size/1000);
    
    // Enable interrupts
    asm volatile("sti");
    
    // Initialize gdt, idt (global descriptor table, interrupt descriptor table)
    init_gdt();
    init_idt();
    
    // Initialize memory
    pmm_init(memory_size); // Physical memory manager
    vmm_init();            // Virtual memory manager
    mem_init();            // Overall memory manager
    
    // Initialize the rtc (real time clock)
    rtc_init();
    
    for(;;);
    return 0;
}
