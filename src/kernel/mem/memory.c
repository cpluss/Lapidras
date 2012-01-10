#include "memory.h"
#include "paging.h"
#include "thread.h"
#include "event.h"

extern page_directory_t *kernel_directory, *current_directory;

extern uint end;
uint placement_address = (uint)&end + 0x1000;

//placeholder to hold the memory being allocated and max memory
uint allocated_memory = 0;

heap_t *kheap = 0;

uint kmalloc_real(uint sz, int align, uint *phys)
{
    if(kheap == 0)
    {
		if(align == 1 && (placement_address & 0xFFFFF000)) //if the address isn't already page aligned
		{
			//align it
			placement_address &= 0xFFFFF000;
			placement_address += 0x1000;
		}
		if(phys)
			*phys = placement_address;
		
		uint tmp = placement_address;
		placement_address += sz;
		return tmp;
    }
    else //memory management is enabled
    {
		void *addr;
		if(align)
			addr = ralloc(sz, 1);
		else
			addr = alloc(sz);
		if(phys != 0)
		{
			page_t *page = get_page((uint)addr, 0, current_directory);
			*phys = (page->frame * 0x1000 + ((uint) addr & 0xFFF));
		}
		notify_event(EVENT_MEM_ALLOC, addr);
		return (uint)addr;
    }
}
uint kmalloc(uint sz)
{
	return kmalloc_real(sz, 0, 0);
}

static block_header_t *get_smallest_block(uint size, uint align)
{
	//fetch smallest free block.
	block_header_t *tmp = kheap->start;
	
	//loop for it
	uint search_size = 0;
	while(tmp->next)
	{
		if(tmp->next == 0)
			return;
		if(tmp->used == 1)
		{
			tmp = tmp->next;
			continue;
		}
		if(align)
		{
			//page alignation space required..
			uint location = tmp->start;
			uint offset = 0;
			if(location & 0xFFF)
				offset = 0x1000 - (location & 0xFFF);
			
			uint hole_size = tmp->size - offset;
			if(hole_size > (size + sizeof(block_header_t) + sizeof(block_footer_t)))
				break;
		}
		if(tmp->size > (size + sizeof(block_header_t) + sizeof(block_footer_t)))
			break;
		
		tmp = tmp->next;
	}
	
	//test if we're at the end.
	block_header_t *htest = kheap->end->header;
	if(htest == tmp && tmp->used == 1)
		return 0; //no space available
		
	return tmp;
}

heap_t *create_heap(uint start, uint size)
{	
	//create a new heap
	heap_t *ret = (heap_t*)start; //kmalloc(sizeof(heap_t), 0, 0);
	//clear it
	memset((byte*)ret, 0, sizeof(heap_t));
	
	//we start with one large block ( free one ) - called a hole
	block_header_t *hole = (block_header_t*)(start + sizeof(heap_t));
	hole->size = size;
	hole->magic = 0xDEADBEEF;
	hole->used = 0;
	hole->start = ((uint)hole + sizeof(block_header_t));
	
	hole->next = hole->prev = 0;
	
	block_footer_t *end_hole = (block_footer_t*)(start + size - sizeof(block_footer_t));
	end_hole->header = hole;
	end_hole->magic = 0xDEADBEEF;
	
	ret->start = hole;
	ret->end = end_hole;
	
	ret->size = size;
	ret->start_addr = start;
		
	return ret;
}

static void make_smaller(block_header_t *block, uint new_size)
{
	//move the footer towards the header and then change size.
	uint new_footer_location = (block->start + new_size);
	
	block_footer_t *new_footer = (block_footer_t*)new_footer_location;
	new_footer->header = block;
	new_footer->magic = block->magic;
	
	//insert another block in the space we just created
	uint n_start, n_end;
	n_start = ((uint)new_footer + sizeof(block_footer_t));
	
	//are we the only block?
	block_header_t *next_header = block->next;
	if(next_header != 0 && 
			((uint)next_header < (KHEAP_START + kheap->size)) &&
			((uint)next_header > KHEAP_START)) //there is another block
		n_end = (uint)next_header;
	else
	{
		//the only one here..
		n_end = (KHEAP_START + kheap->size);
		next_header = 0; //set pointer to null, to clear things up later on
	}
	
	//create a header at n_start, and a footer at n_end
	block_header_t *nblock = (block_header_t*)n_start;
	nblock->start = (n_start + sizeof(block_header_t));
	nblock->magic = block->magic;
	nblock->used = 0;
	nblock->size = (n_end - n_start);
	
	//now the footer
	block_footer_t *nfooter = (block_footer_t*)(n_end - sizeof(block_footer_t));
	nfooter->magic = nblock->magic;
	nfooter->header = nblock;
	
	//then wrap up in the linked list
	block->next = nblock;
	nblock->prev = block;
	
	if(next_header)
	{
		next_header->prev = nblock;
		nblock->next = next_header;
	}
	
	block->size = new_size;
}
static void make_bigger(block_header_t *block)
{
	//check to see for either a free block to the left or right.
	//kprint("make_bigger: ");
	//start with the right one
	if(block->next)
	{
		if(block->next->used == 0)
		{
			//any more?
			block_header_t *tmp = block->next;
			while(tmp->next)
			{
				if(tmp->next->used)
					break; //use last header
					
				tmp = tmp->next;
			}
			
			//tmp holds the last free header in the chain
			block->next = tmp->next; //alter the chain
			block->next->prev = block; //^
			
			//set the tmp footer to point at our block
			block_footer_t *footer = (block_footer_t*)((uint)tmp + tmp->size - sizeof(block_footer_t));
			
			block->size = ((uint)footer - block->start);
		}
	}
	//now proceed to the left
	if(block->prev) //there is somebody to the left
	{
		if(block->prev->used == 0)
		{
			//is there any more?
			block_header_t *tmp = block->prev;
			while(tmp->prev)
			{
				//found a used one, then use the last one
				if(tmp->prev->used)
					break;
					
				tmp = tmp->prev;
			}
			
			//tmp holds the new header
			//set the next value of tmp
			block->prev = tmp->prev;
			block->prev->next = block;
			
			uint new_size = (((uint)block + block->size) - tmp->start);
			tmp->size = new_size;
			
			//set the new header
			block = tmp;
		}
	}
	
	//kprint("\n");
}
static block_header_t *make_page_aligned(block_header_t *block)
{
    //save the previous and next blocks
    block_header_t *next = block->next;
    block_header_t *prev = block->prev;
    
    uint orig_hole_pos = (uint)block->start;
    uint orig_hole_size = (uint)block->size;
    if(orig_hole_pos & 0xFFF) //not page-aligned
    {		
		//First off transform the new block into a hole, to fill
		//the upcoming gap.
		//The new location (is really the old + (difference to 0x1000 from current point))
		uint new_start = orig_hole_pos + (0x1000 - (orig_hole_pos & 0xFFF));
		uint new_location = new_start - sizeof(block_header_t);
		
		block->size = (orig_hole_pos & 0xFFF) - sizeof(block_footer_t); //to fill the gap behind, this is the new size
		block->used = 1;												//THIS IS NOT NEAT!! :'(
		block->start = (uint)block + sizeof(block_header_t);
		//now create a new footer for it
		block_footer_t *new_hole_footer = (block_footer_t*)(new_location + block->size);
		new_hole_footer->magic = 0xDEADBEEF;
		new_hole_footer->header = block;
		
		uint new_size = orig_hole_size - (orig_hole_pos & 0xFFF);
		
		//now resize the one ahead of the block -> it's free by default ( see the get_smallest_block algorithm )
		//uint ahead_location = new_start + new_size + sizeof(block_footer_t);
		//copy the header
		//memcpy((byte*)ahead_location, (byte*)next, sizeof(block_header_t));
		//^it's now copied to another place.
		
		//create a new block header for our block
		block_header_t *new_block = (block_header_t*)new_location;
		new_block->size = new_size;
		new_block->magic = 0xDEADBEEF;
		new_block->start = new_start;
		new_block->used = 1;
		
		block_footer_t *new_footer = (block_footer_t*)(new_location + new_size);
		new_footer->magic = 0xDEADBEEF;
		new_footer->header = new_block;
		
		//Bind the block
		block->next = new_block;
		block->prev = prev;
		//Bind the previous block
		prev->next = block;
		//Bind the new block
		new_block->prev = block;
		new_block->next = next;
		//Bind the next block
		next->prev = new_block;
		
		return new_block;
    }
    return block;
}

void *alloc(uint sz)
{
    return ralloc(sz, 0);
}
void *ralloc(uint sz, int align)
{
	block_header_t *new_block = get_smallest_block(sz, align);
	
	//page align the block
	if(align == 1)
	{
		make_smaller(new_block, sz + 0x1000 + (new_block->start & 0xFFF));
	    block_header_t *block = make_page_aligned(new_block);
	    new_block = block;
	}
	else
	    if(new_block->size > sz) //do we need a smaller block?
		    make_smaller(new_block, sz); //then make it so
		
	//set the block to be used
	new_block->used = 1;
	
	//increment the allocated memory pool counter
	allocated_memory += new_block->size;
	
	//return the address
	return (void*)new_block->start;
}

void free(void *p)
{
	notify_event(EVENT_MEM_FREE, p);
	//fetch the block
	block_header_t *old_block = (block_header_t*)((uint)p - sizeof(block_header_t));
	//does it exist?
	if(old_block->magic != 0xDEADBEEF)
	{
		kprint("Magic checksum failed, process %s(%i) caused this..\n", CurrentThread()->name, CurrentThread()->id);
		kprint("Halting..\n");
		for(;;);
	}
	
	//decrement the allocated memory pool counter
	allocated_memory -= old_block->size;
	
	//make it the biggest available
	make_bigger(old_block);
	
	//set the block to not being in use
	old_block->used = 0;
}

uint get_allocated_memory()
{
	return allocated_memory;
}
