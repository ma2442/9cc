# -std=c11 : c11規格を指定
# -g デバッグ情報出力
# -static スタティックリンクする
CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(filter-out find.o, $(SRCS:.c=.o))
# OBJS=incld.o
# ASEMS=$(SRCS:.c=.s)
ASEMS=find.s
ASEMS_SELF=find_self.s

# link
# 9cc: $(OBJS)
# 	$(CC) -o 9cc $(OBJS) $(LDFLAGS)
9cc_self: $(ASEMS_SELF) 9cc
	$(CC) -o 9cc_self $(ASEMS_SELF) $(OBJS) $(LDFLAGS)

find_self.s: 9cc 9cc_manual.h 9cc.h
	./"9cc" find.c > find_self.s
	./support/shortenasm find_self.s > tmpself.s
	mv tmpself.s find_self.s
	

9cc: $(ASEMS) $(OBJS)
	$(CC) -o 9cc $(ASEMS) $(OBJS) $(LDFLAGS)

$(OBJS): 9cc_auto.h 9cc.h

find.s: 9cc_manual.h 9cc.h

%.s: %.c
	cc $(CFLAGS) -S $< -o $@  -masm=intel 

incld.o : incld.c
	gcc $(CFLAGS) -c -o incld.o incld.c

test: 9cc_self
	./test.sh

testp: 9cc_self
	./test_practical.sh

clean:
	rm -f 9cc *.o *~ tmp* find.s  9cc_self find_self.s

.PHONY: test testp clean

