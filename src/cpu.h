#ifndef GBEMU_CPU_H
#define GBEMU_CPU_H
#include <stdint.h>
#include "opcodes.h"

typedef struct {
    uint8_t A;
    uint8_t F;
    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;
    uint16_t PC;
    uint16_t SP;
} REGISTER;

uint8_t parse_instruction(uint8_t *instruction_address);

uint8_t exec_nop(uint8_t opcode);

#endif //GBEMU_CPU_H