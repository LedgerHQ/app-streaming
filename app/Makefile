# -mabi=ilp32 -march=rv32i

# no compression: rv32id
CFLAGS=-I/usr/local/riscv32-unknown-linux-gnu/include -mstrict-align -march=rv32g -mno-div -mno-fdiv
LDFLAGS=-L/usr/local/riscv32-unknown-linux-gnu/lib

all: test

test: test.o std.o start.o
	$(CC) -static -nostdlib -o $@ $^ $(LDFLAGS) -lc

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f *.o test
