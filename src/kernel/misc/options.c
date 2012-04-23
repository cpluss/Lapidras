#include "system.h"
#include "list.h"
#include "fat.h"

typedef int (*opt_call_t)(char *);
typedef struct
{
    char ident[64]; //The identifier - option to be set
    opt_call_t handler;
} opt_t;

extern fs_node_t *ramfs_root, *bin, *current_node;
extern ata_device_t *maindevice;
list_t *opt_list;
static void add_opt(char *ident, opt_call_t handler)
{
    opt_t *opt = (opt_t*)kmalloc(sizeof(opt_t));
    strcpy(opt->ident, ident);
    opt->handler = handler;
    list_insert(opt_list, (void*)opt);
}

static int set_working_dir(char *val)
{
    //Set the working directory
    //Not yet implemented
    return 1;
}
static int set_boot_device(char *val)
{
    //Set the boot device
    /*
        Valid option values:
                - (hd0,0) -> The first ATA Device, partition 0
                - (hdX,P) -> X'th ATA channel, partition P
                - (mmP,name) -> (P ^) Should always be (mmP,<name>) - Loads the boot image from ramfs (the boot image name is <name>)
    */
    //Not yet implemented
    char id[3];
    //val[0] is always equal to '('
    if(val[0] != '(')
    {
        kprint("\n%s is not a valid boot device.\n", val);
        for(;;);
        return 0;
    }
    id[0] = val[1]; //h - m
    id[1] = val[2]; //d - m
    id[2] = 0;
   
    if(strcmp(id, "hd")) //We use a HDD as boot device
    {
        //mount the device and set current_node
        ata_device_t *ata = (ata_device_t*)kmalloc(sizeof(ata_device_t));
        memset((byte*)ata, 0, sizeof(ata_device_t));
        int channel = (int)val[3] - '0';
     
        ata_mount(ata, 0, channel); //Mount the device -> Needs improvement by detection etc
        if(ata->Type == ATA_DEVICE_INVALID)
        {
            kprint("\n%s is not a valid boot device.\n", val);
            for(;;);
            return 0;
        }
       
        int partition = (int)val[5] - '0'; //4'th is a ,
      
        fs_node_t *tmp = mount_fat16(ata, partition);        
        if(!tmp) //Could not find a fat16 partition at 'partition'
        {
            kprint("\n%s is not a valid fat16 formatted partition.\n");
            for(;;);
            return 0;
        }
       
        //Find the tmp and mount the fat16 partition
        //fs_node_t *dev = finddir_fs(ramfs_root, "dev"); //If this fails, sigh!
        //mounton_fs(dev, tmp); //Mount the fat16 node on dev - using MOUNTPOINT
        set_root_fs(tmp); //Set the new root directory
        bin = finddir_fs(tmp, "bin");
        current_node = tmp; //Set the current root directory
        set_current_root(tmp);
    }
    else if(strcmp(id, "mm")) //We use a HDD image as boot device
    {
        //Mount a memory device ..  
        //(mmP,<name>)
        int partition = (int)val[3] - '0';
        char name[64]; //temp name
        memcpy((char*)name, (char*)((uint) val + 5), strlen(val) - 7);
        name[strlen(val) - 7] = 0;
        
        //find the image
        fs_node_t *image = finddir_fs(ramfs_root, name);
        if(!image)
        {
            kprint("\nCould not find %s\n", name);
            return 0;
        }

        //Mount the ata device
        ata_device_t ata;
        ata_memory_mount(&ata, image); //mount at memory
        
        //Mount the fat16 partition on the memory device
        fs_node_t *tmp = mount_fat16(&ata, partition);
        kprint("fat16 node name: %s\n", tmp->name);
        if(!tmp)
        {
            kprint("\n%s is not a valid fat16 partition.\n", val);
            return 0;
        }

        fs_node_t *dev = finddir_fs(ramfs_root, "dev"); //if this fails, we're doomed ..
        mounton_fs(dev, tmp); //mount inside the fs

        //Set the current directory as well as the bin directory
        bin = finddir_fs(tmp, "bin");
        current_node = ramfs_root;//tmp; //Current directory*/
        set_current_root(tmp);
    }
    else if(strcmp(id, "nn")) //none
    {
        //set the current node as the ramfs root
        current_node = ramfs_root;
        set_current_root(ramfs_root);
        bin = 0;
    }
    else
    {
        kprint("\nCould not define the type of '%s'\n", val);
        for(;;);
        return 0;
    }
    return 1;
}
static int set_keymap(char *val)
{
    if(strcmp(val, "qwerty"))
        set_qwerty();
    else
        set_dvorak();
    return 1;
}

static void get_options(char *opts, char **out, int *argc)
{
    char *save, *pch;
    int tokenid = 0;
    pch = strtok_r((char*)opts, "\n", &save);
    while(pch != 0)
    {
        if(pch[0] != '#')
        {
            out[tokenid] = pch;
            tokenid++;
        }
        pch = strtok_r(0, "\n", &save);
    }
    out[tokenid] = 0;
    *argc = tokenid;
}
static void init_opts()
{
    //Set the current working directory upon start
    add_opt("set wd", &set_working_dir);
    add_opt("set boot", &set_boot_device);
    add_opt("keymap", &set_keymap);
}

static uint parse_opt(char *opt)
{
    char *save;
    char *ident = strtok_r(opt, "=", &save);
    char *val = strtok_r(0, "=", &save);

    foreach(item, opt_list)
    {
        opt_t *opt = (opt_t*)item->value;
        if(strcmp(opt->ident, ident)) //Found option set
        {
            int ret = opt->handler(val);
            if(!ret)
                return 0;
        }
    }
    return 1;
}
uint setup_options()
{
    opt_list = list_create();
    init_opts();

    fs_node_t *opts_file = finddir_fs(ramfs_root, "start.conf");
    if(!opts_file)
        return 0;
    char *opts = (char*)kmalloc(opts_file->length + 0x10);
    //Read the contents of start.conf into opts buffer
    read_fs(opts_file, 0, opts_file->length, opts);
    opts[opts_file->length] = 0; //Set the null terminator

    //Create a buffer to store the options within
    char **argv = (char**)kmalloc(opts_file->length + 0x200); //512 bytes should be enough (0x200 in hex ..)
    int argc = 0;
    get_options(opts, argv, &argc);

    int i;
    for(i = 0; i < argc; i++)
    {
        if(parse_opt(argv[i]) <= 0)
            return 0;
    }
}
