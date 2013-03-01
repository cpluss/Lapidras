#ifndef PAGING_H
#define PAGING_H
#include "types.h"
#include "isr.h"  

//Page entry bytes
#define PDE_MASK	0xFFFFF000
#define PDE_ATTR	0x00000FFF
#define PDE_PRESENT	0x00000001
#define PDE_RW		0x00000002
#define PDE_USER	0x00000004
#define PDE_SIZE	0x1000	//4kB

typedef uint page_t;
typedef struct pdt
{
	page_t pages[1024];
} pdt_t;
typedef struct pde
{
	pdt_t *tables[1024];
	uint tablesphys[1024];
	uint physAddr;
} pde_t;

//Physical memory management .. Not that hard really -> Just bitmap stuff
void pmm_init(uint mem);
void pmm_alloc_page(page_t *page);
void pmm_free_page(page_t *page);

//Virtual memory management
void vmm_init();
page_t *vmm_alloc_page(uint addr, uint attr);
page_t *vmm_alloc_page_pde(uint addr, uint attr, pde_t *pde);
page_t *vmm_get_page(uint addr, uint attr, pde_t *pde);
void vmm_map_page(uint vaddr, uint paddr, uint attr);
void vmm_map_page_pde(uint vaddr, uint paddr, uint attr, pde_t *pde);
void vmm_set_pde(pde_t *pde);
pde_t *clone_pde(pde_t *pde, uint attr);
void page_fault(registers_t *regs);


//handle page faults
void page_fault(registers_t *regs);

#endif
