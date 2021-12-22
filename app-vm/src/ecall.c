#include <string.h>

#include "os.h"

#include "ecall.h"

#include "rv.h"
#include "types.h"


static void debug_write(char *msg)
{
  asm volatile (
     "movs r0, #0x04\n"
     "movs r1, %0\n"
     "svc      0xab\n"
     :: "r"(msg) : "r0", "r1"
  );
}

static void test_send(uint32_t n)
{
    *(uint32_t *)&G_io_apdu_buffer[0] = n;
    G_io_apdu_buffer[4] = 0x90;
    G_io_apdu_buffer[5] = 0x00;

    io_exchange(CHANNEL_APDU | IO_RESET_AFTER_REPLIED, 6);
}

void ecall(struct rv_cpu *cpu)
{
    uint32_t nr = cpu->regs[5];

    switch (nr) {
    case 1:
        debug_write((char *)cpu->regs[10]);
        break;
    case 2:
        test_send((uint32_t)cpu->regs[10]);
        break;
    default:
        os_sched_exit(cpu->regs[10]);
        break;
    }
}

