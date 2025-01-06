// size of 0xFF
typedef unsigned char byte_t;
typedef byte_t opcode_t;
// size of 0xFFFF
typedef unsigned short word_t;

// pointer to opcode execution function
typedef void (*opcode_handler_t)(opcode_t);