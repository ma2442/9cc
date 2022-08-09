#!/bin/bash

# 実用ソースコードによる9ccテストスクリプト
######################################################################
# 関数定義部
DIR=./test
TMPDIR=$DIR/tmp
YOUR_ANS=$TMPDIR/ans_your.txt
EXPECT_ANS=$TMPDIR/ans_expect.txt

judge()(
sed -z -i 's/\r//g' $YOUR_ANS
sed -z -i 's/\r//g' $EXPECT_ANS
d=$(diff $EXPECT_ANS $YOUR_ANS)
if test "$d" == "" ; then
    echo "[ OK! ]" "$compiler"
    cat $YOUR_ANS
    echo
else
    echo "[ WRONG ANSWER! ]" "$compiler"
    echo "expected answer : "
    cat $EXPECT_ANS
    echo "your answer : "
    cat $YOUR_ANS
    exit 1
fi

)

runtest(){
NAME=$1
echo ----- test "$NAME" -----
INPUT="$DIR"/"$NAME".c
# tr -d < $INPUT '\n' > $TMPDIR/tmp.c
# ./9cc "$(cat $TMPDIR/tmp.c)" > tmp.s 
./"$compiler" "$INPUT" > tmp.s 
cc -o tmp tmp.s testfuncs.o
./tmp > $YOUR_ANS

OUTNAME="$TMPDIR"/tmpcc
cc -S "$INPUT" -o "$OUTNAME".s -w
cc -o "$OUTNAME" "$OUTNAME".s testfuncs.o
$OUTNAME > $EXPECT_ANS
judge
}

######################################################################
# テスト実行部

compiler="$1"
runtest nqueen
echo

echo "all done."
echo