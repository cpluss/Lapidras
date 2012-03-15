#ifndef APIC_H
#define APIC_H
#include "types.h"

typedef struct mp
{
	byte signature[4];		//_MP_
	void *physaddr;
	byte length;
	byte specrev;
	byte checksum;			//All bytes must add up to 0
	byte type;				//MP system config type
	byte imcrp;
	byte reserved[3];
} __attribute__((packed)) mp_t;

typedef struct mp_conf
{
	byte signature[4];		//PCMP
	ushort length;
	byte specrev;
	byte checksum;
	byte oem[8];
	byte product[12];
	void *oemaddr;
	ushort oem_length;
	ushort count;
	void *lapic;
	ushort et_length;
	byte et_checksum;
} __attribute__((packed)) mp_conf_t;

#define APIC_PROC 0
#define APIC_IOAP 2
#define APIC_BUS  1
#define APIC_IOIN 3
#define APIC_LINT 4
typedef struct mp_entry
{
	byte type;
} __attribute__((packed)) mp_entry_t;

typedef struct mp_proc
{
	byte type;
	byte id;
	byte specrev;
	byte enabled : 1;
	byte bootstrap : 1;
	byte reserved : 6;
	uint signature;
	uint features;
} __attribute__((packed)) mp_proc_t;
typedef struct mp_ioapic
{
	byte type;
	byte id;
	byte specrev;
	byte flags;
	void *physaddr;
} __attribute__((packed)) mp_ioapic_t;

//Initialize the apic, and each cpu with it
void apic_init();

#endif
