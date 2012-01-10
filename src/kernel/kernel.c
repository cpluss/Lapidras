#include "system.h"
#include "thread.h"

void test_tty();
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
    
    CreateThread("shell", (uint)test_tty, 2, RUNNABLE);
    
    exit();
    
    return 0;
}

extern fs_node_t *ramfs_root;
void test_tty()
{	
	kprint("Experimental shell %s\n", CurrentThread()->name);
	
	char *in_cmd = (char*)alloc(128);
	char *working_directory = (char*)alloc(64);
	working_directory = "/";
	
	for(;;)
	{
		kprint("%s> ", working_directory);
		memset(in_cmd, 0, 128);
		kbd_get_string(in_cmd);
		char *cmd = in_cmd;
		
		if(strcmp(cmd, "proc"))
		{
			//List all running processes
			int i = 0;
			thread_t *th;
			kprint("----------------------------------------------------------------\n");
			kprint("There are %i threads running.\n", get_thread_count());
			kprint("name\t\tpid\t\tpriority\t\tstate\n");
			kprint("----------------------------------------------------------------\n");
			int n = 0;
			while(n != get_thread_count())
			{
				char *states[5] = { "DEAD", "RUNNABLE", "WAITING", "SLEEPING" };
				if((th = GetThread(i)) != 0)
				{
					kprint("%s\t\t%i\t\t%i\t\t\t%s\n", th->name, th->id, th->priority, states[th->state]);
					n++;
				}
				i++;
				if(i >= 100)
					break;
			}
			kprint("----------------------------------------------------------------\n");
		}
		else if(strcmp(cmd, "clear"))
			clear_screen();
		else if(strcmp(cmd, "dir"))
		{
			int i = 0;
			struct dirent *node = 0;
			for(i = 0; (node = readdir_fs(ramfs_root, i)) != 0; i++)
			{
				fs_node_t *fsnode = finddir_fs(ramfs_root, node->name);
				if((fsnode->flags & 0x7) == FS_DIRECTORY)
					ksetforeground(C_GREEN);
				
				kprint("%s ", node->name);
				ksetdefaultcolor();
				if(i > 10)
					break;
			}
			
			kputc('\n');
		}
		else if(strcmp(cmd, "test"))
		{
			int ret = fork();
			kprint("fork() returned %i\n", ret);
			if(ret == 0)
				exit();
		}
		else
		{
			if(finddir_fs(ramfs_root, cmd) == 0)
				kprint("No such file or directory.\n");
			else
			{
				char ext[5];
				memcpy(ext, (char*)((uint)cmd + strlen(cmd) - 5), 4);
				ext[5] = 0;
				if(ext[0] == '.' && ext[1] == 'b' && ext[2] == 'i' && ext[3] == 'n')
				{
					int ret = fork();
					if(ret == 0) //the child
					{
						exec(cmd, 0, 0);
						exit();
					}
					volatile thread_t *child = GetThread(ret);
					while(child->state != DEAD) wait(10);
				}
				else
					kprint("%s is not a executable.\n", cmd);
			}
		}
	}
	
	for(;;); //Just in case
}
