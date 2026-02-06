#include "cpu.h"
#include <stdio.h>

static REGISTER registers = {
    0x0,
    0x0,
    0xFF,
    0x13,
    0x00,
    0xC1,
    0x84,
    0x03,
    0x0100,
    0xFFFE
};


/*want to parse instruction and call function to execute the instruction
 *pass bytes to exec function to execute function then return cycles*/
uint8_t parse_instruction(uint8_t *instruction_address) {
    instruction_address += registers.PC;
    uint8_t bytes;
    uint8_t cycles = 0;
    if (*instruction_address == 0xcb) {
        instruction_address++;
        uint8_t opcode = *instruction_address;
        cycles = cb_opcode_table[opcode].exec_opcode(opcode);
    } else {
        uint8_t opcode = *instruction_address;
        printf("%.2x\n", opcode);
        cycles = opcode_table[opcode].exec_opcode(opcode);
    }
    return cycles;
}

uint8_t exec_nop(uint8_t opcode) {
    registers.PC += 1;
    printf("Executed nop\n");
    return 1;
}