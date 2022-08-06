//関数コール ~ 配列
int foo();
int bar(int, int);
int bar6(int, int, int, int, int, int);
int alloc4(int**, int, int, int, int);
extern int alloc4_2D(int***, int, int, int, int);
int bar(int, int);
int alloc4(int**, int, int, int, int);

//外部関数コール
int main_t0() {
    foo();
    return 1;
}
int main_t1() {
    bar(1, 2);
    return 2;
}
int main_t2() {
    bar6(1, 2, 3, 4, 5, 6);
    return 3;
}
//関数定義&コール
int func1_t3() { return 1; }
int main_t3() { return func1_t3(); }
int func2_t4(int x) { return x + 1; }
int main_t4() { return func2_t4(1); }
int func2_t5(int x, int y) { return x + y; }
int main_t5() { return func2_t5(1, 3) + 4; }
int func_t6(int x) {
    if (x == 1) {
        return 10;
    } else {
        return 20;
    }
}
int main_t6() { return func_t6(2); }
int func_t7(int x) { return x + 1; }
int main_t7() {
    int y;
    y = 1;
    return func_t7(2) + y;
}
int func_t8(int x) { return x + 1; }
int func2_t8(int y) { return y * 2; }
int main_t8() {
    int a;
    int b;
    a = func_t8(2);
    b = func2_t8(3);
    return a + b;
}
int func_t9(int x) { return x + 1; }
int func2_t9(int y) { return y * 2; }
int main_t9() { return func_t9(1) + func2_t9(2); }
int acc_t10(int x) {
    if (x == 1) {
        return 1;
    } else {
        return x + acc_t10(x - 1);
    }
}
int main_t10() { return acc_t10(3); }
int fib_t11(int x) {
    if (x == 1) {
        return 1;
    } else if (x == 2) {
        return 1;
    } else {
        return fib_t11(x - 1) + fib_t11(x - 2);
    }
}
int main_t11() { return fib_t11(1); }
int fib_t12(int x) {
    if (x == 1) {
        return 1;
    } else if (x == 2) {
        return 1;
    } else {
        return fib_t12(x - 1) + fib_t12(x - 2);
    }
}
int main_t12() { return fib_t12(6); }
// 関数コール (引数2の計算により引数1が書き換わらない =
// 全引数のpopがまとめて最後に行われていることの確認)
int func_t13(int x, int y) { return x + y; }
int main_t13() { return func_t13(2 + 3, 20 + 30); }
//単項&、*
int main_t14() {
    int x;
    int* y;
    x = 3;
    y = &x;
    return *y;
}
int main_t15() {
    int x;
    int y;
    int* z;
    x = 4;
    y = 5;
    z = &y - 1;
    return *z;
}
// int* 定義
int main_t16() {
    int x;
    int* y;
    y = &x;
    *y = 3;
    return x;
}
int swap_t17(int* x, int* y) {
    int tmp;
    tmp = *x;
    *x = *y;
    *y = tmp;
    return 1;
}
int main_t17() {
    int x;
    int y;
    x = 1;
    y = 100;
    swap_t17(&x, &y);
    return x - y;
}
// ポインタ加減算
int main_t18() {
    int* p;
    alloc4(&p, 10, 20, 30, 40);
    int* q;
    q = p;
    return *q;
}
int main_t19() {
    int* p;
    alloc4(&p, 10, 20, 30, 40);
    int* q;
    q = p + 2;
    return *q;
}
int main_t20() {
    int* p;
    alloc4(&p, 10, 20, 30, 40);
    int* q;
    q = 3 + p;
    return *q;
}
int main_t21() {
    int* p;
    alloc4(&p, 10, 20, 30, 40);
    int* q;
    q = 3 + p - 1;
    return *q;
}
int main_t22() {
    int** p;
    alloc4_2D(&p, 10, 20, 30, 40);
    int** q;
    q = p;
    return **q;
}
int main_t23() {
    int** p;
    alloc4_2D(&p, 10, 20, 30, 40);
    int** q;
    q = p + 2;
    return **q;
}
int main_t24() {
    int** p;
    alloc4_2D(&p, 10, 20, 30, 40);
    int** q;
    q = 4 + p - 2;
    *q = *q + 1;
    return **q;
}
// sizeof
int main_t25() {
    int x;
    return sizeof(x);
}
int main_t26() {
    int x;
    return sizeof(x + 3);
}
int main_t27() {
    int* y;
    return sizeof(y);
}
int main_t28() {
    int* y;
    return sizeof(y + 3);
}
int main_t29() {
    int* y;
    return sizeof(*y);
}
int main_t30() {
    int* y;
    return sizeof(1);
}
int main_t31() {
    int* y;
    return sizeof(sizeof(1));
}
int main_t32() {
    int* y;
    return sizeof(1 == 1) == 1;
}
int main_t33() {
    int* y;
    return sizeof(1 * 1);
}
int main_t33_added() {
    if (sizeof(int*) != 8) return 1;
    return 0;
}
// 配列
int main_t34() {
    int a[3];
    return sizeof(a);
}
int main_t35() {
    int a[1];
    *a = 1;
    int* p;
    p = a;
    return *p;
}
int main_t36() {
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int* p;
    p = a;
    return *p;
}
int main_t37() {
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int* p;
    p = a;
    return *(p + 1);
}
int main_t38() {
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int* p;
    p = a;
    return *p + *(1 + p);
}
int main_t39() {
    int a[11];
    *a = 1;
    *(a + 10) = 2;
    int* p;
    p = a;
    return *p;
}
int main_t40() {
    int a[11];
    *a = 1;
    *(a + 10) = 2;
    int* p;
    p = a;
    return *(1 + p + 9);
}
int main_t41() {
    int a[11];
    a[0] = 1;
    a[10] = 2;
    int* p;
    p = a;
    return p[0];
}
int main_t42() {
    int a[11];
    a[0] = 1;
    10 [a] = a[0] + 1;
    int* p;
    p = a;
    return (p + 5)[2 + 3];
}
// ポインタを返す関数
int* func_t43(int* p) { return p + 1; }
int main_t43() {
    int a[3];
    a[0] = 10;
    a[1] = 20;
    a[2] = 30;
    return *(func_t43(a) + 1);
}
int main() {
    if (main_t0() != 1) return 0;
    if (main_t1() != 2) return 1;
    if (main_t2() != 3) return 2;
    if (main_t3() != 1) return 3;
    if (main_t4() != 2) return 4;
    if (main_t5() != 8) return 5;
    if (main_t6() != 20) return 6;
    if (main_t7() != 4) return 7;
    if (main_t8() != 9) return 8;
    if (main_t9() != 6) return 9;
    if (main_t10() != 6) return 10;
    if (main_t11() != 1) return 11;
    if (main_t12() != 8) return 12;
    if (main_t13() != 55) return 13;
    if (main_t14() != 3) return 14;
    if (main_t15() != 4) return 15;
    if (main_t16() != 3) return 16;
    if (main_t17() != 99) return 17;
    if (main_t18() != 10) return 18;
    if (main_t19() != 30) return 19;
    if (main_t20() != 40) return 20;
    if (main_t21() != 30) return 21;
    if (main_t22() != 10) return 22;
    if (main_t23() != 30) return 23;
    if (main_t24() != 32) return 24;
    if (main_t25() != 4) return 25;
    if (main_t26() != 4) return 26;
    if (main_t27() != 8) return 27;
    if (main_t28() != 8) return 28;
    if (main_t29() != 4) return 29;
    if (main_t30() != 4) return 30;
    if (main_t31() != 4) return 31;
    if (main_t32() != 1) return 32;
    if (main_t33() != 4) return 33;
    if (main_t33_added()) return 33;
    if (main_t34() != 12) return 34;
    if (main_t35() != 1) return 35;
    if (main_t36() != 1) return 36;
    if (main_t37() != 2) return 37;
    if (main_t38() != 3) return 38;
    if (main_t39() != 1) return 39;
    if (main_t40() != 2) return 40;
    if (main_t41() != 1) return 41;
    if (main_t42() != 2) return 42;
    if (main_t43() != 30) return 43;
    return 255;
}
