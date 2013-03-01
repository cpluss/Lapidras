#include "console.h"
#include "isr.h"

const char *exception_list[] = {
      "Division by zero",
      "Debug",
      "Non maskable interrupt",
      "Breakpoint",
      "Into detected overflow",
      "Out of bounds",
      "Invalid opcode",
      "No coprocessor",
      "Double fault",
      "Coprocessor segment overrun",
      "Bad TSS",
      "Segment not present",
      "Stack fault",
      "General protection fault",
      "Page fault",
      "Unknown interrupt exception",
      "Coprocessor fault",
      "Alignement check",
      "Machine check",
      "RESERVED",
      "RESERVED",
      "RESERVED",
      "RESERVED",
      "RESERVED",
      "RESERVED",
      "RESERVED",
      "RESERVED",
      "RESERVED",
      "RESERVED",
      "RESERVED",
      "RESERVED"
};

isr_t interrupt_handlers[256];
void register_interrupt_handler(byte n, isr_t handler)
{
    interrupt_handlers[n] = handler;
    //ioapicenable(n - IRQ0, current_cpu()->apicid);
}
void register_irq_handler(byte n, isr_t handler)
{
	interrupt_handlers[IRQ0 + n] = handler;
	//ioapicenable(n, current_cpu()->apicid);
}

void isr_handler(registers_t regs)
{ 
    if(interrupt_handlers[regs.int_no] != 0)
    {
		isr_t handler = interrupt_handlers[regs.int_no];	
        handler(&regs);
		return;
    }
    
    if(regs.err_code)
		cprint("%s exception caught: 0x%x ( error code )\n", exception_list[regs.int_no], regs.err_code);
    else
		cprint("%s exception caught:\n", exception_list[regs.int_no]);
    print_registers(regs);
    asm volatile("cli");
    asm volatile("hlt");
}

void irq_handler(registers_t regs)
{
    //send eoi to signal the PICs (eoi = end of interrupt)
    if(regs.int_no >= 40)
		outb(0xA0, 0x20); //send signal to slave
    outb(0x20, 0x20); //send signal to master
    
    if(interrupt_handlers[regs.int_no] != 0)
    {
		isr_t handler = interrupt_handlers[regs.int_no];
		handler(&regs);
    }
}

void print_registers(registers_t regs)
{
    //print all registers within regs to the user, using kprint
    //first off is the e** registers, ( esi, edi, ecx etc.. )
    cprint("EDI: 0x%x \t ESI: 0x%x\n", regs.edi, regs.esi);
    cprint("EBP: 0x%x \t ESP: 0x%x\n", regs.ebp, regs.esp);
    cprint("EBX: 0x%x \t EDX: 0x%x\n", regs.ebx, regs.edx);
    cprint("ECX: 0x%x \t EAX: 0x%x\n", regs.ecx, regs.eax);
    cprint("EIP: 0x%x\n", regs.eip);
    //next up is the ds, cs, and ss registers + eflags
    cprint("DS: 0x%x \t\t CS: 0x%x\n", regs.ds, regs.cs);
    cprint("SS: 0x%x \t\t EFLAGS: 0x%x\n", regs.ss, regs.eflags);
}
