#include "shared_memory.h"
#include "paging.h"

shared_memory_pool_t *smp;
extern page_directory_t *kernel_directory;

void init_shared_memory(int regions, byte size)
{
	//Request the actual pages first hand..
	int i;
	for(i = SHARED_MEMORY_START; i < (SHARED_MEMORY_START + (regions * (size + sizeof(memory_region_t)))); i += 0x1000)
		alloc_frame(get_page(i, 1, kernel_directory), 0, 1); //kernel-mode, rw
	
	//Initialize the pool
	smp = (shared_memory_pool_t*)kmalloc(sizeof(shared_memory_pool_t));
	memset(smp, 0, sizeof(shared_memory_pool_t));
	
	smp->region_count = regions;
	smp->region_size = size;
	smp->memory_total_size = regions * size;
	
	smp->occupied_region_count = 0;
	smp->free_region_count = regions;
	
	//Create the linked list
	int offset;
	memory_region_t *prev;
	for(i = 0; i < regions; i++)
	{
		memory_region_t *region = (memory_region_t*)(SHARED_MEMORY_START + offset);
		offset += sizeof(memory_region_t) + size;
		
		region->shared = 1;
		region->locked = 0;
		region->rw = 1;
		region->occupied = 0;
		region->reserved = 0;
		region->recently_written = 0;
		
		region->id = i;
		region->size = size;
		
		region->start = (uint)((uint)region + sizeof(memory_region_t));
		
		if(prev != 0)
			prev->next = region;
		else
			smp->first = region;
		prev = region;
	}
}

memory_region_t *reserve_shared_memory()
{
	if(smp->free_region_count <= 0)
		return 0;
	//Just search for a non-occupied memory region and return it
	memory_region_t *ret = (memory_region_t*)smp->first;
	while(ret != 0 && ret->occupied == 1 && ret->locked == 0)
		ret = (memory_region_t*)ret->next;
		
	ret->occupied = 1;
	smp->occupied_region_count++;
	smp->free_region_count--;
	
	return ret;
}

void free_shared_memory(memory_region_t *region)
{
	if(region->occupied == 0)
		return;
	if(region->locked)
		region->locked = 0;
	region->occupied = 0;
	
	smp->occupied_region_count--;
	smp->free_region_count++;
}

void lock_shared_memory(memory_region_t *region)
{
	region->locked = 1;
}
void unlock_shared_memory(memory_region_t *region)
{
	region->locked = 0;
	region->recently_written = 1;
}
