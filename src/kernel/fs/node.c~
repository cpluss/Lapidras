#include "node.h"
#include "list.h"
#include "console.h"

extern fs_node_t *initrd_root;
uint cur_inode = 0;
list_t *nodes;
#define VTN(item) ((symlink_node_t*)item->value)

void init_nodes()
{
	nodes = list_create();
}
static symlink_node_t *find_node(uint inode)
{
    foreach(item, nodes)
    {
        if(VTN(item)->inode == inode)
            return VTN(item);
    }
    return 0;
}

static uint node_read(fs_node_t *node, uint offset, uint size, byte *buffer)
{
    symlink_node_t *snode = find_node(node->inode);
    if(offset > snode->size)
        return 0;
    if(offset + size > snode->size)
        return 0;

    memcpy(buffer, (byte*)((uint)snode->region + offset), size);
    return size;
}
static uint node_write(fs_node_t *node, uint offset, uint size, byte *buffer)
{
    symlink_node_t *snode = find_node(node->inode);
    if(offset > snode->size)
        return 0;
    if(offset + size > snode->size)
        return 0;

    memcpy((byte*)((uint)snode->region + offset), buffer, size);
    return size;
}

uint node_find(const char *ident)
{
    
}

uint create_node(uint sz)
{
	symlink_node_t *node = (symlink_node_t*)kmalloc(sizeof(node_t));
	memset(node, 0, sizeof(node_t));
	node->inode = cur_inode++;
	node->region = (void*)kmalloc(sz);
	node->size = sz;
	list_insert(nodes, (void*)node);

    fs_node_t *fsnode = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    memset((byte*)fsnode, 0, sizeof(fs_node_t));
    fsnode->flags = FS_PIPE & FS_FILE & FS_CHARDEVICE;
    fsnode->inode = node->inode;
    fsnode->length = sz;
    fsnode->read = &node_read;
    fsnode->write = &node_write;
    //fsnode->open = &node_open;

    char name[6];
    strcpy(name, "node");
    name[4] = '0' + node->inode;
    name[5] = 0;
    strcpy(fsnode->name, name);

    fs_node_t *dev = finddir_fs(initrd_root, "dev");
    if(!dev)
    {
        ksetforeground(C_RED);
        kprint("Could not find dev directory! HALTING!\n");
        for(;;);
    }

    //Mount it in dev directory
    mounton_fs(dev, fsnode);

	return node->inode;
}

void remove_node(uint inode)
{
    uint n = 0;
    foreach(item, nodes)
    {
        symlink_node_t *node = (symlink_node_t*)item->value;
        n++;
        if(node->inode == inode)
        {
            list_remove(nodes, n);
            break;
        }
    }
}
