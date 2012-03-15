#include "fs.h"
#include "list.h"

fs_node_t *current_root_node;
fs_node_t *current_root_dev;

void set_root_fs(fs_node_t *fsnode)
{
	//Set the current root fs
	current_root_node = fsnode;
	current_root_dev = finddir_fs(fsnode, "dev");
}
fs_node_t *get_root_fs()
{
	return current_root_node;
}

uint read_fs(fs_node_t *node, uint offset, uint size, byte *buffer)
{
	if(node->read != 0)
		return node->read(node, offset, size, buffer);
	else
		return 0;
}
uint write_fs(fs_node_t *node, uint offset, uint size, byte *buffer)
{
	if(node->write != 0)
		return node->write(node, offset, size, buffer);
	else
		return 0;
}

void open_fs(fs_node_t *node)
{
	if(node->open != 0)
		node->open(node);
}
void close_fs(fs_node_t *node)
{
	if(node->close != 0)
		node->close(node);
}

void mounton_fs(fs_node_t *node, fs_node_t *ad)
{
	if(node->mount_on)
		node->mount_on(node, ad);
}

struct dirent *readdir_fs(fs_node_t *node, uint index)
{
	if((node->flags & 0x7) == FS_DIRECTORY && node->readdir != 0)
		return node->readdir(node, index);
	else
		return 0;
}
fs_node_t *finddir_fs(fs_node_t *node, char *name)
{
	if((node->flags & 0x7) == FS_DIRECTORY && node->finddir != 0)
		return node->finddir(node, name);
	else
		return 0;
}

fs_node_t *create_fs(fs_node_t *node, char *name, int attribute)
{
	//if((node->flags & 0xf) == FS_DIRECTORY && node->create != 0)
		//return node->create(node, name, attribute);
	return 0;
}
