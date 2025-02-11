#ifndef TYPES_H
# define TYPES_H

// size of 0xFF
typedef signed char s_byte_t;
typedef unsigned char byte_t;
typedef byte_t opcode_t;
// size of 0xFFFF
typedef unsigned short word_t;

typedef unsigned long int cycle_t;

// pointer to opcode execution function
typedef cycle_t (*opcode_handler_t)(opcode_t);


#endif