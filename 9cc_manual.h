#define bool _Bool
#define true 1
#define false 0
#define NULL 0
typedef long unsigned int size_t;
int printf(char *, ...);
int sprintf(char *__s, char *__format, ...);
size_t strlen(char *__s);
int strncmp(char *__s1, char *__s2, size_t __n);
void *calloc(size_t __nmemb, size_t __size);
void free(void *__ptr);
#include "9cc.h"