// cc -c testfuncs.c
#include <stdio.h>
#include <stdlib.h>
int foo() { printf("foo!!!\n"); }

int bar(int a, int b) { printf("bar!!! %d + %d = %d\n", a, b, a + b); }

int bar6(int a, int b, int c, int d, int e, int f) {
    printf("bar6!!! %d + %d + %d + %d + %d + %d  = %d\n", a, b, c, d, e, f,
           a + b + c + d + e + f);
}

int alloc4(int **p, int x1, int x2, int x3, int x4) {
    *p = calloc(4, sizeof(int));
    (*p)[0] = x1;
    (*p)[1] = x2;
    (*p)[2] = x3;
    (*p)[3] = x4;
    printf("p: %d %d %d %d.\n", (*p)[0], (*p)[1], (*p)[2], (*p)[3]);
}

int alloc4_2D(int ***p, int x1, int x2, int x3, int x4) {
    int xs[4] = {x1, x2, x3, x4};
    *p = calloc(4, sizeof(int *));
    for (int i = 0; i < 4; i++) {
        (*p)[i] = calloc(4, sizeof(int));
        printf("p[%d]: ", i);
        for (int j = 0; j < 4; j++) {
            (*p)[i][j] = xs[i] + j;
            printf("%d ", (*p)[i][j]);
        }
        printf("\n");
    }
    int **q;
    q = *p + 2;
    printf("q=*p+2, **q : %d\n", **q);
    *q = *q + 1;
    printf("q=*(*p+2)+1, **q : %d\n", **q);
}