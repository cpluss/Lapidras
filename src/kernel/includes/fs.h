#ifndef FS_H
#define FS_H
#include "types.h"

struct fs_node;

typedef uint (*read_type_t)(struct fs_node*, uint, uint, byte*);
typedef uint (*write_type_t)(struct fs_node*, uint, uint, byte*);
typedef void (*open_type_t)(struct fs_node*);
typedef void (*close_type_t)(struct fs_node*);
typedef struct dirent *(*readdir_type_t)(struct fs_node*, uint);
typedef struct fs_node *(*finddir_type_t)(struct fs_node*, char *name);

typedef struct fs_node
{
	char name[128]; //filename
	uint mask; //permissions mask
	uint uid; //owner id
	uint gid; //group owner
	uint flags; //includes the inode type
	uint inode; //device specific
	uint length; //size of the file, in bytes
	uint impl; //implementation-defined number
	
	read_type_t read;
	write_type_t write;
	open_type_t open;
	close_type_t close;
	readdir_type_t readdir;
	finddir_type_t finddir;
	
	struct fs_node *ptr; //used by mountpoints
} fs_node_t;

struct dirent //returned by readdir call, (POSIX)
{
	char name[128]; //filename
	uint inode; //inode (POSIX)
};

#define FS_FILE 		0x01
#define FS_DIRECTORY 	0x02
#define FS_CHARDEVICE	0x03
#define FS_BLOCKDEVICE	0x04
#define FS_PIPE			0x05
#define FS_SYMLINK		0x06
#define FS_MOUNTPOINT	0x08

uint read_fs(fs_node_t *node, uint offset, uint size, byte *buffer);
uint write_fs(fs_node_t *node, uint offset, uint size, byte *buffer);
void open_fs(fs_node_t *node);
void close_fs(fs_node_t *node);
struct dirent *readdir_fs(fs_node_t *node, uint index);
fs_node_t *finddir_fs(fs_node_t *node, char *name);

#endif
