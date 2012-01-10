#include "system.h"
#include "thread.h"
extern fs_node_t *ramfs_root;
typedef void (*cmd_call_t)(int, char**);
typedef struct cmd
{
	char ar[24];
	cmd_call_t handler;
} command_t;
list_t *avail_commands;

static void shell_proc(int argc, char **argv)
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
static void shell_clear(int argc, char **argv)
{
	clear_screen();
}
static void shell_dir(int argc, char **argv)
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
static void shell_cat(int argc, char **argv)
{
	if(argc < 2)
	{
		kprint("Usage: %s <file_to_read>\n", argv[0]);
		return;
	}
	
	fs_node_t *node = finddir_fs(ramfs_root, argv[1]);
	if(node <= 0)
	{
		kprint("Could not open file '%s' for reading.\n", argv[1]);
		return;
	}
	
	char *buffer = (char*)kmalloc(node->length);
	read_fs(node, 0, node->length, buffer);
	kprint("%s", buffer);
	free(buffer);
}

int shell_find(int argc, char **argv)
{
	foreach(item, avail_commands)
	{
		command_t *c = (command_t*)item->value;
		if(strcmp(c->ar, argv[0]))
		{
			c->handler(argc, argv);
			return 1;
			break;
		}
	}
	return 0;
}
void register_command(char *n, cmd_call_t handler)
{
	command_t *cmd = (command_t*)kmalloc(sizeof(command_t));
	strcpy(cmd->ar, n);
	cmd->handler = handler;
	list_insert(avail_commands, (void*)cmd);
}
void shell_init()
{
	kprint("Welcome to Lapidras v%s\n", VERSION);
	avail_commands = list_create();
	
	register_command("proc", &shell_proc);
	register_command("clear", &shell_clear);
	register_command("dir", &shell_dir);
	register_command("cat", &shell_cat);
}
void shell()
{
	shell_init();
	
	char *cmd = (char*)alloc(128);
	char *working_directory = (char*)alloc(64);
	working_directory = "/";
	
	for(;;)
	{
		kprint("%s> ", working_directory);
		memset(cmd, 0, 128);
		kbd_get_string(cmd);
		
		char ext[5];
		char *argv[128];
		char *save;
		char *pch;
		pch = strtok_r((char*)cmd, " ", &save);
		int tokenid = 0;
		while(pch != 0)
		{
			argv[tokenid] = (char*)pch;
			tokenid++;
			pch = strtok_r((char*)0, " ", &save);
		}
		argv[tokenid] = 0;
		int argc = tokenid;
		
		//check for preexisting commands
		if(shell_find(argc, argv))
			continue;
			
		if(!finddir_fs(ramfs_root, argv[0]))
		{
			kprint("Could not find command %s.\n", argv[0]);
			continue;
		}
		
		memcpy(ext, (char*)((uint)argv[0] + strlen(argv[0]) - 5), 4);
		ext[5] = 0;
		if(ext[0] == '.' && ext[1] == 'b' && ext[2] == 'i' && ext[3] == 'n')
			system(cmd, argc, argv);
		else
			kprint("%s is not a executable.\n", cmd);
		
	}
	
	for(;;); //Just in case
}
