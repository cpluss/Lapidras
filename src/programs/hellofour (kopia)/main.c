#include "system.h"

void main(int argc, char *argv[])
{
	printf("Enter string: ");
	char *buf = (char*)malloc(64);
	gets(buf);
	printf("You wrote '%s'\n", buf);
	free(buf);
	return;
}
