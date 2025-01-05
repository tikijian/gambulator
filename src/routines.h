#ifndef _ROUTINES
#define _ROUTINES

#define first_bit(byte) byte & 0x0F
#define last_bit(byte) byte & 0xF0

#define bytes_to_word(lsb, msb) (msb << 8) | lsb


#endif