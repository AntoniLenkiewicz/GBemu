#include "cpu.h"

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
    uint8_t cycles = 0;
    if (*instruction_address == 0xcb) {
        uint8_t opcode = instruction_address[1];
        printf("cb\n");
        printf("%.2x", opcode);
        cycles = cb_opcode_table[opcode].exec_opcode(instruction_address);
    } else {
        uint8_t opcode = *instruction_address;
        printf("%.2x\n", opcode);
        cycles = opcode_table[opcode].exec_opcode(instruction_address);
    }
    return cycles;
}

//HELPER FUNCTIONS

static void dec8(uint8_t *reg) {
    registers.F &= ~0x20;
    if ((*reg & 0x0F) == 0x00)
        registers.F = registers.F | 0x20;

    registers.F = registers.F | 0x40;

    (*reg)--;

    registers.F &= ~0x80;
    if (*reg == 0)
        registers.F = registers.F | 0x80;

}

static void hl_write(uint8_t value) {
    uint16_t address;
    address = (registers.H << 8) | registers.L;
    write_mem(address, value);
}
static uint8_t hl_read() {
    uint16_t address;
    address = (registers.H << 8) | registers.L;
    return read_mem(address);
}

static void dec_hl() {
    uint16_t hl = (registers.H << 8) | registers.L;
    hl--;
    registers.H = (hl >> 8);
    registers.L = (hl & 0xFF);
}

static void inc_hl() {
    uint16_t hl = (registers.H << 8) | registers.L;
    hl++;
    registers.H = (hl >> 8);
    registers.L = (hl & 0xFF);
}


//CPU INSTRUCTION FUNCTIONS
uint8_t exec_nop(uint8_t *opcode) {
    registers.PC += 1;
    return 4;
}

uint8_t exec_jp(uint8_t *opcode) {
    uint8_t code = *opcode;
    uint8_t cycles, take;
    uint16_t address = opcode[1] | (opcode[2] << 8);
    if (code == 0xc3) {
        take = 1;
        cycles = opcode_table[code].cycles;
    }
    if (take)
        registers.PC = address;

    return cycles;
}

uint8_t exec_xor (uint8_t *opcode) {
    uint8_t cycles;
    OPCODE instruction = opcode_table[*opcode];
    switch (*opcode) {
        case 0xaf:
            registers.A = 0x00;
            registers.F = 0x80;
            break;
        default:
            return 0;
    }
    cycles = instruction.cycles;
    registers.PC += instruction.bytes;
    return cycles;
}

uint8_t exec_ld (uint8_t *opcode) {
    uint8_t cycles;
    uint16_t address;
    OPCODE instruction = opcode_table[*opcode];
    switch (*opcode) {
        case 0x06:
            registers.B = opcode[1];
            break;
        case 0x0e:
            registers.C = opcode[1];
            break;
        case 0x21:
            registers.H = opcode[1];
            registers.L = opcode[2];
            break;
        case 0x2a:
            registers.A = hl_read();
            inc_hl();
            break;
        case 0x31:
            registers.SP = (opcode[2] << 8) | opcode[1];
            break;
        case 0x32:
            hl_write(registers.A);
            dec_hl();
            break;
        case 0x36:
            hl_write(opcode[1]);
            break;
        case 0x3e:
            registers.A = opcode[1];
            break;
        case 0xe0:
            address = 0xff00 | opcode[1];
            write_mem(address, registers.A);
            break;
        case 0xe2:
            address = 0xff00 | registers.C;
            write_mem(address, registers.A);
            break;
        case 0xea:
            write_mem((uint16_t)(opcode[2]<<8) | opcode[1], registers.A);
            break;
        case 0xf0:
            address = 0xff00 | opcode[1];
            registers.A = read_mem(address);
            break;
        default:
            return 0;
    }

    cycles = instruction.cycles;
    registers.PC += instruction.bytes;

    return cycles;
}

uint8_t exec_dec(uint8_t *opcode) {
    uint8_t cycles;
    OPCODE instruction = opcode_table[*opcode];
    switch (*opcode) {
        case 0x05:
            dec8(&registers.B);
            break;
        case 0x0d:
            dec8(&registers.C);
            break;
        case 0x15:
            dec8(&registers.D);
            break;
        case 0x1d:
            dec8(&registers.E);
            break;
        case 0x25:
            dec8(&registers.H);
            break;
        case 0x3d:
            dec8(&registers.A);
            break;
        default:
            return 0;

    }
    cycles = instruction.cycles;
    registers.PC += instruction.bytes;

    return cycles;
}


uint8_t exec_jr(uint8_t *opcode) {
    uint8_t cycles;
    uint8_t take = 0;
    OPCODE instruction = opcode_table[*opcode];
    registers.PC += instruction.bytes;
    switch(*opcode) {
        case 0x20:
            if ((registers.F & 0x80) == 0x00) {
                int8_t value = (int8_t)opcode[1];
                registers.PC = registers.PC + (value);
                take = 1;
            }
            break;
        default:
            return 0;
    }
    if (take) {
        cycles = instruction.cycles_taken;
    } else {
        cycles = instruction.cycles;
    }

    return cycles;
}

uint8_t exec_cp(uint8_t *opcode) {
    uint8_t cycles;
    OPCODE instruction = opcode_table[*opcode];
    switch (*opcode) {
        case 0xfe:
            uint8_t res;
            res = registers.A - opcode[1];

            registers.F &= ~0x80;
            if (res == 0) {
                registers.F = registers.F | 0x80;
            }

            registers.F |= 0x40;

            registers.F &= ~0x20;
            if ((registers.A & 0x0F) < (opcode[1] & 0x0F))
                registers.F |= 0x20;

            registers.F &= ~0x10;
            if (registers.A < opcode[1])
                registers.F |= 0x10;

            break;
    }
    cycles = instruction.cycles;
    registers.PC += instruction.bytes;

    return cycles;
}


uint8_t exec_di(uint8_t *opcode) {
    //for now just increment PC by one - needs rewriting once all other cpu instructions are finished
    registers.PC++;
    return 4;
}

uint8_t exec_ei(uint8_t *opcode) {
    //for now just increment PC by one - needs rewriting once all other cpu instructions are finished
    registers.PC++;
    return 4;
}