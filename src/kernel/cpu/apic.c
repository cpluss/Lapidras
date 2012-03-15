#include "apic.h"
#include "system.h"
#include "cio.h"
#include "memory.h"

extern volatile uint *lapic;
extern volatile uint *ioapic;
uint ncpu;

static byte sum(byte *addr, uint len)
{
	byte i, sum = 0;
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
		if(memcmp(p, "_MP_", 4) == 0 && sum(p, sizeof(mp_t)) == 0)
		{
			//Sum it all up, if it equals to zero, we're set to go!
			/*byte i, sum = 0;
			for(i = 0; i < sizeof(mp_t); i++)
				sum += p[i];
			if(sum != 0)
				continue;*/
				
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

void apic_init()
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
	if(sum((byte*)mp_conf, sizeof(mp_conf)) != mp_conf->et_checksum && mp->type == 0)
	{
		kprint("Invalid checksum on mp configuration structure (suppose to be zero, but is %i instead.)!\n", sum((byte*)mp_conf, sizeof(mp_conf)));
		for(;;);
	}
	lapic = mp_conf->lapic;
	
	mp_entry_t *e;
	uint ismp = 1;
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
				{
					if(ncpu != p->id)
					{
						kprint("apic_init - ncpu: %i, id: %i\n", ncpu, p->id);
						ismp = 0;
					}
					
					ncpu++;
				}
				offset += 20;
			}break;
			case APIC_IOAP: //IO APIC entry
			{
				mp_ioapic_t *a = (mp_ioapic_t*)(uint)e;
				if(mp->type != 0) //Default configuration
					ioapic = (uint*)DEFAULT_IOAPIC;
				else
					ioapic = (uint*)a->physaddr;
				offset += 8;
			}break;
			default:
				offset += 8;
				continue;
		}
	}
	
	if(!ismp) //uniprocessor ..
	{
		ncpu = 1;
		lapic = 0;
		ioapic = 0;
		return;
	}
	if(mp->imcrp)
	{
		outb(0x22, 0x70); //Select IMCR
		outb(0x23, inb(0x23) | 1); //Mask external interrupts
	}
}
