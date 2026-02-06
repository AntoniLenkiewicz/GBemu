#ifndef GBEMU_OPCODES_H
#define GBEMU_OPCODES_H
#include <stdint.h>


typedef struct {
    uint8_t opcode;
    uint8_t bytes;
    uint8_t cycles;
    uint8_t cycles_taken;
    uint8_t (*exec_opcode)(uint8_t);
} OPCODE;

extern const OPCODE opcode_table[256];
extern const OPCODE cb_opcode_table[256];

#endif //GBEMU_OPCODES_H