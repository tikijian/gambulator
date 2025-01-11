#include <stdlib.h>
#include "stdio.h"

#include "../constants.h"
#include "../routines.h"
#include "cpu.h"
#include "memory.h"
#include "cpu_helpers.h"


opcode_handler_t opcodes[0xFF];

struct CPU cpu = {
        .A = 0x01,
        .B = 0x0,
        .C = 0x13,
        .D = 0x0,
        .E = 0xd8,
        .H = 0x01,
        .L = 0x4d,
        .F = 0xb0,

        .Z = 1,
        
        .SP = 0xfffe,
        .PC = 0x0100,
};

void cpu_exec(opcode_t opcode, void* mem) {
    if (opcode == OPCODE_PREFIX) {
        // read next instruction after prefix-opcode
        // TODO: check if should rewrite opcode variable or make new
        printf("Prefix Opcode! ");
        opcode = mem_read(cpu.PC + 1);
        cpu.PC++;
    }

    // printf("CPU-PC: 0x%04X, OP: 0x%02X\n", cpu.PC, opcode);
    // if (cpu.SP > 0x200)
        // printf("PC: 0x%04X, OP: 0x%02X, A: %02X, SP: %04X, C: %02X, D: %02X, Z: %i\n", cpu.PC, opcode, cpu.A, cpu.SP, cpu.C, cpu.D, cpu.Z);
    // NOP
    if (opcode == 0x00) {
        cpu.PC++;
        return;
    }
    printf("PC: 0x%04X, OP: 0x%02X, A: %02X, SP: %04X, C: %02X, D: %02X, Z: %i\n", cpu.PC, opcode, cpu.A, cpu.SP, cpu.C, cpu.D, cpu.Z);
    // printf("\nCPU-PC: 0x%04X, OP: 0x%02X, A: %02X, B: %02X, C: %02X, D: %02X, Z: %i\n", cpu.PC, opcode, cpu.A, cpu.B, cpu.C, cpu.D, cpu.Z);
    
    if (!opcodes[opcode]) {
        printf("CPU: Unknown OP-code: 0x%02X at 0x%04X\n", opcode, cpu.PC);
        exit(-1);
    }

    // printf("op: 0x%02x - flag %02x, flags %02x \n", opcode, FLAG_ZERO, cpu.F);
    cpu.PC++;
    opcodes[opcode](opcode);
    // printf("op: 0x%02x - flag %02x, flags %02x \n\n", opcode, FLAG_ZERO, cpu.F);

}


/* Control Flow-s */
void HALT(opcode_t) {
    printf("HALT\n");
    exit(0);
}
void STOP(opcode_t) {
    printf("STOP\n");
    exit(0);
}

void JP_nn(opcode_t) {
    cpu.PC = mem_read_word(cpu.PC);
    printf("jumping to %04x\n", cpu.PC);
}

void JR_cond(opcode_t current_opcode) {
    // check: signed8(offset)
    word_t offset = mem_read_word(cpu.PC);
    cpu.PC++;

    byte_t condition_result = 0;
    switch (current_opcode) {
        case 0x20: // NZ
            condition_result = !FLAG_ZERO; break;
        case 0x30: // NC
            condition_result = !FLAG_CARRY; break;
        case 0x28: // Z
            condition_result = FLAG_ZERO; break;
        case 0x38: // C
            condition_result = FLAG_CARRY; break;
        default:
            printf("JR_cond: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }

    if (condition_result) {
        cpu.PC += offset;
    }
}

void RET(opcode_t) {
    // printf("reading mem from SP: %04X\n", cpu.SP);
    byte_t lsb = mem_read(cpu.SP); cpu.SP++;
    byte_t msb = mem_read(cpu.SP); cpu.SP++;
    // printf("mem is %02X %02x - %04X\n", msb, lsb, bytes_to_word(msb, lsb));
    cpu.PC = bytes_to_word(msb, lsb);
}

void CALL_cond(opcode_t current_opcode) {
    word_t addr = mem_read_word(cpu.PC);
    cpu.PC = cpu.PC + 2;

    byte_t condition_result = 0;
    switch (current_opcode) {
        case 0xC4: // NZ
            condition_result = !FLAG_ZERO;
            break;
        case 0xD4: // NC
            condition_result = !FLAG_CARRY;
            break;
        case 0xCC: // Z
            condition_result = FLAG_ZERO;
            // condition_result = cpu.Z;
            break;
        case 0xDC: // C
            condition_result = FLAG_CARRY;
            break;
        default:
            printf("CALL_cond: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }
    printf("addr to call: %04X, Z %02x, cond: %i\n", addr, FLAG_ZERO, cpu.Z, condition_result);

    if (condition_result) {
        cpu_push_pc();
        cpu.PC = addr; // jump
    }
}
/* -------------- */


/* 16-bit loads */
void LD_mem_from_A(opcode_t current_opcode) {
    word_t addr;
    switch (current_opcode) {
        case 0x02:
            addr = cpu_BC(); break;
        case 0x12:
            addr = cpu_DE(); break;
        case 0x22: {
            word_t HL;
            addr = HL;
            cpu_set_HL(HL + 1);
            break;
        }
        case 0x32: {
            word_t HL;
            addr = HL;
            cpu_set_HL(HL - 1);
            break;
        }
        default:
            printf("LD_mem_from_A: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }
    mem_write_byte(addr, cpu.A);
}

void LD_mem_from_SP(opcode_t) {
    word_t addr = mem_read_word(cpu.PC);
    // printf("addr %04X = %02X, addr %04X = %02X\n", addr, LS_BYTE(cpu.SP), addr + 1, MS_BYTE(cpu.SP));
    cpu.PC += 2;
    mem_write_byte(addr, LS_BYTE(cpu.SP));
    mem_write_byte(addr + 1, MS_BYTE(cpu.SP));
}

void LD_16reg_from_mem(opcode_t current_opcode) {
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
/* -------------- */

/* 8-bit loads */
void LD_mem_from_reg(opcode_t current_opcode) {
    byte_t data = cpu_get_reg_by_code(current_opcode);
    mem_write_byte(cpu_HL(), data);
}

void LD_8reg_to_reg(opcode_t current_opcode) {
    byte_t data = cpu_get_reg_by_code(current_opcode);
    cpu_set_reg_by_code(current_opcode, data);
}

void LD_8reg_from_mem(opcode_t current_opcode) {
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
            mem_write_byte(cpu_HL(), value);
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

void LD_A_from_mem_at_16_reg(opcode_t current_opcode) {
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
/* -------------- */

/* 16-bit arithmetics */
void INC_16(opcode_t current_opcode) {
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

void DEC_16(opcode_t current_opcode) {
    printf("BC: %04x\n", bytes_to_word(cpu.B, cpu.C));
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
    printf("BC: %04x\n", bytes_to_word(cpu.B, cpu.C));
}
/* -------------- */

/* 8-Bit Arithmetics */
void ADD_reg_to_A(opcode_t current_opcode) {
    byte_t value = cpu_get_reg_by_code(current_opcode);
    byte_t result = cpu.A + value;
    cpu_update_flags(cpu.A, value, result, "Z0HC");

    // should this happen before flags update??
    if (result > 0xFF) {
        result = result - 256;
    }

    cpu.A = result;    
}

void ADC_reg_to_A(opcode_t current_opcode) {
    byte_t value = cpu_get_reg_by_code(current_opcode) + FLAG_CARRY;
    byte_t result = cpu.A + value;
    cpu_update_flags(cpu.A, value, result, "Z0HC");

    // should this happen before flags update??
    if (result > 0xFF) {
        result = result - 256;
    }

    cpu.A = result;    
}

void INC_8_reg(opcode_t current_opcode) {
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
            mem_write_byte(cpu_HL(), target + 1);
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

void DEC_8_reg(opcode_t current_opcode) {
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
            mem_write_byte(cpu_HL(), target - 1);
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

void AND_8_reg(opcode_t current_opcode) {
    byte_t val;
    if (current_opcode == 0xE6) {
        val = mem_read(cpu.PC);
        cpu.PC++;
    } else {
        val = cpu_get_reg_by_code(current_opcode);
    }
    byte_t new_val = cpu.A & val;
    cpu_update_flags(cpu.A, val, new_val, "Z010");
}

void XOR_8_reg(opcode_t current_opcode) {
    byte_t val;
    if (current_opcode == 0xEE) {
        val = mem_read(cpu.PC);
        cpu.PC++;
    } else {
        val = cpu_get_reg_by_code(current_opcode);
    }
    byte_t new_val = cpu.A ^ val;
    cpu_update_flags(cpu.A, val, new_val, "Z000");
}

void OR_8_reg(opcode_t current_opcode) {
    byte_t val;
    if (current_opcode == 0xF6) {
        val = mem_read(cpu.PC);
        cpu.PC++;
    } else {
        val = cpu_get_reg_by_code(current_opcode);
    }
    byte_t new_val = cpu.A | val;
    cpu_update_flags(cpu.A, val, new_val, "Z000");
}

/* -------------- */

opcode_handler_t opcodes[0xFF] = {
    [0x10] = STOP,

    [0x01] = LD_16reg_from_mem,
    [0x11] = LD_16reg_from_mem,
    [0x21] = LD_16reg_from_mem,
    [0x31] = LD_16reg_from_mem,

    [0x20] = JR_cond,
    [0x30] = JR_cond,
    [0x28] = JR_cond,
    [0x38] = JR_cond,

    [0x02] = LD_mem_from_A,
    [0x12] = LD_mem_from_A,
    [0x22] = LD_mem_from_A,
    [0x32] = LD_mem_from_A,

    [0x03] = INC_16,
    [0x13] = INC_16,
    [0x23] = INC_16,
    [0x33] = INC_16,

    [0x0b] = DEC_16,
    [0x1b] = DEC_16,
    [0x2b] = DEC_16,
    [0x3b] = DEC_16,

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

    [0x70] = LD_mem_from_reg,
    [0x71] = LD_mem_from_reg,
    [0x72] = LD_mem_from_reg,
    [0x73] = LD_mem_from_reg,
    [0x74] = LD_mem_from_reg,
    [0x75] = LD_mem_from_reg,
    [0x76] = HALT,
    [0x77] = LD_mem_from_reg,

    [0x80 ... 0x87] = ADD_reg_to_A,

    [0x88 ... 0x8F] = ADC_reg_to_A,

    [0xC3] = JP_nn,
    [0xC4] = CALL_cond,
    [0xD4] = CALL_cond,
    [0xCC] = CALL_cond,
    [0xD4] = CALL_cond,
    [0xC9] = RET,

    [0xA0 ... 0xA7] = AND_8_reg,
    [0xE6] = AND_8_reg,
    
    [0xA8 ... 0xAF] = XOR_8_reg,
    [0xEE] = XOR_8_reg,
    
    [0xB0 ... 0xB7] = OR_8_reg,
    [0xF6] = OR_8_reg,
};
