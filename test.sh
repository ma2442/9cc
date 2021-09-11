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

assert 0 'main(){return 0;}'
assert 42 'main(){return 42;}'
assert 21 'main(){return 5+20-4;}'
assert 2 'main(){return 5 - 3;}'
assert 41 'main(){return  12 + 34 - 5 ;}'
assert 47 'main(){return 5+6*7;}'
assert 15 'main(){return 5*(9-6);}'
assert 4 'main(){return (3+5)/2;}'

assert 5 'main(){return +10-5;}'
assert 6 'main(){return -3+9;}'
assert 1 'main(){return -(-13)+-12;}'
assert 4 'main(){return (+3+5)/-2*-(1);}'

assert 1 'main(){return 3==1+2;}'
assert 0 'main(){return 3!=1+2;}'
assert 1 'main(){return 11 < 12;}'
assert 0 'main(){return 12 < 12;}'
assert 1 'main(){return 12 <= 12;}'
assert 0 'main(){return 12 <= 11;}'
assert 1 'main(){return 12 > 11;}'
assert 0 'main(){return 11 > 11;}'
assert 1 'main(){return 12 >= 12;}'
assert 0 'main(){return 11 >= 12;}'
assert 1 'main(){return -2 * (4 - 1)==-1 * (-6 * -1);}'
assert 1 'main(){return -2 * (4 - 1)!=-1 + -(6 * -1);}'
assert 1 'main(){return -2 * (4 - 1)<-1 + -(6 * -1);}'
assert 0 'main(){return -2 * (4 - 1)>-1 + -(6 * -1);}'
assert 1 'main(){return 9 == 2* 4 + (1 != (+3+5)/-2*-(1) >= 5 > 1);}'

assert 2 'main(){return a=2;}'
# assert 1 '(a+1)=2;'
assert 2 'main(){return a=b=2;}'
assert 4 'main(){return a=b=(+3+5)/-2*-(1);}'
assert 1 'main(){return z =c =9 == 2* 4 + (1 != (+3+5)/-2*-(1) >= 5 > 1);}' 
assert 5 'main(){ a=1+2; return d=(1==1)*(a+2);}'
assert 9 'main(){ ident_tESt01=1+4; _name__=8/2; val=ident_tESt01 - _name__; return  val+8;}'
assert 1 'main(){ return 1; }'
assert 9 'main(){ return 9; return 1; }'
assert 9 'main(){ return9 = 9; return return9; }'
assert 12 'main(){ val1 = 1+4; val2 = 0; val3=(1==1); return 2*(val1+val3); }'
# assert 2 'return 2 = 2; return return2;'

assert 2 'main(){ i=0; if(1) i=2; else i=3; return i;}'
assert 3 'main(){ i=0; if(0) i=2; else i=3; return i;}'
#assert 2 'if() 2; else 3;'
assert 2 'main(){ if(-10) return 2; return 3; }'
assert 1 'main(){ loop=1; while(loop) return 1; return 2; }'
assert 2 'main(){ loop=0; while(loop) return 1; return 2; }'
assert 0 'main(){ loop=100; while(loop) loop = loop - 1; return loop; }'
#assert 1 'while() return 1; return 2;'
assert 35 'main(){ a=-10; for(i=1; i < 10; i=i+1) a = a + i; return a; }'
assert 5 'main(){ for(i=0;;) if(i<5)i=i+1; else return i; }'
assert 6 'main(){ i=0; for(;i<6;) i=i+1; return i; }'
assert 7 'main(){ i=0; for(;;i=i+1) if(i>=7) return i; }'
assert 23 'main(){ a=3; l=3; if(a==2) a=1; else while(l=l-1) for(i=0; i<5; i= i + 1)  a = a+2; return a; }'

# block test
assert 1 'main(){ {return 1; return 2;} }'
assert 1 'main(){ {} return 1; }'
assert 0 'main(){ i=0; if(i==1){i = i + 1; i = i * 3;} return i; }'
assert 6 'main(){ i=1; if(i==1){i = i + 1; i = i * 3;} return i; }'
assert 2 'main(){ i=0; if(i==0){i=1; i=i+1;} else if (i==3) {i=4; i=i+1;} else {i=7; i=i+1;} return i; }'
assert 5 'main(){ i=3; if(i==0){i=1; i=i+1;} else if (i==3) {i=4; i=i+1;} else {i=7; i=i+1;} return i; }'
assert 8 'main(){ i=6; if(i==0){i=1; i=i+1;} else if (i==3) {i=4; i=i+1;} else {i=7; i=i+1;} return i; }'
assert 45 'main(){ i=0; res=0; while(i<3){ for(k=0; k<5; k = k+1){res= res+1; res= res+2;} res_while = res_while+1; i=i+1;} return res; }'
assert 2 'main(){ i=0; if(i==1){return 1;} if(i==2){} return 2; }'

#外部関数コール
assert 1 'main(){ foo(); return 1; }'
assert 2 'main(){ bar(1, 2); return 2; }'
assert 3 'main(){ bar6(1,2,3,4,5,6); return 3; }'

#関数定義&コール
assert 1 'func1(){ return 1; } main(){ return func1(); }'
assert 8 'func2(x, y){return x + y;} main(){ return func2(1, 3) + 4; }'
assert 20 'func(x){ if(x==1) { return 10;} else { return 20;}} main(){ return func(2); }'
assert 4 'func(x) { return x+1; } main(){ y=1; return func(2) + y; }'
assert 9 'func(x) { return x+1; } func2(y) { return y*2; } main(){ a = func(2);  b = func2(3);return a+b; }'
assert 6 'func(x) { return x+1; } func2(y) { return y*2; } main(){ return func(1) + func2(2); }'
assert 6 'acc(x){ if(x==1) { return 1; } else { return x + acc(x-1); }} main() { return acc(3); }'
assert 1 'fib(x){ if(x==1) { return 1;} else if(x==2) { return 1;} else { return fib(x-1) + fib(x-2);} } main() { return fib(1); }'
assert 8 'fib(x){ if(x==1) { return 1;} else if(x==2) { return 1;} else { return fib(x-1) + fib(x-2);} } main() { return fib(6); }'

echo OK
