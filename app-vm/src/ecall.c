#include <string.h>

#include "ecall.h"

#include "rv.h"
#include "types.h"


static void debug_write(char *buf)
{
  asm volatile (
     "movs r0, #0x04\n"
     "movs r1, %0\n"
     "svc      0xab\n"
     :: "r"(buf) : "r0", "r1"
  );
}

void ecall(struct rv_cpu *cpu)
{
    uint32_t nr = cpu->regs[5];

    switch (nr) {
    case 0:
        debug_write(cpu->regs[11]);
        break;
    default:
        os_sched_exit(1);
    }
}

