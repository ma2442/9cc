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
    if (0 && 0) return 1;
    if (1 && 0) return 2;
    if (0 && 1) return 3;
    if ((1 && 1) != 1) return 4;
    if (0 || 0) return 5;
    if ((1 || 0) != 1) return 6;
    if ((0 || 1) != 1) return 7;
    if ((1 || 1) != 1) return 8;
    if ((1 == 1 && 1 + 1) != 1) return 9;
    if (((x = 0) || (x = 1)) != 1) return 10;
    if ((0 || 2 && 3) != 1) return 11;
    if ((0 && 1 || 2) != 1) return 12;
    return 0;
}
// !x 否定 NOT
int main_t7() {
    if (!2) return 1;
    if (!!0) return 2;
    if (!!999 != 1) return 3;
    if (!((1 == 1 && 1 + 1) == !0)) return 4;
    return 0;
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
    if (sizeof('0') != 4) return 2;
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

// switch case 文
int main_t12() {
    int i = 1;
    int x = 0;
    switch (i) {
        x++;
        case 1:
            x += 10;
        case 2:
        case 3:
            x += 100;
    }
    if (x != 110) return 1;
    i = 2;
    x = 0;
    switch (i) {
        x++;
        case 1:
            x += 10;
        case 2:
        case 3:
            x += 100;
    }
    if (x != 100) return 2;
    return 0;
}

int main_t13() {
    int i = -100;
    int x = 0;
    switch (i) {
        x++;
        case 1:
            x += 10;
        case 2:
        case 3:
            x += 100;
    }
    if (x != 0) return 3;
    i = 0;
    x = 0;
    switch (i)
    case 0:
        x = 10;
    if (x != 10) return 4;
    return 0;
}

// default 文
int main_t14() {
    int i = 1;
    int x = 0;
    switch (i) {
        case 1:
            x += 1;
            x++;
        default:
            x += 10;
            x += 10;
    }
    if (x != 22) return 1;
    i = 2;
    x = 0;
    switch (i) {
        case 1:
            x += 1;
            x++;
        default:
            x += 10;
            x += 10;
    }
    if (x != 20) return 2;
    return 0;
}

int main_t15() {
    int i = -100;
    int x = 0;
    switch (i) {
        default:
        case 1:
            x += 1;
            x++;
            x += 10;
            x += 10;
    }
    if (x != 22) return 1;
    return 0;
}

int main_t16() {
    int i = 120;
    enum { A = 5 };
    int x = 0;
    switch (i / 2) {
        x += 1;
        case 0 ?:
            A * 12 : x++;
        default:
            x += 10;
    }
    if (x != 11) return 1;
    return 0;
}

// switch{while}
int main_t17() {
    int i = 0;
    int x = 0;
    switch (i) {
        case 0:
            while (0) {
                case 2:
                    x += 10;
            }
    }
    if (x != 0) return 1;
    i = 2;
    x = 0;
    switch (i) {
        case 0:
            while (0) {
                case 2:
                    x += 10;
            }
    }
    if (x != 10) return 2;
    return 0;
}

// switch{switch}
int main_t18() {
    int x = 0;
    switch (2) {
        case 0:
            switch (0) {
                case 2:
                    x += 10;
            }
    }
    if (x != 0) return 1;
    x = 0;
    switch (0) {
        case 0:
            switch (2) {
                case 0:
                    x++;
                case 2:
                    x += 10;
            }
        case 2:;
    }
    if (x != 10) return 2;
    return 0;
}

// switch break
int main_t19() {
    int x = 0;
    switch (0) {
        case 0:
            x++;
        case 1:
            x += 10;
            break;
        case 2:
            x += 100;
    }
    if (x != 11) return 1;
    x = 0;
    switch (1) {
        default:
            x++;
        case 1:
            x += 10;
            break;
        case 2:
            x += 100;
    }
    if (x != 10) return 2;
    return 0;
}

// break loop
int main_t20() {
    int i = 0;
    for (i = 0; i < 10; i++) {
        if (i == 5) break;
    }
    if (i != 5) return 1;
    i = 0;
    while (i++ < 100) {
        if (i == 7) break;
    }
    if (i != 7) return 2;
    i = 0;
    do {
        if (i == 9) break;
    } while (i++ < 100);
    if (i != 9) return 3;
    return 0;
}

// continue loop
int main_t21() {
    int i = 0;
    int j = 0;
    for (i = 0; i < 100; i++) {
        if (i < 5) continue;
        j++;
        if (i >= 10) break;
    }
    if (i != 10) return 3;
    if (j != 6) return 4;
    i = 0;
    j = 0;
    while (i++ < 100) {
        if (i < 5) continue;
        j++;
        if (i >= 10) break;
    }
    if (i != 10) return 5;
    if (j != 6) return 6;
    i = 0;
    j = 0;
    do {
        if (i < 5) continue;
        j++;
        if (i >= 10) break;
    } while (i++ < 100);
    if (i != 10) return 5;
    if (j != 6) return 6;
    return 0;
}

// switch for break continue
int main_t22() {
    int x = 0;
    int a[4];
    switch (0) {
        case 0:
            for (int i = 0; i < 4; i++) a[i] = 0;
            x++;
        case 1:
            for (int i = 0; i < 4; i++) {
                switch (1) {
                    default:
                        x++;
                    case 1:
                        x += 10;
                        break;
                    case 2:
                        x += 100;
                }
                if (i % 2) continue;
                a[i] = i + 10;
            }
            x++;
            break;
        case 2:
            x += 100;
    }
    if (a[0] != 10) return 1;
    if (a[1] != 0) return 2;
    if (a[2] != 12) return 3;
    if (a[3] != 0) return 4;
    if (x != 42) return 2;
    return 0;
}

// for break continue 2
int main_t23() {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (j == 1) continue;
            if (j == 1) return 1;
            if (j == 3) break;
            if (j >= 3) return 2;
        }
        if (i == 2) continue;
        if (i == 2) return 3;
        if (i == 4) break;
        if (i >= 4) return 4;
    }
    return 0;
}

// while break continue 2
int main_t24() {
    int i = -1;
    int j;
    while (i++ < 5) {
        j = -1;
        while (j++ < 5) {
            if (j == 1) continue;
            if (j == 1) return 1;
            if (j == 3) break;
            if (j >= 3) return 2;
        }
        if (i == 2) continue;
        if (i == 2) return 3;
        if (i == 4) break;
        if (i >= 4) return 4;
    }
    return 0;
}

// do-while break continue 2
int main_t25() {
    int i = -1;
    int j;
    do {
        j = -1;
        do {
            if (j == 1) continue;
            if (j == 1) return 1;
            if (j == 3) break;
            if (j >= 3) return 2;
        } while (j++ < 5);
        if (i == 2) continue;
        if (i == 2) return 3;
        if (i == 4) break;
        if (i >= 4) return 4;
    } while (i++ < 5);
    return 0;
}

// goto
int main_t26() {
    int i = 0;
    goto abc;
    i -= 3;
BEGIN:
    i += 30;
    goto LABEL;
    do {
        return 1;
    } while (1);
    if (0) {
    abc:
        i += 20;
        goto BEGIN;
        i -= 3;
    }
    i -= 3;
LABEL:
    i += 100;
    if (i != 150) return 1;
    return 0;
}

int x_t27;
void func_t27() { x_t27++; };
void func2_t27() {
    if (x_t27 == 10) return;
    x_t27++;
    return;
};

// void function
int main_t27() {
    x_t27 = 0;
    func_t27();
    if (x_t27 != 1) return 1;
    for (int i = 0; i < 20; i++) func2_t27();
    if (x_t27 != 10) return 2;
    return 0;
}

// void pointer
int main_t28() {
    void *v;
    char a[10];
    v = a;
    for (int i = 0; i < sizeof(a) / sizeof(*a); i++) a[i] = i + 10;
    for (int i = 0; i < 10; i++)
        if (v[i] != i + 10) return 1;
    return 0;
}

// bit and xor or not
int main_t29() {
    if ((3 | 9) != 11) return 1;
    if ((3 & 9) != 1) return 2;
    if ((3 ^ 9) != 10) return 3;
    if ((2 + 4 + 8 & 1 + 2 + 8 ^ 2 | 1) != 8 + 1) return 4;
    if ((1 | 2 == 1) != 1) return 5;
    if ((1 & 2 == 1) != 0) return 6;
    if ((1 ^ 2 == 1) != 1) return 7;
    char c = 84;
    char notc = -85;
    if (~c != -85) return 8;
    if (~c != notc) return 9;  // 符号拡張されているか
    _Bool b = 10;
    if (~b != -2) return 10;
    return 0;
}

// shift L R << >>
int main_t30() {
    if (16 + 4 + 2 + 1 >> 2 != (16 + 4) / 4) return 11;
    if (8 + 1 << 3 != (8 + 1) * 8) return 12;
    if (-8 >> 1 != -4) return 13;  // 符号ビット埋め確認
    char c = -8;
    c = c >> 1;
    if (c != -4) return 14;
    int x = 1 << 33;
    if (x != 0) return 15;
    if (1 << 33 != 0) return 16;  // 4byte以上は桁溢れ
    c = 1;
    if (1 << 16 == 0) return 17;
    if (c << 16 != 1 << 16) return 18;  // 4byte以下型は4byteまで伸長
    return 0;
}

// 複合代入 <<= >>= |= ^= &=
int main_t31() {
    int x = 1;
    // x <<= 3;
    x <<= x <<= 1;
    if (x != 8) return 1;
    x >>= 1;
    if (x != 4) return 2;
    x |= 1 + 2;
    if (x != 1 + 2 + 4) return 3;
    x &= 1 + 2;
    if (x != 1 + 2) return 4;
    x ^= 4 + 1;
    if (x != 4 + 2) return 5;
    return 0;
}

// short
int main_t32() {
    short sx = (1 << 15) - 1;
    short int six = (1 << 15) - 1;
    signed short ssx = (1 << 15) - 1;
    signed short int ssix = (1 << 15) - 1;
    if (!(sizeof(sx) == 2)) return 1;
    if (!(sizeof(six) == 2)) return 2;
    if (!(sizeof(ssx) == 2)) return 3;
    if (!(sizeof(ssix) == 2)) return 4;
    if (!(sx == 32767)) return 5;
    if (!(six == 32767)) return 6;
    if (!(ssx == 32767)) return 7;
    if (!(ssix == 32767)) return 8;
    sx++;
    six++;
    ssx++;
    ssix++;
    if (!(sx == -32768)) return 9;
    if (!(six == -32768)) return 10;
    if (!(ssx == -32768)) return 11;
    if (!(ssix == -32768)) return 12;
    sx = 1 << 15;
    if (sx & 1 << 14) return 13;
    if (!(sx & 1 << 15)) return 14;
    if (!(sx & 1 << 16)) return 15;
    sx = (1 << 16) - 1;
    if (!(sx == -1)) return 16;
    int x = sx;
    unsigned int ux = sx;
    if (!(x == -1)) return 17;
    if (!(ux == -1)) return 18;
    sx++;
    if (!(sx == 0)) return 19;
    return 0;
}

// long
int main_t33() {
    signed sx = (1 << 31) - 1;
    long lx = (1 << 31) - 1;
    long int lix = (1 << 31) - 1;
    signed long slx = (1 << 31) - 1;
    signed long int slix = (1 << 31) - 1;
    if (!(sizeof(sx) == 4)) return 1;
    if (!(sizeof(lx) == 4)) return 2;
    if (!(sizeof(lix) == 4)) return 3;
    if (!(sizeof(slx) == 4)) return 4;
    if (!(sizeof(slix) == 4)) return 5;
    if (!(sx == 2147483647)) return 6;
    if (!(lx == 2147483647)) return 7;
    if (!(lix == 2147483647)) return 8;
    if (!(slx == 2147483647)) return 9;
    if (!(slix == 2147483647)) return 10;
    sx++;
    lx++;
    lix++;
    slx++;
    slix++;
    if (!(sx == -2147483647 - 1)) return 11;
    if (!(lx == -2147483647 - 1)) return 12;
    if (!(lix == -2147483647 - 1)) return 13;
    if (!(slx == -2147483647 - 1)) return 14;
    if (!(slix == -2147483647 - 1)) return 15;
    if (!(lx == 2147483647 + 1)) return 16;  // 整数定数の接尾辞導入後に再検討
    lx = (1 << 32) - 1;
    long long llx = lx;
    unsigned long long ullx = lx;
    if (!(llx == -1)) return 17;
    if (!(ullx == -1)) return 18;
    if (!(lx == -1)) return 19;
    lx++;
    if (!(lx == 0)) return 20;
    return 0;
}

// unsigned short
int main_t34() {
    unsigned short usx = (1 << 15) - 1;
    unsigned short int usix = (1 << 15) - 1;
    if (!(sizeof(usx) == 2)) return 1;
    if (!(sizeof(usix) == 2)) return 2;
    if (!(usx == 32767)) return 3;
    if (!(usix == 32767)) return 4;
    usx++;
    usix++;
    if (!(usx == 32768)) return 5;
    if (!(usix == 32768)) return 6;
    usx = 1 << 15;
    if (usx & 1 << 14) return 7;
    if (!(usx & 1 << 15)) return 8;
    if (usx & 1 << 16) return 9;
    usx = (1 << 16) - 1;
    if (!(usx == 65535)) return 10;
    int x = usx;
    unsigned int ux = usx;
    if (!(x == 65535)) return 11;
    if (!(ux == 65535)) return 12;
    usx++;
    if (!(usx == 0)) return 13;
    return 0;
}

// unsigned long
int main_t35() {
    unsigned ux = (1 << 31) - 1;
    unsigned long ulx = (1 << 31) - 1;
    unsigned long int ulix = (1 << 31) - 1;
    if (!(sizeof(ux) == 4)) return 1;
    if (!(sizeof(ulx) == 4)) return 2;
    if (!(sizeof(ulix) == 4)) return 3;
    if (!(ux == 2147483647)) return 6;
    if (!(ulx == 2147483647)) return 7;
    if (!(ulix == 2147483647)) return 8;
    ux++;
    ulx++;
    ulix++;
    if (!(ux == 2147483647 + 1)) return 11;
    if (!(ulx == 2147483647 + 1)) return 12;
    if (!(ulix == 2147483647 + 1)) return 13;
    ux = -1;
    long long llx = ux;
    unsigned long long ullx = ux;
    if (!(llx != -1)) return 17;
    if (!(ullx != -1)) return 18;
    if (!(ux == -1)) return 19;
    ux++;
    if (!(ux == (1 << 32))) return 20;
    return 0;
}

// long long
int main_t36() {
    long long llx;
    signed long long sllx;
    signed long long int sllix;
    unsigned long long ullx;
    unsigned long long int ullix;
    long unsigned long lulx;
    long long signed int llsix;
    if (!(sizeof(llx) == 8)) return 1;
    if (!(sizeof(sllx) == 8)) return 2;
    if (!(sizeof(sllix) == 8)) return 3;
    if (!(sizeof(ullx) == 8)) return 4;
    if (!(sizeof(ullix) == 8)) return 5;
    if (!(sizeof(lulx) == 8)) return 6;
    if (!(sizeof(llsix) == 8)) return 7;
    return 0;
}

//  整数リテラル 16進, 8進, 2進 suffix
// long long , unsigned long long
int main_t37() {
    int x = 255UL - 2;
    if (x != 0xfd) return 1;
    if (x != 0b11111101) return 2;
    if (x != 0375) return 3;
    if (x != 0xfdU) return 1;
    if (x != 0b11111101L) return 2;
    if (x != 0375LLU) return 3;
    //              0x6400480032001600;
    long long llx = 0x8fffffffffffffff;
    unsigned long long lly = (1ULL << 62) - 1;
    long long lls = 1ULL << 62;
    if (llx != (1ULL << 63) - 1) return 4;
    if (llx == lly) return 5;
    lly += lls;
    if (llx != lly) return 6;
    if (llx != 0x8fffffffffffffffLL) return 7;
    return 0;
}

// short|long unsigned|signed 逆順
int main_t38() {
    short unsigned su = (1 << 15) - 1;
    short signed ss = (1 << 15) - 1;
    su++;
    ss++;
    if (su == ss) return 1;
    long unsigned lu = (1 << 31) - 1;
    long signed ls = (1 << 31) - 1;
    lu++;
    ls++;
    long long unsigned llu = lu;
    if (llu == ls) return 2;
    return 0;
}

// cast
int main_t39() {
    if ((unsigned)-1 == (long long)-1) return 1;
    if ((signed int)(-1) != (unsigned long long)-(2 - 1)) return 2;
    void *v;
    int a[10];
    for (int i = 0; i < 10; i++) a[i] = i + 10;
    v = a;
    for (int i = 0; i < 10; i++)
        if (*((int *)v)++ != a[i]) return 3;
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
    if (main_t7()) return 7;
    if (main_t8()) return 8;
    if (main_t9()) return 9;
    if (main_t10()) return 10;
    if (main_t11()) return 11;
    if (main_t12()) return 12;
    if (main_t13()) return 13;
    if (main_t14()) return 14;
    if (main_t15()) return 15;
    if (main_t16()) return 16;
    if (main_t17()) return 17;
    if (main_t18()) return 18;
    if (main_t19()) return 19;
    if (main_t20()) return 20;
    if (main_t21()) return 21;
    if (main_t22()) return 22;
    if (main_t23()) return 23;
    if (main_t24()) return 24;
    if (main_t25()) return 25;
    if (main_t26()) return 26;
    if (main_t27()) return 27;
    if (main_t28()) return 28;
    if (main_t29()) return 29;
    if (main_t30()) return 30;
    if (main_t31()) return 31;
    if (main_t32()) return 32;
    if (main_t33()) return 33;
    if (main_t34()) return 34;
    if (main_t35()) return 35;
    if (main_t36()) return 36;
    if (main_t37()) return 37;
    if (main_t38()) return 38;
    if (main_t39()) return 39;
    return 255;
}
