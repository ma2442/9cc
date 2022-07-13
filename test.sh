#!/bin/bash

assert() {
    expected="$1"
    input="$2"
    echo "$input" > test/tmp/tmp.c
    ./9cc test/tmp/tmp.c > tmp.s
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

assertf(){
    expected="$1"
    input="$2"
    ./9cc test/"$input" > tmp.s
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

assert 0 'int main(){return 0;}'
assert 42 'int main(){return 42;}'
assert 21 'int main(){return 5+20-4;}'
assert 2 'int main(){return 5 - 3;}'
assert 41 'int main(){return  12 + 34 - 5 ;}'
assert 47 'int main(){return 5+6*7;}'
assert 15 'int main(){return 5*(9-6);}'
assert 4 'int main(){return (3+5)/2;}'

assert 5 'int main(){return +10-5;}'
assert 6 'int main(){return -3+9;}'
assert 1 'int main(){return -(-13)+-12;}'
assert 4 'int main(){return (+3+5)/-2*-(1);}'
assert 10 'int main() { return - -10; }'
assert 10 'int main() { return - - +10; }'

assert 1 'int main(){return 3==1+2;}'
assert 0 'int main(){return 3!=1+2;}'
assert 1 'int main(){return 11 < 12;}'
assert 0 'int main(){return 12 < 12;}'
assert 1 'int main(){return 12 <= 12;}'
assert 0 'int main(){return 12 <= 11;}'
assert 1 'int main(){return 12 > 11;}'
assert 0 'int main(){return 11 > 11;}'
assert 1 'int main(){return 12 >= 12;}'
assert 0 'int main(){return 11 >= 12;}'
assert 1 'int main(){return -2 * (4 - 1)==-1 * (-6 * -1);}'
assert 1 'int main(){return -2 * (4 - 1)!=-1 + -(6 * -1);}'
assert 1 'int main(){return -2 * (4 - 1)<-1 + -(6 * -1);}'
assert 0 'int main(){return -2 * (4 - 1)>-1 + -(6 * -1);}'
assert 1 'int main(){return 9 == 2* 4 + (1 != (+3+5)/-2*-(1) >= 5 > 1);}'

assert 2 'int main(){int a; return a=2;}'
# assert 1 '(a+1)=2;'
assert 2 'int main(){int a; int b; return a=b=2;}'
assert 4 'int main(){int a; int b; return a=b=(+3+5)/-2*-(1);}'
assert 1 'int main(){int z; int c; return z =c =9 == 2* 4 + (1 != (+3+5)/-2*-(1) >= 5 > 1);}' 
assert 5 'int main(){int a; int d; a=1+2; return d=(1==1)*(a+2);}'
assert 9 'int main(){int ident_tESt01; int _name__; int val; ident_tESt01=1+4; _name__=8/2; val=ident_tESt01 - _name__; return  val+8;}'
assert 1 'int main(){ return 1; }'
assert 9 'int main(){ return 9; return 1; }'
assert 9 'int main(){int return9; return9 = 9; return return9; }'
assert 12 'int main(){int val1; int val2; int val3; val1 = 1+4; val2 = 0; val3=(1==1); return 2*(val1+val3); }'
# assert 2 'return 2 = 2; return return2;'

assert 2 'int main(){int i; i=0; if(1) i=2; else i=3; return i;}'
assert 3 'int main(){int i; i=0; if(0) i=2; else i=3; return i;}'
#assert 2 'if() 2; else 3;'
assert 2 'int main(){ if(-10) return 2; return 3; }'
assert 1 'int main(){int loop; loop=1; while(loop) return 1; return 2; }'
assert 2 'int main(){int loop; loop=0; while(loop) return 1; return 2; }'
assert 0 'int main(){int loop; loop=100; while(loop) loop = loop - 1; return loop; }'
#assert 1 'while() return 1; return 2;'
assert 35 'int main(){int a; int i; a=-10; for(i=1; i < 10; i=i+1) a = a + i; return a; }'
assert 5 'int main(){int i; for(i=0;;) if(i<5)i=i+1; else return i; }'
assert 6 'int main(){int i; i=0; for(;i<6;) i=i+1; return i; }'
assert 7 'int main(){int i; i=0; for(;;i=i+1) if(i>=7) return i; }'
assert 23 'int main(){int a; int l; int i; a=3; l=3; if(a==2) a=1; else while(l=l-1) for(i=0; i<5; i= i + 1)  a = a+2; return a; }'

# block test
assert 1 'int main(){ {return 1; return 2;} }'
assert 1 'int main(){ {} return 1; }'
assert 0 'int main(){int i; i=0; if(i==1){i = i + 1; i = i * 3;} return i; }'
assert 6 'int main(){int i; i=1; if(i==1){i = i + 1; i = i * 3;} return i; }'
assert 2 'int main(){int i; i=0; if(i==0){i=1; i=i+1;} else if (i==3) {i=4; i=i+1;} else {i=7; i=i+1;} return i; }'
assert 5 'int main(){int i; i=3; if(i==0){i=1; i=i+1;} else if (i==3) {i=4; i=i+1;} else {i=7; i=i+1;} return i; }'
assert 8 'int main(){int i; i=6; if(i==0){i=1; i=i+1;} else if (i==3) {i=4; i=i+1;} else {i=7; i=i+1;} return i; }'
assert 45 'int main(){int i; int k; int res; int res_while; i=0; res=0; while(i<3){ for(k=0; k<5; k = k+1){res= res+1; res= res+2;} res_while = res_while+1; i=i+1;} return res; }'
assert 2 'int main(){int i; i=0; if(i==1){return 1;} if(i==2){} return 2; }'

#外部関数コール
assert 1 'int main(){ foo(); return 1; }'
assert 2 'int main(){ bar(1, 2); return 2; }'
assert 3 'int main(){ bar6(1,2,3,4,5,6); return 3; }'

#関数定義&コール
assert 1 'int func1(){ return 1; } int main(){ return func1(); }'
assert 2 'int func2(int x){return x+1;} int main(){ return func2(1); }'
assert 8 'int func2(int x, int y){return x + y;} int main(){ return func2(1, 3) + 4; }'
assert 20 'int func(int x){ if(x==1) { return 10;} else { return 20;}} int main(){ return func(2); }'
assert 4 'int func(int x) { return x+1; } int main(){int y; y=1; return func(2) + y; }'
assert 9 'int func(int x) { return x+1; } int func2(int y) { return y*2; } int main(){int a; int b; a = func(2);  b = func2(3);return a+b; }'
assert 6 'int func(int x) { return x+1; } int func2(int y) { return y*2; } int main(){ return func(1) + func2(2); }'
assert 6 'int acc(int x){ if(x==1) { return 1; } else { return x + acc(x-1); }} int main() { return acc(3); }'
assert 1 'int fib(int x){ if(x==1) { return 1;} else if(x==2) { return 1;} else { return fib(x-1) + fib(x-2);} } int main() { return fib(1); }'
assert 8 'int fib(int x){ if(x==1) { return 1;} else if(x==2) { return 1;} else { return fib(x-1) + fib(x-2);} } int main() { return fib(6); }'

# 関数コール (引数2の計算により引数1が書き換わらない = 全引数のpopがまとめて最後に行われていることの確認)
assert 55 'int func(int x, int y){return x+y;} int main(){return func(2+3, 20+30);}' 

#単項&、*
assert 3 'int main(){int x; int *y; x = 3; y = &x; return *y;}'
assert 4 'int main(){int x; int y; int *z; x = 4; y = 5; z = &y + 2; return *z;}'

# int* 定義
assert 3 'int main(){int x; int *y; y = &x; *y = 3; return x;}'
assert 99 'int swap(int *x, int *y){int tmp; tmp = *x; *x = *y; *y = tmp; return 1;} int main(){int x; int y; x=1; y=100; swap(&x, &y); return x-y;}'

# ポインタ加減算
assert 10 'int main() {int *p; alloc4(&p, 10, 20, 30, 40); int *q; q = p; return *q;}'
assert 30 'int main() {int *p; alloc4(&p, 10, 20, 30, 40); int *q; q = p + 2; return *q;}'
assert 40 'int main() {int *p; alloc4(&p, 10, 20, 30, 40); int *q; q = 3 + p; return *q;}'
assert 30 'int main() {int *p; alloc4(&p, 10, 20, 30, 40); int *q; q = 3 + p - 1; return *q;}'
assert 10 'int main() {int **p; alloc4_2D(&p, 10, 20, 30, 40); int **q; q = p; return **q;}'
assert 30 'int main() {int **p; alloc4_2D(&p, 10, 20, 30, 40); int **q; q = p + 2; return **q;}'
assert 32 'int main() {int **p; alloc4_2D(&p, 10, 20, 30, 40); int **q; q = 4 + p - 2; *q = *q + 1; return **q;}'

# sizeof
assert 4 'int main(){ int x; return sizeof(x); }'
assert 4 'int main(){ int x; return sizeof(x+3); }'
assert 8 'int main(){ int *y; return sizeof(y); }'
assert 8 'int main(){ int *y; return sizeof(y+3); }'
assert 4 'int main(){ int *y; return sizeof(*y); }'
assert 4 'int main(){ int *y; return sizeof(1); }'
assert 4 'int main(){ int *y; return sizeof(sizeof(1)); }'
assert 4 'int main(){ int *y; return sizeof(1==1); }'
assert 4 'int main(){ int *y; return sizeof(1*1); }'

# 配列
assert 12 'int main(){ int a[3]; return sizeof(a); }'
assert 1 'int main(){ int a[1]; *a = 1; int *p; p = a; return *p; }'
assert 1 'int main(){ int a[2]; *a = 1; *(a+1) = 2; int *p; p = a; return *p; }'
assert 2 'int main(){ int a[2]; *a = 1; *(a+1) = 2; int *p; p = a; return *(p+1); }'
assert 3 'int main(){ int a[2]; *a = 1; *(a+1) = 2; int *p; p = a; return *p + *(1+p); }'
assert 1 'int main(){ int a[11]; *a = 1; *(a+10) = 2; int *p; p = a; return *p; }'
assert 2 'int main(){ int a[11]; *a = 1; *(a+10) = 2; int *p; p = a; return *(1+p+9); }'

assert 1 'int main(){ int a[11]; a[0] = 1; a[10] = 2; int *p; p = a; return p[0]; }'
assert 2 'int main(){ int a[11]; a[0] = 1; 10[a] = a[0] + 1; int *p; p = a; return (p+5)[2+3]; }'

# ポインタを返す関数
assert 30 'int* func(int* p){return p+1;} int main(){int a[3]; a[0]=10; a[1]=20; a[2]=30; return *(func(a)+1);}'

# グローバル変数
assert 5 'int x; int main(){ x = 5; return x; }'
assert 1 'int x; int main(){ x = 5; int x; x = 1; return x; }'
assert 14 'int x; int y; int main(){ x=4; y=6+x; return y+x; }'
assert 5 'int x; int func(){ x=5; } int main(){ func(); return x; }'
assert 5 'int x; int func(int x){ x=1; } int main(){ x=5; func(); return x; }'
assert 3 'int x; int *y; int main(){y = &x; *y = 3; return x;}'
assert 32 'int **p; int main() { alloc4_2D(&p, 10, 20, 30, 40); int **q; q = 4 + p - 2; *q = *q + 1; return **q;}'
assert 3 'int a[11]; int main(){ *a = 1; *(a+10) = 2; int *p; p = a; return *p + *(1+p+9); }'
assert 2 'int a[11]; int main(){ a[0] = 1; 10[a] = a[0] + 1; int *p; p = a; return (p+5)[2+3]; }'

# char
assert 1 'int main(){ char c; c=1; return c;}'
assert 255 'char x; char y; char func(char z){return z+1;} int main(){char a; a=4; x=1; y=10; return func(239)+x+y+a;}'
assert 3 'int main(){char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y;}'
assert 2 'char x[3]; int main(){x[0] = -1; x[1] = 2; x[2] = 3; return x[1];}'
assert 3 'char x[3]; int main(){x[0] = -1; x[1] = 2; x[2] = 3; return x[2];}'
assert 1 'int main(){char x; return sizeof(x);}'
assert 3 'int main(){ char a[11]; *a = 1; *(a+10) = 2; char *p; p = a; return *p + *(1+p+9); }'
assert 30 'char* func(char* p){return p+1;} int main(){char a[3]; a[0]=10; a[1]=20; a[2]=30; return *(func(a)+1);}'
assert 2 'char a[11]; char *p; int main(){ a[0] = 1; 10[a] = a[0] + 1; p = a; return (p+5)[2+3]; }'

# char overflow
assert 0 'int main(){char x; x = 256; if(x) return 1; else return 0;}'
assert 0 'char func(){return 256;} int main(){if(func()) return 1; else return 0;}'
assert 0 'int func(char x){return x;} int main(){if(func(256)) return 1; else return 0;}'
assert 0 'char x; int main(){ x = 256; if(x) return 1; else return 0;}'

# string literal
assert 1 'int main(){char *str; str = "abcdefg"; if(str[1] == 98) return 1; return 0;}'
assert 3 'int main(){return sizeof("abc");}'
assert 102 'char *str; int main(){str = "df"; return *(str+1);}'
assert 101 'int main(){return "aceg"[2];}' # == 'e'
assert 0 'int main(){printf("Hello, world! %d\n", 20220711); return 0;}'

# INT型 4byte ひとつ前の要素の書き換えの影響を受けないことを確認
assert 3 'int main(){int a[3]; a[1]=3; a[0]=1; return a[1];} ' 
assert 3 'int a[3]; int main(){a[1]=3; a[0]=1; return a[1];} '
assert 3 'int main(){ int x; int y; y=3; x=1; return y;} '
assert 3 'int x; int y; int main(){y=3; x=1; return y;} '

# 二次元配列
assert 40 'int main(){int a[2][5]; return sizeof(a);}'
assert 20 'int main(){int a[4][5]; return sizeof(a[0]);}'
assert 4 'int main(){int a[4][5]; return sizeof(a[0][0]);}'
assert 5 'int main(){int a[2][3]; int i; int j; *(*(a+1)+1)=5; return *(*(a+1)+1);}'
assert 1 'int main(){int a[2][3]; int i; int j; for(i=0; i<sizeof(a)/sizeof(a[0]); i=i+1)for(j=0; j<sizeof(a[0])/sizeof(a[0][0]); j=j+1){a[i][j]=10*i+j+1;} return a[0][0];}'
assert 2 'int main(){int a[2][3]; int i; int j; for(i=0; i<sizeof(a)/sizeof(a[0]); i=i+1)for(j=0; j<sizeof(a[0])/sizeof(a[0][0]); j=j+1){a[i][j]=10*i+j+1;} return a[0][1];}'
assert 3 'int main(){int a[2][3]; int i; int j; for(i=0; i<sizeof(a)/sizeof(a[0]); i=i+1)for(j=0; j<sizeof(a[0])/sizeof(a[0][0]); j=j+1){a[i][j]=10*i+j+1;} return a[0][2];}'
assert 11 'int main(){int a[2][3]; int i; int j; for(i=0; i<sizeof(a)/sizeof(a[0]); i=i+1)for(j=0; j<sizeof(a[0])/sizeof(a[0][0]); j=j+1){a[i][j]=10*i+j+1;} return a[1][0];}'
assert 12 'int main(){int a[2][3]; int i; int j; for(i=0; i<sizeof(a)/sizeof(a[0]); i=i+1)for(j=0; j<sizeof(a[0])/sizeof(a[0][0]); j=j+1){a[i][j]=10*i+j+1;} return a[1][1];}'
assert 13 'int main(){int a[2][3]; int i; int j; for(i=0; i<sizeof(a)/sizeof(a[0]); i=i+1)for(j=0; j<sizeof(a[0])/sizeof(a[0][0]); j=j+1){a[i][j]=10*i+j+1;} return a[1][2];}'
assert 1 'int a[2][3]; int main(){ int i; int j; for(i=0; i<sizeof(a)/sizeof(a[0]); i=i+1)for(j=0; j<sizeof(a[0])/sizeof(a[0][0]); j=j+1){a[i][j]=10*i+j+1;} return a[0][0];}'
assert 2 'int a[2][3]; int main(){ int i; int j; for(i=0; i<sizeof(a)/sizeof(a[0]); i=i+1)for(j=0; j<sizeof(a[0])/sizeof(a[0][0]); j=j+1){a[i][j]=10*i+j+1;} return a[0][1];}'
assert 3 'int a[2][3]; int main(){ int i; int j; for(i=0; i<sizeof(a)/sizeof(a[0]); i=i+1)for(j=0; j<sizeof(a[0])/sizeof(a[0][0]); j=j+1){a[i][j]=10*i+j+1;} return a[0][2];}'
assert 11 'int a[2][3]; int main(){ int i; int j; for(i=0; i<sizeof(a)/sizeof(a[0]); i=i+1)for(j=0; j<sizeof(a[0])/sizeof(a[0][0]); j=j+1){a[i][j]=10*i+j+1;} return a[1][0];}'
assert 12 'int a[2][3]; int main(){ int i; int j; for(i=0; i<sizeof(a)/sizeof(a[0]); i=i+1)for(j=0; j<sizeof(a[0])/sizeof(a[0][0]); j=j+1){a[i][j]=10*i+j+1;} return a[1][1];}'
assert 13 'int a[2][3]; int main(){ int i; int j; for(i=0; i<sizeof(a)/sizeof(a[0]); i=i+1)for(j=0; j<sizeof(a[0])/sizeof(a[0][0]); j=j+1){a[i][j]=10*i+j+1;} return a[1][2];}'

assertf 1 arr.c
echo OK
