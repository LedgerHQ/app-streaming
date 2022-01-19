CFLAGS=-I/usr/local/riscv32-unknown-linux-gnu/include -mstrict-align -march=rv32g -mno-div -mno-fdiv -Os -Wall
LDFLAGS=-L/usr/local/riscv32-unknown-linux-gnu/lib -Wl,-z,noseparate-code

all: ux.a

#ux: bagl.o bagl_fonts.o glyphs.o ux_idea.o ux_flow_engine.o ux_layout_pb.o ux_layout_pn.o ux_stack.o
#	$(CC) -static -nostdlib -o $@ $^ $(LDFLAGS) -lc

ux.a: bagl.o bagl_fonts.o bagl_glyphs.o glyphs.o ux_idea.o ux_flow_engine.o ux_layout_pb.o ux_layout_pn.o ux_stack.o
	ar rs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^
