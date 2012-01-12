#include "system.h"

void main(int argc, char *argv[])
{
	if(argc != 2)
	{
		kprint("Usage: %s <file_to_read>\n", argv[0]);
		return;
	}
	
	char *filename = argv[1];	
	int f = fopen(filename);
	if(f < 0)
	{
		kprint("%s can't be found.\n", filename);
		return;
	}
	
	int n = ftell_size(f);
	char *buffer = (char*)kmalloc(n + 2);
	fread(buffer, n, 1, f);
	fclose(f);
	int i;
	for(i = 0; i < n; i++)
		kputc(buffer[i]);
	kfree(buffer);
	
	return;
}
