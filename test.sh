#!/bin/bash

assertf(){
    expected="$1"
    input="$2"
    "$compiler" test/"$input" > tmp.s
    res="$?"
    if [ "$res" -ne 0 ]; then exit "$res"; fi
    cc -o tmp tmp.s test/testfuncs.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

compiler="$1"
echo "--" "$compiler" "test --"
assertf 255 test1.c
assertf 255 test2.c
assertf 255 test3.c
assertf 1 comment.c
assertf 255 test_quick.c
assertf 255 test4.c
assertf 255 test5.c

echo "[ OK ]" "$compiler"
echo
exit 0
