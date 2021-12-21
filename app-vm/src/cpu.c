#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "ecall.h"
#include "rv.h"
#include "types.h"

// Used to set least significant bit to zero.
static const u32 ZERO_LSB = ~((u32) 1);

static const u32 MOD_32 = 0x1f;

static const u32 INST_SIZE = 4;

// Register mnemonics as defined in the "Programmer's Handbook"
static const char *ABI_MNEMONIC[RV_REG_COUNT] = {
    "zero", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2",
    "s0",   "s1", "a0",  "a1",  "a2", "a3", "a4", "a5",
    "a6",   "a7", "s2",  "s3",  "s4", "s5", "s6", "s7",
    "s8",   "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void rv_cpu_reset(struct rv_cpu *cpu)
{
    memset(cpu, 0, sizeof(struct rv_cpu));
}

void rv_cpu_dump_regs(struct rv_cpu *cpu)
{
    int i;

    for (i = 0; i < RV_REG_COUNT; i++) {
        /*printf("%2d: %4s = 0x%08x (%"PRIu32")\n",
            i,
            ABI_MNEMONIC[i],
            cpu->regs[i],
            cpu->regs[i]);*/
    }
}

enum rv_op rv_cpu_decode(u32 inst)
{
    switch (inst & 0x0000007f) {
        case 0x00000037: return RV_OP_LUI;
        case 0x00000017: return RV_OP_AUIPC;
        case 0x0000006f: return RV_OP_JAL;
        case 0x00000067:
            switch (inst & 0x0000707f) {
                case 0x00000067: return RV_OP_JALR;
                default: return RV_OP_UNKNOWN;
            }
        case 0x00000063:
            switch (inst & 0x0000707f) {
                case 0x00000063: return RV_OP_BEQ;
                case 0x00001063: return RV_OP_BNE;
                case 0x00004063: return RV_OP_BLT;
                case 0x00005063: return RV_OP_BGE;
                case 0x00006063: return RV_OP_BLTU;
                case 0x00007063: return RV_OP_BGEU;
                default: return RV_OP_UNKNOWN;
            }
        case 0x00000003:
            switch (inst & 0x0000707f) {
                case 0x00000003: return RV_OP_LB;
                case 0x00001003: return RV_OP_LH;
                case 0x00002003: return RV_OP_LW;
                case 0x00004003: return RV_OP_LBU;
                case 0x00005003: return RV_OP_LHU;
                default: return RV_OP_UNKNOWN;
            }
        case 0x00000023:
            switch (inst & 0x0000707f) {
                case 0x00000023: return RV_OP_SB;
                case 0x00001023: return RV_OP_SH;
                case 0x00002023: return RV_OP_SW;
                default: return RV_OP_UNKNOWN;
            }
        case 0x00000013:
            switch (inst & 0x0000707f) {
                case 0x00000013: return RV_OP_ADDI;
                case 0x00002013: return RV_OP_SLTI;
                case 0x00003013: return RV_OP_SLTIU;
                case 0x00004013: return RV_OP_XORI;
                case 0x00006013: return RV_OP_ORI;
                case 0x00007013: return RV_OP_ANDI;
                case 0x00001013:
                    switch (inst & 0xfe00707f) {
                        case 0x00001013: return RV_OP_SLLI;
                        default: return RV_OP_UNKNOWN;
                    }
                case 0x00005013:
                    switch (inst & 0xfe00707f) {
                        case 0x00005013: return RV_OP_SRLI;
                        case 0x40005013: return RV_OP_SRAI;
                        default: return RV_OP_UNKNOWN;
                    }
                default: return RV_OP_UNKNOWN;
            }
        case 0x00000033:
            switch (inst & 0xfe00707f) {
                case 0x00000033: return RV_OP_ADD;
                case 0x40000033: return RV_OP_SUB;
                case 0x00001033: return RV_OP_SLL;
                case 0x00002033: return RV_OP_SLT;
                case 0x00003033: return RV_OP_SLTU;
                case 0x00004033: return RV_OP_XOR;
                case 0x00005033: return RV_OP_SRL;
                case 0x40005033: return RV_OP_SRA;
                case 0x00006033: return RV_OP_OR;
                case 0x00007033: return RV_OP_AND;
                default: return RV_OP_UNKNOWN;
            }
        case 0x0000000f:
            switch (inst & 0x0000707f) {
                case 0x0000000f:
                    switch (inst & 0xffffffff) {
                        case 0x8330000f: return RV_OP_FENCE_TSO;
                        case 0x0100000f: return RV_OP_PAUSE;
                        default: return RV_OP_FENCE;
                    }
                default: return RV_OP_UNKNOWN;
            }
        case 0x00000073:
            switch (inst & 0xffffffff) {
                case 0x00000073: return RV_OP_ECALL;
                case 0x00100073: return RV_OP_EBREAK;
                default: return RV_OP_UNKNOWN;
            }
        default: return RV_OP_UNKNOWN;
    }
}

static inline u32 imm_i(union rv_inst inst)
{
    return (u32) inst.i.i11_0;
}

static inline u32 imm_u(union rv_inst inst)
{
    return (u32) (inst.u.i31_12 << 12);
}

static inline u32 imm_s(union rv_inst inst)
{
    return (u32) (inst.s.i11_5 << 5 | inst.s.i4_0);
}


static inline u32 imm_b(union rv_inst inst)
{
    return (u32) (inst.b.i12   << 12 |
                  inst.b.i11   << 11 |
                  inst.b.i10_5 <<  5 |
                  inst.b.i4_1  <<  1);
}

static inline u32 imm_j(union rv_inst inst)
{
    return (u32) (inst.j.i20    << 20 |
                  inst.j.i19_12 << 12 |
                  inst.j.i11    << 11 |
                  inst.j.i10_1  <<  1);
}

// sign extend a 16 bit value
static inline uint32_t sign_extend_h(uint32_t x) {
  return (int32_t)((int16_t)x);
}

// sign extend an 8 bit value
static inline uint32_t sign_extend_b(uint32_t x) {
  return (int32_t)((int8_t)x);
}

uint32_t mem_read(uint32_t addr, size_t size);
void mem_write(uint32_t addr, size_t size, uint32_t value);

void rv_cpu_execute(struct rv_cpu *cpu, u32 instruction)
{
    union rv_inst inst = { .inst = instruction };
    enum rv_op op = rv_cpu_decode(instruction);
    u32 pc_inc = INST_SIZE;
    u32 pc_set = cpu->pc;

    // Execute
    switch (op) {
        case RV_OP_ADD:
            cpu->regs[inst.rd] = cpu->regs[inst.rs1] + cpu->regs[inst.rs2];
            break;

        case RV_OP_ADDI:
            cpu->regs[inst.rd] = cpu->regs[inst.rs1] + imm_i(inst);
            break;

        case RV_OP_SUB:
            cpu->regs[inst.rd] = cpu->regs[inst.rs1] - cpu->regs[inst.rs2];
            break;

        case RV_OP_AND:
            cpu->regs[inst.rd] = cpu->regs[inst.rs1] & cpu->regs[inst.rs2];
            break;

        case RV_OP_ANDI:
            cpu->regs[inst.rd] = cpu->regs[inst.rs1] & imm_i(inst);
            break;

        case RV_OP_OR:
            cpu->regs[inst.rd] = cpu->regs[inst.rs1] | cpu->regs[inst.rs2];
            break;

        case RV_OP_ORI:
            cpu->regs[inst.rd] = cpu->regs[inst.rs1] | imm_i(inst);
            break;

        case RV_OP_XOR:
            cpu->regs[inst.rd] = cpu->regs[inst.rs1] ^ cpu->regs[inst.rs2];
            break;

        case RV_OP_XORI:
            cpu->regs[inst.rd] = cpu->regs[inst.rs1] ^ imm_i(inst);
            break;

        case RV_OP_SLL:
            cpu->regs[inst.rd] = cpu->regs[inst.rs1] << (cpu->regs[inst.rs2] & MOD_32);
            break;

        case RV_OP_SRL:
            cpu->regs[inst.rd] = cpu->regs[inst.rs1] >> (cpu->regs[inst.rs2] & MOD_32);
            break;

        case RV_OP_SLLI:
            cpu->regs[inst.rd] = cpu->regs[inst.rs1] << (imm_i(inst) & MOD_32);
            break;

        case RV_OP_SRLI:
            cpu->regs[inst.rd] = cpu->regs[inst.rs1] >> (imm_i(inst) & MOD_32);
            break;

        case RV_OP_SRA:
            cpu->regs[inst.rd] = (u32) ((i32) cpu->regs[inst.rs1] >> (cpu->regs[inst.rs2] & MOD_32));
            break;

        case RV_OP_SRAI:
            cpu->regs[inst.rd] = (u32) ((i32) cpu->regs[inst.rs1] >> (imm_i(inst) & MOD_32));
            break;

        case RV_OP_SLT:
            cpu->regs[inst.rd] = (i32) cpu->regs[inst.rs1] < (i32) cpu->regs[inst.rs2];
            break;

        case RV_OP_SLTI:
            cpu->regs[inst.rd] = (i32) cpu->regs[inst.rs1] < (i32) imm_i(inst);
            break;

        case RV_OP_SLTU:
            cpu->regs[inst.rd] = cpu->regs[inst.rs1] < cpu->regs[inst.rs2];
            break;

        case RV_OP_SLTIU:
            cpu->regs[inst.rd] = cpu->regs[inst.rs1] < imm_i(inst);
            break;

        case RV_OP_LUI:
            cpu->regs[inst.rd] = imm_u(inst);
            break;

        case RV_OP_AUIPC:
            cpu->regs[inst.rd] = cpu->pc + imm_u(inst);
            break;

        case RV_OP_JAL:
            pc_inc = imm_j(inst);
            cpu->regs[inst.rd] = cpu->pc + INST_SIZE;
            break;

        case RV_OP_JALR:
            //! Attention, rd and rs1 may be the same register.
            pc_set = (cpu->regs[inst.rs1] + imm_i(inst)) & ZERO_LSB;
            pc_inc = 0;
            cpu->regs[inst.rd] = cpu->pc + INST_SIZE;
            break;

        case RV_OP_BEQ:
            pc_inc = inst.rs1 == inst.rs2 ? imm_b(inst) : INST_SIZE;
            break;

        case RV_OP_BNE:
            pc_inc = inst.rs1 != inst.rs2 ? imm_b(inst) : INST_SIZE;
            break;

        case RV_OP_BLT:
            pc_inc = (i32) inst.rs1 < (i32) inst.rs2 ? imm_b(inst) : INST_SIZE;
            break;

        case RV_OP_BGE:
            pc_inc = (i32) inst.rs1 >= (i32) inst.rs2 ? imm_b(inst) : INST_SIZE;
            break;

        case RV_OP_BLTU:
            pc_inc = inst.rs1 < inst.rs2 ? imm_b(inst) : INST_SIZE;
            break;

        case RV_OP_BGEU:
            pc_inc = inst.rs1 >= inst.rs2 ? imm_b(inst) : INST_SIZE;
            break;

        case RV_OP_LB:
            cpu->regs[inst.rd] = sign_extend_b(mem_read(cpu->regs[inst.rs1] + (i32) imm_i(inst), 1));
            break;

        case RV_OP_LH:
            cpu->regs[inst.rd] = sign_extend_h(mem_read(cpu->regs[inst.rs1] + (i32) imm_i(inst), 2));
            break;

        case RV_OP_LW:
            cpu->regs[inst.rd] = mem_read(cpu->regs[inst.rs1] + (i32) imm_i(inst), 4);
            break;

        case RV_OP_LBU:
            cpu->regs[inst.rd] = mem_read(cpu->regs[inst.rs1] + (i32) imm_i(inst), 1);
            break;

        case RV_OP_LHU:
            cpu->regs[inst.rd] = mem_read(cpu->regs[inst.rs1] + (i32) imm_i(inst), 2);
            break;

        case RV_OP_SB:
            mem_write(cpu->regs[inst.rs1] + (i32) imm_s(inst), 1, cpu->regs[inst.rs2]);
            break;

        case RV_OP_SH:
            mem_write(cpu->regs[inst.rs1] + (i32) imm_s(inst), 2, cpu->regs[inst.rs2]);
            break;

        case RV_OP_SW:
            mem_write(cpu->regs[inst.rs1] + (i32) imm_s(inst), 4, cpu->regs[inst.rs2]);
            break;

        case RV_OP_FENCE:
            // Do nothing
            break;

        case RV_OP_FENCE_TSO:
            // Do nothing
            break;

        case RV_OP_PAUSE:
            // Do nothing
            break;

        case RV_OP_ECALL:
            //os_sched_exit(1);
            ecall(cpu);
            break;

        case RV_OP_EBREAK:
            break;

        default:
            // TODO: Handle unsupported opperations
            //printf("Operation not supported: %d\n", op);
            break;
    }
    //! Make sure that no operation can change the x0 register
    cpu->regs[0] = 0;
    cpu->pc = pc_set + pc_inc;
}
