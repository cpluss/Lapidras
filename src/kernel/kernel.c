#include "system.h"
#include "thread.h"
#include "fat.h"
#define TO_ENTRY(ptr) ((fat16_entry_t*)ptr)
extern void shell();
int kmain(multiboot_t *multiboot, uint esp)
{
    //setup the screen
    init_video_console();
    //Change to appropriate colors etc
    kchangedefaultcolor(C_BLACK, C_LIGHT_GRAY);
    kset_status("");
    ksetdefaultcolor();
    clear_screen();
    
    //Init the descriptors
    //Initializes and replaces the initial GDT with our own
    //Initializes the IDT, with the specific handler
    init_descriptors();
    //make sure we don't overwrite the ramfs ( initrd ) by accident.
    uint initrd_location = setup_initrd(multiboot);
    if(initrd_location == 0) //invalid one
    {
		kprint("Could not locate the initrafms.\n");
		for(;;);
	}
    //Initialize events
    init_events();
	//Initialize paging
	init_paging();
	//Initialize the shared memory regions
	//init_shared_memory(57, 64); //64 memory regions should do..
    //Initialize the Programmable Interrupt Timer ( PIT )
    init_pit(1);
    //init the Real Time Clock ( RTC )
    init_rtc();
    //Mount the initramfs
    fs_node_t *fs_root = (fs_node_t*)init_initrd(initrd_location);
	mount_fs(FS_TYPE_RAMFS, fs_root);
    //Init the syscall interrupt ( 0x70 ) -> Initial plan to use 0x80 discarded, weird error pop up.
    //The 0x70 interrupt will be overwritten by it's original purpose, which at the moment is unclear to me.
    init_syscalls();
    //Initialize the keyboard
    init_kbd();
    //Initialize the threading
    start_multithreading(esp);
    
    //Mount hd0 - a fat16.
    ata_device_t ata;
	ata_mount(&ata, 0, 0);
	ushort ident[256];
	read_identify(&ata, ident);
	fs_node_t *tmp = mount_fat16(&ata, 0);
	if(!tmp)
	{
		kprint("Could not find the harddrive partition.\n");
		return 0;
	}
	fs_node_t *dev = finddir_fs(fs_root, "dev");
	if(!dev)
	{
		kprint("Could not find /dev.\n");
		return 0;
	}
	mounton_fs(dev, tmp);
    
    //Start a basic shell thread
    CreateThread("shell", (uint)shell, 2, RUNNABLE);
    
    exit();
    
    return 0;
}
