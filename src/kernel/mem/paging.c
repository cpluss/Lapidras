#include "paging.h" 
#include "memory.h"
#include "cio.h"

//frame allocation system.. We initialize a frame address, a index
//using a bitmap
uint *frames; //frames
uint nframes; //number of total frames

//defined in memory.c
extern uint placement_address;
extern heap_t *kheap;

//function defined in functions.asm
extern void copy_page_physical(uint src, uint dest);
extern void set_page_directory(uint dir);

//keep track of the kernel directory as well as the current directory
page_directory_t *kernel_directory, *current_directory;

//frame allocation algorithms
#define INDEX_FROM_BIT(a) (a / 32)
#define OFFSET_FROM_BIT(a) (a % 32)

//statical functions to handle the bitmap allocation method
static void set_frame(uint addr)
{
    uint frame = addr / 0x1000;
    uint index = INDEX_FROM_BIT(frame);
    uint offset = OFFSET_FROM_BIT(frame);
    frames[index] |= (0x1 << offset);
}
static void clear_frame(uint addr)
{
    uint frame = addr / 0x1000;
    uint index = INDEX_FROM_BIT(frame);
    uint offset = OFFSET_FROM_BIT(frame);
    frames[index] &= ~(0x1 << offset);
}
static uint test_frame(uint addr)
{
    uint frame = addr / 0x1000;
    uint index = INDEX_FROM_BIT(frame);
    uint offset = OFFSET_FROM_BIT(frame);
    return (frames[index] & (0x1 << offset));
}
static uint first_frame()
{
    uint i, j;
    for(i = 0; i < INDEX_FROM_BIT(nframes); i++)
    {
	if(frames[i] != 0xFFFFFFFF) //nothing free, all bits is taken..
	{
	    //find a free bit
	    for(j = 0; j < 32; j++)
	    {
		if(!(frames[i] & (0x1 << j)))
		    return i * 32 + j; //return the offset
	    }
	}
    }
}

//function to allocate a page a frame
void alloc_frame(page_t *page, int is_kernel, int is_writeable)
{
    if(page->frame != 0) //already got a page frame
	return;
    
    //get the first free frame
    uint index = first_frame();
    if(index == (uint)-1) //no free frames left
    {
		kprint("NO FREE PAGE FRAMES FOR OUR PAGES LEFT!\nPANIC!!\n");
		for(;;);
    }
    
    //this frame is now taken!
    set_frame(index * 0x1000);
    page->present = 1; //present page
    //some sloppy if's here ;)
    page->rw = (is_writeable) ? 1 : 0; //rw?
    page->user = (is_kernel) ? 0 : 1; //usermode or kernel?
    page->frame = index;
}
//function to deallocate a page frame
void free_frame(page_t *page)
{
    if(page->frame == 0) //has no frame to free
		return;
    
    //clear frame position in the bitset / bitmap, and reset the pageframe
    clear_frame(page->frame);
    page->frame = 0;
}

void init_paging()
{
    //assume the physical memory is 16mb
    uint mem_end_page = 0x1000000;
    
    //create a clean and fine bitmap / bitset
    nframes = mem_end_page / 0x1000;
    frames = (uint*)kmalloc(INDEX_FROM_BIT(nframes));
    memset(frames, 0, INDEX_FROM_BIT(nframes));
    
    //create the main kernel page directory
    kernel_directory = (page_directory_t*)kmalloc_real(sizeof(page_directory_t), 1, 0);
    memset(kernel_directory, 0, sizeof(page_directory_t));
    kernel_directory->physicalAddr = (uint)kernel_directory->tablesPhysical;
    
    //set our current directory to the kernel directory
    current_directory = kernel_directory;
    
    int i;
    //we create each page for the page areas that isn't going to be
    //identity mapped before we identity map, then allocate the page after the identity mapping
    //initialize the table for our precious memoryhandler
    for(i = KHEAP_START; i < (KHEAP_START + KHEAP_SIZE); i += 0x1000)
		get_page(i, 1, kernel_directory);
    
    //identity map!
    i = 0;
    while(i < placement_address)
    {
		alloc_frame(get_page(i, 1, kernel_directory), 0, 0); //kernel-mode
		i += 0x1000;
    }
    
    for(i = KHEAP_START; i < (KHEAP_START + KHEAP_SIZE); i += 0x1000)
		alloc_frame(get_page(i, 1, kernel_directory), 0, 1); //kernel-mode, rw
    
    //register page fault handler
    register_interrupt_handler(14, &page_fault);
    
    //enable the actual paging
    switch_page_directory(kernel_directory);
    
    //create the current heap, for our memory handler
    kheap = create_heap(KHEAP_START, KHEAP_SIZE);
}

void switch_page_directory(page_directory_t *dir)
{
    current_directory = dir;
    
    //set current page directory
    asm volatile("mov %0, %%cr3" : : "r"(current_directory->physicalAddr));
    
    //enable paging
    uint cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; //set the paging bit
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

page_t *get_page(uint address, int make, page_directory_t *dir)
{
    //make the address an index
    address /= 0x1000;
    //find the page table with this address
    uint table_index = address / 1024;
    if(dir->tables[table_index]) //table is already assigned
    {
		return &dir->tables[table_index]->pages[address % 1024];
    }
    else if(make)
    {
		uint tmp;
		dir->tables[table_index] = (page_table_t*)kmalloc_real(sizeof(page_table_t), 1, &tmp);
		memset(dir->tables[table_index], 0, 0x1000);
		
		//set page table attributes
		dir->tablesPhysical[table_index] = tmp |= 0x7; //present, rw, user
		return &dir->tables[table_index]->pages[address % 1024];
    }
    else
		return 0;
}

static page_table_t *clone_table(page_table_t *src, uint *physAddr)
{
    // Make a new page table, which is page aligned.
    page_table_t *table = (page_table_t*)kmalloc_real(sizeof(page_table_t), 1, physAddr);
    // Ensure that the new table is blank.
    memset(table, 0, sizeof(page_directory_t));

    // For every entry in the table...
    int i;
    for (i = 0; i < 1024; i++)
    {
        // If the source entry has a frame associated with it...
        if (src->pages[i].frame)
        {
            // Get a new frame.
            alloc_frame(&table->pages[i], 0, 0);
            // Clone the flags from source to destination.
            if (src->pages[i].present) table->pages[i].present = 1;
            if (src->pages[i].rw) table->pages[i].rw = 1;
            if (src->pages[i].user) table->pages[i].user = 1;
            if (src->pages[i].accessed) table->pages[i].accessed = 1;
            if (src->pages[i].dirty) table->pages[i].dirty = 1;
            // Physically copy the data across. This function is in process.s.
            copy_page_physical(src->pages[i].frame * 0x1000, table->pages[i].frame * 0x1000);
        }
    }
    return table;
}

page_directory_t *clone_directory(page_directory_t *src)
{
    uint phys;
    // Make a new page directory and obtain its physical address.
    page_directory_t *dir = (page_directory_t*)kmalloc_real(sizeof(page_directory_t), 1, &phys);
    // Ensure that it is blank.
    memset(dir, 0, sizeof(page_directory_t));

    // Get the offset of tablesPhysical from the start of the page_directory_t structure.
    uint offset = (uint)dir->tablesPhysical - (uint)dir;

    // Then the physical address of dir->tablesPhysical is:
    dir->physicalAddr = phys + offset;

    // Go through each page table. If the page table is in the kernel directory, do not make a new copy.
    int i;
    for (i = 0; i < 1024; i++)
    {
        if (!src->tables[i] || (uint)src->tables[i] == (uint)0xFFFFFFFF)
            continue;

        if (kernel_directory->tables[i] == src->tables[i])
        {
            // It's in the kernel, so just use the same pointer.
            dir->tables[i] = src->tables[i];
            dir->tablesPhysical[i] = src->tablesPhysical[i];
        }
        else
        {
            // Copy the table.
            uint phys;
            dir->tables[i] = clone_table(src->tables[i], &phys);
            dir->tablesPhysical[i] = phys | 0x07;
        }
    }
    return dir;
}

void free_directory(page_directory_t *dir)
{
	uint i;
	for(i = 0; i < 1024; i++)
	{
		if(!dir->tables[i] || (uint)dir->tables[i] == (uint)0xFFFFFFFF)
			continue;
		if(kernel_directory->tables[i] != dir->tables[i])
		{
			uint j;
			for(j = 0; j < 1024; j++)
			{
				if(dir->tables[i]->pages[j].frame)
					free_frame(&dir->tables[i]->pages[j]);
			}
			free(dir->tables[i]);
		}
	}
}

void page_fault(registers_t *regs)
{
    //a page fault has occured
    //fault address
    uint fault_address;
    asm volatile("mov %%cr2, %0" : "=r"(fault_address));
    
    //the error code will give us detailed information about what just happened
    int present = !(regs->err_code & 0x1); 	//page not present
    int rw = regs->err_code & 0x2;		//write operation?
    int user = regs->err_code & 0x4;		//user mode or kernel mode?
    int reserved = regs->err_code & 0x8;		//overwritten cpu-reserved bits of page entry?
    int id = regs->err_code & 0x10;		//caused by an instruction fetch
    
    //Red for asterisk!
    ksetforeground(C_RED);
    kprint("Page fault occured at 0x%x\n", fault_address);
    if(present)
		kprint("\tNot present\n");
    if(rw)
		kprint("\tRead write operation\n");
    if(user)
		kprint("\tUser mode\n");
    if(reserved)
		kprint("\tOverwrite protected cpu-reserved bits.\n");
    if(id)
		kprint("\tInstruction fetch\n");
	
	print_registers(*regs);
    for(;;);
}
