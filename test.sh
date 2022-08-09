#!/bin/bash

assert() {
    expected="$1"
    input="$2"
    echo "$input" > test/tmp/tmp.c
    "$compiler" test/tmp/tmp.c > tmp.s
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
    "$compiler" test/"$input" > tmp.s
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

compiler="./9cc"
echo "--  9cc made by cc test --"
assertf 255 test1.c
assertf 255 test2.c
assertf 255 tmp.c
assertf 255 test3.c
assertf 1 comment.c
assertf 255 test_quick.c
assertf 255 test4.c
assertf 255 test5.c
echo "[9cc COMPLETE]"
echo

compiler="./9cc_self"
echo "--  9cc_self made by 9cc test --"
assertf 255 test1.c
assertf 255 test2.c
assertf 255 tmp.c
assertf 255 test3.c
assertf 1 comment.c
assertf 255 test_quick.c
assertf 255 test4.c
assertf 255 test5.c
exit 0
echo "[9cc_self COMPLETE]"
echo

echo OK
echo

