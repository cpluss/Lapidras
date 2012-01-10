#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct info_header
{
	unsigned char magic; //magic identifier
	unsigned short num; //number of files
	
	unsigned int hdr_offset; //place of the headers, if 0 then (orig_p + sizeof(info_header))
} info_header_t;
typedef struct header
{
	unsigned char magic;
	char name[128]; //name of the file
	
	unsigned int f_offset; //file content location
	unsigned int f_size; //file size
} header_t;
typedef struct mfile
{
	unsigned char magic; //as above ones
	
	unsigned int offset; //our offset
	unsigned int size; //file size
	
	unsigned int hdr_offset; //offset to the header
} mfile_t;

int main(int argc, char *argv[])
{
	if(argc < 1)
	{
		printf("Usage: %s initrd_file\n", argv[0]);
		return 0;
	}
	//our target is argv[1]
	char *target = argv[1];
	FILE *fs = fopen(target, "r");
	
	//obtain the info_structure
	info_header_t inf_header;
	fread(&inf_header, sizeof(info_header_t), 1, fs);
	//print out the data
	printf("Info header obtained.\n");
	printf("\tMagic: 0x%x\n\tNum: %d\n\tHdr offset: 0x%x\n", inf_header.magic, inf_header.num, inf_header.hdr_offset);
	
	//read the corresponding data into header structures, and then print out the contents of each file
	header_t headers[inf_header.num];
	fread(headers, sizeof(header_t), inf_header.num, fs);
	printf("Headers found: \n");
	
	//printing headers and file contents
	int i = 0;
	for(i = 0; i < inf_header.num; i++)
	{
		printf("\tName: %s\n\t\tMagic: 0x%x\n\t\tSize: %d bytes\n\t\tOffset: 0x%x\n", headers[i].name, headers[i].magic, headers[i].f_size, headers[i].f_offset);
		
		mfile_t file;
		fread(&file, sizeof(mfile_t), 1, fs);
		//read the file contents.
		unsigned char *buf = (unsigned char*)malloc(headers[i].f_size);
		fread(buf, 1, headers[i].f_size, fs);
		
		//print 'em out
		//printf("\t\tContents:\n\t\t\t%s\n", buf);
		
		free(buf);
	}
	
	
	//free(&inf_header);
	fclose(fs);
	
	return 0;
}
