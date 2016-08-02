//By Job Talle, used with permission.

#include <string.h>
#include <malloc.h>

#pragma once

//TODO see if still useful to have as header

inline int ccStrLength(char* str)
{
	int i;
	for(i=0; str[i]!=0; i++);
	return i;
}

inline void ccStrCopy(char* source, char* destination)
{
    int i;
	for(i=0; source[i]!=0; i++) {
		destination[i]=source[i];
	}
}

inline void ccStrTrimToSlash(char* str)
{
	int i;
	for(i=ccStrLength(str); str[i]!='\\'; i--);
	str[i+1]=0;
}


char* concat(char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
