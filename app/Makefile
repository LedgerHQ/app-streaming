# -mabi=ilp32 -march=rv32i
# no compression: rv32id

CFLAGS=-I/usr/local/riscv32-unknown-linux-gnu/include -mstrict-align -march=rv32g -mno-div -mno-fdiv -Os -Wall
LDFLAGS=-L/usr/local/riscv32-unknown-linux-gnu/lib -Wl,-z,noseparate-code

all: test

test: test.o std.o sdk.o start.o sha256sum.o
	$(CC) -static -nostdlib -o $@ $^ $(LDFLAGS) -lc
	#riscv32-unknown-linux-gnu-strip $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f *.o test
