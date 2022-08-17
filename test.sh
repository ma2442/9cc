#!/bin/bash

assertf(){
    expected="$1"
    input="$2"
    print="$3"
    "$compiler" test/"$input" > tmp.s
    res="$?"
    if [ "$res" -ne 0 ]; then exit "$res"; fi
    cc -o tmp tmp.s test/testfuncs.o

    if [ "$print" == "" ]; then
        ./tmp
        actual="$?"
    elif [ "$print" == "noprint" ]; then
        ./tmp >/dev/null
        actual="$?"
    elif [ "$print" == "testprint" ]; then
        ./tmp > test/tmp/tmpout.txt
        actual="$?"
        diff test/tmp/tmpout.txt test/"$input".ans.txt > test/tmp/tmpdiff.txt
        print_result="$?"

        if [ "$print_result" -ne 0 ]; then
            echo "$input"  "=> differ output and answer"
            cat test/tmp/tmpdiff.txt
            exit 2
        fi
    fi

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
assertf 255 test2.c testprint
assertf 255 test3.c testprint
assertf 1 comment.c 
assertf 255 test_quick.c testprint
assertf 255 test4.c
assertf 255 test5.c
assertf 255 test6.c noprint

echo "[ OK ]" "$compiler"
echo
exit 0
