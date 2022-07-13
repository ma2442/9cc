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
    echo "[ OK! ]"
    cat $YOUR_ANS
    echo
else
    echo "[ WRONG ANSWER! ]"
    echo "expected answer : "
    cat $EXPECT_ANS
    echo "your answer : "
    cat $YOUR_ANS
    exit 1
fi

)

runtest(){
NAME=$1
echo ----- test $NAME -----
INPUT_9CC=${DIR}/${NAME}_9cc.c
INPUT_CC=${DIR}/${NAME}.c
# tr -d < $INPUT_9CC '\n' > $TMPDIR/tmp.c
# ./9cc "$(cat $TMPDIR/tmp.c)" > tmp.s 
./9cc $INPUT_9CC > tmp.s 
cc -o tmp tmp.s testfuncs.o
./tmp > $YOUR_ANS

cc $INPUT_CC -o ${TMPDIR}/a.out
./${TMPDIR}/a.out > $EXPECT_ANS
judge
}

######################################################################
# テスト実行部
make 
runtest 8queen
echo "all done."