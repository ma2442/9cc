#define bool _Bool
#define true 1
#define false 0
#define NULL 0
typedef void FILE;
typedef long unsigned int size_t;
int printf(char *, ...);
int sprintf(char *__s, char *__format, ...);
int fprintf(FILE *__stream, char *__fmt, ...);

/* Standard streams.  */
extern FILE *stdin;  /* Standard input stream.  */
extern FILE *stdout; /* Standard output stream.  */
extern FILE *stderr; /* Standard error output stream.  */
/* C89/C99 say they're macros.  Make them happy.  */
#define stdin stdin
#define stdout stdout
#define stderr stderr

size_t strlen(char *__s);
int strncmp(char *__s1, char *__s2, size_t __n);
char *strncpy(char *__dest, char *__src, size_t __n);
char *strstr(char *__haystack, char *__needle);
char *strcat(char *__dest, char *__src);
long long int strtoll(char *__nptr, char **__endptr, int __base);
void *calloc(size_t __nmemb, size_t __size);
void free(void *__ptr);
/* The possibilities for the third argument to `fseek'.
   These values should not be changed.  */
#define SEEK_SET 0 /* Seek from beginning of file.  */
#define SEEK_CUR 1 /* Seek from current position.  */
#define SEEK_END 2 /* Seek from end of file.  */
long int ftell(FILE *__stream);
int fseek(FILE *__stream, long int __off, int __whence);
char *strerror(int __errnum);
FILE *fopen(char *__filename, char *__modes);
size_t fread(void *__ptr, size_t __size, size_t __n, FILE *__stream);
int fclose(FILE *__stream);
int *__errno_location();
// #define errno (*__errno_location())
void exit(int __status);
int isspace(int);
int isdigit(int);
int isalpha(int);

#include "9cc.h"