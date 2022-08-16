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

// #if ネスト
int main_t1b() {
#if 0
return 1;
#if 1
return 2;
#elif 2
return 3;
#else
return 4;
#endif
return 5;
#endif
    return 0;
}

#define ZERO() 0
#define MINUS_ONE(x) x - 1
#define FUNCNAME func_t2
#define MINUS_ONE2 (-1)

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
    if (MINUS_ONE2 != -1) return 6;
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

#define __x86_64__

int main_t5a() {
    int x = 1;
#ifdef __x86_64__
    x *= 2;
#else
    return 3;
#endif
    if (x != 2) return 4;
    return 0;
}

int main_t5b() {
    int x = 1;
#ifdef __x86_64__
    x *= 2;
#elif 0
    return 3;
#endif
    if (x != 2) return 4;
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

#define MULTIPLE_LINES 1 + 2
#define MULTIPLE_LINES2(x,\  
 y) \
    \ 
 x \  
 + y

#define M_t9 1##2

int main_t9() {
    if (M_t9 != 12) return 9;
    return 0;
}

#define MA_t10 a
#define MB_t10 b
#define MC_t10(x) M##x##_t10 + 1

int main_t10() {
    int a = 10;
    int b = 20;
    if (MC_t10(A) != 11) return 1;
    if (MC_t10(B) != 21) return 2;
    return 0;
}

int main_t11() {
    int x = 0;
#if UNDEFINED == 0
    x = 1;
#endif
    if (x != 1) return 1;
#if UNDEFINED == 1
    return 2;
#endif
#if 0
#elif UNDEFINED == 0
    x = 2;
#endif
    if (x != 2) return 2;
    return 0;
}

int main_t12() {
    int x = 0;
#define DEFINED 1
#ifdef DEFINED
    x = 1;
#endif
    if (x != 1) return 1;
#undef DEFINED
#ifdef DEFINED
    return 2;
#endif
#define DEFINED 2
#if DEFINED == 2
    x = 2;
#endif
    if (x != 2) return 2;
    return 0;
}

#define MA_t13 MB_t13
#define MB_t13 MC_t13
#define MC_t13 MD_t13
#define MD_t13 d

int main_t13() {
    int d = 123;
    if (MA_t13 != 123) return 1;
    return 0;
}

#define m1(x) m2(x)(m3(x))
#define m2(x) m##x
#define m3(x) x + 2
#define m4(x) 10 + x
int main_t14() {
    if (m1(4) != 10 + 4 + 2) return 1;
    return 0;
}

#undef m1
#define m1(x, y) f2(x)(m##x##1(y))(10)
#define f2(x) n##x
#define m41(x) x
#define n4(x) m##x
#define mm41(x) f##x
#define n10 100

int main_t15() {
    if (m1(4, 2) != 100) return 1;
    return 0;
}

#undef m1
#define m1(x) m##x
#define m2(x) m##x
#define m3(x) m##x
// define はみだし再帰確認
int main_t16() {
    // m1 = 10
    int m1(1) = 10;
    // m1 != 10 ?
    if (m1(1)(1) != 10) return 1;
    // m3 = 30
    int m3(3) = 30;
    // m3 != 30 ?
    if (m2(1)(2)(1)(1)(2)(3) != 30) return 2;
    return 0;
}

// define マクロ展開内 再帰ロック確認
int main_t17() {
#define lock_recur lock_recur
    int lock_recur = 40;
    if (lock_recur != 40) return 1;
#undef lock_recur
#define lock_recur lock_recur + 1
    if (lock_recur != 41) return 2;
    return 0;
}

int main() {
    if (main_t1()) return 1;
    if (main_t1b()) return 1;
    if (main_t2()) return 2;
    if (main_t3()) return 3;
    if (main_t4()) return 4;
    if (main_t5a()) return 51;
    if (main_t5b()) return 52;
    if (main_t5()) return 5;
    if (main_t6()) return 6;
    if (MULTIPLE_LINES != 3) return 7;
    if (MULTIPLE_LINES2(3, 4) != 7) return 8;
    if (main_t9()) return 9;
    if (main_t10()) return 10;
    if (main_t11()) return 11;
    if (main_t12()) return 12;
    if (main_t13()) return 13;
    if (main_t14()) return 14;
    if (main_t15()) return 15;
    if (main_t16()) return 16;
    if (main_t17()) return 17;
    return 255;
}