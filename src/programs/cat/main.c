#include "system.h"

void main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("Usage: %s <file_to_read>\n", argv[0]);
		return;
	}
	
	char *filename = argv[1];
    printf("Opening %s\n", filename);
	int f = fopen(filename);
	if(f < 0)
	{
		printf("%s can't be found.\n", filename);
		return;
	}
	putc('a');
	int n = ftell_size(f);
    putc('b');
	char *buffer = (char*)malloc(n + 2);
    putc('c');
	fread(buffer, n, 1, f);
    putc('d');
	fclose(f);
    putc('e');
	int i;
	for(i = 0; i < n; i++)
		putc(buffer[i]);
	free(buffer);
	return;
}
