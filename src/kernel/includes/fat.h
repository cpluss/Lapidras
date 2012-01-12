#ifndef FAT_H
#define FAT_H
#include "types.h"
#include "fs.h"
#include "list.h"
 //reserved_sectors + FATS * sectors_per_fat
typedef struct fat16_bpb
{
	byte fjmp[3];
	byte fres[8];
	
	short bytes_per_sector;
	byte sectors_per_cluster;
	short reserved_sectors;
	byte FATS;
	short directory_entries;
	short sectors;
	byte media_descriptor_type;
	short sectors_per_fat;
	short sectors_per_track;
	short heads;
	int hidden_sectors;
	int large_amount_sector;
	
	byte drive;
	byte ntflags;
	byte signature;
	
	int volumeid;
	
	char label[11];
	char system_ident[8];
	
	uint relative_sector;
	uint first_data_sector;
} __attribute__((packed)) fat16_bpb_t;

#define FAT16_ATTR_FILE			0x00
#define FAT16_ATTR_RONLY 		0x01
#define FAT16_ATTR_HIDDEN		0x02
#define FAT16_ATTR_SYSTEM		0x04
#define FAT16_ATTR_VOLUME		0x08
#define FAT16_ATTR_DIRECTORY 	0x10
#define FAT16_ATTR_ARCHIVE		0x20
typedef struct fat16_dir
{
	char filename[11];
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
	
	int size;
} __attribute__((packed)) fat16_dir_t;
typedef struct fat16_dir_longfilename
{
	byte order;
	
	char name_low[10];
	byte attribute; //always equal to 0x0F
	byte type;
	byte checksum;
	char name_mid[12];
	
	short always_zero;
	
	char name_high[4];
} __attribute__((packed)) fat16_dir_longfilename_t;

typedef struct fat16_entry
{
	fat16_dir_longfilename_t *lnfname;
	fat16_dir_t *dir;
	
	//Used when you search for a specific node, the inode number.
	int ident; //identification number, to specify the actual index
	ata_device_t *device;
	fat16_bpb_t *bpb; //the boot parameter block. Only specified if the name of the directory is "/" -> Root
	
	//Filename of the entry
	char filename[64];
	short cluster;
	int size;
	
	list_t *children;
	fs_node_t *node;
} fat16_entry_t;

fs_node_t *mount_fat16(ata_device_t *ata, int partition);

#endif
