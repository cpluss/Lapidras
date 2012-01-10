#ifndef FILE_H
#define FILE_H
#include "types.h"
#include "fs.h"

/*
 * Files will be handled through handles(integer), each open file will
 * have a specific file-handle to access it through. 
 * Files are opened through fopen(const char *path) and closed through fclose(int handle)
 * 
 * There will be an array that defines each open handle, with a maximum 256 handles open at once ( not likely .. )
 * */
 #define MAX_HANDLES 		256
 #define RAMFS_PATH 		"/ramfs/"
 //Opens by default as write-read when it's not the ramfs ( ramfs = /ramfs/ )
 //In case there isn't any filesystem mounted than the actual ramfs -> use the ramfs by default
 int fopen(const char *path);
 void fclose(int handle);
 
 #define FS_TYPE_RAMFS 		0xFF
 #define FS_TYPE_FAT16 		0x01
 #define FS_TYPE_EXT2 		0x02
 void mount_ramfs(fs_node_t *root);
 //To be done later..
 //void mount_fat16(fs_node_t *root);
 //void mount_ext2(fs_node_t *root);
 void mount_fs(int type, fs_node_t *root);
 
 int ftell_size(int handle);
 
 //To set the current working directory, or get
 void set_current_directory(const char *path);
 char *get_current_directory();
 
 //Read file
 int fread(byte *buffer, uint size, uint n, int handle);
 //Write to file
 int fwrite(byte *buffer, uint size, uint n, int handle);
 
#endif
