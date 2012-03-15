#include "fat.h"
#include "memory.h"
#include "list.h"
#include "cio.h"

fat16_entry_t *fat16_root_entry;
fs_node_t *fat16_root;
list_t *nodes_list;
uint ident = 0;

struct dirent dirent;
static uint fat16_write(fs_node_t *fsnode, uint offset, uint size, byte *buffer);
static struct dirent *fat16_readdir(fs_node_t *node, uint index);
static fs_node_t *fat16_finddir(fs_node_t *node, char *name);

static uint fat16_read(fs_node_t *fsnode, uint offset, uint size, byte *buffer)
{
	fat16_entry_t *entry = (fat16_entry_t*)fsnode->_ptr;
	uint cluster_size = fat16_root_entry->bpb->sectors_per_cluster * fat16_root_entry->bpb->bytes_per_sector;
	uint read_size = (fsnode->length < cluster_size) ? cluster_size : (((fsnode->length / cluster_size) + 1) * cluster_size);

	if(size > entry->size)
		size = entry->size;
	if(size != cluster_size || offset != 0) //we need to do it the slow way :( -> using memcpy
	{
		byte *tmp = (byte*)kmalloc(read_size);
		int n = fat16_read_clusters(entry->cluster, tmp, 0);
		memcpy(buffer, (byte*)((uint)tmp + offset), size);
		buffer[size] = 0;
		free(tmp);
	}
	else
		fat16_read_clusters(entry->cluster, buffer, 0);
	return size;
}

static fs_node_t *fat16_create(fs_node_t *parent, char *name, int attribute)
{
	//Get the parent
	fat16_entry_t *entry = (fat16_entry_t*)parent->_ptr;
	
	/*
	 //Create a fat16_entry_t for the directory or file
		fat16_entry_t *entry = (fat16_entry_t*)kmalloc(sizeof(fat16_entry_t));
		memset((byte*)entry, 0, sizeof(fat16_entry_t));
		entry->dir = dir;
		entry->ident = ident;
		entry->device = device;
		entry->size = dir->size;
		if(dir->attribute == FAT16_ATTR_DIRECTORY)
			populate_children(device, entry);
		
		if(ln->type == 0 && ln->attribute == 0x0F)
			fat16_read_name(ln, entry->filename);
		else
			fat16_convert_readable_filename(dir->filename, entry->filename);
		if(strcmp(entry->filename, "NN") || strcmp(entry->filename, "N"))
			continue;
			
		entry->cluster = dir->cluster;
		entry->children = list_create(); //Create the list of child nodes
		list_insert(parent->children, (void*)entry); //Insert into childrens*/
	
	//Create a new entry from the scratch
	fat16_entry_t *new = (fat16_entry_t*)kmalloc(sizeof(fat16_entry_t));
	memset((byte*)new, 0, sizeof(fat16_entry_t));
	new->ident = ident;
	new->device = entry->device;
	new->size = 0; //Anything hasn't been written yet..
	new->children = list_create();
	strcpy(new->filename, name);
	list_insert(entry->children, (void*)new); //Insert into parent children
	
	fat16_dir_t *dir = (fat16_dir_t*)kmalloc(sizeof(fat16_dir_t));
	memset((byte*)dir, 0, sizeof(fat16_dir_t));
	/*
	 * char filename[11];
	byte attribute;
	byte reserved;
	
	byte creation_time_s;
	short creation_time; //first 5 bits = hour, 6 bits = minutes, 5 bits = seconds
	short creation_date; //first 7 bits = year, 4 bits = month, 5 bits = day
	short last_accessed_date; //^same format
	
	short always_zero;
	
	short last_modification_time; //format as before
	short last_modification_date; //^
	
	short cluster;
	
	int size;*/
	//TODO: Add date support
	
	//Select a free cluster
	short selected_cluster = 0;
	/*
	 * 
	uint cluster_size = fat16_root_entry->bpb->sectors_per_cluster * fat16_root_entry->bpb->bytes_per_sector;
	//next read the fat16 FAT table
	byte *FAT_table = (byte*)kmalloc(cluster_size);
	uint fat_offset = cluster * 2;
	uint fat_sector = (fat16_root_entry->bpb->reserved_sectors + fat16_root_entry->bpb->relative_sector) +
					   (fat_offset / cluster_size);
	uint ent_offset = fat_offset % cluster_size;
	
	//Read the FAT table from the harddisk
	read_hdd(fat16_root_entry->device, fat_sector, FAT_table, cluster_size);
	
	//ushort table_value = *(ushort*)((uint)FAT_table + fat_offset);
	ushort table_value = *(ushort*)&FAT_table[ent_offset];*/
	uint cluster_size = fat16_root_entry->bpb->sectors_per_cluster * fat16_root_entry->bpb->bytes_per_sector;
	byte *FAT_table = (byte*)kmalloc(cluster_size);
	uint fat_sector = (fat16_root_entry->bpb->reserved_sectors + fat16_root_entry->bpb->relative_sector);
	read_hdd(fat16_root_entry->device, fat_sector, FAT_table, cluster_size); //Read the first FAT table
	
	uint fat_offset = 0, ent_offset;
	for(fat_offset = 0; ; fat_offset++)
	{
		ent_offset = fat_offset % cluster_size;
		ushort val = *(ushort*)&FAT_table[ent_offset];
		if(val == 0)
			break; //This fat offset will do.
	}
	
	dir->cluster = selected_cluster;
	
	
	/*
	 * //Create a node to the entry
		fs_node_t *node = (fs_node_t*)kmalloc(sizeof(fs_node_t));
		memset((byte*)node, 0, sizeof(fs_node_t));
		node->finddir = &fat16_finddir;
		node->readdir = &fat16_readdir;
		//node->create = &fat16_create;
		node->write = &fat16_write;
		node->read = &fat16_read;
		switch(dir->attribute)
		{
			case FAT16_ATTR_DIRECTORY:
				node->flags = FS_DIRECTORY;
				break;
			case FAT16_ATTR_FILE:
				node->flags = FS_FILE;
				break;
		}
		node->inode = ident++;
		strcpy(node->name, entry->filename); //Copy the filename
		node->length = entry->size;
		list_insert(nodes_list, (void*)node); //Insert it into the node list*/
	fs_node_t *node = (fs_node_t*)kmalloc(sizeof(fs_node_t));
	memset((byte*)node, 0, sizeof(fs_node_t));
	node->inode = ident++;	
	node->finddir = &fat16_finddir;
	node->readdir = &fat16_readdir;
	node->read = &fat16_read;
	node->write = &fat16_write;
	switch(attribute)
	{
		case FS_DIRECTORY:
			node->flags = FS_DIRECTORY;
			dir->attribute = FAT16_ATTR_DIRECTORY;
			break;
		case FS_FILE:
			node->flags = FS_FILE;
			dir->attribute = FAT16_ATTR_FILE;
			break;
	}
	strcpy(node->name, name);
	node->length = 0;
	list_insert(nodes_list, (void*)node);
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
    read_hdd(fat16_root_entry->device, sector, (byte*)((uint)buffer + ol_size), cluster_size); //read hdd
	
	//next read the fat16 FAT table
	byte *FAT_table = (byte*)kmalloc(cluster_size);
	uint fat_offset = cluster * 2;
	uint fat_sector = (fat16_root_entry->bpb->reserved_sectors + fat16_root_entry->bpb->relative_sector) +
					   (fat_offset / cluster_size);
	uint ent_offset = fat_offset % cluster_size;
	
	//Read the FAT table from the harddisk
    //kprint("Reading sector %i\n", fat_sector);
	read_hdd(fat16_root_entry->device, fat_sector, FAT_table, cluster_size);
	
	//ushort table_value = *(ushort*)((uint)FAT_table + fat_offset);
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
	
	return fat16_read_clusters(table_value, buffer, cluster_size + ol_size);
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
	out[j] = 0; //null terminator
}
static void fat16_read_name(fat16_dir_longfilename_t *ln, char *out)
{
	int j = 0, i;
	for(i = 0; i < 10; i++) //first 10 bytes of the long file name
	{
		if(ln->name_low[i] != 0x20 && ln->name_low[i] != 0 && ln->name_low[i] != 0xFFFFFFFF)
			out[j++] = ln->name_low[i];
	}
	for(i = 0; i < 12; i++) //second 12 bytes of the long file name
	{
		if(ln->name_mid[i] != 0x20 && ln->name_mid[i] != 0 && ln->name_mid[i] != 0xFFFFFFFF)
			out[j++] = ln->name_mid[i];
	}
	for(i = 0; i < 4; i++) //third 4 bytes of the long file name
	{
		if(ln->name_high[i] != 0x20 && ln->name_high[i] != 0 && ln->name_high[i] != 0xFFFFFFFF)
			out[j++] = ln->name_high[i];
	}
	out[j] = 0; //null terminate
}
static void populate_children(ata_device_t *device, fat16_entry_t *parent)
{
	if(parent->cluster <= 0)
		return;
		
	uint cluster_size = fat16_root_entry->bpb->sectors_per_cluster * fat16_root_entry->bpb->bytes_per_sector;
	char *b = (char*)kmalloc(cluster_size);
	fat16_read_clusters(parent->cluster, b, 0);
	
	int offset = 0;//sizeof(fat16_dir_longfilename_t) + sizeof(fat16_dir_t);
	int idx = 0;
	while(1)
	{
		idx++;
		
		fat16_dir_longfilename_t *ln = (fat16_dir_longfilename_t*)((uint)b + offset);//(fat16_dir_longfilename_t*)kmalloc(sizeof(fat16_dir_longfilename_t));
		//offset += sizeof(fat16_dir_longfilename_t);
		if(ln->type == 0)
			offset += sizeof(fat16_dir_longfilename_t);
		else
			continue;
			
		fat16_dir_t *dir = (fat16_dir_t*)((uint)b + offset);//(fat16_dir_t*)kmalloc(sizeof(fat16_dir_t));
		offset += sizeof(fat16_dir_t);
		if(dir->filename[0] == '.' && ln->name_high[0] == '.')
			continue;
		
		if(ln->name_low[0] < 0x20 && ln->attribute != 0x0F)
		{				
			
			break;
		}
		if(dir->filename[0] & 0xFFFFFF00)
		{
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
		
		if(ln->type == 0 && ln->attribute == 0x0F)
			fat16_read_name(ln, entry->filename);
		else
			fat16_convert_readable_filename(dir->filename, entry->filename);
		if(strcmp(entry->filename, "NN") || strcmp(entry->filename, "N"))
			continue;
			
		entry->cluster = dir->cluster;
		entry->children = list_create(); //Create the list of child nodes
		list_insert(parent->children, (void*)entry); //Insert into childrens
		
		//Create a node to the entry
		fs_node_t *node = (fs_node_t*)kmalloc(sizeof(fs_node_t));
		memset((byte*)node, 0, sizeof(fs_node_t));
		node->finddir = &fat16_finddir;
		node->readdir = &fat16_readdir;
		node->write = &fat16_write;
		node->read = &fat16_read;
		switch(dir->attribute)
		{
			case FAT16_ATTR_DIRECTORY:
				node->flags = FS_DIRECTORY;
				populate_children(device, entry);
				break;
			case FAT16_ATTR_FILE:
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
	
	free(b);
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
		if(ln->type == 0)
			offset += sizeof(fat16_dir_longfilename_t);
		else
		{
			free((void*)ln);
			free((void*)dir);
			continue;
		}
		
		memcpy((byte*)dir, (byte*)(b + offset), sizeof(fat16_dir_t));
		offset += sizeof(fat16_dir_t);
		if(/*ln->name_low[0] < 0x20 && */ln->attribute != 0x0F)
		{
			free((void*)ln);
			free((void*)dir);
			break;
		}
		if(dir->filename[0] & 0xFFFFFF00)
		{
			free((void*)ln);
			free((void*)dir);
			continue;
		}
		
		//Create a fat16_entry_t for the directory or file
		fat16_entry_t *entry = (fat16_entry_t*)kmalloc(sizeof(fat16_entry_t));
		memset((byte*)entry, 0, sizeof(fat16_entry_t));
		entry->dir = dir;
		entry->ident = ident;
		entry->device = device;
		entry->size = dir->size;
		
		if(ln->type == 0 && ln->attribute == 0x0F)
			fat16_read_name(ln, entry->filename);
		else
			fat16_convert_readable_filename(dir->filename, entry->filename);
		/*if(strcmp(entry->filename, "NN") || strcmp(entry->filename, "N"))
		{
			free((void*)entry);
			free((void*)ln);
			free((void*)dir);
			continue;
		}*/
		
		
		entry->cluster = dir->cluster;
		entry->children = list_create(); //Create the list of child nodes
		list_insert(fat16_root_entry->children, (void*)entry); //Insert into childrens
		if(dir->attribute == FAT16_ATTR_DIRECTORY)
			populate_children(device, entry);
			
		//Create a node to the entry
		fs_node_t *node = (fs_node_t*)kmalloc(sizeof(fs_node_t));
		memset((byte*)node, 0, sizeof(fs_node_t));
		node->finddir = &fat16_finddir;
		node->readdir = &fat16_readdir;
		//node->create = &fat16_create;
		node->write = &fat16_write;
		node->read = &fat16_read;
		switch(dir->attribute)
		{
			case FAT16_ATTR_DIRECTORY:
				node->flags = FS_DIRECTORY;
				break;
			case FAT16_ATTR_FILE:
				node->flags = FS_FILE;
				break;
		}
		node->inode = ident++;
		strcpy(node->name, entry->filename); //Copy the filename
		node->length = entry->size;
		list_insert(nodes_list, (void*)node); //Insert it into the node list
		
        //kprint("Found %s in root.\n", node->name);

		entry->node = node; //Set the node pointer for fast retrieval
		node->_ptr = (void*)entry;
	}
	free(b);
	return fat16_root;
}
