#ifndef HEADER_H
#define HEADER_H
// #include "gcc_defines.h"

#undef __x86_64__
#define __x86_64__
#undef __LP64__
#define __LP64__
#undef __ILP32__
#include <errno.h>
// int *__errno_location();
// #define errno (*__errno_location())

#define const
#define __restrict
#undef __USE_XOPEN2K8
#undef __USE_XOPEN2K
#include <string.h>
// #define NULL ((void *)0)
// size_t strlen(char *__s);
// int strncmp(char *__s1, char *__s2, size_t __n);
// char *strncpy(char *__dest, char *__src, size_t __n);
// char *strstr(char *__haystack, char *__needle);
// char *strcat(char *__dest, char *__src);
// typedef long unsigned int size_t;

#include <stdbool.h>
// #define bool _Bool
// #define true 1
// #define false 0

#define __builtin_va_list int ****************************************
#include <stdarg.h>

#undef __THROW
#define __THROW
#undef __attribute__
#define __attribute__(x)
#define __NO_CTYPE  // 入れ子型の定義回避用
#include <ctype.h>
// int isspace(int);
// int isdigit(int);
// int isalpha(int);

#define float int ****************************************
#define double int ****************************************
#define __ino_t_defined
#define __off_t_defined
#define __flags_
#define static
#define __inline
#define __extension__
#define __LOCK_ALIGNMENT
#undef __USE_MISC
#undef __USE_XOPEN
#undef __nonnull
#define __nonnull(x)
// #include <stdlib.h>

#include <stdio.h>
// typedef void FILE;
// /* Standard streams.  */
// extern FILE *stdin;  /* Standard input stream.  */
// extern FILE *stdout; /* Standard output stream.  */
// extern FILE *stderr; /* Standard error output stream.  */
// /* C89/C99 say they're macros.  Make them happy.  */
// #define stdin stdin
// #define stdout stdout
// #define stderr stderr
// int printf(char *, ...);
// int sprintf(char *__s, char *__format, ...);
// int fprintf(FILE *__stream, char *__fmt, ...);
// long int ftell(FILE *__stream);
// int fseek(FILE *__stream, long int __off, int __whence);
// FILE *fopen(char *__filename, char *__modes);
// size_t fread(void *__ptr, size_t __size, size_t __n, FILE *__stream);
// int fclose(FILE *__stream);

long long int strtoll(char *__nptr, char **__endptr, int __base);
void *calloc(size_t __nmemb, size_t __size);
void free(void *__ptr);
/* The possibilities for the third argument to `fseek'.
   These values should not be changed.  */
#define SEEK_SET 0 /* Seek from beginning of file.  */
#define SEEK_CUR 1 /* Seek from current position.  */
#define SEEK_END 2 /* Seek from end of file.  */

char *strerror(int __errnum);

void exit(int __status);

#include "9cc.h"
#endif  // HEADER_H
