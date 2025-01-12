#include "constants.h"

#ifndef _ROUTINES
#define _ROUTINES

#define first_bit(byte) (byte & 0x0F)
#define last_bit(byte) (byte & 0xF0)

#define MS_BYTE(word) ((byte_t)(word >> 8))
#define LS_BYTE(word) ((byte_t)(word & 0x00FF))

#define bytes_to_word(msb, lsb) (((msb) << 8) | (lsb))

#define CHECK_BIT(bit) (!!((cpu.F) & (1<<(bit))))

#define CHECK_ZERO(val) ((val & 0xFF) == 0x00)
#define CHECK_OVERFLOW(val) (val > 0x00FF)
#define CHECK_UNDERFLOW(val) ((val & 0xFF00) ? 1 : 0)
#define CHECK_HALF_CARRY(a, b, result) (((a ^ b ^ result) & 0x10) ? 1 : 0);

#endif