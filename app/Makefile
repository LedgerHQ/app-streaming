all: test

test: test.o
	$(CC) -static -nostdlib -o $@ $^

test.o: test.c
	$(CC) -mstrict-align -mabi=ilp32e -march=rv32e -c -o $@ $^
