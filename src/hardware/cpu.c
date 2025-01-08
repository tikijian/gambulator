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
    // NOP
    if (opcode == 0x00) {
        cpu.PC++;
        return;
    }
    // printf("CPU-PC: 0x%04X, OP: 0x%02X\n", cpu.PC, opcode);
    
    if (!opcodes[opcode]) {
        printf("CPU: Unknown OP-code: 0x%02X at 0x%04X\n", opcode, cpu.PC);
        exit(-1);
    }

    printf("op: 0x%02x - flag %02x, flags %02x \n", opcode, FLAG_ZERO, cpu.F);
    cpu.PC++;
    opcodes[opcode](opcode);
    printf("op: 0x%02x - flag %02x, flags %02x \n\n", opcode, FLAG_ZERO, cpu.F);

}


/* Control Flow-s */
void JP_nn(opcode_t) {
    cpu.PC = mem_read_word(cpu.PC);
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
            break;
        case 0xDC: // C
            condition_result = FLAG_CARRY;
            break;
        default:
            printf("CALL_cond: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }
    printf("flag %02x, flags %02x, cond: %i\n", FLAG_ZERO, cpu.F, condition_result);

    if (condition_result) {
        cpu_push_pc();
        cpu.PC = addr; // jump
    }
}
/* -------------- */


/* 16-bit loads */
void LD_mem_from_SP(opcode_t) {
    word_t addr = mem_read_word(cpu.PC);
    mem_write_byte(addr, LS_BYTE(cpu.SP));
    mem_write_byte(addr + 1, MS_BYTE(cpu.SP));
    cpu.PC += 2;
}
/* -------------- */

/* 8-bit loads */
void LD_mem_from_reg(opcode_t current_opcode) {
    byte_t data = cpu_get_reg_by_code(current_opcode);
    mem_write_byte(cpu_HL(), data);
}

void LD_8_reg(opcode_t current_opcode) {
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
            printf("LD_8_reg: unknown case 0x%02x\n", current_opcode);
            exit(-1);
    }
    cpu.PC++;
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

/* -------------- */

opcode_handler_t opcodes[0xFF] = {
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

    [0x06] = LD_8_reg,
    [0x16] = LD_8_reg,
    [0x26] = LD_8_reg,
    [0x36] = LD_8_reg,
    [0x0E] = LD_8_reg,
    [0x1E] = LD_8_reg,
    [0x2E] = LD_8_reg,
    [0x3E] = LD_8_reg,

    [0x70] = LD_mem_from_reg,
    [0x71] = LD_mem_from_reg,
    [0x72] = LD_mem_from_reg,
    [0x73] = LD_mem_from_reg,
    [0x74] = LD_mem_from_reg,
    [0x75] = LD_mem_from_reg,
    [0x76] = NULL, // TODO: HALT
    [0x77] = LD_mem_from_reg,

    [0x80] = ADD_reg_to_A,
    [0x81] = ADD_reg_to_A,
    [0x82] = ADD_reg_to_A,
    [0x83] = ADD_reg_to_A,
    [0x84] = ADD_reg_to_A,
    [0x85] = ADD_reg_to_A,
    [0x86] = ADD_reg_to_A,
    [0x87] = ADD_reg_to_A,

    [0x88] = ADC_reg_to_A,
    [0x89] = ADC_reg_to_A,
    [0x8A] = ADC_reg_to_A,
    [0x8B] = ADC_reg_to_A,
    [0x8C] = ADC_reg_to_A,
    [0x8D] = ADC_reg_to_A,
    [0x8E] = ADC_reg_to_A,
    [0x8F] = ADC_reg_to_A,

    [0xC3] = JP_nn,
    [0xC4] = CALL_cond,
    [0xD4] = CALL_cond,
    [0xCC] = CALL_cond,
    [0xD4] = CALL_cond,
};
