# -mabi=ilp32 -march=rv32i
# no compression: rv32id

ifeq ($(shell clang -v 2>/dev/null && echo -n yes),yes)
	CC=clang
	CFLAGS=--target=riscv32 -Oz
else
	CFLAGS=-mno-div -mno-fdiv -mstrict-align -Os
endif

CFLAGS += -I/usr/local/riscv32-unknown-linux-gnu/include -Iux/ -march=rv32g -Wall
LDFLAGS = -L/usr/local/riscv32-unknown-linux-gnu/lib -z noseparate-code

all: test

test: test.o math.o std.o sdk.o start.o sha256sum.o ux/ux.a
	$(LD) -static -nostdlib -o $@ $^ $(LDFLAGS) -lc
	#riscv32-unknown-linux-gnu-strip $@

ux/ux.o:
	make -C ux/

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f *.o test
