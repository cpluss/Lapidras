#include "apic.h"
#include "console.h"
#include "memory.h"

static byte sum(uint loc, uint len)
{
	byte i, sum, *addr;
	addr = (byte*)loc;
	for(i = 0; i < len; i++)
		sum += addr[i];
	return sum;
}
static mp_t *mp_search(uint loc, uint len)
{
	byte *e, *p, *addr;
	
	addr = (byte*)loc;
	e = (byte*)(loc + len);
	for(p = addr; p < e; p += sizeof(mp_t))
	{
		if(memcmp(p, "_MP_", 4) == 0)
		{
			//Sum it all up, if it equals to zero, we're set to go!
			byte i, sum = 0;
			for(i = 0; i < sizeof(mp_t); i++)
				sum += p[i];
			if(sum != 0)
				continue;
				
			return (mp_t*)p;
		}
	}
	return 0;
}

static mp_t *get_apic_mp()
{
    byte *bda = (byte*)0x400;
    uint p = ((bda[0x0F]<<8)| bda[0x0E]) << 4;
    mp_t *mp;
    if(mp = mp_search(p, 1024))
		return mp;
	else
	{
		p = ((bda[0x14] << 8) | bda[0x13]) * 1024;
		if(mp = mp_search(p - 1024, 1024))
			return mp;
	}
	
	return mp_search(0xF0000, 0x10000);
}

void apic_init2()
{
	mp_t *mp = get_apic_mp();
	if(mp == 0)
	{
		kprint("Could not find apic structure!\n");
		for(;;);
	}
	
	mp_conf_t *mp_conf = (mp_conf_t*)mp->physaddr;
	if(memcmp(mp_conf->signature, "PCMP", 4) != 0)
	{
		kprint("Could not initialize the apic configuration structure!\n");
		for(;;);
	}
	
	mp_entry_t *e;
	int i, offset = sizeof(mp_conf_t);
	for(i = 0; i < mp_conf->count; i++)
	{
		e = (mp_entry_t*)((uint)mp_conf + offset);
		switch(e->type)
		{
			case APIC_PROC: //Processor entry
			{
				static int id = 0;
				mp_proc_t *p = (mp_proc_t*)(uint)e;
				if(p->enabled && !p->bootstrap)
					kprint("cpu%i found.\n", id++);
				offset += 20;
			}break;
			case APIC_IOAP: //IO APIC entry
			{
				mp_ioapic_t *a = (mp_ioapic_t*)(uint)e;
				kprint("ioapic found.\n", (uint)a->physaddr, a->flags);
				offset += 8;
			}break;
			default:
				offset += 8;
				continue;
		}
	}
}
