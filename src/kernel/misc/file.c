#include "file.h"

char *current_directory;
/*
 * Every other fs is mounted at ramfs/fs/
 * -> Create that later on when implementing other fs's
 * */
fs_node_t *ramfs_root;
fs_node_t *current_node;

//Keep track of all handles
fs_node_t file_handles[MAX_HANDLES];
int nhandles;
static int getbyhandle(int handle)
{
	//Search for the specific inode
	int i;
	for(i = 0; i < nhandles; i++)
		if(file_handles[i].inode == handle)
			return i;
}
fs_node_t *getnodebyhandle(int handle)
{
	//Search for the specific inode
	return &file_handles[getbyhandle(handle)];
}
static int add_file_handle(fs_node_t *node)
{
	file_handles[nhandles] = *node;
	int ret = nhandles;
	nhandles++;
	return ret; //Return the id
}
static void remove_file_handle(int handle)
{
	int n = getbyhandle(handle);
	//move every node above n one step lower
	int i;
	for(i = n; i < (nhandles - 1); i++)
		file_handles[i] = file_handles[i + 1];
	nhandles--;
}

void mount_fs(int type, fs_node_t *root)
{
	if(type == FS_TYPE_RAMFS)
		ramfs_root = root;
	current_node = root;
}

fs_node_t *evaluate_path(char *path)
{
	char *argv[128];
	char *save;
	char *pch;
	pch = (char*)strtok_r((char*)path, "/", &save);
	int tokenid = 0;
	while(pch != 0)
	{
		argv[tokenid] = (char*)pch;
		tokenid++;
		pch = (char*)strtok_r((char*)0, "/", &save);
	}
	argv[tokenid] = 0;
	if(tokenid == 1) //Well, look inside the root directory
		return finddir_fs(current_node, argv[0]);
	
	int i;
	fs_node_t *next_node = 0;
	fs_node_t *cur_node = current_node;
	for(i = 0; i < tokenid; i++)
	{
		fs_node_t *node = finddir_fs(cur_node, argv[i]);
		if(!node)
			continue;
		
		if(node->flags == FS_DIRECTORY)
			cur_node = node;
		else
		{
			if(i + 1 == tokenid)
				return node;
			return 0;
		}
	}
	return 0;
}
int fopen(const char *path)
{		
	fs_node_t *file = evaluate_path((char*)path);
	
	if((file->flags & 0xF) == FS_DIRECTORY)
		return -1;
	//File legit, now add it to the queue.
	add_file_handle(file);
	return file->inode;
	//Add code to open handle otherwise
}
void fclose(int handle)
{
	remove_file_handle(handle);
}

int fread(byte *buffer, uint size, uint n, int handle)
{
	fs_node_t *node = getnodebyhandle(handle);
	if(node == 0)
		return 0;
	
	uint offset = 0;
	int i;
	for(i = 0; i < n; i++, offset += size)
		read_fs(node, offset, size, (char*)((uint)buffer + offset));
	return size * n;
}
int fwrite(byte *buffer, uint size, uint n, int handle)
{
	fs_node_t *node = getnodebyhandle(handle);
	if(node == 0)
		return 0;
	
	uint offset = 0;
	int i;
	for(i = 0; i < n; i++, offset += size)
		write_fs(node, offset, size, (char*)((uint)buffer + offset));
	return size * n;
}

int ftell_size(int handle)
{
	return getnodebyhandle(handle)->length;
}
