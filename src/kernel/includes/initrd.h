#ifndef INITRD_H
#define INITRD_H
#include "types.h"
#include "fs.h"
#include "multiboot.h"

typedef struct initrd_info_header
{
	unsigned char magic; //magic identifier
	unsigned short num; //number of files
	
	unsigned int hdr_offset; //place of the headers, if 0 then (orig_p + sizeof(info_header))
} initrd_info_header_t;
typedef struct initrd_header
{
	unsigned char magic;
	char name[128]; //name of the file
	
	unsigned int f_offset; //file content location
	unsigned int f_size; //file size
} initrd_header_t;
typedef struct initrd_mfile
{
	unsigned char magic; //as above ones
	
	unsigned int offset; //our offset
	unsigned int size; //file size
	
	unsigned int hdr_offset; //offset to the header
} initrd_mfile_t;

//Mount the ramdisk to a fs_node. It gets passed the address in memory where the initrd content is stored
//and returns a complete filesystem node
fs_node_t *init_initrd(uint location);
int setup_initrd(multiboot_t *multiboot);

byte *initrd_get_content_offset(fs_node_t *node);
#endif 
