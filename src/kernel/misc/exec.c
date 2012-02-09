#include "file.h"
#include "fs.h"
#include "paging.h"
#include "thread.h"
#include "elf.h"

extern page_directory_t *current_directory;
typedef void (*call_t)(uint, char**);
#define UCODE_START 0x400000

int exec(fs_node_t *path, int argc, char **argv)
{
	//first set the thread name
	strcpy(CurrentThread()->name, argv[0]);
		
    //Read binary contents
    Elf32_Ehdr *ehdr = (Elf32_Ehdr*)kmalloc(path->length + 100);
    read_fs(path, 0, path->length, (byte*)ehdr);
    
    //Elf read, verify that it is an elf
    if(ehdr->e_ident[0] != ELFMAG0 || ehdr->e_ident[1] != ELFMAG1 ||
	   ehdr->e_ident[2] != ELFMAG2 || ehdr->e_ident[3] != ELFMAG3)
    {
		//It is not an elf
		free(ehdr);
		return 2; //Not an elf header -> -1
    }
    
    asm volatile("cli");
    //Allocate the page tables ..
    uint i;
    for(i = UCODE_START; i < (UCODE_START + path->length + 0x100); i += 0x1000)
        alloc_frame(get_page(i, 1, current_directory), 0, 0);
    //Set the memory to zero ..
    memset((char*)UCODE_START, 0, path->length);

    //Lead the loadable segments from the binary
    uint x, l = 0;
    for(x = 0; x < ehdr->e_shentsize * ehdr->e_shnum; x += ehdr->e_shentsize)
    {
		Elf32_Shdr *shdr = (Elf32_Shdr*)((uint)ehdr + (ehdr->e_shoff + x));
		if(shdr->sh_addr) //loadable section or not
		{
			l++;
			//Allocate pages
			for(i = 0; i < shdr->sh_size + 0x2000; i += 0x1000)
			{
				//doesn't reallocate -> allocating those who isn't allocated already..
				alloc_frame(get_page(shdr->sh_addr + i, 1, current_directory), 0, 0); //1);
			}
			
			if(shdr->sh_type == SHT_NOBITS) //Is it the .bss?
				memset((byte*)shdr->sh_addr, 0, shdr->sh_size); //The .bss is uninitialized memory ..
			else //Nope .. then copy it
				memcpy((byte*)shdr->sh_addr, (byte*)((uint)ehdr + shdr->sh_offset), shdr->sh_size);
				
		}
	}
	//flush TLB to notify the page changes
	uint pd_addr;
	asm volatile("mov %%cr3, %0" : "=r"(pd_addr));
	asm volatile("mov %0, %%cr3" : : "r"(pd_addr));
	
	asm volatile("sti");
	uint entry = (uint)ehdr->e_entry;
	
	free(ehdr);	
	if(l == 0)
	{
		//no segments loaded, exit nice and peacefully..
		return 2;
	}

	call_t caller = (call_t)entry;
	asm volatile("sti");
    caller(argc, argv);
	
	return 1;
}

int system(fs_node_t *path, int argc, char **argv)
{
    int ret = fork();
    if(ret == 0)
    {
        int r = exec(path, argc, argv);
        exit();
    }
    else
    {
        volatile thread_t *child = GetThread(ret);
        if(!child)
            return 0;
        while(child->state != STATE_DEAD) wait(10);
    }
    return 1;
}
