#ifndef RV_CPU_H
#define RV_CPU_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define RV_REG_COUNT (32)

enum {
    RV_REG_ZERO=0,
    RV_REG_RA,
    RV_REG_SP,
    RV_REG_GP,
    RV_REG_TP,
    RV_REG_T0,
    RV_REG_T1,
    RV_REG_T2,
    RV_REG_S0,
    RV_REG_S1,
    RV_REG_A0,
    RV_REG_A1,
    RV_REG_A2,
    RV_REG_A3,
    RV_REG_A4,
    RV_REG_A5,
    RV_REG_A6,
    RV_REG_A7,
    RV_REG_S2,
    RV_REG_S3,
    RV_REG_S4,
    RV_REG_S5,
    RV_REG_S6,
    RV_REG_S7,
    RV_REG_S8,
    RV_REG_S9,
    RV_REG_S10,
    RV_REG_S11,
    RV_REG_T3,
    RV_REG_T4,
    RV_REG_T5,
    RV_REG_T6,
};

enum rv_op {
    RV_OP_UNKNOWN,
    RV_OP_LUI,
    RV_OP_AUIPC,
    RV_OP_JAL,
    RV_OP_JALR,
    RV_OP_BEQ,
    RV_OP_BNE,
    RV_OP_BLT,
    RV_OP_BGE,
    RV_OP_BLTU,
    RV_OP_BGEU,
    RV_OP_LB,
    RV_OP_LH,
    RV_OP_LW,
    RV_OP_LBU,
    RV_OP_LHU,
    RV_OP_SB,
    RV_OP_SH,
    RV_OP_SW,
    RV_OP_ADDI,
    RV_OP_SLTI,
    RV_OP_SLTIU,
    RV_OP_XORI,
    RV_OP_ORI,
    RV_OP_ANDI,
    RV_OP_SLLI,
    RV_OP_SRLI,
    RV_OP_SRAI,
    RV_OP_ADD,
    RV_OP_SUB,
    RV_OP_SLL,
    RV_OP_SLT,
    RV_OP_SLTU,
    RV_OP_XOR,
    RV_OP_SRL,
    RV_OP_SRA,
    RV_OP_OR,
    RV_OP_AND,
    RV_OP_FENCE,
    RV_OP_FENCE_TSO,
    RV_OP_PAUSE,
    RV_OP_ECALL,
    RV_OP_EBREAK,
    RV_OP_MUL,
    RV_OP_MULH,
    RV_OP_MULHSU,
    RV_OP_MULHU,
    RV_OP_DIV,
    RV_OP_DIVU,
    RV_OP_REM,
    RV_OP_REMU
};

union rv_inst {
    u32 inst;
    struct {
        u32 opcode : 7;
        u32 rd     : 5;
        u32 funct3 : 3;
        u32 rs1    : 5;
        u32 rs2    : 5;
        u32 funct7 : 7;
    };
    struct {
        u32 opcode : 7;
        u32 rd     : 5;
        u32 funct3 : 3;
        u32 rs1    : 5;
        u32 rs2    : 5;
        u32 funct7 : 7;
    } r;
    struct {
        u32 opcode : 7;
        u32 rd     : 5;
        u32 funct3 : 3;
        u32 rs1    : 5;
        i32 i11_0  : 12; // Sign extension
    } i;
    struct {
        u32 opcode : 7;
        u32 rd     : 5;
        i32 i31_12 : 20; // Sign extension
    } u;
    struct {
        u32 opcode : 7;
        u32 i4_0   : 5;
        u32 funct3 : 3;
        u32 rs1    : 5;
        u32 rs2    : 5;
        i32 i11_5  : 7; // Sign extension
    } s;
    struct {
        u32 opcode : 7;
        u32 i11    : 1;
        u32 i4_1   : 4;
        u32 funct3 : 3;
        u32 rs1    : 5;
        u32 rs2    : 5;
        u32 i10_5  : 6;
        i32 i12    : 1; // Sign extension
    } b;
    struct {
        u32 opcode : 7;
        u32 rd     : 5;
        u32 i19_12 : 8;
        u32 i11    : 1;
        u32 i10_1  : 10;
        i32 i20    : 1; // Sign extension
    } j;
};

struct rv_cpu {
    u32 pc;
    u32 regs[RV_REG_COUNT];
};

void rv_cpu_reset(struct rv_cpu *cpu);
void rv_cpu_dump_regs(struct rv_cpu *cpu);
bool rv_cpu_execute(struct rv_cpu *cpu, u32 inst);

enum rv_op rv_cpu_decode(u32 inst);

#endif // RV_CPU_H
