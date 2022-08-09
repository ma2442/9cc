#define bool _Bool
#define true 1
#define false 0
#define NULL 0
typedef long unsigned int size_t;
int printf(char *, ...);
int sprintf(char *__s, char *__format, ...);
size_t strlen(char *__s);
void *calloc(size_t __nmemb, size_t __size);
void free(void *__ptr);
#include "9cc.h"