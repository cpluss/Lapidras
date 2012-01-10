#include "string.h"
#include "args.h"
#include "types.h"

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

int sprintf(char *s, char *format, ...)
{
	va_list ap;
	int i, n, j = 0;
			
	va_start(ap, format);
	
	int k = 0;
	for(i = 0; i < strlen(format); i++)
	{
		if(format[i] == '%')
		{
			i++;
			char type = format[i];
			switch(type)
			{
				case 'c':
				{
					char c = va_arg(ap, int);
					s[k++] = c;
				}
				case 's':
				{
					char *str = va_arg(ap, char*);
					for(j = 0; j < strlen(str); j++)
						s[k++] = str[j];
				}break;
				case 'i':
				{
					int num = va_arg(ap, int);
					if(num == 0)
						s[k++] = '0';
					else
					{
						char c[32];
						j = 0;
						while(num > 0)
						{
							c[j] = '0' + (num % 10);
							num /= 10;
							j++;
						}
						c[j] = 0; //null terminator
						
						while(j >= 0)
							s[k++] = c[j--];
					}
				}break;
				case 'x':
				{
					int num = va_arg(ap, int);
					const char *hexarray = "0123456789ABCDEF";
					
					byte f = 0;
					for(j = 28; j > -1; j -= 4)
					{
						char o = hexarray[(num >> j) & 0xF];
						s[k++] = hexarray[(num >> j) & 0xF];
					}
				}break;
			}
		}
		else
			s[k++] = format[i];
	}
	
	va_end(ap);

	return 1;
}
