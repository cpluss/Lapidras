#include "string.h"
#include "args.h"
#include "types.h"

void memcpy(unsigned char *dest, unsigned char *source, int len)
{
	const unsigned char *sp = (const unsigned char *)source;
    unsigned char *dp = (unsigned char *)dest;
    for(; len != 0; len--) *dp++ = *sp++;
}
void memset(unsigned char *dest, unsigned char value, int len)
{
	unsigned char *temp = (unsigned char *)dest;
    for ( ; len != 0; len--) *temp++ = value;
}
int memcmp(unsigned char *p1, unsigned char *p2, int n)
{
	const unsigned char *s1, *s2;
    
	s1 = p1;
	s2 = p2;
	while(n-- > 0)
	{
		if(*s1 != *s2)
            return *s1 - *s2;
		s1++, s2++;
	}
    
    return 0;
}


int strlen(const char *s)
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
	{
		return 0; //false
	}
	for(i = 0; i < strlen(s1); i++)
	{
		if(s1[i] != s2[i])
		{
			return 0;
		}
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

//Tagna från klange - github repository
uint strspn(const char * str, const char * accept) 
{
	const char * ptr;
	const char * acc;
	uint size = 0;
	for (ptr = str; *ptr != '\0'; ++ptr) 
	{
		for (acc = accept; *acc != '\0'; ++acc) 
		{
			if (*ptr == *acc) 
			{
				break;
			}
		}
		if (*acc == '\0') 
		{
			return size;
		} 
		else 
		{
			++size;
		}
	}
	return size;
}
char *strpbrk(const char *str, const char *accept)
{
	while(*str != '\0')
	{
		const char *acc = accept;
		while(*acc != '\0')
		{
			if(*acc++ == *str)
				return (char*)str;
		}
		str++;
	}
	return 0;
}
uint lfind(const char *str, const char accept)
{
	uint i = 0;
	while(str[i] != accept)
		i++;
	return (uint)str + i;
}
char *strtok_r(char *str, const char *delim, char **saveptr)
{
	char *token;
	if(str == 0L)
		str = *saveptr;
	str += strspn(str, delim);
	if(*str == '\0')
	{
		*saveptr = str;
		return 0;
	}
	token = str;
	str = strpbrk(token, delim);
	if(str == 0)
		*saveptr = (char*)lfind(token, '\0');
	else
	{
		*str = '\0';
		*saveptr = str + 1;
	}
	return token;
}