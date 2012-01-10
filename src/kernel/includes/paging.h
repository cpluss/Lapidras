#ifndef PAGING_H
#define PAGING_H
#include "types.h"
#include "isr.h"

typedef struct page
{
    uint present	: 1; //page present in memory?
    uint rw		: 1; //read-only if clear .. you get my point
    uint user		: 1; //user or kernel? ( kernel = 0 )
    uint accessed	: 1; //has the page been accessed since last refresh?
    uint dirty		: 1; //has the page been written since last refresh?
    uint unused		: 7; //unused bits
    uint frame		: 20; //physical address
} page_t;

typedef struct page_table
{
    page_t pages[1024];
} page_table_t;

typedef struct page_directory
{
    page_table_t *tables[1024];
    uint tablesPhysical[1024];
    uint physicalAddr;
} page_directory_t;

//enables paging
void init_paging();

//switches the current page directory
void switch_page_directory(page_directory_t *dir);
//function enables paging, and sets the current page directory as page directory
void enable_paging(); //NOTE: SHOULD ONLY BE CALLED ONCE!

//get page
//if make == 1, the page is created.
page_t *get_page(uint address, int make, page_directory_t *dir);
//allocate a page frame
void alloc_frame(page_t *page, int is_kernel, int is_writeable);
//free a page frame, deallocate it
void free_frame(page_t *page);

//clone a page directory, link it if necessary, or copy
page_directory_t *clone_directory(page_directory_t *src);

//handle page faults
void page_fault(registers_t *regs);

#endif
