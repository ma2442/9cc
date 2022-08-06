// 四則演算 ~ ブロックまで
int foo();
int bar(int a, int b);
int bar6(int a, int b, int c, int d, int e, int f);
int alloc4(int **p, int x1, int x2, int x3, int x4);
int alloc4_2D(int ***p, int x1, int x2, int x3, int x4);

int main_t0() { return 0; }
int main_t1() { return 42; }
int main_t2() { return 5 + 20 - 4; }
int main_t3() { return 5 - 3; }
int main_t4() { return 12 + 34 - 5; }
int main_t5() { return 5 + 6 * 7; }
int main_t6() { return 5 * (9 - 6); }
int main_t7() { return (3 + 5) / 2; }
int main_t8() { return +10 - 5; }
int main_t9() { return -3 + 9; }
int main_t10() { return -(-13) + -12; }
int main_t11() { return (+3 + 5) / -2 * -(1); }
int main_t12() { return - -10; }
int main_t13() { return - -+10; }
int main_t14() { return 3 == 1 + 2; }
int main_t15() { return 3 != 1 + 2; }
int main_t16() { return 11 < 12; }
int main_t17() { return 12 < 12; }
int main_t18() { return 12 <= 12; }
int main_t19() { return 12 <= 11; }
int main_t20() { return 12 > 11; }
int main_t21() { return 11 > 11; }
int main_t22() { return 12 >= 12; }
int main_t23() { return 11 >= 12; }
int main_t24() { return -2 * (4 - 1) == -1 * (-6 * -1); }
int main_t25() { return -2 * (4 - 1) != -1 + -(6 * -1); }
int main_t26() { return -2 * (4 - 1) < -1 + -(6 * -1); }
int main_t27() { return -2 * (4 - 1) > -1 + -(6 * -1); }
int main_t28() { return 9 == 2 * 4 + (1 != (+3 + 5) / -2 * -(1) >= 5 > 1); }
int main_t29() {
    int a;
    return a = 2;
}
// assert 1 '(a+1)=2;'
int main_t30() {
    int a;
    int b;
    return a = b = 2;
}
int main_t31() {
    int a;
    int b;
    return a = b = (+3 + 5) / -2 * -(1);
}
int main_t32() {
    int z;
    int c;
    return z = c = 9 == 2 * 4 + (1 != (+3 + 5) / -2 * -(1) >= 5 > 1);
}
int main_t33() {
    int a;
    int d;
    a = 1 + 2;
    return d = (1 == 1) * (a + 2);
}
int main_t34() {
    int ident_tESt01;
    int _name__;
    int val;
    ident_tESt01 = 1 + 4;
    _name__ = 8 / 2;
    val = ident_tESt01 - _name__;
    return val + 8;
}
int main_t35() { return 1; }
int main_t36() {
    return 9;
    return 1;
}
int main_t37() {
    int return9;
    return9 = 9;
    return return9;
}
int main_t38() {
    int val1;
    int val2;
    int val3;
    val1 = 1 + 4;
    val2 = 0;
    val3 = (1 == 1);
    return 2 * (val1 + val3);
}
// assert 2 'return 2 = 2; return return2;'
int main_t39() {
    int i;
    i = 0;
    if (1)
        i = 2;
    else
        i = 3;
    return i;
}
int main_t40() {
    int i;
    i = 0;
    if (0)
        i = 2;
    else
        i = 3;
    return i;
}
// assert 2 'if() 2; else 3;'
int main_t41() {
    if (-10) return 2;
    return 3;
}
int main_t42() {
    int loop;
    loop = 1;
    while (loop) return 1;
    return 2;
}
int main_t43() {
    int loop;
    loop = 0;
    while (loop) return 1;
    return 2;
}
int main_t44() {
    int loop;
    loop = 100;
    while (loop) loop = loop - 1;
    return loop;
}
// assert 1 'while() return 1; return 2;'
int main_t45() {
    int a;
    int i;
    a = -10;
    for (i = 1; i < 10; i = i + 1) a = a + i;
    return a;
}
int main_t46() {
    int i;
    for (i = 0;;)
        if (i < 5)
            i = i + 1;
        else
            return i;
}
int main_t47() {
    int i;
    i = 0;
    for (; i < 6;) i = i + 1;
    return i;
}
int main_t48() {
    int i;
    i = 0;
    for (;; i = i + 1)
        if (i >= 7) return i;
}
int main_t49() {
    int a;
    int l;
    int i;
    a = 3;
    l = 3;
    if (a == 2)
        a = 1;
    else
        while (l = l - 1)
            for (i = 0; i < 5; i = i + 1) a = a + 2;
    return a;
}
// block test
int main_t50() {
    {
        return 1;
        return 2;
    }
}
int main_t51() {
    {}
    return 1;
}
int main_t52() {
    int i;
    i = 0;
    if (i == 1) {
        i = i + 1;
        i = i * 3;
    }
    return i;
}
int main_t53() {
    int i;
    i = 1;
    if (i == 1) {
        i = i + 1;
        i = i * 3;
    }
    return i;
}
int main_t54() {
    int i;
    i = 0;
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
int main_t55() {
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
int main_t56() {
    int i;
    i = 6;
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
int main_t57() {
    int i;
    int k;
    int res;
    int res_while;
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
int main_t58() {
    int i;
    i = 0;
    if (i == 1) {
        return 1;
    }
    if (i == 2) {
    }
    return 2;
}

int main() {
    if (main_t0() != 0) return 0;
    if (main_t1() != 42) return 1;
    if (main_t2() != 21) return 2;
    if (main_t3() != 2) return 3;
    if (main_t4() != 41) return 4;
    if (main_t5() != 47) return 5;
    if (main_t6() != 15) return 6;
    if (main_t7() != 4) return 7;
    if (main_t8() != 5) return 8;
    if (main_t9() != 6) return 9;
    if (main_t10() != 1) return 10;
    if (main_t11() != 4) return 11;
    if (main_t12() != 10) return 12;
    if (main_t13() != 10) return 13;
    if (main_t14() != 1) return 14;
    if (main_t15() != 0) return 15;
    if (main_t16() != 1) return 16;
    if (main_t17() != 0) return 17;
    if (main_t18() != 1) return 18;
    if (main_t19() != 0) return 19;
    if (main_t20() != 1) return 20;
    if (main_t21() != 0) return 21;
    if (main_t22() != 1) return 22;
    if (main_t23() != 0) return 23;
    if (main_t24() != 1) return 24;
    if (main_t25() != 1) return 25;
    if (main_t26() != 1) return 26;
    if (main_t27() != 0) return 27;
    if (main_t28() != 1) return 28;
    if (main_t29() != 2) return 29;
    if (main_t30() != 2) return 30;
    if (main_t31() != 4) return 31;
    if (main_t32() != 1) return 32;
    if (main_t33() != 5) return 33;
    if (main_t34() != 9) return 34;
    if (main_t35() != 1) return 35;
    if (main_t36() != 9) return 36;
    if (main_t37() != 9) return 37;
    if (main_t38() != 12) return 38;
    if (main_t39() != 2) return 39;
    if (main_t40() != 3) return 40;
    if (main_t41() != 2) return 41;
    if (main_t42() != 1) return 42;
    if (main_t43() != 2) return 43;
    if (main_t44() != 0) return 44;
    if (main_t45() != 35) return 45;
    if (main_t46() != 5) return 46;
    if (main_t47() != 6) return 47;
    if (main_t48() != 7) return 48;
    if (main_t49() != 23) return 49;
    if (main_t50() != 1) return 50;
    if (main_t51() != 1) return 51;
    if (main_t52() != 0) return 52;
    if (main_t53() != 6) return 53;
    if (main_t54() != 2) return 54;
    if (main_t55() != 5) return 55;
    if (main_t56() != 8) return 56;
    if (main_t57() != 45) return 57;
    if (main_t58() != 2) return 58;
    return 255;
}
