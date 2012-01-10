#include "string.h"
#include "system.h"

int strlen(char *s)
{
	int ret = 0;
	do
		ret++;
	while (*s++);
	return ret;
}

void strcpy(char *dest, char *source)
{
	do
	{
		*dest++ = *source++;
	}while(*source);
	
	//string terminator
	*dest++ = 0x00;
}

int strcmp(char *s1, char *s2)
{
	int i = 0;
	if(strlen(s1) != strlen(s2))
		return 0; //false
	for(i = 0; i < strlen(s1); i++)
	{
		if(s1[i] != s2[i])
			return 0;
	}
	return 1;
}

void strapp(char *dest, char *add)
{
	int i = strlen(dest);
	do
	{
		dest[i] = *add;
		i++;
	}while(*add++);
}
void strapp_c(char *dest, char add)
{
	int i = strlen(dest);
	dest[i] = add;
}

int strchr(char *s, char c)
{
	int i;
	for(i = 0; i < strlen(s); i++)
		if(s[i] == c)
			return i;
}
void strrev(char *dest, char *source)
{
	int i;
	for(i = strlen(source); i > 0; i--)
		*dest++ = source[i];
}
int pow(int d, int n)
{
	int i = 0;
	int ret = d;
	for(i = 0; i < n - 1; i++)
		ret *= d;
	return ret;
}
int atoi(char *s)
{
	 char digits[] = "0123456789";  /* legal digits in order */
	 int dig; //current digit
	 int val = 0;
	 int i = strlen(s) - 1;
	 int k = 0;
	 for(; i > -1 && k < strlen(s); i--, k++)
	 {
		 dig = (int)((int)s[k] - (int)'0'); //holds the digit
		 if(i != 0)
			dig *= pow(10, i);
		 val += dig;
	 }
	 
	 return val;
}
