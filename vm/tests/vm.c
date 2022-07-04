/**
 * This VM loads and executes a RISC-V ELF binary.
 *
 * The goal is to run unit tests for the RISC-V processor to ensure that the
 * instructions implementation (from the streaming VM) is correct.
 *
 * The tests from https://github.com/riscv-software-src/riscv-tests can be built
 * with the following command-line:
 *
 *   RISCV_PREFIX=riscv32-unknown-linux-gnu- RISCV_GCC_OPTS='-static -mcmodel=medany
 *   -fvisibility=hidden -nostdlib -nostartfiles -I/usr/local/riscv32-unknown-linux-gnu/include/'
 *   XLEN=32 make -j -B
 *
 * The patch env.diff should be applied against the git sub-module
 * https://github.com/riscv-software-src/riscv-tests first.
 *
 * find riscv-tests/isa/ -name '*-p-*' | grep -v '\.dump' | while read l; do (echo $l;
 * ./build/vm "$l"); done
 */

#include <elf.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "rv_cpu.h"
#include "stream.h"

static void *stack;
static void *code;
static void *data;

#define STACK_ADDR 0xdead0000
#define STACK_SIZE 8192

struct vm_section_s {
    uint32_t addr;
    size_t size;
    uint8_t *data;
};

enum vm_section_e {
    VM_SECTION_CODE,
    VM_SECTION_DATA_RO,
    VM_SECTION_DATA_RW,
    VM_SECTION_STACK,
    VM_SECTION_MAX,
};

static struct vm_section_s vm_sections[VM_SECTION_MAX];
static size_t VM_PAGE_SIZE;

static bool in_section(enum vm_section_e n, const uint32_t addr, const size_t size)
{
    return ((addr >= vm_sections[n].addr) &&
            (addr + size <= vm_sections[n].addr + vm_sections[n].size));
}

static bool get_pointer(enum vm_section_e n, uint32_t addr, size_t size, void **p)
{
    if (addr < vm_sections[n].addr) {
        return false;
    }

    if (addr + size > vm_sections[n].addr + vm_sections[n].size) {
        return false;
    }

    *p = (uint8_t *)vm_sections[n].data + (addr - vm_sections[n].addr);

    return true;
}

bool mem_read(const uint32_t addr, const size_t size, uint32_t *value)
{
    void *p;

    if (!get_pointer(VM_SECTION_CODE, addr, size, &p) &&
        !get_pointer(VM_SECTION_STACK, addr, size, &p) &&
        !get_pointer(VM_SECTION_DATA_RO, addr, size, &p) &&
        !get_pointer(VM_SECTION_DATA_RW, addr, size, &p)) {
        return false;
    }

    switch (size) {
    case 1:
        *value = *(uint8_t *)p;
        break;
    case 2:
        *value = *(uint16_t *)p;
        break;
    case 4:
        *value = *(uint32_t *)p;
        break;
    default:
        return false;
    }

    return true;
}

bool mem_write(const uint32_t addr, const size_t size, const uint32_t value)
{
    void *p;

    if (!get_pointer(VM_SECTION_STACK, addr, size, &p) &&
        !get_pointer(VM_SECTION_DATA_RW, addr, size, &p)) {
        return false;
    }

    switch (size) {
    case 1:
        *(uint8_t *)p = value & 0xff;
        break;
    case 2:
        *(uint16_t *)p = value & 0xffff;
        break;
    case 4:
        *(uint32_t *)p = value;
        break;
    default:
        return false;
    }

    return true;
}

static bool ecall_pass = false;

bool ecall(struct rv_cpu *cpu)
{
    if (cpu->regs[RV_REG_A0] == 1337) {
        fprintf(stderr, "pass!\n");
        ecall_pass = true;
    }

    return false;
}

static bool map_segment(int fd, Elf32_Phdr *ph, bool verbose)
{
    enum vm_section_e n;
    int prot;

    switch (ph->p_flags) {
    case PF_R | PF_X:
        n = VM_SECTION_CODE;
        prot = PROT_READ | PROT_EXEC;
        break;
    case PF_R:
        n = VM_SECTION_DATA_RO;
        prot = PROT_READ;
        break;
    case PF_R | PF_W:
        n = VM_SECTION_DATA_RW;
        prot = PROT_READ | PROT_WRITE;
        break;
    default:
        return false;
    }

    if (vm_sections[n].size != 0) {
        fprintf(stderr, "duplicate section\n");
        return false;
    }

    if (ph->p_memsz <= ph->p_filesz) {
        off_t offset = ph->p_offset & ~(VM_PAGE_SIZE - 1);
        size_t size = ph->p_memsz + (ph->p_offset - offset);
        int flags = MAP_PRIVATE;

        if (verbose) {
            fprintf(stderr, "map segment: 0x%lx bytes at 0x%08x (offset: 0x%lx)\n", size,
                    ph->p_vaddr, offset);
        }

        if (lseek(fd, 0, SEEK_SET) != 0) {
            warn("lseek");
            return false;
        }

        uint8_t *data = mmap(NULL, size, prot, flags, fd, offset);
        if (data == MAP_FAILED) {
            warn("mmap");
            return false;
        }

        vm_sections[n].data = data + (ph->p_offset - offset);
    } else {
        uint8_t *data = calloc(ph->p_memsz, sizeof(uint8_t));
        if (data == NULL) {
            warn("calloc");
            return false;
        }

        if (lseek(fd, ph->p_offset, SEEK_SET) != ph->p_offset) {
            warn("lseek");
            return false;
        }

        if (read(fd, data, ph->p_filesz) != ph->p_filesz) {
            warn("read");
            return false;
        }

        vm_sections[n].data = data;
    }

    vm_sections[n].addr = ph->p_vaddr;
    vm_sections[n].size = ph->p_memsz;

    return true;
}

static bool setup_memory(int fd, uint32_t *entrypoint, bool verbose)
{
    struct stat st;
    if (fstat(fd, &st) != 0) {
        warn("fstat");
        return false;
    }

    uint8_t *elf = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (elf == MAP_FAILED) {
        warn("mmap");
        return false;
    }

    Elf32_Ehdr *eh = (Elf32_Ehdr *)elf;
    if (eh->e_machine != EM_RISCV) {
        warn("unexpected ELF machine");
        return false;
    }

    for (int i = 0; i < eh->e_phnum; i++) {
        Elf32_Phdr *ph = (Elf32_Phdr *)(elf + (eh->e_phoff + eh->e_phentsize * i));
        if (verbose) {
            fprintf(stderr, "program header: 0x%x 0x%x 0x%x\n", ph->p_vaddr, ph->p_offset,
                    ph->p_flags);
        }
        if (ph->p_type == PT_LOAD) {
            if (!map_segment(fd, ph, verbose)) {
                return false;
            }
        }
    }

    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
    void *data = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, flags, 0, 0);
    if (data == MAP_FAILED) {
        warn("mmap");
        return false;
    }

    vm_sections[VM_SECTION_STACK].addr = STACK_ADDR;
    vm_sections[VM_SECTION_STACK].size = STACK_SIZE;
    vm_sections[VM_SECTION_STACK].data = data;

    *entrypoint = eh->e_entry;

    if (munmap(elf, st.st_size) != 0) {
        warn("munmap");
        return false;
    }

    return true;
}

static uint32_t get_instruction(uint32_t pc, uint32_t *instruction)
{
    return mem_read(pc, sizeof(*instruction), instruction);
}

/**
 * @return false if the test failed, true otherwise
 */
static bool run_test(uint32_t entrypoint, bool verbose)
{
    struct rv_cpu cpu;
    uint32_t instruction;
    bool stop;

    memset(&cpu, 0, sizeof(cpu));
    cpu.pc = entrypoint;
    cpu.regs[RV_REG_SP] = STACK_ADDR + STACK_SIZE;

    do {
        if (!get_instruction(cpu.pc, &instruction)) {
            fprintf(stderr, "get_instruction failed\n");
            break;
        }
        if (verbose) {
            fprintf(stderr, "vm: 0x%x: 0x%x\n", cpu.pc, instruction);
        }
        stop = !rv_cpu_execute(&cpu, instruction);
    } while (!stop);

    return ecall_pass;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        err(EXIT_FAILURE, "Usage: %s <program>", argv[0]);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        err(EXIT_FAILURE, "open(%s)", argv[1]);
    }

    VM_PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

    uint32_t entrypoint;
    if (!setup_memory(fd, &entrypoint, false)) {
        return EXIT_FAILURE;
    }

    int status = run_test(entrypoint, false) ? EXIT_SUCCESS : EXIT_FAILURE;

    close(fd);

    return EXIT_SUCCESS;
}
