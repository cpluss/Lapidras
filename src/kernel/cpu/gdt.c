#include "gdt.h"

//assembler function to replace current gdt
extern void gdt_flush(uint);
//assembler function to flush the tss
extern void tss_flush();

//internal function prototype
static void gdt_set_gate(int, uint, uint, byte, byte);
static void write_tss(int, short, uint);

//internal variables, the gdt for an example
gdt_entry_t gdt_entries[6];
gdt_ptr_t gdt_ptr;
tss_entry_t tss_entry;

void init_gdt()
{
    //currently there is 5 gdt entries in total
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 6) - 1;
    gdt_ptr.base = (uint)&gdt_entries;
    
    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment
    write_tss(5, 0x10, 0x0);
    
    gdt_flush((uint)&gdt_ptr);
    tss_flush();
}

static void gdt_set_gate(int index, uint base, uint limit, byte access, byte granularity)
{
    gdt_entries[index].base_low = (base & 0xFFFF);
    gdt_entries[index].base_middle = (base >> 16) & 0xFF;
    gdt_entries[index].base_high = (base >> 24) & 0xFF;
    
    gdt_entries[index].limit_low = (limit & 0xFFFF);
    gdt_entries[index].granularity = (limit >> 16) & 0x0F;
    
    gdt_entries[index].granularity |= granularity & 0xF0;
    gdt_entries[index].access = access;
}
static void write_tss(int num, short ss0, uint esp0)
{
    uint base = (uint) &tss_entry;
    uint limit = base + sizeof(tss_entry);
    
    gdt_set_gate(num, base, limit, 0xE9, 0x00);
    memset(&tss_entry, 0, sizeof(tss_entry));

    tss_entry.ss0  = ss0;  // Set the kernel stack segment.
    tss_entry.esp0 = esp0; // Set the kernel stack pointer.
    
    // Here we set the cs, ss, ds, es, fs and gs entries in the TSS
    tss_entry.cs   = 0x0b;     
    tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;
}

void set_kernel_stack(uint stack)
{
	tss_entry.esp0 = stack;
}
