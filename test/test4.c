// ローカル変数スタック長可変化テスト
int main_t0() {
    int a[100];
    int i;
    for (i = 0; i < sizeof(a) / sizeof(a[0]); i = i + 1) {
        a[i] = i;
    }
    return a[1];
}
// ポインタのインクリメント, デクリメント
int main_t1() {
    int a[5];
    int *p = a;
    int i = 0;
    int j = 0;
    if (i++ == 1) return 1;
    if (0 == ++j) return 2;
    for (i = 0; i < 5; i++) {
        *p++ = i;
        if (a[i] != i) return 3;
    }
    if (i != 5) return 4;
    p = a;
    for (j = 0; j > -5; j--) {
        *(++p - 1) = j;
        if (a[-j] != j) return 5;
    }
    p = a;
    *p = 1;
    (*p)++;
    ++(*p);
    a[0]++;
    ++a[0];
    if (*p != 5) return 6;
    return (j != -5);
}

// 2重ポインタのインクリメント･デクリメント
int main_t2() {
    // int a[3][10];
    int **q;
    alloc4_2D(&q, 10, 20, 30, 40);
    int **p = q;
    if (*((*p)++) != 10) return 1;
    if (12 != *((*p)++)) return 2;
    if (*(++*p) != 16) return 3;
    (*p)--;
    --*p;
    --*p;
    if (*(*p++) != 10) return 4;
    if (30 != *((*++p)++)) return 5;
    if (--(*--*p) != 29) return 5;
    return 0;
}

// 複合代入 += -= *= /=
int main_t3() {
    int x = 0;
    if (5 != (x += 5)) return 1;
    x -= 1 + 1;
    x *= 30;
    int y;
    if ((x /= 10 - 5) != 18) return 2;
    x += x += 2;
    if (x != 40) return 3;
    int a[10];
    int *p = a;
    int i;
    for (i = 0; i < 10; i += 2) {
        *p = i;
        p += 2;
    }
    for (i = 0; i < 10; i += 2) {
        if (a[i] != i) return 4;
    }
    return 0;
}

// 余り mod % %=
int main_t4() {
    if (5 % 2 != 1) return 1;
    if (12 % 3 != 0) return 2;
    if (100 + 21 % 10 != 101) return 3;
    if ((100 + 21) % 10 != 1) return 4;
    if (123 % 100 % 10 != 3) return 5;
    if (123 / 100 % 10 != 1) return 6;
    int x = 123;
    if (x %= 10 != 3) return 7;
    return 0;
}

_Bool main_t5() {
    if (1 != (100 == 100)) return 1;
    _Bool b = 123;
    if (b != 1) return 2;
    b = 0;
    if (b != 0) return 3;
    b++;
    b++;
    b++;
    if (b != 1) return 4;
    return 0;
}

int main_t6() {
    int x;
    if ((1 && 1) == 0) return 1;
    if (1 && 0) return 2;
    if ((1 || 0) == 0) return 3;
    if (0 || 0) return 4;
    if ((1 == 1 && 1 + 1) == 0) return 1;
    if (((x = 0) || (x = 1)) == 0) return 2;
    if ((0 || 2 && 3) == 0) return 3;
    if ((0 && 1 || 2) == 0) return 4;
    return 0;
}

int main() {
    if (main_t0() != 1) return 0;
    if (main_t1() != 0) return 1;
    if (main_t2() != 0) return 2;
    if (main_t3() != 0) return 3;
    if (main_t4() != 0) return 4;
    if (main_t5() != 0) return 5;
    if (main_t6() != 0) return 6;
    return 255;
}
