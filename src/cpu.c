#include "cpu.h"

REGISTER cpu_register;

void init_registers(REGISTER *registers) {
    registers -> A = 0x0;
    registers -> F = 0x0;
    registers -> B = 0xFF;
    registers -> C = 0x13;
    registers -> D = 0x00;
    registers -> E = 0xC1;
    registers -> H = 0x84;
    registers -> L = 0x03;
    registers -> PC = 0x0100;
    registers -> SP = 0xFFFE;
}