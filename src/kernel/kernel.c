#include "system.h"
#include "thread.h"
#include "fat.h"

extern fs_node_t *current_node, *bin;
extern void shell();
extern page_directory_t *current_directory;

void print_log_begin(const char *s)
{
	int n = 75 - strlen((char*)s);
	kprint((char*)s);
	for(; n > 0; n--)
		kputc(0x20);
}
void print_log_end(const char *s)
{
	kputc('[');
	if(strcmp((char*)s, "OK"))
		ksetforeground(C_GREEN);
	else
		ksetforeground(C_RED);
	kprint((char*)s);
	ksetdefaultcolor();
	kprint("]\n");
}

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
    print_log_begin("Setting up descriptors");
    init_descriptors();
    print_log_end("OK");
    //make sure we don't overwrite the ramfs ( initrd ) by accident.
    uint initrd_location = setup_initrd(multiboot);
    if(initrd_location == 0) //invalid one
    {
		kprint("Could not locate the initrafms.");
		for(;;);
	}
    //Initialize events
    init_events();
	//Initialize paging
	print_log_begin("Setting up paging");
	init_paging();
	print_log_end("OK");
	//Initialize the shared memory regions
	//init_shared_memory(57, 64); //64 memory regions should do..
    //Initialize the Programmable Interrupt Timer ( PIT )
    init_pit(1);
    //init the Real Time Clock ( RTC )
    init_rtc();   
    //Mount the initramfs
    print_log_begin("Mounting RAMFS");
    fs_node_t *fs_root = (fs_node_t*)init_initrd(initrd_location);
	mount_fs(FS_TYPE_RAMFS, fs_root);
	if(fs_root)
		print_log_end("OK");
	else
		print_log_end("FAIL");
    //Init the syscall interrupt ( 0x70 ) -> Initial plan to use 0x80 discarded, weird error pop up.
    //The 0x70 interrupt will be overwritten by it's original purpose, which at the moment is unclear to me.
    print_log_begin("Initiating syscalls");
    init_syscalls();
    print_log_end("OK");
    //Initialize the keyboard
    init_kbd();
    print_log_begin("Starting multithreading");
    //Initialize the threading
    start_multithreading(esp);
    print_log_end("OK");
    //Initialize the options, read options from start.conf
    print_log_begin("Reading options, setting static variables");
    if(setup_options() <= 0)
        print_log_end("FAIL");
    else
        print_log_end("OK");

    CreateThread("shell", (uint)shell, STATE_RUNNABLE);
    exit();

    return 0;
}
