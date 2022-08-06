int foo();
int bar(int, int);
int bar6(int, int, int, int, int, int);
int alloc4(int **, int, int, int, int);
int alloc4_2D(int ***, int, int, int, int);

struct stc {
    int x;
    char c;
    char c2;
    int y;
    struct stc *next;
};

// ローカル変数の構造体へのメンバ代入･アクセス
int main_t0() {
    struct stc st1;
    struct stc *p = &st1;
    st1.x = 1;
    st1.y = 2;
    st1.c = 49;
    st1.c2 = 50;
    st1.next = &st1;
    if (st1.x != 1) return 1;
    if (st1.y != 2) return 2;
    if (st1.c != 49) return 3;
    if (st1.c2 != 50) return 4;
    if (p->x != st1.x) return 5;
    if (st1.x != st1.next->x) return 6;
    if (sizeof(st1) != 24) return 7;
    return 0;
}

// グローバル変数の構造体へのメンバ代入･アクセス
struct stc g;
int main_t1() {
    struct stc *p = &g;
    g.x = 1;
    g.y = 2;
    g.c = 49;
    g.c2 = 50;
    g.next = &g;
    if (g.x != 1) return 1;
    if (g.y != 2) return 2;
    if (g.c != 49) return 3;
    if (g.c2 != 50) return 4;
    if (p->x != g.x) return 5;
    if (g.x != g.next->x) return 6;
    if (sizeof(g) != 24) return 7;
    return 0;
}

struct pos {
    int x;
    int y;
    struct pos *next;
};

// 構造体配列、ポインタアクセス
int main_t2() {
    struct pos a[3];
    struct pos *p;
    int i;
    for (i = 0; i < sizeof(a) / sizeof(a[0]); i++) {
        a[i].x = i;
        a[i].y = i * 10;
        if (i != sizeof(a) / sizeof(a[0]) - 1)
            a[i].next = &a[i + 1];
        else
            a[i].next = 0;
    }
    for (i = 0; i < sizeof(a) / sizeof(a[0]); i++) {
        if (a[i].x != i) return i;
        if (a[i].y != i * 10) return i * 10;
    }
    i = 0;
    for (p = a; p->next; p++) {
        if (p->x != i) return i;
        if (p->y != i * 10) return i * 10;
        i++;
    }
    return 0;
}

// 構造体コピー
struct pos g_t3;
int main_t3() {
    struct pos a;
    struct pos b;
    a.x = 1;
    a.y = 2;
    a.next = &a;
    b = a;
    if (a.x != b.x) return 1;
    if (a.y != b.y) return 2;
    if (a.next != b.next) return 3;
    g_t3 = a;
    if (a.x != g_t3.x) return 4;
    if (a.y != g_t3.y) return 5;
    if (a.next != g_t3.next) return 6;
    return 0;
}

struct sizeint {  // sizeof(sizeint) == 12
    char c;
    int x;
    char c2;
};

struct sizetest {  // sizeof(sizetest) == 4 + 12 + 12 == 28
    char c;
    struct sizeint stc;
    char a[11];
};

int main_t4() {
    struct sizetest szts[3];
    struct sizetest tmp;
    if (sizeof(szts) != 84) return 1;
    int i;
    for (i = sizeof(szts) / sizeof(szts[0]) - 1; i >= 0; i--) {
        tmp.c = i;
        tmp.stc.c = 10 + i;
        tmp.stc.c2 = 20 + i;
        tmp.a[10] = 30 + i;
        szts[i] = tmp;
    }
    for (i = sizeof(szts) / sizeof(szts[0]) - 1; i >= 0; i--) {
        if (szts[i].c != i) return 2;
        if (szts[i].stc.c != 10 + i) return 3;
        if (szts[i].stc.c2 != 20 + i) return 4;
        if (szts[i].a[10] != 30 + i) return 5;
    }
    struct sizeint szin;
    szin.c = 49;
    if (szin.c != 49) return 6;
    return 0;
}

struct {
    int val;
} noname;

struct lst {
    int val;
    struct lst *next;
} globalls;

struct lst func_t5(struct lst ls) {
    ls.val++;
    return ls;
}
// 無名 struct 定義テスト
// struct tag スコープテスト
// 関数引数、返り値の型をstructにするテスト
int main_t5() {
    noname.val = 1;
    if (noname.val != 1) return 1;
    struct {
        int local;
    } noname;
    noname.local = 2;
    struct {
        int local;
    };
    if (noname.local != 2) return 2;
    globalls.val = 10;
    struct lst localls = globalls;
    localls = func_t5(localls);
    if (localls.val != 11) return 3;
    return 0;
}

enum { A, B, C };
enum { D = 3 };
// 列挙体 タグ無し 定義、スコープ テスト
int main_t6() {
    if (A != 0) return 1;
    if (B != 1) return 2;
    if (C != 2) return 3;
    if (D != 3) return 4;
    enum { A = 10, B = B + A, C, D = C * 10 % 100 };
    if (A != 10) return 4;
    if (B != 11) return 5;
    if (D != 20) return 6;
    enum { E = 1 ? 2 : 3, F = 0 ? 2 : 3 };
    if (E != 2) return 7;
    if (F != 3) return 8;
    return 0;
}

enum enm1 { E, F, G } enm1;
// 列挙体 タグあり 定義、スコープ 変数定義 テスト
int main_t7() {
    enm1 = E;
    if (enm1 != E) return 1;
    enm1 = 1000;
    if (enm1 != 1000) return 2;
    enm1 = G * (4 + ((F + 2) == 3));
    if (enm1 + 1 != 11) return 3;
    enum enm1 { E = ((16 >> 1) | (1 << 1) | 1 ^ 0) & ~1, F, G } enm1 = E;
    if (enm1 != E) return 4;
    if (enm1 != 10) return 5;
    return 0;
}

// for初期化式スコープ、ブロックスコープ テスト
// (struct, enum, int について)
int main_t8() {
    int i = -1;
    int j = -10;
    struct lst {
        int val;
        struct lst *next;
    } globalls;
    enum enm1 { E, F, G } enm1;
    globalls.val = 5;
    enm1 = 6;

    for (int i = 0; i < 10; i++) {
        if (j != -10) return 1;
        int j = i;
        if (i != j) return 2;
        int i = 20;
        if (i != 20) return 3;
        if (globalls.val != 5) return 4;
        if (enm1 != 6) return 5;
        struct lst {
            int val;
            struct lst *next;
        } globalls;
        enum enm1 { E, F, G } enm1;
        globalls.val = 7;
        enm1 = 8;
        if (globalls.val != 7) return 6;
        if (enm1 != 8) return 7;
    }
    if (i != -1) return 8;
    if (globalls.val != 5) return 9;
    if (enm1 != 6) return 10;
    return 0;
}

struct stc8 {
    enum enm8 { A8 } e;
    struct stc8a {
        int x;
    } s;
};
// 構造体内のenum, struct定義が外側で行われる確認
int main_t8b() {
    struct stc8 st;
    st.e = A8;
    st.s.x = A8;
    if (st.e != st.s.x) return 1;
    struct stc8 {
        enum enm8 { A8 = 10 } f;
        struct stc8a {
            int y;
        } q;
    };
    struct stc8 lcl;
    lcl.f = A8;
    lcl.q.y = 10;
    if (lcl.f != lcl.q.y) return 2;
    return 0;
}

// typedef
int main_t9() {
    typedef signed char *ucp;
    ucp *pp;
    char a[10];
    ucp p = a;
    for (int i = 0; i < 10; i++) a[i] = i + 10;
    pp = &p;
    for (int i = 0; i < 10; i++)
        if ((*pp)[i] != i + 10) return 1;
    typedef char arr[10];
    arr *ap = &a;
    for (int i = 0; i < 10; i++) (*ap)[i] = i + 10;
    for (int i = 0; i < 10; i++)
        if ((*ap)[i] != i + 10) return 2;
    return 0;
}

typedef enum { A_t10 = 5, B_t10 } enm_t10;
typedef struct {
    enm_t10 x;
    struct stc_t10_1 {
        int yx;
    } y1;
} stc_t10;

// typedef scope
int main_t10() {
    struct stc_t10_1 stc1;
    stc_t10 s;
    s.x = A_t10;
    typedef enum { A_t10 = 10, B_t10 } enm_t10;
    typedef struct {
        int x[10];
    } stc_t10;
    stc_t10 lcl;
    lcl.x[0] = A_t10;
    if (s.x + 5 != lcl.x[0]) return 1;
    if (sizeof(s) != 8) return 2;
    if (sizeof(lcl) != 40) return 3;
    return 0;
}

typedef enum enm_t11 enm_t11;
typedef struct stc_t11 stc_t11;
enum enm_t11 { A_t11 = 5, B_t11 };
struct stc_t11 {
    enm_t11 x;
};

// typedef scope
int main_t11() {
    stc_t11 s;
    s.x = 10;
    if (s.x != A_t11 + 5) return 1;
    if (sizeof(s) != 4) return 2;
}

int main_t11b() {
    typedef enum enm_t11 enm_t11;
    typedef struct stc_t11 stc_t11;
    enum enm_t11 { A_t11 = 10, B_t11 };
    struct stc_t11 {
        enm_t11 x[10];
    };
    stc_t11 lcl;
    lcl.x = 5;
    if (lcl.x + 5 != A_t11) return 1;
    if (sizeof(lcl) != 4) return 2;
    return 0;
}

int main_t11c() {
    enum enm_t11 { A_t11 = 10, B_t11 };
    struct stc_t11 {
        enm_t11 x[10];
    };
    typedef enum enm_t11 enm_t11;
    typedef struct stc_t11 stc_t11;
    stc_t11 lcl;
    lcl.x[1] = 5;
    if (lcl.x[1] + 5 != A_t11) return 1;
    if (sizeof(lcl) != 40) return 2;
    return 0;
}

int main() {
    if (main_t0()) return 0;
    if (main_t1()) return 1;
    if (main_t2()) return 2;
    if (main_t3()) return 3;
    if (main_t4()) return 4;
    if (main_t5()) return 5;
    if (main_t6()) return 6;
    if (main_t7()) return 7;
    if (main_t8()) return 8;
    if (main_t8b()) return 8;
    if (main_t9()) return 9;
    if (main_t10()) return 10;
    if (main_t11()) return 11;
    if (main_t11b()) return 112;
    if (main_t11c()) return 113;
    return 255;
}
