#include <stdlib.h>
#include "stdio.h"

#include "../constants.h"
#include "../routines.h"
#include "cpu.h"
#include "memory.h"
#include "clock.h"
#include "cpu_helpers.h"


opcode_handler_t opcodes[256];

struct CPU cpu = {
        .A = 0x01,
        .B = 0x0,
        .C = 0x13,
        .D = 0x0,
        .E = 0xd8,
        .H = 0x01,
        .L = 0x4d,

        .FZ = 1,
        .FN = 0,
        .FH = 1,
        .FC = 1,
        
        .SP = 0xfffe,
        .PC = 0x0100,

        .IME = 0,
};

cycle_t cpu_exec() {
    opcode_t opcode = mem_read(cpu.PC);
    clock.cycles++;

    if (opcode == OPCODE_PREFIX) {
        // read next instruction after prefix-opcode
        cpu.PC++;
        opcode = mem_read(cpu.PC);
        clock.cycles++;
        cpu.PC++;

        return cpu_exec_CB_opcode(opcode);
    }

    // printf("CPU-PC: 0x%04X, OP: 0x%02X\n", cpu.PC, opcode);
    // if (cpu.SP > 0x200)
        // printf("PC: 0x%04X, OP: 0x%02X, A: %02X, SP: %04X, C: %02X, D: %02X, Z: %i\n", cpu.PC, opcode, cpu.A, cpu.SP, cpu.C, cpu.D, cpu.Z);
    // NOP
    if (opcode == 0x00) {
        cpu.PC++;
        return 1;
    }
    log_cpu_full_16(opcode);
    // printf("PC: 0x%04X, OP: 0x%02X, A: %02X, SP: %04X, C: %02X, D: %02X, Z: %i\n", cpu.PC, opcode, cpu.A, cpu.SP, cpu.C, cpu.D, cpu.FZ);
    // printf("\nCPU-PC: 0x%04X, OP: 0x%02X, A: %02X, B: %02X, C: %02X, D: %02X, Z: %i\n", cpu.PC, opcode, cpu.A, cpu.B, cpu.C, cpu.D, cpu.Z);
    
    if (!opcodes[opcode]) {
        printf("CPU: Unknown OP-code: 0x%02X at 0x%04X\n", opcode, cpu.PC);
        exit(-1);
    }

    // printf("op: 0x%02x - flag %02x, flags %02x \n", opcode, cpu.Z, cpu.F);
    cpu.PC++;
    return opcodes[opcode](opcode);
    // printf("op: 0x%02x - flag %02x, flags %02x \n\n", opcode, cpu.Z, cpu.F);
}


/* Control Flow-s */
static cycle_t HALT(opcode_t) {
    printf("HALT\n");
    return 1;
}

static cycle_t STOP(opcode_t) {
    mem_write(REG_DIV, 0);
    // TODO: Implementation
    // return 1;
    printf("STOP\n");
    exit(0);
}

static cycle_t JP_nn(opcode_t) {
    cpu.PC = mem_read_word(cpu.PC);
    printf("jumping to %04x\n", cpu.PC);
    return 4;
}

static cycle_t JP_cond(opcode_t current_opcode) {
    word_t addr = mem_read_word(cpu.PC);
    cpu.PC = cpu.PC + 2;

    byte_t condition_result = 0;
    switch (current_opcode) {
        case 0xC2: // NZ
            condition_result = !cpu.FZ;
            break;
        case 0xD2: // NC
            condition_result = !cpu.FC;
            break;
        case 0xCA: // Z
            condition_result = cpu.FZ;
            break;
        case 0xDA: // C
            condition_result = cpu.FC;
            break;
        default:
            printf("CALL_cond: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }

    if (condition_result) {
        printf("jumping to %04X\n", addr);
        cpu.PC = addr; // jump
        return 4;
    }
    return 3;
}

static cycle_t JR_cond(opcode_t current_opcode) {
    byte_t offset = mem_read(cpu.PC);
    cpu.PC++;

    byte_t condition_result = 0;
    switch (current_opcode) {
        case 0x20: // NZ
            condition_result = !cpu.FZ; break;
        case 0x30: // NC
            condition_result = !cpu.FC; break;
        case 0x28: // Z
            condition_result = cpu.FZ; break;
        case 0x38: // C
            condition_result = cpu.FC; break;
        default:
            printf("JR_cond: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }

    if (condition_result) {
        cpu.PC = cpu.PC + (s_byte_t)offset;
        return 3;
    }
    return 2;
}

static cycle_t RET(opcode_t) {
    cpu.PC = cpu_stack_pop();
    return 4;
}

static cycle_t RET_I(opcode_t) {
    RET(0);
    cpu.IME = 1;
    return 4;
}

static cycle_t RET_cond(opcode_t current_opcode) {
    byte_t condition_result = 0;
    switch (current_opcode) {
        case 0xC0: // NZ
            condition_result = !cpu.FZ;
            break;
        case 0xD0: // NC
            condition_result = !cpu.FC;
            break;
        case 0xC8: // Z
            condition_result = cpu.FZ;
            break;
        case 0xD8: // C
            condition_result = cpu.FC;
            break;
        default:
            printf("CALL_cond: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }

    if (condition_result) {
        cpu.PC = cpu_stack_pop();
        return 5;
    }
    return 2;
}

static cycle_t RST(opcode_t current_opcode) {
    byte_t addr;
    switch (current_opcode) {
        case 0xC7:
            addr = 0x00; break;
        case 0xD7:
            addr = 0x10; break;
        case 0xE7:
            addr = 0x20; break;
        case 0xF7:
            addr = 0x30; break;
        case 0xCF:
            addr = 0x08; break;
        case 0xDF:
            addr = 0x18; break;
        case 0xEF:
            addr = 0x28; break;
        case 0xFF:
            addr = 0x38; break;
        default:
            printf("RST: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }
    cpu_push_pc();
    cpu.PC = bytes_to_word(0x00, addr);
}

static cycle_t CALL_cond(opcode_t current_opcode) {
    word_t addr = mem_read_word(cpu.PC);
    cpu.PC = cpu.PC + 2;

    byte_t condition_result = 0;
    switch (current_opcode) {
        case 0xC4: // NZ
            condition_result = !cpu.FZ;
            break;
        case 0xD4: // NC
            condition_result = !cpu.FC;
            break;
        case 0xCC: // Z
            condition_result = cpu.FZ;
            break;
        case 0xDC: // C
            condition_result = cpu.FC;
            break;
        default:
            printf("CALL_cond: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }
    // printf("addr to call: %04X, Z %02x, cond: %i\n", addr, cpu.Z, cpu.Z, condition_result);

    if (condition_result) {
        cpu_push_pc();
        cpu.PC = addr; // jump
    }
}

static cycle_t CALL(opcode_t) {
    word_t addr = mem_read_word(cpu.PC);
    cpu.PC = cpu.PC + 2;
    cpu.SP--;
    mem_write(cpu.SP, MS_BYTE(cpu.PC));
    cpu.SP--;
    mem_write(cpu.SP, LS_BYTE(cpu.PC));
    
    cpu.PC = addr;
}

static cycle_t JR(opcode_t) {
    byte_t offset = mem_read(cpu.PC);
    cpu.PC++;
    cpu.PC = cpu.PC + (s_byte_t)offset;
}

static cycle_t JP_HL(opcode_t) {
    cpu.PC = cpu_HL();
}
/* -------------- /


/* 16-bit loads */
static cycle_t LD_A_from_mem(opcode_t) {
    word_t target_addr = mem_read_word(cpu.PC);
    cpu.PC += 2;
    cpu.A = mem_read(target_addr);
}

static cycle_t LD_mem_from_A(opcode_t current_opcode) {
    word_t addr;
    switch (current_opcode) {
        case 0x02:
            addr = cpu_BC(); break;
        case 0x12:
            addr = cpu_DE(); break;
        case 0x22: {
            word_t HL = cpu_HL();
            addr = HL;
            cpu_set_HL(HL + 1);
            break;
        }
        case 0x32: {
            word_t HL = cpu_HL();
            addr = HL;
            cpu_set_HL(HL - 1);
            break;
        }
        default:
            printf("LD_mem_from_A: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }
    mem_write(addr, cpu.A);
}

static cycle_t LD_mem_from_SP(opcode_t) {
    word_t addr = mem_read_word(cpu.PC);
    // printf("addr %04X = %02X, addr %04X = %02X\n", addr, LS_BYTE(cpu.SP), addr + 1, MS_BYTE(cpu.SP));
    cpu.PC += 2;
    mem_write(addr, LS_BYTE(cpu.SP));
    mem_write(addr + 1, MS_BYTE(cpu.SP));
}

static cycle_t LD_16reg_from_mem(opcode_t current_opcode) {
    word_t value = mem_read_word(cpu.PC);
    cpu.PC += 2;

    switch (current_opcode) {
        case 0x01:
            cpu_set_BC(value); break;
        case 0x11:
            cpu_set_DE(value); break;
        case 0x21:
            cpu_set_HL(value); break;
        case 0x31:
            cpu.SP = value; break;
        default:
            printf("LD_16reg_from_mem: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }
}

static cycle_t LD_SP_HL(opcode_t) {
    cpu.SP = cpu_HL();
}
/* -------------- */

/* 8-bit loads */
static cycle_t LD_mem_from_reg(opcode_t current_opcode) {
    byte_t data = cpu_get_reg_by_code(current_opcode);
    mem_write(cpu_HL(), data);
}

static cycle_t LD_8reg_to_reg(opcode_t current_opcode) {
    byte_t data = cpu_get_reg_by_code(current_opcode);
    cpu_set_reg_by_code(current_opcode, data);
}

static cycle_t LD_8reg_from_mem(opcode_t current_opcode) {
    byte_t value = mem_read(cpu.PC);
    switch (current_opcode) {
        case 0x06:
            cpu.B = value;
            break;
        case 0x16:
            cpu.D = value;
            break;
        case 0x26:
            cpu.H = value;
            break;
        case 0x36:
            mem_write(cpu_HL(), value);
            break;
        case 0x0E:
            cpu.C = value;
            break;
        case 0x1E:
            cpu.E = value;
            break;
        case 0x2E:
            cpu.L = value;
            break;
        case 0x3E:
            cpu.A = value;
            break;
        default:
            printf("LD_8reg_from_mem: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }
    cpu.PC++;
}

static cycle_t LD_A_from_mem_at_16_reg(opcode_t current_opcode) {
    byte_t data;
    switch (current_opcode) {   
        case 0x0A:
            data = mem_read(cpu_BC());
            break;
        case 0x1A:
            data = mem_read(cpu_DE());
            break;
        case 0x2A: {
            word_t HL = cpu_HL(); 
            data = mem_read(HL);
            cpu_set_HL(HL + 1);
            break;
        }
        case 0x3A: {
            word_t HL = cpu_HL(); 
            data = mem_read(HL);
            cpu_set_HL(HL - 1);
            break;
        }
        default:
            printf("LD_A_from_mem_at_16_reg: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }
    cpu.A = data;
}

static cycle_t LD_mem_at_PC_from_A(opcode_t) {
    word_t target_addr = mem_read_word(cpu.PC);
    cpu.PC += 2;
    mem_write(target_addr, cpu.A);
}

static cycle_t LDH_mem_from_A(opcode_t) {
    byte_t lsb = mem_read(cpu.PC);
    cpu.PC++;
    word_t target_addr = bytes_to_word(0xFF, lsb);
    mem_write(target_addr, cpu.A);
}

static cycle_t LDH_A_from_mem(opcode_t) {
    byte_t target_addr_lsb = mem_read(cpu.PC);
    cpu.PC++;
    word_t target_addr = bytes_to_word(0xFF, target_addr_lsb);
    cpu.A = mem_read(target_addr);
}
/* -------------- */

/* 16-bit arithmetics */
static cycle_t INC_16(opcode_t current_opcode) {
    switch (last_bit(current_opcode)) {
        case 0x00: {
            word_t value = bytes_to_word(cpu.B, cpu.C);
            value++;
            cpu_set_BC(value);
            break;
        };
        case 0x10: {
            word_t value = bytes_to_word(cpu.D, cpu.E);
            value++;
            cpu_set_DE(value);
            break;
        };
        case 0x20: {
            word_t value = bytes_to_word(cpu.H, cpu.L);
            value++;
            cpu_set_HL(value);
            break;
        };
        case 0x30:
            cpu.SP++;
            break;
        default:
            printf("INC_16: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }
}

static cycle_t DEC_16(opcode_t current_opcode) {
    switch (last_bit(current_opcode)) {
        case 0x00: {
            word_t value = bytes_to_word(cpu.B, cpu.C);
            value--;
            cpu_set_BC(value);
            break;
        };
        case 0x10: {
            word_t value = bytes_to_word(cpu.D, cpu.E);
            value--;
            cpu_set_DE(value);
            break;
        };
        case 0x20: {
            word_t value = bytes_to_word(cpu.H, cpu.L);
            value--;
            cpu_set_HL(value);
            break;
        };
        case 0x30:
            cpu.SP--;
            break;
        default:
            printf("DEC_16: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }
}

static cycle_t ADD_16reg_to_HL(opcode_t current_opcode) {
    word_t hl = cpu_HL();
    word_t target;
    switch (last_bit(current_opcode)) {
        case 0x00:
            target = cpu_BC(); break;
        case 0x10:
            target = cpu_DE(); break;
        case 0x20:
            target = hl; break;
        case 0x30:
            target = cpu.SP; break;
        default:
            printf("DEC_16: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }

    unsigned int result = hl + target;
    cpu.FH = CHECK_HALF_CARRY_WORD(hl, target, result);
    cpu.FC = result > 0xFFFF;
    cpu.FN = 0;
    cpu_set_HL((word_t)result);
}

/* -------------- */

/* 8-Bit Arithmetics */
static cycle_t ADD_reg_to_A(opcode_t current_opcode) {
    byte_t value = cpu_get_reg_by_code(current_opcode);
    word_t result = cpu.A + value;
    
    cpu.A = (byte_t)result;    
    cpu_update_flags(cpu.A, value, result, "Z0HC");
}

static cycle_t ADC_reg_to_A(opcode_t current_opcode) {
    byte_t value = cpu_get_reg_by_code(current_opcode);
    word_t result = cpu.A + value + cpu.FC;
    
    cpu.A = (byte_t)result;    
    cpu_update_flags(cpu.A, value, result, "Z0HC");
}

static cycle_t SUB_reg_from_A(opcode_t current_opcode) {
    byte_t value = cpu_get_reg_by_code(current_opcode);
    word_t result = cpu.A - value;
    
    cpu.A = (byte_t)result;    
    cpu_update_flags(cpu.A, value, result, "Z1HC");
}

static cycle_t SBC_reg_from_A(opcode_t current_opcode) {
    byte_t value = cpu_get_reg_by_code(current_opcode);
    word_t result = cpu.A - value - cpu.FC;
    
    cpu.A = (byte_t)result;    
    cpu_update_flags(cpu.A, value, result, "Z1HC");
}

static cycle_t INC_8_reg(opcode_t current_opcode) {
    byte_t target;
    switch (current_opcode) {
        case 0x04:
            target = cpu.B;
            cpu.B++;
            break;
        case 0x14:
            target = cpu.D;
            cpu.D++;
            break;
        case 0x24:
            target = cpu.H;
            cpu.H++;
            break;
        case 0x34:
            target = mem_read(cpu_HL());
            mem_write(cpu_HL(), target + 1);
            break;
        case 0x0C:
            target = cpu.C;
            cpu.C++;
            break;
        case 0x1C:
            target = cpu.E;
            cpu.E++;
            break;
        case 0x2C:
            target = cpu.L;
            cpu.L++;
            break;
        case 0x3C:
            target = cpu.A;
            cpu.A++;
            break;
        default:
            printf("INC_8_reg: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }
    
    cpu_update_flags(target, 1, target + 1, "Z0H-");
}

static cycle_t DEC_8_reg(opcode_t current_opcode) {
    byte_t target;
    switch (current_opcode) {
        case 0x05:
            target = cpu.B;
            cpu.B--;
            break;
        case 0x15:
            target = cpu.D;
            cpu.D--;
            break;
        case 0x25:
            target = cpu.H;
            cpu.H--;
            break;
        case 0x35:
            target = mem_read(cpu_HL());
            mem_write(cpu_HL(), target - 1);
            break;
        case 0x0D:
            target = cpu.C;
            cpu.C--;
            break;
        case 0x1D:
            target = cpu.E;
            cpu.E--;
            break;
        case 0x2D:
            target = cpu.L;
            cpu.L--;
            break;
        case 0x3D:
            target = cpu.A;
            cpu.A--;
            break;
        default:
            printf("DEC_8_reg: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }
    cpu_update_flags(target, 1, target - 1, "Z1H-");
}

static cycle_t AND_8_reg(opcode_t current_opcode) {
    byte_t val = cpu_get_reg_by_code(current_opcode);
    byte_t new_val = cpu.A & val;
    cpu.A = new_val;
    cpu_update_flags(cpu.A, val, new_val, "Z010");
}

static cycle_t XOR_8_reg(opcode_t current_opcode) {
    byte_t val = cpu_get_reg_by_code(current_opcode);
    byte_t new_val = cpu.A ^ val;
    cpu.A = new_val;
    cpu_update_flags(cpu.A, val, new_val, "Z000");
}

static cycle_t OR_8_reg(opcode_t current_opcode) {
    byte_t val = cpu_get_reg_by_code(current_opcode);
    byte_t new_val = cpu.A | val;
    cpu.A = new_val;
    cpu_update_flags(cpu.A, val, new_val, "Z000");
}

static cycle_t CP_8_reg(opcode_t current_opcode) {
    byte_t val = val = cpu_get_reg_by_code(current_opcode);
    word_t new_val = cpu.A - val;
    cpu_update_flags(cpu.A, val, new_val, "Z1HC");
}

// from https://blog.ollien.com/posts/gb-daa/
static cycle_t DAA(opcode_t) {
    byte_t offset, carry = 0;
    byte_t value = cpu.A;

    if ((cpu.FN == 0 && (value & 0xF) > 0x09) || cpu.FH) {
        offset = offset | 0x06;
    }
    
    if ((cpu.FN == 0 && value > 0x99) || cpu.FC) {
        offset = offset | 0x60;
        carry = 1;
    }

    cpu.FC = carry;

    byte_t result;
    if (cpu.FN) {
        result = cpu.A - offset;
    } else {
        result = cpu.A + offset;
    }

    cpu.FZ = result == 0 ? 1 : 0;
    cpu.FH = 0;
    cpu.A = result;
}

static cycle_t RRCA(opcode_t) {
    cpu.FZ = cpu.FH = cpu.FN = 0;
    cpu.FC = cpu.A & 0x1;
    cpu.A = R_ROTATE(cpu.A);
}

static cycle_t RRA(opcode_t) {
    cpu.FZ = cpu.FH = cpu.FN = 0;
    cpu.FC = cpu.A & 0x1;
    cpu.A = R_ROTATE_THROUGH_CARRY(cpu.A);
}
/* -------------- */

/* Stack operations */
static cycle_t PUSH(opcode_t current_opcode) {
    switch (current_opcode) {
        case 0xC5:
            cpu.SP--; mem_write(cpu.SP, cpu.B);
            cpu.SP--; mem_write(cpu.SP, cpu.C);
            break;
        case 0xD5:
            cpu.SP--; mem_write(cpu.SP, cpu.D);
            cpu.SP--; mem_write(cpu.SP, cpu.E);
            break;
        case 0xE5:
            cpu.SP--; mem_write(cpu.SP, cpu.H);
            cpu.SP--; mem_write(cpu.SP, cpu.L);
            break;
        case 0xF5:
            cpu.SP--; mem_write(cpu.SP, cpu.A);
            cpu.SP--; mem_write(cpu.SP, cpu_F());
            break;
        default:
            printf("PUSH: unknown case 0x%02x\n", current_opcode);
            exit(-1);
        } 
}

static cycle_t POP(opcode_t current_opcode) {
    switch (current_opcode) {
        case 0xC1:
            cpu.C = mem_read(cpu.SP); cpu.SP++;
            cpu.B = mem_read(cpu.SP); cpu.SP++;
            break;
        case 0xD1:
            cpu.E = mem_read(cpu.SP); cpu.SP++;
            cpu.D = mem_read(cpu.SP); cpu.SP++;
            break;
        case 0xE1:
            cpu.L = mem_read(cpu.SP); cpu.SP++;
            cpu.H = mem_read(cpu.SP); cpu.SP++;
            break;
        case 0xF1: {
            byte_t F = mem_read(cpu.SP); cpu.SP++;
            cpu.A    = mem_read(cpu.SP); cpu.SP++;
            cpu.FZ = CHECK_BIT(F, FLAG_ZERO_BIT);
            cpu.FN = CHECK_BIT(F, FLAG_SUB_BIT);
            cpu.FH = CHECK_BIT(F, FLAG_HALF_CARRY_BIT);
            cpu.FC = CHECK_BIT(F, FLAG_CARRY_BIT);
            break;
        }
        default:
            printf("POP: unknown case 0x%02x\n", current_opcode);
            exit(-1);
        } 
}
/* -------------- */

/* Misc */
static cycle_t DI(opcode_t) {
    cpu.IME = 0;
}

// TODO: Schedules interrupt handling to be enabled after the next machine cycle.
static cycle_t EI(opcode_t) {
    cpu.IME = 1;
}
/* -------------- */

opcode_handler_t opcodes[256] = {
    [0x10] = STOP,

    [0x0F] = RRCA,
    [0x1F] = RRA,

    [0x01] = LD_16reg_from_mem,
    [0x11] = LD_16reg_from_mem,
    [0x21] = LD_16reg_from_mem,
    [0x31] = LD_16reg_from_mem,

    [0x02] = LD_mem_from_A,
    [0x12] = LD_mem_from_A,
    [0x22] = LD_mem_from_A,
    [0x32] = LD_mem_from_A,

    [0x27] = DAA,

    [0x03] = INC_16,
    [0x13] = INC_16,
    [0x23] = INC_16,
    [0x33] = INC_16,

    [0x0b] = DEC_16,
    [0x1b] = DEC_16,
    [0x2b] = DEC_16,
    [0x3b] = DEC_16,

    [0x09] = ADD_16reg_to_HL,
    [0x19] = ADD_16reg_to_HL,
    [0x29] = ADD_16reg_to_HL,
    [0x39] = ADD_16reg_to_HL,

    [0x04] = INC_8_reg,
    [0x14] = INC_8_reg,
    [0x24] = INC_8_reg,
    [0x34] = INC_8_reg,
    [0x0C] = INC_8_reg,
    [0x1C] = INC_8_reg,
    [0x2C] = INC_8_reg,
    [0x3C] = INC_8_reg,

    [0x05] = DEC_8_reg,
    [0x15] = DEC_8_reg,
    [0x25] = DEC_8_reg,
    [0x35] = DEC_8_reg,
    [0x0D] = DEC_8_reg,
    [0x1D] = DEC_8_reg,
    [0x2D] = DEC_8_reg,
    [0x3D] = DEC_8_reg,

    [0x08] = LD_mem_from_SP,

    [0x06] = LD_8reg_from_mem,
    [0x16] = LD_8reg_from_mem,
    [0x26] = LD_8reg_from_mem,
    [0x36] = LD_8reg_from_mem,
    [0x0E] = LD_8reg_from_mem,
    [0x1E] = LD_8reg_from_mem,
    [0x2E] = LD_8reg_from_mem,
    [0x3E] = LD_8reg_from_mem,

    [0x0A] = LD_A_from_mem_at_16_reg,
    [0x1A] = LD_A_from_mem_at_16_reg,
    [0x2A] = LD_A_from_mem_at_16_reg,
    [0x3A] = LD_A_from_mem_at_16_reg,

    [0x40 ... 0x6F] = LD_8reg_to_reg,
    [0x78 ... 0x7F] = LD_8reg_to_reg,

    [0x70] = LD_mem_from_reg,
    [0x71] = LD_mem_from_reg,
    [0x72] = LD_mem_from_reg,
    [0x73] = LD_mem_from_reg,
    [0x74] = LD_mem_from_reg,
    [0x75] = LD_mem_from_reg,
    [0x76] = HALT,
    [0x77] = LD_mem_from_reg,

    [0xE0] = LDH_mem_from_A,
    [0xF0] = LDH_A_from_mem,
    [0xEA] = LD_mem_at_PC_from_A,
    [0xFA] = LD_A_from_mem,

    [0x80 ... 0x87] = ADD_reg_to_A,

    [0x88 ... 0x8F] = ADC_reg_to_A,

    [0x90 ... 0x97] = SUB_reg_from_A,
    
    [0x98 ... 0x9F] = SUB_reg_from_A,

    [0x18] = JR,
    [0x20] = JR_cond,
    [0x30] = JR_cond,
    [0x28] = JR_cond,
    [0x38] = JR_cond,

    [0xC0] = RET_cond,
    [0xD0] = RET_cond,
    [0xC8] = RET_cond,
    [0xD8] = RET_cond,

    [0xC7] = RST,
    [0xD7] = RST,
    [0xE7] = RST,
    [0xF7] = RST,
    [0xCF] = RST,
    [0xDF] = RST,
    [0xEF] = RST,
    [0xFF] = RST,

    [0xC2] = JP_cond,
    [0xD2] = JP_cond,
    [0xCA] = JP_cond,
    [0xDA] = JP_cond,

    [0xC3] = JP_nn,

    [0xC4] = CALL_cond,
    [0xD4] = CALL_cond,
    [0xCC] = CALL_cond,
    [0xDC] = CALL_cond,

    [0xCD] = CALL,
    [0xC9] = RET,
    [0xD9] = RET_I,
    [0xE9] = JP_HL,

    [0xF9] = LD_SP_HL,

    [0xA0 ... 0xA7] = AND_8_reg,
    [0xE6]          = AND_8_reg,
    
    [0xA8 ... 0xAF] = XOR_8_reg,
    [0xEE]          = XOR_8_reg,
    
    [0xB0 ... 0xB7] = OR_8_reg,
    [0xF6]          = OR_8_reg,

    [0xB8 ... 0xBF] = CP_8_reg,
    [0xFE]          = CP_8_reg,

    [0xC1] = POP,
    [0xD1] = POP,
    [0xE1] = POP,
    [0xF1] = POP,

    [0xF3] = DI,
    [0xFB] = EI,

    [0xC5] = PUSH,
    [0xD5] = PUSH,
    [0xE5] = PUSH,
    [0xF5] = PUSH,

    [0xC6] = ADD_reg_to_A,
    [0xD6] = SUB_reg_from_A,
    [0xE6] = AND_8_reg,
    [0xF6] = OR_8_reg,

    [0xCE] = ADC_reg_to_A,
    [0xDE] = SBC_reg_from_A,
    [0xEE] = XOR_8_reg,
    [0xFE] = CP_8_reg,
};
