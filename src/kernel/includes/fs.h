#ifndef FS_H
#define FS_H
#include "types.h"

struct fs_node;
struct ata_device;

typedef uint (*read_type_t)(struct fs_node*, uint, uint, byte*);
typedef uint (*write_type_t)(struct fs_node*, uint, uint, byte*);
typedef void (*open_type_t)(struct fs_node*);
typedef void (*close_type_t)(struct fs_node*);
typedef void (*attach_type_t)(struct fs_node*, struct fs_node*);
typedef struct dirent *(*readdir_type_t)(struct fs_node*, uint);
typedef struct fs_node *(*finddir_type_t)(struct fs_node*, char *name);
typedef struct fs_node *(*create_node_t)(struct fs_node*, char *, int);

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
	attach_type_t mount_on;
	//create_node_t create;
	
	struct fs_node *ptr; //used by mountpoints
	
	void *_ptr;
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
void mounton_fs(fs_node_t *node, fs_node_t *ad);
fs_node_t *create_fs(fs_node_t *parent, char *name, int attribute);

#define ATA_DEVICE_ATAPI 	0x00
#define ATA_DEVICE_SATA		0x01
#define ATA_DEVICE_ATA		0x02
#define ATA_DEVICE_INVALID	0x09
#define ATA_DEVICE_MEMORY   0xFF //Memory device .. Located in memory

typedef struct ata_device
{
	byte drive; //drive number
	byte slave;
	uint base;
	
	byte Type;
	uint Size;
	byte Model[41]; //Model string
	
	uint CommandSets;

    void *image; //NULL if not a memory device
} ata_device_t;

void ata_mount(ata_device_t *device, byte drive, int channel);
void ata_memory_mount(ata_device_t *device, fs_node_t *image);
void setup_ata(ata_device_t *device, int channel);

void read_ata_sector(ata_device_t *device, int lba, char *buffer);
void write_ata_sector(ata_device_t *device, int lba, char *buffer);

void read_hdd(ata_device_t *device, int lba, char *buffer, int size);

byte test_ata_device(ata_device_t *device);
byte test_ata_drive(ata_device_t *device);

int read_identify(ata_device_t *device, ushort *buffer);

typedef struct partitiontable
{
	byte bootflag;
	byte systemid;
	uint relative_sector; //start of the partition ( LBA value )
	uint sectors_size; //size of the partition in sectors
} partitiontable_t;

enum PARTITION_FS_TYPES
{
	PARTITION_FS_DOSFAT = 0x01,
	PARTITION_FS_VFAT = 0x06,
	PARTITION_FS_NTFS = 0x07,
	PARTITION_FS_LINUX_SWAP = 0x82,
	PARTITION_FS_EXT2 = 0x83
};

void get_partition_table(ata_device_t *device, partitiontable_t *table, byte index);

//Change the root fs of the current os
void set_root_fs(fs_node_t *fsnode);
fs_node_t *get_root_fs();

#endif
