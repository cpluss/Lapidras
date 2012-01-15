#include "fs.h"

void get_partition_table(ata_device_t *device, partitiontable_t *table, byte index)
{
	int offset = 0;
	switch(index)
	{
		case 0:
			offset = 0x01BE;
			break;
		case 1:
			offset = 0x01CE;
			break;
		case 2:
			offset = 0x01DE;
			break;
		case 3:
			offset = 0x01EE;
			break;
		default:
			offset = 0x01BE;
			break;
	}
	
	char *buffer = (char*)alloc(512); //read bootsector, LBA 0
	memset(buffer, 0, 512);
	//read data
	//read_ata_sector(device, 0, buffer);
	read_hdd(device, 0, buffer, 512);

	//insert the data by hand, the manual way
	table->bootflag = *(byte*)(buffer + offset);
	table->systemid = *(byte*)(buffer + offset + 4);
	table->relative_sector = *(uint*)(buffer + offset + 8);
	table->sectors_size = *(uint*)(buffer + offset + 12);
	
	//cleanup
	free(buffer);
}
