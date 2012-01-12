#include "fat.h"
#include "memory.h"
#include "list.h"
#include "cio.h"

fat16_entry_t *fat16_root_entry;
fs_node_t *fat16_root;
list_t *nodes_list;
uint ident = 0;

struct dirent dirent;

static uint fat16_read(fs_node_t *fsnode, uint offset, uint size, byte *buffer)
{
	fat16_entry_t *entry = (fat16_entry_t*)fsnode->_ptr;
	uint cluster_size = fat16_root_entry->bpb->sectors_per_cluster * fat16_root_entry->bpb->bytes_per_sector;
	uint read_size = (fsnode->length < cluster_size) ? cluster_size : (((fsnode->length / cluster_size) + 1) * cluster_size);
	
	if(size > entry->size)
		size = entry->size;
	if(size < entry->size || offset != 0) //we need to do it the slow way :( -> using memcpy
	{
		byte *tmp = (byte*)kmalloc(read_size);
		fat16_read_clusters(entry->cluster, tmp, 0);
		memcpy(buffer, (byte*)((uint)tmp + offset), size);
		free(tmp);
	}
	else
		fat16_read_clusters(entry->cluster, buffer, 0);
	return size;
}
static uint fat16_write(fs_node_t *fsnode, uint offset, uint size, byte *buffer)
{
	
}

static struct dirent *fat16_readdir(fs_node_t *node, uint index)
{
	if(node->flags != FS_DIRECTORY)
		return 0;
		
	int n = 0;
	fat16_entry_t *mentry = (fat16_entry_t*)node->_ptr;
	foreach(item, mentry->children)
	{
		if(n == index)
		{
			fat16_entry_t *entry = (fat16_entry_t*)item->value;
			strcpy(dirent.name, entry->filename);
			dirent.inode = entry->node->inode;
			return &dirent;
		}
		n++;
	}
	return 0;
}
static fs_node_t *fat16_finddir(fs_node_t *node, char *name)
{
	if(node->flags != FS_DIRECTORY)
		return 0;
	
	fat16_entry_t *entry = (fat16_entry_t*)node->_ptr;
	foreach(item, entry->children)
	{
		fat16_entry_t *f = (fat16_entry_t*)item->value;
		//kprint("\tComparing '%s'(%i) to '%s'(%i)\n", f->filename, strlen(f->filename), name, strlen(name));
		if(strcmp(f->filename, name) > 0)
			return f->node;
	}
	return 0;
}

int fat16_read_clusters(uint cluster, byte *buffer, uint ol_size)
{	
	//uint absolute_cluster = cluster - 2 + fat16_root_entry->bpb->first_data_sector;
	uint sector = (fat16_root_entry->bpb->sectors_per_cluster * (cluster - 2)) + fat16_root_entry->bpb->first_data_sector;
	uint cluster_size = fat16_root_entry->bpb->sectors_per_cluster * fat16_root_entry->bpb->bytes_per_sector;
	read_hdd(fat16_root_entry->device, sector, (byte*)buffer, cluster_size); //read hdd
	
	//next read the fat16 FAT table
	byte *FAT_table = (byte*)kmalloc(cluster_size);
	uint fat_offset = cluster * 2;
	uint fat_sector = (fat16_root_entry->bpb->reserved_sectors + fat16_root_entry->bpb->relative_sector) +
					   (fat_offset / cluster_size);
	uint ent_offset = fat_offset % cluster_size;
	
	//Read the FAT table from the harddisk
	read_hdd(fat16_root_entry->device, fat_sector, FAT_table, cluster_size);
	
	ushort table_value = *(ushort*)&FAT_table[ent_offset];
	free(FAT_table);
	
	if(table_value >= 0xFFF8) //no more clusters to read
		return cluster_size + ol_size;
	else if(table_value == 0xFFF7) //bad cluster
	{
		ksetforeground(C_RED);
		kprint("Bad cluster stumbled upon, returning..\n");
		ksetdefaultcolor();
		return cluster_size + ol_size;
	}
	
	return fat16_read_clusters(table_value, (byte*)(buffer + cluster_size), cluster_size + ol_size);
}

static void fat16_convert_readable_filename(char *in, char *out)
{
	int i, j = 0, d = 0;
	for(i = 0; i < 11; i++) //11 chars maximum .. for now ;)
	{
		char c = in[i]; //make lower case
		if(c == 0x20 && in[i + 1] == 0x20 && in[10] != 0x20 && !d)
		{
			out[j++] = '.'; //enter a dot.
			d = 1;
			continue;
		}
		if(c == 0x20)
			continue;
		else
			out[j++] = c + 0x20;
	}
	out[j++] = 0; //null terminator
}
static void populate_children(ata_device_t *device, fat16_entry_t *parent)
{
	//Not implemented atm
}
fs_node_t *mount_fat16(ata_device_t *device, int partition)
{
	//Check that the partition exists, and that it's actually a FAT16 partition.
	partitiontable_t table;
	get_partition_table(device, &table, partition);
	if(table.systemid != PARTITION_FS_VFAT)
		return 0;
		
	//Initiate the nodes list
	nodes_list = list_create();
		
	//Fetch the Boot Parameter Block ( BPB )
	fat16_bpb_t *bpb = (fat16_bpb_t*)kmalloc(sizeof(fat16_bpb_t));
	read_hdd(device, table.relative_sector, (byte*)bpb, sizeof(fat16_bpb_t));
	bpb->relative_sector = table.relative_sector;
	bpb->first_data_sector = bpb->reserved_sectors + 
							(bpb->FATS * bpb->sectors_per_fat) +
							(bpb->directory_entries * 32 / bpb->bytes_per_sector) + 
							 bpb->relative_sector;
	
	//Create a root entry for the filesystem
	fat16_root_entry = (fat16_entry_t*)kmalloc(sizeof(fat16_entry_t));
	memset((byte*)fat16_root_entry, 0, sizeof(fat16_entry_t));
	fat16_root_entry->device = device;
	fat16_root_entry->children = list_create();
	strcpy(fat16_root_entry->filename, "/"); //The / -> root directory
	fat16_root_entry->bpb = bpb;
	fat16_root_entry->ident = ident++;
	fat16_root_entry->size = 0;
	//Create the root node
	fat16_root = (fs_node_t*)kmalloc(sizeof(fs_node_t));
	memset((byte*)fat16_root, 0, sizeof(fs_node_t));
	strcpy(fat16_root->name, "fat16");
	fat16_root->flags = FS_DIRECTORY;
	fat16_root->readdir = &fat16_readdir;
	fat16_root->finddir = &fat16_finddir;
	//Bind them together
	fat16_root_entry->node = fat16_root;
	fat16_root->_ptr = (void*)fat16_root_entry;
	
	//Read all of the directory entries into a three structure. In that way you only need the root directory.
	int root_directory = (bpb->reserved_sectors + (bpb->FATS * bpb->sectors_per_fat)) + table.relative_sector;
	int root_size = (bpb->directory_entries * sizeof(fat16_dir_t)) + (bpb->directory_entries * sizeof(fat16_dir_longfilename_t));
	char *b = (char*)alloc(root_size);
	read_hdd(device, root_directory, b, root_size);
	int offset = 0;
	while(1)
	{
		fat16_dir_longfilename_t *ln = (fat16_dir_longfilename_t*)kmalloc(sizeof(fat16_dir_longfilename_t));
		fat16_dir_t *dir = (fat16_dir_t*)kmalloc(sizeof(fat16_dir_t));
		
		//copy the contents
		memcpy((byte*)ln, (byte*)(b + offset), sizeof(fat16_dir_longfilename_t));
		offset += sizeof(fat16_dir_longfilename_t);
		memcpy((byte*)dir, (byte*)(b + offset), sizeof(fat16_dir_t));
		offset += sizeof(fat16_dir_t);
		
		if(ln->attribute != 0x0F)
		{
			free(ln);
			free(dir);
			break;
		}
		if(dir->filename[0] & 0xFFFFFF00)
		{
			free(ln);
			free(dir);
			continue;
		}
		
		//Create a fat16_entry_t for the directory or file
		fat16_entry_t *entry = (fat16_entry_t*)kmalloc(sizeof(fat16_entry_t));
		memset((byte*)entry, 0, sizeof(fat16_entry_t));
		entry->dir = dir;
		entry->ident = ident;
		entry->device = device;
		entry->size = dir->size;
		if(dir->attribute == FAT16_ATTR_DIRECTORY)
			populate_children(device, entry);
		/*int i, j = 0;
		for(i = 0; i < 11; i++)
			if(dir->filename[i] != 0x20)
				entry->filename[j++] = dir->filename[i];
		entry->filename[j] = 0;*/
		fat16_convert_readable_filename(dir->filename, entry->filename);
		entry->cluster = dir->cluster;
		entry->children = list_create(); //Create the list of child nodes
		list_insert(fat16_root_entry->children, (void*)entry); //Insert into childrens
		
		//Create a node to the entry
		fs_node_t *node = (fs_node_t*)kmalloc(sizeof(fs_node_t));
		memset((byte*)node, 0, sizeof(fs_node_t));
		switch(dir->attribute)
		{
			case FAT16_ATTR_DIRECTORY:
				node->finddir = &fat16_finddir;
				node->readdir = &fat16_readdir;
				node->flags = FS_DIRECTORY;
				break;
			case FAT16_ATTR_FILE:
				node->write = &fat16_write;
				node->read = &fat16_read;
				node->flags = FS_FILE;
				break;
		}
		node->inode = ident++;
		strcpy(node->name, entry->filename); //Copy the filename
		node->length = entry->size;
		list_insert(nodes_list, (void*)node); //Insert it into the node list
		
		entry->node = node; //Set the node pointer for fast retrieval
		node->_ptr = (void*)entry;
	}
	return fat16_root;
}
