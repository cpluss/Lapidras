#include "memory.h"
#include "cio.h"
#include "paging.h"
#include "event.h"

uint *blocks;
uint nblocks;
uint free_blocks;

extern page_directory_t *current_directory;
extern uint end;
uint placement_address = (uint)&end + 0x1000;

uint kmalloc_real(uint sz, int align, uint *phys)
{
    if(nblocks <= 0)
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

void set_bit(uint *t, uint offset)
{
    *t |= (0x1 << offset);
}
void clear_bit(uint *t, uint offset)
{
    *t &= ~(0x1 << offset);
}
static uint test_bit(uint t, uint offset)
{
    return (t & (0x1 << offset));
}
static uint GetNextBlock(uint i)
{
    if(i == (nblocks - 1))
        return 0xFFFFFFFF;

    uint r = i + 1;
    while(blocks[r] & FLAG_BLOCK_LINK)
        r++;
    return r;
}
static uint GetBlockSize(uint i)
{
    uint size;
    if(i == (nblocks - 1))
        size = (KMEM_START + KMEM_SIZE) - GET_BLOCK_LOCATION(blocks[i]);
    else
        size = GET_BLOCK_LOCATION(blocks[GetNextBlock(i)]) - GET_BLOCK_LOCATION(blocks[i]);
    return size;
}
static uint GetIndexFromPointer(uint ptr)
{
    int i;
    for(i = 0; i < nblocks; i++)
    {
        if(blocks[i] & FLAG_BLOCK_LINK)
            continue;
        if(GET_BLOCK_LOCATION(blocks[i]) == ptr)
            return i;
    }
    return (uint)-1;
}
//Find best fit -> Fetch a block that fits this size, and return its index
static uint find_best_fit(uint sz, uint align)
{
    int i;
    for(i = 0; i < nblocks; i++)
    {
        if(blocks[i] & FLAG_BLOCK_LINK)
            continue; //This is linked, it can not be used ..
        if(blocks[i] & FLAG_BLOCK_USED)
            continue;
        
        uint size = GetBlockSize(i);
            
        if(align)
        {
            /*
            uint offset = 0;
            if(GET_BLOCK_LOCATION(blocks[i]) & 0xFFF) //Needs a page alignation ..
                offset = 0x1000 - (GET_BLOCK_LOCATION(blocks[i]) & 0xFFF);
            uint hole_size = size - offset;*/
            if(size > (sz + 0x1000))
                return i;
        }
        else
        {
            if(size > sz + 4)
                return i;
        }    
    }
    
    //none found..
    return (uint)0xFFFFFFFF;
}
int valid_magic(void *addr)
{
	return 1;
}

void mem_initialize()
{
    //Allocate memory for each block ;)
    uint n_blocks = KMEM_SIZE / KMEM_CHUNK_SIZE;
    blocks = (uint*)kmalloc(n_blocks * sizeof(uint));
    nblocks = KMEM_SIZE / KMEM_CHUNK_SIZE;
    free_blocks = nblocks;

    int i, offset = 0;
    for(i = 0; i < nblocks; i++)
    {
        blocks[i] = 0;
        //Set the blocks offset
        blocks[i] |= offset << 4; //Push up four bits ( 0.5bytes )
        blocks[i] |= FLAG_BLOCK_RW;
        //Extend the offset to point to our next chunk
        offset += KMEM_CHUNK_SIZE;
    }
}

void *alloc(uint sz)
{
	return ralloc(sz, 0);
}
//List used blocks
void list_blocks()
{
    int i, l = 0;
    for(i = 0; i < nblocks; i++)
    {
        if(blocks[i] & FLAG_BLOCK_LINK)
        {
            kprint("0x%x [linked]\n", GET_BLOCK_LOCATION(blocks[i]));
            l++;
		}
        if(blocks[i] & FLAG_BLOCK_USED)
            kprint("0x%x [used  ] [0x%x]\n", GET_BLOCK_LOCATION(blocks[i]), GetBlockSize(i));
    }
    kprint("did not print %i free blocks ( 0x%x bytes of free memory .. )\n", (free_blocks - l), (free_blocks - l) * KMEM_CHUNK_SIZE);
}
int get_allocated_memory()
{
	int ret, i;
	for(i = 0; i < nblocks; i++)
	{
		if(blocks[i] & FLAG_BLOCK_LINK)
			continue;
		if(blocks[i] & FLAG_BLOCK_USED)
			ret += GetBlockSize(i);
	}

	return ret;
}

void *ralloc(uint sz, int align)
{
    uint idx = find_best_fit(sz, align);
    if(idx == (uint)0xFFFFFFFF && sz >= KMEM_CHUNK_SIZE)
    {
        //There isn't enough space .. We need to link some blocks together here
        //Let's get a ground block -> finding the best spot here isn't always easy, especially if we link ..
        //kprint("Needs linkage (0x%x bytes)\n", sz);
        
        idx = 0;
        uint nidx = 0;
        uint gathered_size = 0;
        int i;
        for(i = 0; i < nblocks && gathered_size < (sz + ((align == 1) ? 0x1000 : 0)); i++)
        {
            if(blocks[i] & FLAG_BLOCK_LINK || blocks[i] & FLAG_BLOCK_USED)
            {
                //We have to reset :'(
                gathered_size = 0;
                idx = i;
                continue;
            }
            gathered_size += GetBlockSize(i);
            if(gathered_size >= sz)
            {
                nidx = i;
                break;
            }
        }
        //We've either found one, or we have not ..
        if(gathered_size >= sz) //we did!
        {
            //Link them all together from idx to nidx
            for(i = idx + 1; i <= nidx; i++)
                blocks[i] |= FLAG_BLOCK_LINK; //Linked
                
           /* free_blocks--;
            blocks[idx] |= FLAG_BLOCK_USED;
            return (void*)GET_BLOCK_LOCATION(blocks[idx]);*/
        }
        else
        {
            //Panic ..
            ksetforeground(C_RED);
            kprint("Could not link %x bytes of memory, no more memory available .. :(\n");
            for(;;);
        }
    }
    else if(idx == (uint)0xFFFFFFFF)
    {
		ksetforeground(C_RED);
		kprint("No more available memory, please implement sbrk()\n");
		list_blocks();
		for(;;);
	}

    if(align)
    {
        if(GET_BLOCK_LOCATION(blocks[idx]) & 0xFFF) //needs page alignation
        {
            //Move the pointer to a position available, preferably in front of us
            uint offset = 0x1000 - (GET_BLOCK_LOCATION(blocks[idx]) & 0xFFF);
			
            //This is kind of great, we just have to move this pointer ( making the one behind us bigger of course .. )
            uint new_offset = GET_BLOCK_LOCATION(blocks[idx]) + offset;
            if(idx != (nblocks - 1))
            {
                if(new_offset >= GET_BLOCK_LOCATION(blocks[GetNextBlock(idx)]))
                {
                    ksetforeground(C_RED);
                    kprint("This should simply not happen.. This is actually really bad ( memory allocation )\n");
                    for(;;);
                }
            }
            else
            {
                if(new_offset >= (KMEM_START + KMEM_SIZE)) //This is bad ..
                {
                    ksetforeground(C_RED);
                    kprint("We just need a bit more memory here .. :( \n");
                    for(;;);
                }
            }

            //Move the location
            blocks[idx] &= ~FLAG_BLOCK_OFFSET; //Reset
            blocks[idx] |= new_offset << 4; //Set
        }
    }
    if(GetBlockSize(idx) > sz) //Oversized ..
    {
        //We have an issue here, we could either
        //move the current block forward, or move the one before us backwards ..
        //Let's check if we can do it ! ;)
        //Check the one in front
        if(idx != (nblocks - 1) && !(blocks[GetNextBlock(idx)] & FLAG_BLOCK_USED)) //It's free ..
        {
            //Set it's new offset to be right in front of us
            uint offset = GET_BLOCK_LOCATION(blocks[idx]) + sz; //The new size
            
            blocks[GetNextBlock(idx)] &= ~FLAG_BLOCK_OFFSET; //We have to clear the old offset incase we miss any bits 
            blocks[GetNextBlock(idx)] |= offset << 4; //It's really this easy..
        }
        else if(idx != 0 && !(blocks[idx - 1] & FLAG_BLOCK_USED)) //check behind
        {
            //We don't have to do anything here really .. 0.o
        }
    }
    
    free_blocks--;

    //Piece a cake
    blocks[idx] |= FLAG_BLOCK_USED; //Set the used flag ;)
    return (void*)GET_BLOCK_LOCATION(blocks[idx]);
}

void free(void *p)
{
    uint idx = GetIndexFromPointer((uint)p);
    if(blocks[idx] & FLAG_BLOCK_USED == 0)
        return; //Block is already free ...
    if(blocks[idx] & FLAG_BLOCK_LINK)
    {
        ksetforeground(C_RED);
        kprint("TODO: Add support to free linked chains..\n");
        ksetdefaultcolor();
    }
    if(blocks[GetNextBlock(idx)] != blocks[idx + 1]) //linked .. :(
    {
        //Unset all ..
        int i;
        for(i = idx + 1; i < GetNextBlock(idx); i++)
            blocks[i] &= ~FLAG_BLOCK_LINK;
    }
    
    free_blocks++;

    blocks[idx] &= ~FLAG_BLOCK_USED; //Unset the used block
    
    //TODO: TIDY UP THIS THINGIE!!!
    /*
    //Let's try to tidy up some things here, if there are blocks that doesn't add upp ..
    int i;
    for(i = 0; i < nblocks; i++)
    {
        if(block[i] & FLAG_BLOCK_LINK || block[i] & FLAG_BLOCK_USED)
            continue;
        if(GetBlockSize(i) <= KMEM_CHUNK_SIZE)
        {
            //Try to get the desired chunk size ..
            
        }
    }*/
}
