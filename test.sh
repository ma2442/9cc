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
assert 2 'a=b=2;'
assert 4 'a=b=(+3+5)/-2*-(1);'
assert 1 'z =c =9 == 2* 4 + (1 != (+3+5)/-2*-(1) >= 5 > 1);' 
assert 5 'a=1+2;d=(1==1)*(a+2);'
assert 9 'ident_tESt01=1+4; _name__=8/2; val=ident_tESt01 - _name__; val+8;'
# assert 1 '(a+1)=2;'
echo OK
