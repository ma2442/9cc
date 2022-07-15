# -std=c11 : c11規格を指定
# -g デバッグ情報出力
# -static スタティックリンクする
CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

# link
9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./test.sh

testp: 9cc
	./test_practical.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test testp clean

