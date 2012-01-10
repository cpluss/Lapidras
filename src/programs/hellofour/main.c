#include "system.h"

void test(void *p)
{
	char c = *(char*)p;
	kprint("Character '%c' caught.\n", c);
}
int pmain()
{
	/*char *name = (char*)kmalloc(256);
	kprint("Enter your name: ");
	kread(name);
	kprint("Hello %s, have a nice day!\n", name);
	kfree(name);*/
	kprint("Registering.\n");
	register_event(EVENT_KBD_CHAR, &test);
	for(;;);
	return 0;
}
