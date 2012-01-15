#include "system.h"
#include "thread.h"
#include "fat.h"
extern fs_node_t *ramfs_root;
extern fs_node_t *current_node;
fs_node_t *bin;

typedef void (*cmd_call_t)(int, char**);
typedef struct cmd
{
	char ar[24];
	cmd_call_t handler;
} command_t;
list_t *avail_commands;
extern fs_node_t *evaluate_path(char *path);

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

static void list_directory_tree(fs_node_t *dir, int t)
{
	int i = 0;
	struct dirent *node = 0;
	for(i = 0; (node = readdir_fs(dir, i)) != 0; i++)
	{
		fs_node_t *fsnode = finddir_fs(dir, node->name);
		if(fsnode->flags == FS_DIRECTORY)
			ksetforeground(C_GREEN);
		
		int n;
		for(n = 0; n < t; n++)
			kputc('\t');
			
		kprint("%s\n", node->name);
		ksetdefaultcolor();
		
		if(fsnode->flags == FS_DIRECTORY)
			list_directory_tree(fsnode, t + 1);
	}	
}
static void shell_dir(int argc, char **argv)
{
	list_directory_tree(current_node, 0);
}
static void shell_ls(int argc, char **arg)
{
	fs_node_t *directory;
	if(argc >= 2)
	{
		char *path = arg[1];
		
		char *argv[128];
		char *save;
		char *pch;
		pch = (char*)strtok_r((char*)path, "/", &save);
		int tokenid = 0;
		while(pch != 0)
		{
			argv[tokenid] = (char*)pch;
			tokenid++;
			pch = (char*)strtok_r((char*)0, "/", &save);
		}
		argv[tokenid] = 0;
		if(tokenid == 1) //Well, look inside the current directory
			directory = finddir_fs(current_node, argv[0]);
		else
		{
			int i;
			fs_node_t *next_node = 0;
			fs_node_t *cur_node = current_node;
			for(i = 0; i < tokenid; i++)
			{
				fs_node_t *node = finddir_fs(cur_node, argv[i]);
				if(!node)
					continue;
				
				if(node->flags == FS_DIRECTORY && (i + 1 != tokenid))
					cur_node = node;
				else
				{
					if(i + 1 == tokenid)
						directory = node;
					break;
				}
			}
		}
		
		if(!directory)
		{
			kprint("Could not find %s\n", arg[1]);
			return;
		}
	}
	else
		directory = current_node;
	
	int i = 0;
	struct dirent *node = 0;
	for(i = 0; (node = readdir_fs(directory, i)) != 0; i++)
	{
		fs_node_t *fsnode = finddir_fs(directory, node->name);
		if(fsnode->flags == FS_DIRECTORY)
			ksetforeground(C_GREEN);
		
		kprint("%s ", node->name);
		ksetdefaultcolor();
		if(i > 10)
			break;
	}
	
	kputc('\n');
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
	strcpy(CurrentThread()->name, "shell");
	
	kprint("Welcome to Lapidras v%s\n", VERSION);
	avail_commands = list_create();
	
	register_command("proc", &shell_proc);
	register_command("clear", &shell_clear);
	register_command("dir", &shell_dir);
	register_command("ls", &shell_ls);
}
void shell()
{
	shell_init(bin);
	
	char *cmd = (char*)alloc(128);
	char *working_directory = (char*)alloc(64);
	memset(working_directory, 0, 64);
	working_directory = "/";
	
	for(;;)
	{
		kprint("/> ", current_node->name);
		memset(cmd, 0, 128);
		kbd_get_string(cmd);
		
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
		
		//check for preexisting commands
		if(shell_find(tokenid, argv))
			continue;
			
		fs_node_t *node = finddir_fs(current_node, argv[0]);
		if(!node)
		{
			node = finddir_fs(bin, argv[0]);
			if(!node)
			{
				kprint("Could not find file %s\n", argv[0]);
				continue;
			}
		}
		
		int ret = exec(node, tokenid, argv);
		if(ret == 0)
			kprint("Could not file %s\n", cmd);
		else if(ret == 2)
			kprint("%s is not a valid executable.\n", cmd);
	}
	
	for(;;); //Just in case
}
