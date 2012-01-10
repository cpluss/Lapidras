#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H
#include "gdt.h"
#include "idt.h"

static void init_descriptors()
{
    init_gdt();
    init_idt();
}

#endif
