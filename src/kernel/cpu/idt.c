#include "idt.h"

//assembler function to replace the idt pointer
extern void idt_flush(uint);

//function prototype to set a specific idt gate
static void idt_set_gate(byte, uint, short, byte);
//function prototype to remap the irq calls
static void remap_irq();

idt_entry_t idt_entries[256];
idt_ptr_t idt_ptr;

void init_idt()
{
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base = (uint)&idt_entries;
    
    //nullify all idt entries, so we won't triple fault
    memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);
    
    //remap the irq gates
    remap_irq();
    
    idt_set_gate(0, (uint)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint)isr31, 0x08, 0x8E);
    
    idt_set_gate(32, (uint)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint)irq15, 0x08, 0x8E);
    
    idt_set_gate(112, (uint)isr112, 0x08, 0x8E);
    
    idt_flush((uint)&idt_ptr);
}

static void idt_set_gate(byte index, uint base, short sel, byte flags)
{
    idt_entries[index].base_low = base & 0xFFFF;
    idt_entries[index].base_high = (base >> 16) & 0xFFFF;
    
    idt_entries[index].sel = sel;
    idt_entries[index].always_zero = 0;
    
    idt_entries[index].flags = flags; //or this bi 0x60 to enable user mode
}

static void remap_irq()
{
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x00);
	outb(0xA1, 0x00);
}
