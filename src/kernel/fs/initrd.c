#include "initrd.h"
#include "fs.h"
#include "list.h"

initrd_info_header_t *info_header;	//The information header
initrd_header_t *file_headers;		//All the file headers
fs_node_t *initrd_root;				//The root node of our directory
fs_node_t *initrd_dev;				//Add a directory for /dev so we can add devfs later on
list_t *dev_children;				//Children under dev

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

byte *initrd_get_content_offset(fs_node_t *node)
{
	initrd_header_t header = file_headers[node->inode];
	return (byte*)(header.f_offset + sizeof(initrd_mfile_t));
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
	if(node == initrd_root && index == 0 && node != initrd_dev)
	{
		strcpy(dirent.name, "dev");
		dirent.name[3] = 0; //make sure string is null-terminated
		dirent.inode = 0;
		return &dirent;
	}
	if(node == initrd_dev)
	{
		if(index >= dev_children->length)
			return 0;
			
		int n = 0;
		foreach(item, dev_children)
		{
			if(n == index)
			{
				fs_node_t *n = (fs_node_t*)item->value;
				strcpy(dirent.name, n->name);
				dirent.name[strlen(n->name)] = 0;
				dirent.inode = n->inode;
				return &dirent;
			}
			n++;
		}
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
	if(node == initrd_dev)
	{
		foreach(item, dev_children)
		{
			fs_node_t *n = (fs_node_t*)item->value;
			if(strcmp(n->name, name))
			{
				if(n->flags == FS_MOUNTPOINT) //this is a mountpoint, return the pointer
					return (fs_node_t*)n->ptr;
				else
					return n;
				break;
			}
		}
	}
		
	int i;
	for(i = 0; i < nroot_nodes; i++)
	{
		if(strcmp(name, root_nodes[i].name))
			return &root_nodes[i];
	}
	return 0;
}

static void initrd_mounton(fs_node_t *node, fs_node_t *ad)
{
	if(initrd_dev != node)
		return;
		
	//Create a symlink
	fs_node_t *n = (fs_node_t*)kmalloc(sizeof(fs_node_t));
	memset((byte*)n, 0, sizeof(fs_node_t));
	strcpy(n->name, ad->name);
	n->flags = FS_MOUNTPOINT;
	n->ptr = ad;
	n->inode = ad->inode;
	list_insert(dev_children, (void*)n); //Insert the mountpoint
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
    initrd_root->parent = 0;
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
    initrd_dev->parent = initrd_root;
	initrd_dev->ptr = 0;
	initrd_dev->mount_on = &initrd_mounton;
	initrd_dev->impl = 0; 
	dev_children = list_create();
	
	//root node information
	root_nodes = (fs_node_t*)kmalloc(sizeof(fs_node_t) * info_header->num, 0, 0);
	nroot_nodes = info_header->num - 1;
	int i, j = 0;
	for(i = 0; i < info_header->num; i++)
	{
		//edit file header offset to include the location
		file_headers[i].f_offset += location;
		//create a new file node
        if(strcmp((char*)&file_headers[i].name, "initrd_ins"))
            continue;
		strcpy((char*)root_nodes[j].name, (char*)&file_headers[i].name);
		root_nodes[j].mask = root_nodes[j].uid = root_nodes[j].gid = 0;
		root_nodes[j].length = file_headers[i].f_size;
		root_nodes[j].inode = i;
		root_nodes[j].flags = FS_FILE;
		root_nodes[j].read = &initrd_read;
		root_nodes[j].write = 0;
		root_nodes[j].readdir = 0;
		root_nodes[j].finddir = 0;
		root_nodes[j].open = 0;
		root_nodes[j].close = 0;
		root_nodes[j].impl = 0;
        root_nodes[j].ptr = 0;
        root_nodes[j].mount_on = 0;
        root_nodes[j].parent = initrd_root;
        j++;
	}
	
	set_root_fs(initrd_root);
	return initrd_root;
}
