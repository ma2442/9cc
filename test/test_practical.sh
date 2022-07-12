#!/bin/bash

# 実用ソースコードによる9ccテストスクリプト

DIR=./test
INPUT_9CC=$DIR/8queen_9cc.c
INPUT_CC=$DIR/8queen.c

TMPDIR=$DIR/tmp
YOUR_ANS=$TMPDIR/ans_your.txt
EXPECT_ANS=$TMPDIR/ans_expect.txt

judge(){
sed -z -i 's/\r//g' $YOUR_ANS
sed -z -i 's/\r//g' $EXPECT_ANS
d=$(diff $EXPECT_ANS $YOUR_ANS)
if test "$d" == "" ; then
    echo "[ OK! ]"
else
    echo "[ WRONG ANSWER! ]"
    echo "expected answer : "
    cat $EXPECT_ANS
    echo "your answer : "
fi
cat $YOUR_ANS
echo
echo
}

make 

tr -d < $INPUT_9CC '\n' > $TMPDIR/tmp.c
./9cc "$(cat $TMPDIR/tmp.c)" > tmp.s 
cc -o tmp tmp.s testfuncs.o
./tmp > $YOUR_ANS

cc $INPUT_CC -o $TMPDIR/a.out
./$TMPDIR/a.out > $EXPECT_ANS

judge
