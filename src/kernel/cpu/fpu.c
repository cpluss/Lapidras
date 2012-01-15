#include "system.h"

void set_fpu_cw(const ushort cw)
{
    asm volatile("fldcw %0" :: "m"(cw));
}

void enable_fpu()
{
    //Don't bother checking, all new i386 (686) machines got an FPU
    uint cr4;
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= 0x200;
    asm volatile("mov %0, %%cr4" :: "r"(cr4));
    //The value written by F(N)INIT
    //set_fpu_cw(0x37F);
    //0x37E, invalid operand exceptions enabled
    //set_fpu_cw(0x37E);
    //0x37A, both division by zero and ^ exceptions
    set_fpu_cw(0x37A);
}
