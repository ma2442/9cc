# -std=c11 : c11規格を指定
# -g デバッグ情報出力
# -static スタティックリンクする
CFLAGS=-std=c11 -g -static
SRCS=$(filter-out src/incld.c, $(wildcard src/*.c))
FILES=$(notdir $(SRCS))
OBJS=incld.o
ASEMS=$(addprefix asm/, $(FILES:.c=.s))
ASEMS_SELF=$(ASEMS:.s=_self.s)
ASEMS_SELF2=$(ASEMS:.s=_self2.s)
all:
	@echo $(ASEMS_SELF2)

9cc: $(ASEMS) $(OBJS)
	$(CC) -o $@ $(ASEMS) $(OBJS) $(LDFLAGS)

9cc_self: $(ASEMS_SELF) 9cc
	$(CC) -o $@ $(ASEMS_SELF) $(OBJS) $(LDFLAGS)

9cc_self2: $(ASEMS_SELF2) 9cc_self
	$(CC) -o $@ $(ASEMS_SELF2) $(OBJS) $(LDFLAGS)

$(OBJS): src/9cc_auto.h src/9cc.h
$(ASEMS): src/9cc_manual.h src/9cc.h
$(ASEMS_SELF): 9cc src/9cc_manual.h src/9cc.h
$(ASEMS_SELF2): 9cc_self src/9cc_manual.h src/9cc.h

asm/%_self.s: src/%.c 9cc
	./"9cc" $< > $@; if [ $$? -ne 0 ]; then rm $@; fi
	./support/shortenasm $@ > asm/_self.s
	mv asm/_self.s $@

asm/%_self2.s: src/%.c 9cc_self
	./"9cc_self" $< > $@; if [ $$? -ne 0 ]; then rm $@; fi
	./support/shortenasm $@ > asm/_self2.s
	cmp asm/_self2.s asm/$*_self.s
	mv asm/_self2.s $@
	rm asm/$*_self.s

asm/%.s: src/%.c
	cc $(CFLAGS) -S $< -o $@  -masm=intel 

%.o : src/%.c
	gcc $(CFLAGS) -c $< -o $@

test: test9cc testself testself2

test9cc: 9cc test/testfuncs.o
	./test.sh ./9cc

testself: 9cc_self test/testfuncs.o
	./test.sh ./9cc_self

testself2: 9cc_self2
	cmp 9cc_self2 9cc_self

testp: testp9cc testpself

testp9cc: 9cc test/testfuncs.o
	./test_practical.sh ./9cc

testpself: 9cc_self test/testfuncs.o
	./test_practical.sh ./9cc_self

clean:
	rm -f 9cc 9cc_self *.o *~ tmp* $(ASEMS) $(ASEMS_SELF) $(ASEMS_SELF2)

.PHONY: test testp test9cc testp9cc testself testself2 testpself clean

