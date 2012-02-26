#include "system.h"

void main(int argc, char *argv[])
{
	printf("Testing exec on 'hellothree'.\n");
    execve("hellothree");
    printf("Did it go well?\n");
    return;
}
