CFLAGS=-I/usr/local/riscv32-unknown-linux-gnu/include
LFLAGS=-L/usr/local/riscv32-unknown-linux-gnu/lib

all: test

test: test.o
	$(CC) -static -nostdlib -o $@ $^ $(LDFLAGS)

test.o: test.c
	$(CC) $(CFLAGS) -mstrict-align -mabi=ilp32e -march=rv32e -c -o $@ $^
