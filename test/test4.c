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

int main() {
    if (main_t0() != 1) return 0;
    if (main_t1() != 0) return 1;
    if (main_t2() != 0) return 2;
    return 255;
}
