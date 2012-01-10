#ifndef MEMORY_H
#define MEMORY_H
#include "types.h"

#define KHEAP_START 	0xC0000000
#define KHEAP_SIZE  	0x0FFFFFFF //16 MB
#define KERNEL_STACK	0xE0000000

struct heap;

typedef struct block_header
{
	//size, where do we start?
	uint start, size, magic;
	//process owner - not used / reserved
	byte owner, used;
	
	//linked list chain - flat memory model
	struct block_header *next, *prev;
	
	//pointer to its heap
	struct heap *heap;
} block_header_t;

typedef struct block_footer
{
	//this is just to mark the end, -> point to the header
	block_header_t *header;
	uint magic;
} block_footer_t;

typedef struct heap
{
	//heap size, max_size and start address
	uint size, max_size, start_addr;
	
	byte owner, rw; //reserved
	
	//where do the chain start?
	block_header_t *start;
	//and where does it end?
	block_footer_t *end;
} heap_t;

uint kmalloc(uint size);
uint kmalloc_real(uint size, int align, uint *phys);

void *alloc(uint sz);
void *ralloc(uint sz, int align);
void free(void *p);

heap_t *create_heap(uint start, uint size);

uint get_allocated_memory();

#endif
