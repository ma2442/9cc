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
// _Bool
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
// && ||
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
// !x 否定 NOT
int main_t7() {
    if (!1) return 1;
    if (!2) return 2;
    if (!!0) return 3;
    if (!((1 == 1 && 1 + 1) == 0)) return 4;
    return !!1;
}

// 文字リテラル
int main_t8() {
    if (' ' != 32) return 1;
    if ('0' != 48) return 2;
    if ('@' != 64) return 3;
    if ('A' != 65) return 4;
    if ('a' != 97) return 5;
    if ('[' != 91) return 6;
    if ('~' != 126) return 7;
    if ('"' != 34) return 8;
    if ('?' != 63) return 9;
    if ('\"' != 34) return 10;
    if ('\'' != 39) return 11;
    if ('\?' != 63) return 12;
    if ('\\' != 92) return 13;
    if ('\0' != 0) return 14;
    if ('\a' != 7) return 15;
    if ('\b' != 8) return 16;
    if ('\t' != 9) return 17;
    if ('\n' != 10) return 18;
    if ('\r' != 13) return 19;
    return 0;
}

char func_t9(char c) { return c + 1; }

char main_t9() {
    if (func_t9(('0' + 1) * 2) != 99) return 1;
    if (sizeof('0') != 1) return 2;
    int a['['];
    a['\0'] = ' ';
    if (sizeof(a) != 91 * 4) return 3;
    if (a[0] != 32) return 4;
    return 0;
}

// do
int main_t10() {
    int i = 0;
    do i++;
    while (i < 0);
    if (i != 1) return 1;
    int j = 10;
    do {
        i++;
        j += 10;
    } while (i < 100);
    if (j != 1000) return 2;
    return 0;
}

// 三項演算子
int main_t11() {
    if ((1 ? 2 : 3) != 2) return 1;
    if ((0 ? 2 : 3) != 3) return 2;
    if ((1 ?: 3) != 1) return 3;
    if ((0 ?: 3) != 3) return 4;
    char c = 2 * 3 + 0 ? 'a' : 1 ? 'b' : 'c';
    if (c != 'a') return 5;
    char c2 = 0;
    c = '0' ? 0 ? c2 = '1' : '2' : '3';
    if (c != '2') return 6;
    if (c2 != '\0') return 7;
    return 0;
}

int main() {
    if (main_t0() != 1) return 0;
    if (main_t1()) return 1;
    if (main_t2()) return 2;
    if (main_t3()) return 3;
    if (main_t4()) return 4;
    if (main_t5()) return 5;
    if (main_t6()) return 6;
    if (!main_t7()) return 7;
    if (main_t8()) return 8;
    if (main_t9()) return 9;
    if (main_t10()) return 10;
    if (main_t11()) return 11;
    return 255;
}
