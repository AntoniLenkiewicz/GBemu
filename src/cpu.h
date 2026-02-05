#ifndef GBEMU_CPU_H
#define GBEMU_CPU_H
#include <stdint.h>

typedef struct {
    uint8_t A;
    uint8_t F;
    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;
    uint16_t SP;
    uint16_t PC;
} REGISTER;

void init_registers(REGISTER *registers);

#endif //GBEMU_CPU_H