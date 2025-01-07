#include "constants.h"

#ifndef _ROUTINES
#define _ROUTINES

#define first_bit(byte) (byte & 0x0F)
#define last_bit(byte) (byte & 0xF0)

#define bytes_to_word(msb, lsb) ((msb << 8) | lsb)

#define FLAG_SET(value, flags, bit) ((value) > 0 ? flags | (1 << bit) : flags & ~(1 << bit))
#define FLAG_SET_ZERO(value, flags) FLAG_SET(value, flags, FLAG_ZERO_BIT)
#define FLAG_SET_SUB(value, flags) FLAG_SET(value, flags, FLAG_SUB_BIT)
#define FLAG_SET_HALF_CARRY(value, flags) FLAG_SET(value, flags, FLAG_HALF_CARRY_BIT)
#define FLAG_SET_CARRY(value, flags) FLAG_SET(value, flags, FLAG_CARRY_BIT)
#define FLAG_ADD_HALF_CARRY(a, b, result) ((a ^ b ^ result) & 0x10) // TODO: check
#define FLAG_SUB_HALF_CARRY(a, b, result) ((a ^ (-b) ^ result) & 0x10)

#endif