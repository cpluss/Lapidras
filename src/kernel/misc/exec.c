#include "file.h"
#include "fs.h"
#include "paging.h"
#include "thread.h"

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
    
    byte *buffer = (byte*)alloc(n);
    fread(buffer, n, 1, handle);
    
    //TODO: Add support for ELF files
    
    fclose(handle);
    
    asm volatile("cli");
    //Allocate the page frames for 0x40000 to 0x40000 + progr size
    int i;
    for(i = UCODE_START; i < (UCODE_START + n + 0x1000); i += 0x1000)
		alloc_frame(get_page(i, 1, current_directory), 0, 1); //kernel-mode, rw
		
	//flush TLB
	uint pd_addr;
	asm volatile("mov %%cr3, %0" : "=r"(pd_addr));
	asm volatile("mov %0, %%cr3" : : "r"(pd_addr));
    
	//Load the program to 0x400000
	uint ptr = UCODE_START;
	//copy the entire program using a very slow method. Not a large file I sincerely hope.
	memcpy((void*)ptr, buffer, n); 
	
	call_t caller = (call_t)ptr;
	asm volatile("sti");
	caller(argc, argv);
	
	//clear the memory
	memset((void*)ptr, 0, n);
	free(buffer);
	
	return 1;
}

int system(const char *path, int argc, char **argv)
{
	int ret = fork();
	if(ret == 0)
	{
		exec(path, argc, argv);
		exit();
	}
	else
	{
		volatile thread_t *child = GetThread(ret);
		if(!child)
			return -1;
		while(child->state != DEAD) wait(10);
		return 1;
	}
}
