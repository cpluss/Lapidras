#include "types.h"
#include "memory.h"
#include "vmm.h"

uint *frames;
uint nframes;

#define INDEX_FROM_BIT(a) (a / 32)
#define OFFSET_FROM_BIT(a) (a % 32)

static inline void set_pageframe(uint addr)
{
	frames[INDEX_FROM_BIT(addr / 0x1000)] |= (0x1 << OFFSET_FROM_BIT(addr / 0x1000));
}
static inline void clear_pageframe(uint addr)
{
	frames[INDEX_FROM_BIT(addr / 0x1000)] &= ~(0x1 << OFFSET_FROM_BIT(addr / 0x1000));
}
static byte test_pageframe(uint addr)
{
	return (frames[INDEX_FROM_BIT(addr / 0x1000)] & (0x1 << OFFSET_FROM_BIT(addr / 0x1000)));
}
static uint first_pageframe()
{
	uint i, j;
	for(i = 0; i < INDEX_FROM_BIT(nframes); i++)
	{
		if(frames[i] == 0xFFFFFFFF)
			continue;
		for(j = 0; j < 32; j++)
		{
			if(!(frames[i] & (0x1 << j)))
				return i * 32 + j; //return the offset index
		}
	}
}

void pmm_init(uint mem)
{
	nframes = mem / 0x1000;
	frames = (uint*)kmalloc(INDEX_FROM_BIT(nframes));
	memset(frames, 0, INDEX_FROM_BIT(nframes));
}

void pmm_alloc_page(uint *page)
{
	if((*page & PDE_MASK) != 0)
		return;
	
	uint index = first_pageframe();
	if(index == (uint)-1) //no free frames
		PANIC("Could not find any free frames.\n");
	
	set_pageframe(index * 0x1000);
	*page |= (index << 12) & PDE_MASK;
}
void pmm_map_page(uint *page, uint addr)
{
	if((*page & PDE_MASK) != 0)
		return;
	
	set_pageframe(addr);
	*page |= (INDEX_FROM_BIT(addr) << 12) & PDE_MASK;
}
void pmm_free_page(uint *page)
{
	if((*page & PDE_MASK) == 0)
		return;
	
	clear_pageframe((*page & PDE_MASK) >> 12);
	*page &= ~(*page & PDE_MASK);
}
