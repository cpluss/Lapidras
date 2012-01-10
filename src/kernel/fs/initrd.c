#include "initrd.h"
#include "fs.h"

initrd_info_header_t *info_header;	//The information header
initrd_header_t *file_headers;		//All the file headers
fs_node_t *initrd_root;				//The root node of our directory
fs_node_t *initrd_dev;				//Add a directory for /dev so we can add devfs later on
fs_node_t *root_nodes;				//List of file nodes
int nroot_nodes;					//Number of file nodes

struct dirent dirent;

extern uint placement_address;
int setup_initrd(multiboot_t *multiboot)
{
	if(multiboot->mods_count >= 1)
	{
		uint initrd_location = *((uint*)multiboot->mods_addr);
		uint initrd_end = *(uint*)(multiboot->mods_addr + 4);
		placement_address = initrd_end + 0x10;
		return initrd_location;
	}
	return 0;
}

//function we read the initrd from
static uint initrd_read(fs_node_t *node, uint offset, uint size, byte *buffer)
{
	initrd_header_t header = file_headers[node->inode];
	if(offset > header.f_size)
		return 0;
	if(offset + size > header.f_size)
		size = header.f_size - offset;
		
	memcpy(buffer, (byte*)(header.f_offset + offset + sizeof(initrd_mfile_t)), size);
	return size;
} 

static struct dirent *initrd_readdir(fs_node_t *node, uint index)
{
	if(node == initrd_root && index == 0)
	{
		strcpy(dirent.name, "dev");
		dirent.name[3] = 0; //make sure string is null-terminated
		dirent.inode = 0;
		return &dirent;
	}
	if(index - 1 >= nroot_nodes)
		return 0;
		
	strcpy(dirent.name, root_nodes[index - 1].name);
	dirent.name[strlen(root_nodes[index - 1].name)] = 0; //string is null terminated now
	dirent.inode = root_nodes[index - 1].inode;
	return &dirent;
}

static fs_node_t *initrd_finddir(fs_node_t *node, char *name)
{
	if(node == initrd_root && strcmp(name, "dev"))
		return initrd_dev;
		
	int i;
	for(i = 0; i < info_header->num; i++)
	{
		if(strcmp(name, root_nodes[i].name))
			return &root_nodes[i];
	}
	return 0;
}

fs_node_t *init_initrd(uint location)
{
	//fetch the info header
	info_header = (initrd_info_header_t*)location;
	//fetch the file headers
	file_headers = (initrd_header_t*)(location + info_header->hdr_offset);
	
	//init the root directory
	initrd_root = (fs_node_t*)kmalloc(sizeof(fs_node_t), 0, 0);
	strcpy(initrd_root->name, "initrd");
	initrd_root->mask = initrd_root->uid = initrd_root->gid = initrd_root->inode = initrd_root->length = 0;
	initrd_root->flags = FS_DIRECTORY;
	initrd_root->read = 0;
	initrd_root->write = 0;
	initrd_root->open = 0;
	initrd_root->close = 0;
	initrd_root->readdir = &initrd_readdir;
	initrd_root->finddir = &initrd_finddir;
	initrd_root->ptr = 0;
	initrd_root->impl = 0;
	
	//init the dev directory
	initrd_dev = (fs_node_t*)kmalloc(sizeof(fs_node_t), 0, 0);
	strcpy(initrd_dev->name, "dev");
	initrd_dev->mask = initrd_dev->uid = initrd_dev->gid = initrd_dev->inode = initrd_dev->length = 0;
	initrd_dev->flags = FS_DIRECTORY;
	initrd_dev->read = 0;
	initrd_dev->write = 0;
	initrd_dev->open = 0;
	initrd_dev->close = 0;
	initrd_dev->readdir = &initrd_readdir;
	initrd_dev->finddir = &initrd_finddir;
	initrd_dev->ptr = 0;
	initrd_dev->impl = 0; 
	
	//root node information
	root_nodes = (fs_node_t*)kmalloc(sizeof(fs_node_t) * info_header->num, 0, 0);
	nroot_nodes = info_header->num;
	int i;
	for(i = 0; i < info_header->num; i++)
	{
		//edit file header offset to include the location
		file_headers[i].f_offset += location;
		//create a new file nade
		strcpy((char*)root_nodes[i].name, (char*)&file_headers[i].name);
		root_nodes[i].mask = root_nodes[i].uid = root_nodes[i].gid = 0;
		root_nodes[i].length = file_headers[i].f_size;
		root_nodes[i].inode = i;
		root_nodes[i].flags = FS_FILE;
		root_nodes[i].read = &initrd_read;
		root_nodes[i].write = 0;
		root_nodes[i].readdir = 0;
		root_nodes[i].finddir = 0;
		root_nodes[i].open = 0;
		root_nodes[i].close = 0;
		root_nodes[i].impl = 0;
	}
	
	return initrd_root;
}
