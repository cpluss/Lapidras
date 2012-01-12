#include "file.h"
#include "fs.h"
#include "paging.h"
#include "thread.h"
#include "elf.h"

extern page_directory_t *current_directory;
typedef void (*call_t)(uint, char**);
#define UCODE_START 0x400000

int exec(const char *path, int argc, char **argv)
{
	//first set the thread name
	strcpy(CurrentThread()->name, path);
	
    int handle = fopen(path);    
    int n = ftell_size(handle);
    if(n <= 0)
		return 0;
    
    //byte *buffer = (byte*)alloc(n);
    //fread(buffer, n, 1, handle);
    
    //Read binary contents
    Elf32_Ehdr *ehdr = (Elf32_Ehdr*)kmalloc(n + 100);
    fread((byte*)ehdr, n, 1, handle);
    
    //Elf read, verify that it is an elf
    if(ehdr->e_ident[0] != ELFMAG0 || ehdr->e_ident[1] != ELFMAG1 ||
	   ehdr->e_ident[2] != ELFMAG2 || ehdr->e_ident[3] != ELFMAG3)
    {
		//It is not an elf
		fclose(handle);
		free(ehdr);
		return 2; //Not an elf header -> -1
    }
    
    asm volatile("cli");
    //Lead the loadable segments from the binary
    uint x, l = 0;
    for(x = 0; x < ehdr->e_shentsize * ehdr->e_shnum; x += ehdr->e_shentsize)
    {
		Elf32_Shdr *shdr = (Elf32_Shdr*)((uint)ehdr + (ehdr->e_shoff + x));
		if(shdr->sh_addr) //loadable section or not
		{
			l++;
			//Allocate pages
			uint i;
			for(i = 0; i < shdr->sh_size + 0x2000; i += 0x1000)
			{
				//doesn't reallocate -> allocating those who isn't allocated already..
				alloc_frame(get_page(shdr->sh_addr + i, 1, current_directory), 0, 1);
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
	fclose(handle);
	
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

int system(const char *path, int argc, char **argv)
{
	int ret = fork();
	if(ret == 0)
	{
		int r = exec(path, argc, argv);
		
		//Got to find a better wait to inform the parent of my death..
		CurrentThread()->status = r;
		exit();
	}
	else
	{
		volatile thread_t *child = GetThread(ret);
		if(!child)
			return -1;
		
		while(child->state != DEAD) wait(10);
		return child->status;
	}
}
