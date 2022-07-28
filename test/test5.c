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
    if (enm1 != 10) return 3;
    enum enm1 { E = 10, F, G } enm1 = E;
    if (enm1 != E) return 4;
    if (enm1 != 10) return 5;
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
    return 255;
}
