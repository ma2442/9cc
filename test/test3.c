// グローバル変数 ~ string literal、二次元配列、変数宣言と同時代入まで
// グローバル変数
int x_t0;
int main_t0() {
    x_t0 = 5;
    return x_t0;
}
int x_t1;
int main_t1() {
    x_t1 = 5;
    int x_t1;
    x_t1 = 1;
    return x_t1;
}
int x_t2;
int y_t2;
int main_t2() {
    x_t2 = 4;
    y_t2 = 6 + x_t2;
    return y_t2 + x_t2;
}
int x_t3;
int func_t3() {
    x_t3 = 5;
    return 0;
}
int main_t3() {
    func_t3();
    return x_t3;
}
int x_t4;
int func_t4(int x_t4) {
    x_t4 = 1;
    return 0;
}
int main_t4() {
    x_t4 = 5;
    func_t4(1);
    return x_t4;
}
int x_t5;
int* y_t5;
int main_t5() {
    y_t5 = &x_t5;
    *y_t5 = 3;
    return x_t5;
}
int** p_t6;
int main_t6() {
    alloc4_2D(&p_t6, 10, 20, 30, 40);
    int** q;
    q = 4 + p_t6 - 2;
    *q = *q + 1;
    return **q;
}
int a_t7[11];
int main_t7() {
    *a_t7 = 1;
    *(a_t7 + 10) = 2;
    int* p;
    p = a_t7;
    return *p + *(1 + p + 9);
}
int a_t8[11];
int main_t8() {
    a_t8[0] = 1;
    10 [a_t8] = a_t8[0] + 1;
    int* p;
    p = a_t8;
    return (p + 5)[2 + 3];
}
// char
int main_t9() {
    char c;
    c = 1;
    return c;
}
char x_t10;
char y_t10;
char func_t10(char z) { return z + 1; }
int main_t10() {
    char a;
    a = 4;
    x_t10 = 1;
    y_t10 = 10;
    return func_t10(239) + x_t10 + y_t10 + a;
}
int main_t11() {
    char x[3];
    x[0] = -1;
    x[1] = 2;
    int y;
    y = 4;
    return x[0] + y;
}
char x_t12[3];
int main_t12() {
    x_t12[0] = -1;
    x_t12[1] = 2;
    x_t12[2] = 3;
    return x_t12[1];
}
char x_t13[3];
int main_t13() {
    x_t13[0] = -1;
    x_t13[1] = 2;
    x_t13[2] = 3;
    return x_t13[2];
}
int main_t14() {
    char x;
    return sizeof(x);
}
int main_t15() {
    char a[11];
    *a = 1;
    *(a + 10) = 2;
    char* p;
    p = a;
    return *p + *(1 + p + 9);
}
char* func_t16(char* p) { return p + 1; }
int main_t16() {
    char a[3];
    a[0] = 10;
    a[1] = 20;
    a[2] = 30;
    return *(func_t16(a) + 1);
}
char a_t17[11];
char* p_t17;
int main_t17() {
    a_t17[0] = 1;
    10 [a_t17] = a_t17[0] + 1;
    p_t17 = a_t17;
    return (p_t17 + 5)[2 + 3];
}
// char overflow
int main_t18() {
    char x;
    x = 256;
    if (x)
        return 1;
    else
        return 0;
}
char func_t19() { return 256; }
int main_t19() {
    if (func_t19())
        return 1;
    else
        return 0;
}
int func_t20(char x) { return x; }
int main_t20() {
    if (func_t20(256))
        return 1;
    else
        return 0;
}
char x_t21;
int main_t21() {
    x_t21 = 256;
    if (x_t21)
        return 1;
    else
        return 0;
}
// string literal
int main_t22() {
    char* str;
    str = "abcdefg";
    if (str[1] == 98) return 1;
    return 0;
}
int main_t23() { return sizeof("abc"); }
char* str_t24;
int main_t24() {
    str_t24 = "df";
    return *(str_t24 + 1);
}
int main_t25() { return "aceg"[2]; }
// == 'e'
int main_t26() {
    printf("Hello, world! %d\n", 20220711);
    return 0;
}
// INT型 4byte ひとつ前の要素の書き換えの影響を受けないことを確認
int main_t27() {
    int a[3];
    a[1] = 3;
    a[0] = 1;
    return a[1];
}
int a_t28[3];
int main_t28() {
    a_t28[1] = 3;
    a_t28[0] = 1;
    return a_t28[1];
}
int main_t29() {
    int x;
    int y;
    y = 3;
    x = 1;
    return y;
}
int x_t30;
int y_t30;
int main_t30() {
    y_t30 = 3;
    x_t30 = 1;
    return y_t30;
}
// 二次元配列
int main_t31() {
    int a[2][5];
    return sizeof(a);
}
int main_t32() {
    int a[4][5];
    return sizeof(a[0]);
}
int main_t33() {
    int a[4][5];
    return sizeof(a[0][0]);
}
int main_t34() {
    int a[2][3];
    int i;
    int j;
    *(*(a + 1) + 1) = 5;
    return *(*(a + 1) + 1);
}
int main_t35() {
    int a[2][3];
    int i;
    int j;
    for (i = 0; i < sizeof(a) / sizeof(a[0]); i = i + 1)
        for (j = 0; j < sizeof(a[0]) / sizeof(a[0][0]); j = j + 1) {
            a[i][j] = 10 * i + j + 1;
        }
    return a[0][0];
}
int main_t36() {
    int a[2][3];
    int i;
    int j;
    for (i = 0; i < sizeof(a) / sizeof(a[0]); i = i + 1)
        for (j = 0; j < sizeof(a[0]) / sizeof(a[0][0]); j = j + 1) {
            a[i][j] = 10 * i + j + 1;
        }
    return a[0][1];
}
int main_t37() {
    int a[2][3];
    int i;
    int j;
    for (i = 0; i < sizeof(a) / sizeof(a[0]); i = i + 1)
        for (j = 0; j < sizeof(a[0]) / sizeof(a[0][0]); j = j + 1) {
            a[i][j] = 10 * i + j + 1;
        }
    return a[0][2];
}
int main_t38() {
    int a[2][3];
    int i;
    int j;
    for (i = 0; i < sizeof(a) / sizeof(a[0]); i = i + 1)
        for (j = 0; j < sizeof(a[0]) / sizeof(a[0][0]); j = j + 1) {
            a[i][j] = 10 * i + j + 1;
        }
    return a[1][0];
}
int main_t39() {
    int a[2][3];
    int i;
    int j;
    for (i = 0; i < sizeof(a) / sizeof(a[0]); i = i + 1)
        for (j = 0; j < sizeof(a[0]) / sizeof(a[0][0]); j = j + 1) {
            a[i][j] = 10 * i + j + 1;
        }
    return a[1][1];
}
int main_t40() {
    int a[2][3];
    int i;
    int j;
    for (i = 0; i < sizeof(a) / sizeof(a[0]); i = i + 1)
        for (j = 0; j < sizeof(a[0]) / sizeof(a[0][0]); j = j + 1) {
            a[i][j] = 10 * i + j + 1;
        }
    return a[1][2];
}
int a_t41[2][3];
int main_t41() {
    int i;
    int j;
    for (i = 0; i < sizeof(a_t41) / sizeof(a_t41[0]); i = i + 1)
        for (j = 0; j < sizeof(a_t41[0]) / sizeof(a_t41[0][0]); j = j + 1) {
            a_t41[i][j] = 10 * i + j + 1;
        }
    return a_t41[0][0];
}
int a_t42[2][3];
int main_t42() {
    int i;
    int j;
    for (i = 0; i < sizeof(a_t42) / sizeof(a_t42[0]); i = i + 1)
        for (j = 0; j < sizeof(a_t42[0]) / sizeof(a_t42[0][0]); j = j + 1) {
            a_t42[i][j] = 10 * i + j + 1;
        }
    return a_t42[0][1];
}
int a_t43[2][3];
int main_t43() {
    int i;
    int j;
    for (i = 0; i < sizeof(a_t43) / sizeof(a_t43[0]); i = i + 1)
        for (j = 0; j < sizeof(a_t43[0]) / sizeof(a_t43[0][0]); j = j + 1) {
            a_t43[i][j] = 10 * i + j + 1;
        }
    return a_t43[0][2];
}
int a_t44[2][3];
int main_t44() {
    int i;
    int j;
    for (i = 0; i < sizeof(a_t44) / sizeof(a_t44[0]); i = i + 1)
        for (j = 0; j < sizeof(a_t44[0]) / sizeof(a_t44[0][0]); j = j + 1) {
            a_t44[i][j] = 10 * i + j + 1;
        }
    return a_t44[1][0];
}
int a_t45[2][3];
int main_t45() {
    int i;
    int j;
    for (i = 0; i < sizeof(a_t45) / sizeof(a_t45[0]); i = i + 1)
        for (j = 0; j < sizeof(a_t45[0]) / sizeof(a_t45[0][0]); j = j + 1) {
            a_t45[i][j] = 10 * i + j + 1;
        }
    return a_t45[1][1];
}
int a_t46[2][3];
int main_t46() {
    int i;
    int j;
    for (i = 0; i < sizeof(a_t46) / sizeof(a_t46[0]); i = i + 1)
        for (j = 0; j < sizeof(a_t46[0]) / sizeof(a_t46[0][0]); j = j + 1) {
            a_t46[i][j] = 10 * i + j + 1;
        }
    return a_t46[1][2];
}
// 変数宣言と同時に代入
int main_t47() {
    int x = 1;
    if (x != 1) return 1;
    return 0;
}
int main_t48() {
    int x;
    int y = x = 1 + 3;
    if (y != 4) return 1;
    return 0;
}
int main_t49() {
    int x;
    int y = 2 + (x = 1);
    if (y != 3) return 1;
    return 0;
}
int main_t50() {
    int a[2];
    int* p = a;
    a[0] = 1;
    a[1] = 2;
    if (*p != 1) return 1;
    return 0;
}
int main() {
    if (main_t0() != 5) return 0;
    if (main_t1() != 1) return 1;
    if (main_t2() != 14) return 2;
    if (main_t3() != 5) return 3;
    if (main_t4() != 5) return 4;
    if (main_t5() != 3) return 5;
    if (main_t6() != 32) return 6;
    if (main_t7() != 3) return 7;
    if (main_t8() != 2) return 8;
    if (main_t9() != 1) return 9;
    if (main_t10() != -1) return 10;
    if (main_t11() != 3) return 11;
    if (main_t12() != 2) return 12;
    if (main_t13() != 3) return 13;
    if (main_t14() != 1) return 14;
    if (main_t15() != 3) return 15;
    if (main_t16() != 30) return 16;
    if (main_t17() != 2) return 17;
    if (main_t18() != 0) return 18;
    if (main_t19() != 0) return 19;
    if (main_t20() != 0) return 20;
    if (main_t21() != 0) return 21;
    if (main_t22() != 1) return 22;
    if (main_t23() != 3) return 23;
    if (main_t24() != 102) return 24;
    if (main_t25() != 101) return 25;
    if (main_t26() != 0) return 26;
    if (main_t27() != 3) return 27;
    if (main_t28() != 3) return 28;
    if (main_t29() != 3) return 29;
    if (main_t30() != 3) return 30;
    if (main_t31() != 40) return 31;
    if (main_t32() != 20) return 32;
    if (main_t33() != 4) return 33;
    if (main_t34() != 5) return 34;
    if (main_t35() != 1) return 35;
    if (main_t36() != 2) return 36;
    if (main_t37() != 3) return 37;
    if (main_t38() != 11) return 38;
    if (main_t39() != 12) return 39;
    if (main_t40() != 13) return 40;
    if (main_t41() != 1) return 41;
    if (main_t42() != 2) return 42;
    if (main_t43() != 3) return 43;
    if (main_t44() != 11) return 44;
    if (main_t45() != 12) return 45;
    if (main_t46() != 13) return 46;
    if (main_t47() != 0) return 47;
    if (main_t48() != 0) return 48;
    if (main_t49() != 0) return 49;
    if (main_t50() != 0) return 50;
    return 255;
}
