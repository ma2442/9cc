#!/bin/bash

assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 '0;'
assert 42 '42;'
assert 21 '5+20-4;'
assert 2 '5 - 3;'
assert 41 ' 12 + 34 - 5 ;'
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'

assert 5 '+10-5;'
assert 6 '-3+9;'
assert 1 '-(-13)+-12;'
assert 4 '(+3+5)/-2*-(1);'

assert 1 '3==1+2;'
assert 0 '3!=1+2;'
assert 1 '11 < 12;'
assert 0 '12 < 12;'
assert 1 '12 <= 12;'
assert 0 '12 <= 11;'
assert 1 '12 > 11;'
assert 0 '11 > 11;'
assert 1 '12 >= 12;'
assert 0 '11 >= 12;'
assert 1 '-2 * (4 - 1)==-1 * (-6 * -1);'
assert 1 '-2 * (4 - 1)!=-1 + -(6 * -1);'
assert 1 '-2 * (4 - 1)<-1 + -(6 * -1);'
assert 0 '-2 * (4 - 1)>-1 + -(6 * -1);'
assert 1 '9 == 2* 4 + (1 != (+3+5)/-2*-(1) >= 5 > 1);'

assert 2 'a=2;'
# assert 1 '(a+1)=2;'
assert 2 'a=b=2;'
assert 4 'a=b=(+3+5)/-2*-(1);'
assert 1 'z =c =9 == 2* 4 + (1 != (+3+5)/-2*-(1) >= 5 > 1);' 
assert 5 'a=1+2;d=(1==1)*(a+2);'
assert 9 'ident_tESt01=1+4; _name__=8/2; val=ident_tESt01 - _name__; val+8;'
assert 1 'return 1;'
assert 9 'return 9; return 1;'
assert 9 'return9 = 9; return return9;'
assert 12 'val1 = 1+4; val2 = 0; val3=(1==1); return 2*(val1+val3);'
# assert 2 'return 2 = 2; return return2;'

assert 2 'if(1) 2; else 3;'
assert 3 'if(0) 2; else 3;'
#assert 2 'if() 2; else 3;'
assert 2 'if(-10) return 2; return 3;'
assert 1 'loop=1; while(loop) return 1; return 2;'
assert 2 'loop=0; while(loop) return 1; return 2;'
assert 0 'loop=100; while(loop) loop = loop - 1; return loop;'
#assert 1 'while() return 1; return 2;'
assert 35 'a=-10; for(i=1; i < 10; i=i+1) a = a + i; return a;'
assert 5 'for(i=0;;) if(i<5)i=i+1; else return i;'
assert 6 'i=0; for(;i<6;) i=i+1; return i;'
assert 7 'i=0; for(;;i=i+1) if(i>=7) return i;'
assert 23 'a=3; l=3; if(a==2) 1; else while(l=l-1) for(i=0; i<5; i= i + 1)  a = a+2; return a;'

echo OK
