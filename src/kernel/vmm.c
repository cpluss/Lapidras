#include "vmm.h"
#include "memory.h"

pde_t *kernel_pde, *current_pde;

void vmm_init()
{
	//Create and initialize the kernel page directory
	kernel_pde = (pde_t*)kmalloc_real(sizeof(pde_t), 1, 0);
	memset(kernel_pde, 0, sizeof(pde_t));
	kernel_pde->physAddr = (uint)kernel_pde->tablesphys;
	
	//We need to allocate space for our memory handlers pages before we do this step.
	int i;
	for(i = KMEM_START; i <= (KMEM_START + KMEM_SIZE); i += 0x1000)
		vmm_get_page(i, PDE_PRESENT | PDE_RW, kernel_pde);
	
	//Identity map up to process space
	for(i = 0; i < 0x400000; i += 0x1000)
		vmm_alloc_page_pde(i, PDE_PRESENT | PDE_RW, kernel_pde);
		
	//Get the page frames as well
	for(i = KMEM_START; i <= (KMEM_START + KMEM_SIZE); i += 0x1000)
		vmm_alloc_page_pde(i, PDE_PRESENT | PDE_RW, kernel_pde);
	//register our page fault handler
	register_interrupt_handler(14, &page_fault);
	
	//set the kernel pde to be our current pde
	vmm_set_pde(kernel_pde);
	
	//enable paging
	uint cr0;
	asm volatile("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

void vmm_set_pde(pde_t *pde)
{
	current_pde = pde;
	asm volatile("mov %0, %%cr3" : : "r"(current_pde->physAddr));
}

page_t *vmm_get_page(uint addr, uint attr, pde_t *pde)
{
	addr /= 0x1000;
	//page table containing the page?
	uint idx = addr / 0x400;
	if(pde->tables[idx])
		return &pde->tables[idx]->pages[addr % 1024];
	else
	{
		uint tmp;
		pde->tables[idx] = (pdt_t*)kmalloc_real(sizeof(pdt_t), 1, &tmp);
		memset(pde->tables[idx], 0, 0x1000);
		pde->tablesphys[idx] = tmp | attr;
		return &pde->tables[idx]->pages[addr % 1024];
	}
}
page_t *vmm_alloc_page_pde(uint addr, uint attr, pde_t *pde)
{
	page_t *page = vmm_get_page(addr, attr, pde);
	pmm_alloc_page(page);
	*page |= attr;
	return page;
}
page_t *vmm_alloc_page(uint addr, uint attr)
{
	return vmm_alloc_page_pde(addr, attr, current_pde);
}
void vmm_map_page_pde(uint vaddr, uint paddr, uint attr, pde_t *pde)
{
	//get the page
	page_t *page = vmm_get_page(vaddr, attr, pde);
	pmm_map_page(page, paddr); //map the page to paddr
	*page |= attr; //set desired attributes
}
void vmm_map_page(uint vaddr, uint paddr, uint attr)
{
	vmm_map_page_pde(vaddr, paddr, attr, current_pde);
}

static pdt_t *clone_pdt(pdt_t *src, uint *physAddr)
{
	pdt_t *pdt = (pdt_t*)kmalloc_real(sizeof(pdt_t), 1, physAddr);
	memset(pdt, 0, sizeof(pdt_t));
	
	//clone every page inside the table
	int i;
	for(i = 0; i < 0x400; i++)
	{
		if(!(pdt->pages[i] & PDE_MASK))
			continue;
		
		//clone the frame
		pmm_alloc_page(&pdt->pages[i]);
		//clone the attributes
		pdt->pages[i] |= (src->pages[i] & PDE_ATTR);
		//physically copy the page data
		copy_page_physical((src->pages[i] & PDE_MASK) * 0x1000, (pdt->pages[i] & PDE_MASK) * 0x1000);
	}
	return pdt;
}
pde_t *clone_pde(pde_t *src, uint attr)
{
	uint phys;
	pde_t *pde = (pde_t*)kmalloc_real(sizeof(pde_t), 1, &phys);
	memset(pde, 0, sizeof(pde_t));
	
	//set the new physicallAddr of the pde
	pde->physAddr = phys + (src->physAddr - (uint)src);
	
	//copy the tables
	int i;
	for(i = 0; i < 0x400; i++)
	{
		if(!src->tables[i] || (uint)src->tables[i] == 0xFFFFFFFF)
			continue;
		
		if(kernel_pde->tables[i] == src->tables[i]) //Identity mapping ...
		{
			pde->tables[i] = src->tables[i];
			pde->tablesphys[i] = src->tablesphys[i];
		}
		else
		{
			pde->tables[i] = clone_pdt(pde->tables[i], &phys);
			pde->tablesphys[i] = phys | attr;
		}
	}
	
	return pde;
}
pde_t *free_pde(pde_t *pde)
{
	int i, j;
	for(i = 0; i < 0x400; i++)
	{
		if(!pde->tables[i] || (uint)pde->tables[i] == 0xFFFFFFFF)
			continue;
		
		if(kernel_pde->tables[i] != pde->tables[i])
		{
			for(j = 0; j < 0x400; j++)
				pmm_free_page(&pde->tables[i]->pages[j]);
			free(pde->tables[i]);
		}
	}
}

//Page fault handler!
void page_fault(registers_t *regs)
{
	uint addr, present, rw, user;
	asm volatile("mov %%cr2, %0" : "=r"(addr));
	present = regs->err_code & 0x01;
	rw = regs->err_code & 0x02;
	user = regs->err_code & 0x04;
	
	cprint("Page fault at 0x%x\n", addr);
	
	if(!present && !user && !rw)
		cprint("Kernel process tried to read a non-present entry.\n");
	else if(present && !user && !rw)
		cprint("Kernel process tried to read a page and caused protection fault.\n");
	else if(!present && rw && !user)
		cprint("Kernel process tried to write to a non-present entry.\n");
	else if(!user && present && rw)
		cprint("Kernel process tried to write to a page and caused protection fault.\n");
	else if(user && !present && !rw)
		cprint("User process tried to read a non-present entry.\n");
	else if(user && !rw && present)
		cprint("User process tried to read a page and caused protection fault.\n");
	else if(user && rw && !present)
		cprint("User process tried to write a non-present entry.\n");
	else if(user && rw && present)
		cprint("User process tried to write a page and caused protection fault.\n");
		
	cprint("Panic halt.\n");
	for(;;);
}
