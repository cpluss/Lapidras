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
	int nfiles = argc - 2;
	char *outfile = argv[1];
	
	info_header_t inf_header;
	header_t headers[nfiles];
	mfile_t mfiles[nfiles];
	
	printf("Total number of files: %d\n", nfiles);
	printf("Total size of headers: %d\n", (nfiles * sizeof(header_t)));
	
	//create the info header
	inf_header.magic = 0xDE;
	inf_header.num = nfiles;
	inf_header.hdr_offset = sizeof(info_header_t); //starts right after the info header
	
	//fill headers
	//calculate where the actual files begin
	unsigned int f_offset_begin = (sizeof(info_header_t) + (sizeof(header_t) * nfiles));
	char current_file[128];
	//initialize the headers
	int i = 0;
	for(i = 0; i < nfiles; i++)
	{
		strcpy(current_file, argv[i + 2]);
		if(strcmp(current_file, argv[0]) == 0)
			continue;
		printf("writing %s -> %s at 0x%x\n", outfile, current_file, f_offset_begin);
		
		headers[i].magic = inf_header.magic;
		
		strcpy(headers[i].name, current_file);
		
		headers[i].f_offset = f_offset_begin;
		
		FILE *cf = fopen(current_file, "r");
		if(cf == 0)
		{
			printf("Error: could not find file %s\n", current_file);
			return 1;
		}
		
		fseek(cf, 0, SEEK_END);
		
		//tell the file lenght
		headers[i].f_size = ftell(cf);
		
		//initialize the mfile_t structure for i
		mfiles[i].magic = inf_header.magic;
		mfiles[i].hdr_offset = (sizeof(info_header_t) + (sizeof(header_t) * i));
		
		//increase the next file offset
		f_offset_begin += (sizeof(mfile_t) + headers[i].f_size);
		
		fclose(cf);
	}
	
	//insert it all into the actual output file argv[1] = outfile
	FILE *fs = fopen(outfile, "w");
	unsigned char *data = (unsigned char*)malloc(f_offset_begin);
	
	//at the beginning: the info header
	fwrite(&inf_header, sizeof(info_header_t), 1, fs);
	//write the headers
	fwrite(headers, sizeof(header_t), nfiles, fs);
	
	//write each file
	for(i = 0; i < nfiles; i++)
	{
		//open corresponding file
		strcpy(current_file, argv[i + 2]);
		FILE *cf = fopen(current_file, "r");
		
		//read the data
		unsigned char *buf = (unsigned char*)malloc(headers[i].f_size);
		fread(buf, 1, headers[i].f_size, cf); //read into buffer ( buf )
		
		//write the mfile structure
		fwrite(&mfiles[i], sizeof(mfile_t), 1, fs);
		//write the filedata
		fwrite(buf, 1, headers[i].f_size, fs);
		
		//close the file
		fclose(cf);
		free(buf);
	}
	
	printf("Done.\n");
	
	fclose(fs);
	free(data);
	
	return 0;
}
