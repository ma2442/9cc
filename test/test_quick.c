//!/bin/bash
int foo();
int bar(int, int);
int bar6(int, int, int, int, int, int);
int alloc4(int**, int, int, int, int);
int alloc4_2D(int***, int, int, int, int);
int printf(char* __format, ...);

// 四則演算、不等号, return
char main_t0() {
    return 5 + 10 * (+3 + 6 - 6) / 2 + (3 == 1 + 2) + (3 != 1 + 1) + (11 < 12) +
           (12 <= 12) + (12 > 11) + (12 >= 12);
}
char main_t1() {
    return -(-13) + -12 + (3 == 1 + 3) + (3 != 1 + 2) + (11 < 11) + (12 <= 11) +
           (11 > 11) + (12 >= 13);
}
char main_t2() {
    return (+3 + 5) / -2 * -(1) +
           (9 == 2 * 4 + (1 != (+3 + 5) / -2 * -(1) >= 5 > 1));
}
// ローカル変数、if for while, block
char main_t3() {
    int ident_tESt01;
    int _name__;
    int val;
    ident_tESt01 = 1 + 4;
    _name__ = 8 / 2;
    val = ident_tESt01 - _name__;
    int z;
    int c;
    return (z = c = 9 == 2 * 4 + (1 != (+3 + 5) / -2 * -(1) >= 5 > 1)) + val;
}
char main_t4() {
    int a;
    int i;
    a = -10;
    for (i = 1; i < 10; i = i + 1) a = a + i;
    return a;
}
char main_t5() {
    int i;
    i = 0;
    for (;;)
        if (i < 5)
            i = i + 1;
        else
            return i;
}
char main_t6() {
    int i;
    i = 3;
    if (i == 0) {
        i = 1;
        i = i + 1;
    } else if (i == 3) {
        i = 4;
        i = i + 1;
    } else {
        i = 7;
        i = i + 1;
    }
    return i;
}
char main_t7() {
    int i;
    int k;
    int res;
    int res_while;
    res_while = 0;
    i = 0;
    res = 0;
    while (i < 3) {
        for (k = 0; k < 5; k = k + 1) {
            res = res + 1;
            res = res + 2;
        }
        res_while = res_while + 1;
        i = i + 1;
    }
    return res;
}
//外部関数コール、関数定義&コール
int func1_t8() { return 1; }
int func2_t8(int x, int y) { return x + y; }
char main_t8() {
    bar6(1, 2, 3, 4, 5, 6);
    return func1_t8() + func2_t8(2 + 3, 20 + 30) + 4;
}
int fib_t9(int x) {
    if (x == 1) {
        return 1;
    } else if (x == 2) {
        return 1;
    } else {
        return fib_t9(x - 1) + fib_t9(x - 2);
    }
}
char main_t9() { return fib_t9(6); }
// int* 定義, 代入・アクセス
int swap_t10(int* x, int* y) {
    int tmp;
    tmp = *x;
    *x = *y;
    *y = tmp;
    return 1;
}
char main_t10() {
    int x = 1;
    int y = 100;
    swap_t10(&x, &y);
    return x - y;
}
// ポインタ加減算
char main_t11() {
    int** p;
    alloc4_2D(&p, 10, 20, 30, 40);
    int** q;
    q = 4 + p - 2;
    *q = *q + 1;
    return **q;
}
// sizeof
char main_t12() {
    int x;
    int* y;
    int a[3];
    return (sizeof(x + 3) != 4) + (sizeof(y + 3) != 8) + (sizeof(*y) != 4) +
           (sizeof(sizeof(1)) != 4) + (sizeof(1 * 1) != 4) + (sizeof(a) != 12);
}
// 配列
char main_t13() {
    int b[11];
    *b = 1;
    *(b + 10) = 2;
    int* q;
    q = b;
    int a[11];
    a[0] = 1;
    10 [a] = a[0] + 1;
    int* p;
    p = a;
    return (*q != 1) + (*(1 + q + 9) != 2) + ((p + 5)[2 + 3] != 2);
}
// ポインタを返す関数
int* func_t14(int* p) { return p + 1; }
char main_t14() {
    int a[3];
    a[0] = 10;
    a[1] = 20;
    a[2] = 30;
    return *(func_t14(a) + 1);
}
// グローバル変数
int x_t15;
int y_t15;
int func_t15(int x_t15) { x_t15 = 1; }
char main_t15() {
    x_t15 = 4;
    func_t15(0);
    if (x_t15 == 4) {
        y_t15 = 6 + x_t15;
        if ((y_t15 + x_t15 == 14)) {
            int x_t15;
            x_t15 = 1;
            return (x_t15 != 1);
        }
        return 1;
    }
    return 2;
}
int x_t16;
int* y_t16;
int a_t16[11];
int* p_t16;
char main_t16() {
    y_t16 = &x_t16;
    *y_t16 = 3;
    a_t16[0] = 1;
    10 [a_t16] = a_t16[0] + 1;
    p_t16 = a_t16;
    return ((p_t16 + 5)[2 + 3] != 2) + (x_t16 != 3);
}
// char
char x_t17;
char y_t17;
char func_t17(char z) { return z + 1; }
char main_t17() {
    char a;
    a = 4;
    x_t17 = 1;
    y_t17 = 10;
    return func_t17(239) + x_t17 + y_t17 + a;
}
char main_t18() {
    char x[3];
    x[0] = -1;
    x[1] = 2;
    int y;
    y = 4;
    return x[0] + y;
}
char main_t19() {
    char a[11];
    *a = 1;
    *(a + 10) = 2;
    char* p;
    p = a;
    return *p + *(1 + p + 9);
}
char* func_t20(char* p) { return p + 1; }
char main_t20() {
    char a[3];
    a[0] = 10;
    a[1] = 20;
    a[2] = 30;
    return *(func_t20(a) + 1);
}
// char overflow
char c_t21;
char func_t21(char x) {
    if (x == 0) return 256;
    return 1;
}
char main_t21() {
    c_t21 = 1;
    char x;
    x = 256;
    if (x == 0) c_t21 = 256;
    if (c_t21 == 0) {
        if (func_t21(256) == 0) return 0;
    }
    return 1;
}
// string literal
char* str_t22;
char main_t22() {
    printf("Hello, world! %d\n", 20220711);
    str_t22 = "abcdefg";
    char* df;
    df = "df";
    return (str_t22[1] != 98) + (sizeof("abc") != 3) + ("aceg"[2] != 101) +
           (*(df + 1) != 102);
}
// 101 == 'e', 102 == 'f'
// 4バイト型 ひとつ前の要素の書き換えの影響を受けないことを確認
int a_t23[3];
char main_t23() {
    a_t23[1] = 3;
    a_t23[0] = 1;
    if (a_t23[1] == 3) {
        int b[3];
        b[1] = 4;
        b[0] = 1;
        if (b[1] == 4)
            return 0;
        else
            return 1;
    }
    return 2;
}
int x_t24;
int y_t24;
char main_t24() {
    int a;
    int b;
    y_t24 = 3;
    x_t24 = 1;
    b = 4;
    a = 1;
    return (b == 4) + (y_t24 == 3);
}
// 二次元配列
char main_t25() {
    int a[2][3];
    int i;
    int j;
    *(*(a + 1) + 1) = 5;
    return *(*(a + 1) + 1);
}
char main_t26() {
    int a[2][3];
    int i;
    int j;
    for (i = 0; i < sizeof(a) / sizeof(a[0]); i = i + 1)
        for (j = 0; j < sizeof(a[0]) / sizeof(a[0][0]); j = j + 1) {
            a[i][j] = 10 * i + j + 1;
        }
    if (a[0][0] == 1)
        if (a[0][2] == 3)
            if (a[1][0] == 11)
                if (a[1][2] == 13) return 1;
    return 0;
}
int a_t27[2][3];
char main_t27() {
    int i;
    int j;
    for (i = 0; i < sizeof(a_t27) / sizeof(a_t27[0]); i = i + 1)
        for (j = 0; j < sizeof(a_t27[0]) / sizeof(a_t27[0][0]); j = j + 1) {
            a_t27[i][j] = 10 * i + j + 1;
        }
    if (a_t27[0][0] == 1)
        if (a_t27[0][2] == 3)
            if (a_t27[1][0] == 11)
                if (a_t27[1][2] == 13) return 1;
    return 0;
}
int main() {
    char c;
    if (main_t0() != 26) return 0;
    if (main_t1() != 1) return 1;
    if (main_t2() != 5) return 2;
    if (main_t3() != 2) return 3;
    if (main_t4() != 35) return 4;
    if (main_t5() != 5) return 5;
    if (main_t6() != 5) return 6;
    if (main_t7() != 45) return 7;
    if (main_t8() != 60) return 8;
    if (main_t9() != 8) return 9;
    if (main_t10() != 99) return 10;
    if (main_t11() != 32) return 11;
    if (main_t12() != 0) return 12;
    if (main_t13() != 0) return 13;
    if (main_t14() != 30) return 14;
    if (main_t15() != 0) return 15;
    if (main_t16() != 0) return 16;
    if (main_t17() != -1) return 17;
    if (main_t18() != 3) return 18;
    if (main_t19() != 3) return 19;
    if (main_t20() != 30) return 20;
    if (main_t21() != 0) return 21;
    if (main_t22() != 0) return 22;
    if (main_t23() != 0) return 23;
    if (main_t24() != 2) return 24;
    if (main_t25() != 5) return 25;
    if (main_t26() != 1) return 26;
    if (main_t27() != 1) return 27;
    return 255;
}
