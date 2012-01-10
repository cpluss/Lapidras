#ifndef STRING_H
#define STRING_H

#define NULL 0

int strlen(char *s);

//char *strtok(char *s, const char *delim);
void strcpy(char *dest, char *source);
int strcmp(char *s1, char *s2);

void strapp(char *dest, char *add);
void strapp_c(char *dest, char add);

int strchr(char *s, char c);
void strrev(char *dest, char *source);
int atoi(char *str);

//combine strings
int sprintf(char *s, char *format, ...);

//got nothing to do with strings, just math
int pow(int d, int n);

#endif
