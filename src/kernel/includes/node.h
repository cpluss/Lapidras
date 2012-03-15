#ifndef NODE_H
#define NODE_H
#include "types.h"
#include "fs.h"

typedef struct symlink_node
{
	uint inode;
	uint size;
	void *region;
} symlink_node_t;

//Create a new node at dev/nodeX 
uint create_node(uint sz);
void remove_node(uint inode);

void init_nodes();

#endif
