#include "test.h"

int main_t1() {
    int x = 0;
#ifdef TEST1
    return 1;
#endif

#ifdef TEST1
    return 2;
#else
    x = 1;
#endif
    if (x != 1) return 3;
    printf("%d\n", x);

#ifndef TEST1
    x = 2;
#endif
    if (x != 2) return 4;
    printf("%d\n", x);

#ifndef TEST1
    x = 3;
#else
    return 5;
#endif
    if (x != 3) return 7;
    printf("%d\n", x);

#define TEST1
#ifdef TEST1
    x = 4;
#endif
    if (x != 4) return 8;
    printf("%d\n", x);

#ifdef TEST1
    x = 5;
#else
    return 6;
#endif
    if (x != 5) return 9;
    printf("%d\n", x);

#ifndef TEST1
    return 10;
#endif

#ifndef TEST1
    return 11;
#else
    x = 6;
#endif
    if (x != 6) return 12;
    printf("%d\n", x);

    return 0;
}

#define ZERO() 0
#define MINUS_ONE(x) x - 1
#define FUNCNAME func_t2

int func_t2() { return 15; }

int main_t2() {
    int ZERO = 10;
    if (ZERO()) return 0;
    if (ZERO != 10) return 1;
    int x = 10;
    if (MINUS_ONE(10) != 9) return 2;
    if (MINUS_ONE() != -1) return 3;
    if (TWO != 2) return 4;
    if (FUNCNAME() != 15) return 5;
    return 0;
}

#define abc "def"
#define STRING(x) "abc"[0] == 'a' && x[0] == '1' && abc[0] == 'd'

int main_t3() {
    if (abc[0] != 'd') return 1;
    if (!(STRING("123"))) return 2;
    return 0;
}

#define A_t4(x) 10 - x
#define B_t4(y) A_t4(y) + A_t4(y)

int main_t4() {
    if (A_t4(1) != 9) return 1;
    if (B_t4(1) != 18) return 2;
    return 0;
}

int main_t5() {
    int x = 0;
#if defined A_t4 && A_t4(1) < 100
    x = 1;
#endif
    if (x != 1) return 1;
#if !defined A_t4
    return 2;
#else
    x = 2;
#endif
    if (x != 2) return 2;
#if !defined(NOT_DEFINED) && !defined STRING
    return 3;
#elif defined(STRING) && A_t4(1) > 0
    x = 3;
#endif
    if (x != 3) return 3;

    return 0;
}

#define __asm__(x, y)
int main_t6() __asm__((10), (20)) { return 0; }

int main() {
    if (main_t1()) return 1;
    if (main_t2()) return 2;
    if (main_t3()) return 3;
    if (main_t4()) return 4;
    if (main_t5()) return 5;
    if (main_t6()) return 6;
    return 255;
}