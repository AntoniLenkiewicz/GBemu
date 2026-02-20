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
        printf("cb");
        printf("%.2x\n", opcode);
        cycles = cb_opcode_table[opcode].exec_opcode(++instruction_address);
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
static void inc8(uint8_t *reg) {
    registers.F &= ~0x20;
    if ((*reg & 0x0F) == 0x0F)
        registers.F = registers.F | 0x20;

    registers.F &= ~0x40;

    (*reg)++;

    registers.F &= ~0x80;
    if (*reg == 0)
        registers.F = registers.F | 0x80;
}

static void inc16(uint8_t *reg1, uint8_t *reg2) {
    uint16_t value = (*reg1 << 8) | *reg2;
    value--;
    *reg1 = (value >> 8);
    *reg2 = value;
}

static void dec16(uint8_t *reg1, uint8_t *reg2) {
    uint16_t value = (*reg1 << 8) | *reg2;
    value++;
    *reg1 = (value >> 8);
    *reg2 = value;
}

static void reg_or(uint8_t *reg) {
    if (registers.A = registers.A | *reg)
        registers.F = 0x80;
    else
        registers.F = 0x00;
}

static void reg_and(uint8_t *reg) {
    registers.A &= *reg;
    if (registers.A == 0)
        registers.F = 0xa0;
    else
        registers.F = 0x20;
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

static void swap_nibbles(uint8_t *byte) {
    uint8_t lower;
    lower = (*byte) | 0x0f;
    *byte = ((*byte) >> 4) | (lower << 4);
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

uint8_t exec_or (uint8_t *opcode) {
    uint8_t cycles;
    OPCODE instruction = opcode_table[*opcode];
    switch (*opcode) {
        case 0xb0:
            reg_or(&registers.B);
        case 0xb1:
            reg_or(&registers.C);
            break;
        case 0xb2:
            reg_or(&registers.D);
            break;
        case 0xb3:
            reg_or(&registers.E);
            break;
        case 0xb4:
            reg_or(&registers.H);
            break;
        case 0xb5:
            reg_or(&registers.L);
            break;
        case 0xb7:
            reg_or(&registers.A);
            break;
        default:
            return 0;
    }

    cycles = instruction.cycles;
    registers.PC += instruction.bytes;

    return cycles;
}

uint8_t exec_and(uint8_t *opcode) {
    uint8_t cycles;
    OPCODE instruction = opcode_table[*opcode];
    switch (*opcode) {
        case 0xa0:
            reg_and(&registers.B);
            break;
        case 0xa1:
            reg_and(&registers.C);
            break;
        case 0xa2:
            reg_and(&registers.D);
            break;
        case 0xa3:
            reg_and(&registers.E);
            break;
        case 0xa4:
            reg_and(&registers.H);
            break;
        case 0xa5:
            reg_and(&registers.L);
            break;
        case 0xa7:
            reg_and(&registers.A);
            break;
        case 0xe6:
            reg_and(&opcode[1]);
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
        case 0x01:
            registers.B = opcode[2];
            registers.C = opcode[1];
            break;
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
        case 0x40:
            registers.B = registers.B;
            break;
        case 0x41:
            registers.B = registers.C;
            break;
        case 0x42:
            registers.B = registers.D;
            break;
        case 0x43:
            registers.B = registers.E;
            break;
        case 0x44:
            registers.B = registers.H;
            break;
        case 0x45:
            registers.B = registers.L;
            break;
        case 0x47:
            registers.B = registers.A;
            break;
        case 0x48:
            registers.C = registers.B;
            break;
        case 0x49:
            registers.C = registers.C;
            break;
        case 0x4A:
            registers.C = registers.D;
            break;
        case 0x4B:
            registers.C = registers.E;
            break;
        case 0x4C:
            registers.C = registers.H;
            break;
        case 0x4D:
            registers.C = registers.L;
            break;
        case 0x4F:
            registers.C = registers.A;
            break;
        case 0x50:
            registers.D = registers.B;
            break;
        case 0x51:
            registers.D = registers.C;
            break;
        case 0x52:
            registers.D = registers.D;
            break;
        case 0x53:
            registers.D = registers.E;
            break;
        case 0x54:
            registers.D = registers.H;
            break;
        case 0x55:
            registers.D = registers.L;
            break;
        case 0x57:
            registers.D = registers.A;
            break;
        case 0x58:
            registers.E = registers.B;
            break;
        case 0x59:
            registers.E = registers.C;
            break;
        case 0x5A:
            registers.E = registers.D;
            break;
        case 0x5B:
            registers.E = registers.E;
            break;
        case 0x5C:
            registers.E = registers.H;
            break;
        case 0x5D:
            registers.E = registers.L;
            break;
        case 0x5F:
            registers.E = registers.A;
            break;
        case 0x60:
            registers.H = registers.B;
            break;
        case 0x61:
            registers.H = registers.C;
            break;
        case 0x62:
            registers.H = registers.D;
            break;
        case 0x63:
            registers.H = registers.E;
            break;
        case 0x64:
            registers.H = registers.H;
            break;
        case 0x65:
            registers.H = registers.L;
            break;
        case 0x67:
            registers.H = registers.A;
            break;
        case 0x68:
            registers.L = registers.B;
            break;
        case 0x69:
            registers.L = registers.C;
            break;
        case 0x6a:
            registers.L = registers.D;
            break;
        case 0x6b:
            registers.L = registers.E;
            break;
        case 0x6c:
            registers.L = registers.H;
            break;
        case 0x6d:
            registers.L = registers.L;
            break;
        case 0x6f:
            registers.L = registers.A;
            break;
        case 0x78:
            registers.A = registers.B;
            break;
        case 0x79:
            registers.A = registers.C;
            break;
        case 0x7a:
            registers.A = registers.D;
            break;
        case 0x7b:
            registers.A = registers.E;
            break;
        case 0x7c:
            registers.A = registers.H;
            break;
        case 0x7d:
            registers.A = registers.L;
            break;
        case 0x7f:
            registers.A = registers.A;
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

uint8_t exec_inc (uint8_t *opcode) {
    uint8_t cycles;
    OPCODE instruction = opcode_table[*opcode];
    switch (*opcode) {
        case 0x03:
            inc16(&registers.B, &registers.C);
            break;
        case 0x04:
            inc8(&registers.B);
            break;
        case 0x0c:
            inc8(&registers.C);
            break;
        case 0x13:
            inc16(&registers.D, &registers.E);
            break;
        case 0x14:
            inc8(&registers.D);
            break;
        case 0x1c:
            inc8(&registers.E);
            break;
        case 0x23:
            inc16(&registers.H, &registers.L);
            break;
        case 0x24:
            inc8(&registers.H);
            break;
        case 0x2c:
            inc8(&registers.L);
            break;
        case 0x33:
            registers.SP++;
            break;
        case 0x3c:
            inc8(&registers.A);
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
        case 0x0b:
            dec16(&registers.B, &registers.C);
            break;
        case 0x0d:
            dec8(&registers.C);
            break;
        case 0x15:
            dec8(&registers.D);
            break;
        case 0x1b:
            dec16(&registers.D, &registers.E);
            break;
        case 0x1d:
            dec8(&registers.E);
            break;
        case 0x25:
            dec8(&registers.H);
            break;
        case 0x2b:
            dec16(&registers.H, &registers.L);
            break;
        case 0x2d:
            dec8(&registers.L);
            break;
        case 0x3b:
            registers.SP--;
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

uint8_t exec_call(uint8_t *opcode) {
    uint8_t cycles;
    uint16_t pc;
    OPCODE instruction = opcode_table[*opcode];
    switch (*opcode) {
        case 0xcd:
            pc = instruction.bytes + registers.PC;
            registers.SP--;
            write_mem(registers.SP, (uint8_t) (pc >> 8));
            registers.SP--;
            write_mem(registers.SP, (uint8_t) pc);
            registers.PC = (opcode[2] << 8)| opcode[1];
            break;
        default:
            return 0;
    }
    cycles = instruction.cycles;
    return cycles;
}

uint8_t exec_ret(uint8_t *opcode) {
    uint8_t cycles;
    OPCODE instruction = opcode_table[*opcode];
    switch (*opcode) {
        case 0xc9:
            registers.PC = read_mem(registers.SP);
            registers.SP++;
            registers.PC |= (read_mem(registers.SP) << 8);
            registers.SP++;
            break;
        default:
            return 0;
    }

    cycles = instruction.cycles;
    registers.PC += instruction.bytes;

    return cycles;
}

uint8_t exec_cpl(uint8_t *opcode) {
    registers.A = ~registers.A;
    registers.PC += opcode_table[*opcode].bytes;
    registers.F |= 0x60;
    return opcode_table[*opcode].cycles;
}

//CB instructions

uint8_t exec_swap(uint8_t *opcode) {
    uint8_t cycles, lower;
    OPCODE instruction = cb_opcode_table[*opcode];
    switch (*opcode) {
        case 0x30:
            swap_nibbles(&registers.B);
            break;
        case 0x31:
            swap_nibbles(&registers.C);
            break;
        case 0x32:
            swap_nibbles(&registers.D);
            break;
        case 0x33:
            swap_nibbles(&registers.E);
            break;
        case 0x34:
            swap_nibbles(&registers.H);
            break;
        case 0x35:
            swap_nibbles(&registers.L);
            break;
        case 0x37:
            swap_nibbles(&registers.A);
            break;
        default:
            return 0;
    }
    cycles = instruction.cycles;
    registers.PC += instruction.bytes;

    return cycles;
}