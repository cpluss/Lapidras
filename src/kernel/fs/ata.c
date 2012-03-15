#include "fs.h"
#include "ports.h"

void ata_mount(ata_device_t *device, byte drive, int channel)
{
	device->drive = drive;
	switch(channel)
	{
		case 0:
			device->slave = 0;
			device->base = 0x1F0;
			break;
		case 1:
			device->slave = 1;
			device->base = 0x170;
			break;
		case 2:
			device->slave = 0;
			device->base = 0x1E8;
			break;
		case 3:
			device->slave = 1;
			device->base = 0x16F;
			break;
		default:
			device->slave = 0;
			device->base = 0x1F0;
			break;
	}
	
	//fill the rest of the information
	ushort buffer[256];
	byte ret = read_identify(device, buffer);
	if(ret == 0)
	{
		device->Type = ATA_DEVICE_INVALID;
		return; //this is no valid ATA device
	}
	
	//read the model string
	char model[41];
	int k;
	for(k = 0; k < 40; k += 2)
	{
		//offset = 54 + k		
		model[k] = *(char*)((uint)buffer + 54 + k + 1);
		model[k + 1] = *(char*)((uint)buffer + 54 + k);
	}
	model[41] = '\0';
	//Copy the memory
	memcpy(device->Model, model, 41);
	
	//Fetch commandsets
	device->CommandSets = *(uint*)((uint)buffer + 164);
	
	if(device->CommandSets & (1 << 26)) //48-Bit addressing
		//48-Bit addressing
		device->Size = *(uint*)((uint)buffer + 200) / 2;
	else //28-Bit addressing / CHS		
		device->Size = *(uint*)((uint)buffer + 120) / 2;
    device->image = 0;
}

void ata_memory_mount(ata_device_t *device, fs_node_t *image)
{
    //create a buffer where we will store the image within'
    byte *b = (byte*)kmalloc(image->length + 0x10); 
    
    read_fs(image, 0, image->length, b);

    //Now set the device options
    device->Type = ATA_DEVICE_MEMORY;
    //Set the model
    strcpy(device->Model, image->name);
    if(strlen(image->name) < 41)
    {
        int n = 41 - strlen(image->name);
        for(; n > 0; n--)
            device->Model[n] = 0x20; //Space
    }

    device->Size = image->length;
    //set the device image pointer
    device->image = (void*)b; //The image is loaded at b
}

void read_hdd(ata_device_t *device, int lba, char *buffer, int size)
{   
    if(device->Type == ATA_DEVICE_MEMORY)
    {
        //Piece a cake!
        kprint("Reading memory device..\n");
        memcpy(buffer, (char*)((uint)device->image + (lba * 512)), size);
        return;
    }
    
    int sectors = size / 512; //the amount of sectors to read
	sectors += 1; //read one sector more than we should..
	char *buf = (char*)alloc(sectors * 512);

	int i = 0, j = lba;
	for(i = 0; i < sectors; i++, j++)
	{
		char *tmp = (char*)(buf + (i * 512));
		read_ata_sector(device, j, tmp);
	}
	//copy over the memory
	memcpy(buffer, buf, size);
	free(buf);
}

void read_ata_sector(ata_device_t *device, int lba, char *buffer)
{
	//send drive indicator and the highest bits (4) of the lba
	outb(device->base + 6, (0xE0 | (device->drive << 4) | ((lba >> 24) & 0x0F)));
	//send null descriptor, to waste time
	//outb(device->base + 1, 0);
	//sector count to port 2
	outb(device->base + 2, 1);
	
	//now send the lba in three 8 bits (bytes)
	outb(device->base + 3, (byte)(lba)); //low
	outb(device->base + 4, (byte)(lba >> 8)); //mid
	outb(device->base + 5, (byte)(lba >> 16)); //high
	
	//wait 1ms - took to long with several file operations
	wait(1);
	/*int n = 4;
	while(1)
	{
		byte in = inb(device->base + 7);
		if(n > 0)
		{
			n--;
			continue;
		}
		if(!(in & 0x80) && !(in & 0x08))
			break;
	}*/
	
	//send command to read
	outb(device->base + 7, 0x20);
	
	//wait for the device to get ready
	//while(!(inb(device->base + 7) & 0x08));
    //kprint("Waiting for device(0x%x)...", device->base);
    while((inb(device->base + 7) & 0xC0) != 0x40);
    //kprint("done.\n");
    
	//read using the insert word assembler command ( inline assembler )
	insw(device->base, buffer, 256);
	//reset device
	outb(device->base + 7, 0xE7);
}

void write_ata_sector(ata_device_t *device, int lba, char *buffer)
{
	//send drive indicator and the highest bits (4) of the lba
	outb(device->base + 6, (0xE0 | (device->drive << 4) | ((lba >> 24) & 0x0F)));
	//send null descriptor, to waste time
	outb(device->base + 1, 0);
	//sector count to port 2
	outb(device->base + 2, 1);
	
	//now send the lba in three 8 bits (bytes)
	outb(device->base + 3, (byte)(lba)); //low
	outb(device->base + 4, (byte)(lba >> 8)); //mid
	outb(device->base + 5, (byte)(lba >> 16)); //high
	
	//wait 1ms
	wait(1);
	
	//send write command
	outb(device->base, 0x30);
	kputc('a');
	//poll, poll, and poll.. -> or just wait for the device, whatever you prefer
	while(!(inb(device->base + 7) & 0x08));
	kputc('d');
	//write using the output work assembler command ( inline assembler )
	outsw(device->base, buffer, 256);
	
	//write 256 words = 512 chars = 1 sector
	/*int i = 0;
	ushort tmp = 0;
	for(i = 0; i < 256; i++)
	{
		tmp = buffer[8 + i * 2] | (buffer[8 + i * 2] << 8);
		outw(device->base, tmp);
	}*/
	
	//reset
	outb(device->base + 7, 0xE7);
}

byte test_ata_device(ata_device_t *device)
{
	//send a dummy packet to await response, if there was a proper response then the device exists.
	outb(device->base + 6, 0xA0);
	wait(1);
	if(inb(device->base + 7) & 0x40)
		return test_ata_drive(device);
	else
		return 0;
}
byte test_ata_drive(ata_device_t *device)
{
	outb(device->base + 3, 0x88);
	wait(1);
	if(inb(device->base + 3) == 0x88)
		return 1;
	else
		return 0;
}

int read_identify(ata_device_t *device, ushort *buffer)
{
	if(device->base == 0x1F0 || device->base == 0x1E8) //master drive
		outb(device->base + 6, 0xA0);
	else //slave drive
		outb(device->base + 6, 0xB0);
		
	outb(device->base + 2, 0);
	outb(device->base + 3, 0);
	outb(device->base + 4, 0);
	outb(device->base + 5, 0);
	
	//Send identify command
	outb(device->base + 7, 0xEC);
	
	byte status = inb(device->base + 7);
	if(status == 0)
		return 0;
		
	while(!(status & 0x80))
	{
		status = inb(device->base + 7);
		if(status & 0x80 || status & 0x08)
		{
			device->Type = ATA_DEVICE_ATA;
			break;
		}
		
		if(status & 0x01) //abortion
		{
			//Is it a ATAPI / SATA Device?
			if(inb(device->base + 4) == 0x14 && inb(device->base + 5) == 0xEB)
			{
				device->Type = ATA_DEVICE_ATAPI;
				break;
			}
			else if(inb(device->base + 4) == 0x3C && inb(device->base + 5) == 0xC3)
			{
				device->Type = ATA_DEVICE_SATA;
				break;
			}
			else
				return 0;
		}
	}
	
	insw(device->base, buffer, 256);
	return 1;
}


