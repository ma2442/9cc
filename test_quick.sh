#!/bin/bash

assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s testfuncs.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

# 四則演算、不等号, return
assert 26 'int main(){return 5+10*(+3+6-6)/2 + (3==1+2) + (3!=1+1) + (11 < 12) + (12 <= 12) + (12 > 11) + (12 >= 12);}'
assert 1 'int main(){return -(-13)+-12       + (3==1+3) + (3!=1+2) + (11 < 11) + (12 <= 11) + (11 > 11) + (12 >= 13);}'
assert 5 'int main(){return (+3+5)/-2*-(1) + ( 9 == 2* 4 + (1 != (+3+5)/-2*-(1) >= 5 > 1));}'

# ローカル変数、if for while, block 
assert 2 'int main(){int ident_tESt01; int _name__; int val; ident_tESt01=1+4; _name__=8/2; val=ident_tESt01 - _name__;  int z; int c; return (z =c =9 == 2* 4 + (1 != (+3+5)/-2*-(1) >= 5 > 1)) + val;}' 
assert 35 'int main(){int a; int i; a=-10; for(i=1; i < 10; i=i+1) a = a + i; return a; }'
assert 5 'int main(){int i; i=0; for(;;) if(i<5)i=i+1; else return i; }'
assert 5 'int main(){int i; i=3; if(i==0){i=1; i=i+1;} else if (i==3) {i=4; i=i+1;} else {i=7; i=i+1;} return i; }'
assert 45 'int main(){int i; int k; int res; int res_while; i=0; res=0; while(i<3){ for(k=0; k<5; k = k+1){res= res+1; res= res+2;} res_while = res_while+1; i=i+1;} return res; }'

#外部関数コール、関数定義&コール
assert 9 'int func1(){ return 1; } int func2(int x, int y){return x + y;} int main(){ bar6(1,2,3,4,5,6); return func1() + func2(1, 3) + 4; }'
assert 8 'int fib(int x){ if(x==1) { return 1;} else if(x==2) { return 1;} else { return fib(x-1) + fib(x-2);} } int main() { return fib(6); }'

# int* 定義, 代入・アクセス
assert 99 'int swap(int *x, int *y){int tmp; tmp = *x; *x = *y; *y = tmp; return 1;} int main(){int x; int y; x=1; y=100; swap(&x, &y); return x-y;}'

# ポインタ加減算
assert 32 'int main() {int **p; alloc4_2D(&p, 10, 20, 30, 40); int **q; q = 4 + p - 2; *q = *q + 1; return **q;}'

# sizeof
assert 12 'int main(){ int x; int *y; return sizeof(x+3) + sizeof(y+3); }'
assert 24 'int main(){ int *y;int a[3]; return sizeof(*y) + sizeof(sizeof(1)) + sizeof(1*1) + sizeof(a); }'

# 配列
assert 3 'int main(){ int a[11]; *a = 1; *(a+10) = 2; int *p; p = a; return *p + *(1+p+9); }'
assert 2 'int main(){ int a[11]; a[0] = 1; 10[a] = a[0] + 1; int *p; p = a; return (p+5)[2+3]; }'

# ポインタを返す関数
assert 30 'int* func(int* p){return p+1;} int main(){int a[3]; a[0]=10; a[1]=20; a[2]=30; return *(func(a)+1);}'

# グローバル変数
assert 14 'int x; int y; int main(){ x=4; y=6+x; return y+x; }'
assert 1 'int x; int main(){ x = 5; int x; x = 1; return x; }'
assert 5 'int x; int func(int x){ x=1; } int main(){ x=5; func(); return x; }'
assert 3 'int x; int *y; int main(){y = &x; *y = 3; return x;}'
assert 2 'int a[11]; int *p; int main(){ a[0] = 1; 10[a] = a[0] + 1; p = a; return (p+5)[2+3]; }'

# char
assert 255 'char x; char y; char func(char z){return z+1;} int main(){char a; a=4; x=1; y=10; return func(239)+x+y+a;}'
assert 3 'int main(){char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y;}'
assert 3 'int main(){ char a[11]; *a = 1; *(a+10) = 2; char *p; p = a; return *p + *(1+p+9); }'
assert 30 'char* func(char* p){return p+1;} int main(){char a[3]; a[0]=10; a[1]=20; a[2]=30; return *(func(a)+1);}'

# char overflow
assert 0 'char c; char func(char x){if(x==0) return 256; return 1;} int main(){c=1; char x; x=256; if(x==0) c=256; if(c==0) {if(func(256)==0) return 0;} return 1;}'

# string literal
assert 1 'int main(){char *str; str = "abcdefg"; if(str[1] == 98) return 1; return 0;}'
assert 3 'int main(){return sizeof("abc");}'
assert 102 'char *str; int main(){str = "df"; return *(str+1);}'
assert 101 'int main(){return "aceg"[2];}' # == 'e'

echo OK
