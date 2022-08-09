# -std=c11 : c11規格を指定
# -g デバッグ情報出力
# -static スタティックリンクする
CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
ASEMS=find.s nest.s parse.s type.s util.s
OBJS=$(filter-out $(ASEMS:.s=.o), $(SRCS:.c=.o))
ASEMS_SELF=$(ASEMS:.s=_self.s)

9cc: $(ASEMS) $(OBJS)
	$(CC) -o 9cc $(ASEMS) $(OBJS) $(LDFLAGS)

$(OBJS): 9cc_auto.h 9cc.h

9cc_self: $(ASEMS_SELF) 9cc
	$(CC) -o 9cc_self $(ASEMS_SELF) $(OBJS) $(LDFLAGS)

$(ASEMS): 9cc_manual.h 9cc.h
$(ASEMS_SELF): 9cc 9cc_manual.h 9cc.h
%_self.s: %.c
	./"9cc" $< > $@; if [ $$? -ne 0 ]; then rm $@; fi
	./support/shortenasm $@ > tmpself.s
	mv tmpself.s $@
	

%.s: %.c
	cc $(CFLAGS) -S $< -o $@  -masm=intel 

incld.o : incld.c
	gcc $(CFLAGS) -c -o incld.o incld.c

test: test9cc testself

test9cc: 9cc
	./test.sh ./9cc

testself: 9cc_self
	./test.sh ./9cc_self

testp: testp9cc testpself

testp9cc: 9cc
	./test_practical.sh ./9cc

testpself: 9cc_self
	./test_practical.sh ./9cc_self

clean:
	rm -f 9cc *.o *~ tmp* 9cc_self $(ASEMS) $(ASEMS_SELF)

.PHONY: test testp test9cc testp9cc testself testpself clean

