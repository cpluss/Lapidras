#include "system.h"

void main(int argc, char *argv[])
{
	printf("Testing fork().\n");
    int ret = fork();
    if(ret == 0)
    {
        printf("I'm tha child!\n");
        //exit();
        for(;;);
    }
    printf("Parent here!\n");
    for(;;);
}
