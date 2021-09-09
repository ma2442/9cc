// cc -c testfuncs.c
#include <stdio.h>
int foo() { printf("foo!!!\n"); }

int bar(int a, int b) { printf("bar!!! %d + %d = %d\n", a, b, a + b); }

int bar6(int a, int b, int c, int d, int e, int f) {
    printf("bar6!!! %d + %d + %d + %d + %d + %d  = %d\n", a, b, c, d, e, f,
           a + b + c + d + e + f);
}
