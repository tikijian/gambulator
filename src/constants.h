#ifndef _CONSTANTS
#define _CONSTANTS

#define CPU_CYCLES_PER_M_CYCLE 4

#define ADDR_ROM_START 0x100
#define MEM_TOTAL 0xFFFF

#define OPCODE_PREFIX 0xCB

#define FLAG_ZERO_BIT 7
#define FLAG_SUB_BIT 6
#define FLAG_HALF_CARRY_BIT 5
#define FLAG_CARRY_BIT 4

// Hardware registers
#define REG_DIV 0xFF04
#define REG_TIMA 0xFF05
#define REG_TMA 0xFF06
#define REG_TAC 0xFF07
    #define REG_TAC_ENABLE_BIT 2
    #define REG_TAC_CLOCK_BIT_MASK 0b00000011


#endif