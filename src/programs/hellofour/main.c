#include "system.h"

void main(int argc, char *argv[])
{
	kprint("Enter string: ");
	char *buf = (char*)kmalloc(64);
	kread(buf);
	kprint("You wrote '%s'\n", buf);
	kfree(buf);
	return;
}
