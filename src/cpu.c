#include "cpu.h"

#include <unistd.h>

static REGISTER registers = {
    0x01,
    0xB0,
    0x00,
    0x13,
    0x00,
    0xD8,
    0x01,
    0x4D,
    0x0100,
    0xFFFE
};

/*want to parse instruction and call function to execute the instruction
 *pass bytes to exec function to execute function then return cycles*/
uint8_t parse_instruction(uint8_t *instruction_address) {
    instruction_address += registers.PC;
    FILE *fptr;
    fptr = fopen("log.txt", "a");
    fprintf(fptr, "A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X \n",
        registers.A, registers.F, registers.B, registers.C, registers.D, registers.E, registers.H, registers.L, registers.SP,
        registers.PC, read_mem(registers.PC), read_mem(registers.PC+1), read_mem(registers.PC+2), read_mem(registers.PC+3));
    fclose(fptr);
    printf("PC:%.4x\n", registers.PC);
    uint8_t cycles = 0;
    if (*instruction_address == 0xcb) {
        uint8_t opcode = instruction_address[1];
        printf("cb");
        printf("INST:%.2X\n", opcode);
        cycles = cb_opcode_table[opcode].exec_opcode(++instruction_address);
    } else {
        uint8_t opcode = *instruction_address;
        printf("INST:%.2X\n", opcode);
        cycles = opcode_table[opcode].exec_opcode(instruction_address);
    }
    return cycles;
}

//HELPER FUNCTIONS

static void dec8(uint8_t *reg) {

    registers.F &= ~FLAG_H;
    if ((*reg & 0x0F) == 0x00)
        registers.F |= FLAG_H;

    registers.F |= FLAG_N;

    (*reg)--;

    registers.F &= ~FLAG_Z;
    if (*reg == 0)
        registers.F |= FLAG_Z;

}
static void inc8(uint8_t *reg) {
    registers.F &= ~FLAG_H;
    if ((*reg & 0x0F) == 0x0F)
        registers.F |= FLAG_H;

    registers.F &= ~FLAG_N;

    (*reg)++;

    registers.F &= ~FLAG_Z;
    if (*reg == 0)
        registers.F |= FLAG_Z;
}

static void inc16(uint8_t *reg1, uint8_t *reg2) {
    uint16_t value = (*reg1 << 8) | *reg2;
    value++;
    *reg1 = (value >> 8);
    *reg2 = value;
}

static void dec16(uint8_t *reg1, uint8_t *reg2) {
    uint16_t value = (*reg1 << 8) | *reg2;
    value--;
    *reg1 = (value >> 8);
    *reg2 = value;
}

static void reg_or(const uint8_t *reg) {
    if (registers.A = registers.A | *reg)
        registers.F = 0x00;
    else
        registers.F = FLAG_Z;
}

static void reg_and(const uint8_t *reg) {
    registers.A &= *reg;
    registers.F = 0x00;
    if (registers.A == 0) {
        registers.F |= FLAG_Z;
        registers.F |= FLAG_H;
    }
    else
        registers.F = 0x20;
}

static void reg_xor(const uint8_t *reg) {
    registers.A ^= *reg;
    registers.F = 0x00;
    if (registers.A == 0) {
        registers.F = FLAG_Z;
    }
}

static void reg_add8(const uint8_t reg) {
    uint8_t result = registers.A + reg;
    uint16_t sum = (uint16_t) registers.A + reg;
    registers.F = 0x00;

    if (result == 0) {
        registers.F = FLAG_Z;
    }
    if ((registers.A&0x0f) + (reg&0x0f) & 0x10) {
        registers.F |=FLAG_H;
    }
    if (sum & 0x0100) {
        registers.F |= FLAG_C;
    }

    registers.A = result;
}
static void reg_add16(const uint8_t high, const uint8_t low) {
    uint16_t  combined = (high << 8) | low;
    uint16_t hl = (registers.H << 8) | registers.L;
    uint16_t result = hl + combined;
    uint32_t sum = (uint32_t) hl + combined;
    registers.F &= ~FLAG_N;
    registers.F &= ~FLAG_H;
    if ((combined&0x0fff) + (hl&0x0fff) &0x10000) {
        registers.F |= FLAG_H;
    }

    registers.F &= ~FLAG_C;
    if (sum & 0x010000) {
        registers.F |= FLAG_C;
    }
    registers.H = (result >> 8);
    registers.L = (result & 0xFF);

}

static void reg_sub8(const uint8_t reg) {
    uint8_t result = registers.A - reg;

    registers.F &= ~FLAG_Z;
    if (result == 0) {
        registers.F |= FLAG_Z;
    }
    registers.F |= FLAG_N;

    registers.F &= ~FLAG_H;
    if ((reg & 0x0F) > (registers.F & 0x0F)) {
        registers.F |= FLAG_H;
    }

    registers.F &= ~FLAG_C;
    if (reg > registers.A) {
        registers.F |= FLAG_C;
    }
    registers.A = result;
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
    lower = (*byte) & 0x0f;
    *byte = ((*byte) >> 4) | (lower << 4);
}


//CPU INSTRUCTION FUNCTIONS
uint8_t exec_nop(uint8_t *opcode) {
    registers.PC += 1;
    return 4;
}

uint8_t exec_jp(uint8_t *opcode) {
    uint8_t code = *opcode;
    uint8_t cycles;
    uint8_t take = 0;
    uint16_t address;
    OPCODE instruction = opcode_table[*opcode];
    switch (*opcode) {
        case 0xc3:
            address = (opcode[2] << 8) | opcode[1] ;
            take = 1;
            cycles = opcode_table[code].cycles;
            break;
        case 0xe9:
            address = (registers.H << 8) | registers.L;
            take = 1;
            cycles = opcode_table[code].cycles;
            break;
        default:
            return 0;
    }
    if (take)
        registers.PC = address;
    else
        registers.PC += instruction.bytes;

    return cycles;
}

uint8_t exec_xor (uint8_t *opcode) {
    uint8_t cycles;
    OPCODE instruction = opcode_table[*opcode];
    uint16_t address;
    switch (*opcode) {
        case 0xa8:
            reg_xor(&registers.B);
            break;
        case 0xa9:
            reg_xor(&registers.C);
            break;
        case 0xaa:
            reg_xor(&registers.D);
            break;
        case 0xab:
            reg_xor(&registers.E);
            break;
        case 0xac:
            reg_xor(&registers.H);
            break;
        case 0xad:
            reg_xor(&registers.L);
            break;
        case 0xae:
            address = (registers.H << 8) | registers.L;
            uint8_t value = read_mem(address);
            reg_xor(&value);
            break;
        case 0xaf:
            reg_xor(&registers.A);
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
            break;
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
        case 0xf6:
            reg_or(&(opcode[1]));
            break;
        default:
            return 0;
    }

    cycles = instruction.cycles;
    registers.PC += instruction.bytes;

    return cycles;
}

uint8_t exec_add(uint8_t *opcode) {
    uint8_t cycles;
    OPCODE instruction = opcode_table[*opcode];
    switch (*opcode) {
        case 0x09:
            reg_add16(registers.B, registers.C);
            break;
        case 0x19:
            reg_add16(registers.D, registers.E);
            break;
        case 0x1b:
            dec16(&registers.D, &registers.E);
            break;
        case 0x29:
            reg_add16(registers.H,registers.L);
            break;
        case 0x39:
            uint8_t high = registers.SP >> 8;
            uint8_t low = registers.SP & 0xFF;
            reg_add16(high, low);
            break;
        case 0x80:
            reg_add8(registers.B);
            break;
        case 0x81:
            reg_add8(registers.C);
            break;
        case 0x82:
            reg_add8(registers.D);
            break;
        case 0x83:
            reg_add8(registers.E);
            break;
        case 0x84:
            reg_add8(registers.H);
            break;
        case 0x85:
            reg_add8(registers.L);
            break;
        case 0x87:
            reg_add8(registers.A);
            break;
        case 0xc6:
            reg_add8(opcode[1]);
            break;
        default:
            return 0;
    }

    cycles = instruction.cycles;
    registers.PC += instruction.bytes;

    return cycles;

}

uint8_t exec_sub(uint8_t *opcode) {
    uint8_t cycles;
    OPCODE instruction = opcode_table[*opcode];
    switch (*opcode) {
        case 0xd6:
            reg_sub8(opcode[1]);
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
        case 0x11:
            registers.E = opcode[1];
            registers.D = opcode[2];
            break;
        case 0x12:
            address = (registers.D << 8) | registers.E;
            write_mem(address, registers.A);
            break;
        case 0x16:
            registers.D = opcode[1];
            break;
        case 0x1a:
            address = (registers.D << 8) | registers.E;
            registers.A = read_mem(address);
            break;
        case 0x21:
            registers.L = opcode[1];
            registers.H = opcode[2];
            break;
        case 0x22:
            address = (registers.H << 8) | registers.L;
            write_mem(address, registers.A);
            inc16(&registers.H, &registers.L);
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
        case 0x46:
            address = (uint16_t) (registers.H << 8) | registers.L;
            registers.B = read_mem(address);
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
        case 0x4e:
            address = (uint16_t) (registers.H << 8) | registers.L;
            registers.C = read_mem(address);
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
        case 0x56:
            address = (registers.H <<8) | registers.L;
            registers.D = read_mem(address);
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
        case 0x5e:
            address = (uint16_t) (registers.H << 8) | registers.L;
            registers.E = read_mem(address);
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
        case 0x66:
            address = (uint16_t) (registers.H << 8) | registers.L;
            registers.H = read_mem(address);
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
        case 0x6e:
            address = (uint16_t) (registers.H << 8) | registers.L;
            registers.L = read_mem(address);
            break;
        case 0x6f:
            registers.L = registers.A;
            break;
        case 0x77:
            address = (registers.H << 8) | registers.L;
            write_mem(address, registers.A);
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
        case 0x7e:
            address = (uint16_t) (registers.H << 8) | registers.L;
            registers.A = read_mem(address);
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
        case 0xf8:
            registers.F &= ~0xF0;
            if ((registers.SP & 0xFF)+ opcode[1] > 0xFF)
                registers.F |= FLAG_C;
            if ((registers.SP & 0xF) + (0xf & opcode[1]) > 0xF)
                registers.F |= FLAG_H;
            address = registers.SP + (int8_t) opcode[1];
            registers.H = (address >> 8);
            registers.L = address;
            break;
        case 0xfa:
            address = (opcode[2] << 8) | opcode[1];
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
        case 0x18:
            int8_t value = (int8_t) opcode[1];
            registers.PC = registers.PC + (int8_t) value;
            take = 0;
            break;
        case 0x20:
            if ((registers.F & FLAG_Z) == 0x00) {
                int8_t value = (int8_t)opcode[1];
                registers.PC = registers.PC + (int8_t)value;
                take = 1;
            }
            break;
        case 0x28:
            if ((registers.F & FLAG_Z) == FLAG_Z) {
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

            registers.F &= ~FLAG_Z;
            if (res == 0) {
                registers.F |= FLAG_Z;
            }

            registers.F |= FLAG_N;

            registers.F &= ~FLAG_H;
            if ((registers.A & 0x0F) < (opcode[1] & 0x0F))
                registers.F |= FLAG_H;

            registers.F &= ~FLAG_C;
            if (registers.A < opcode[1])
                registers.F |= FLAG_C;

            break;
        default:
            return 0;
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
        case 0xc4:
            if (!(FLAG_Z & registers.F)) {
                pc = instruction.bytes + registers.PC;
                registers.SP--;
                write_mem(registers.SP, (uint8_t) (pc >> 8));
                registers.SP--;
                write_mem(registers.SP, (uint8_t) pc);
                registers.PC = (opcode[2] << 8)| opcode[1];
            } else {
                registers.PC += instruction.bytes;
            }
            break;
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

    return cycles;
}

uint8_t exec_cpl(uint8_t *opcode) {
    registers.A = ~registers.A;
    registers.PC += opcode_table[*opcode].bytes;
    registers.F |= FLAG_N;
    registers.F |= FLAG_H;
    return opcode_table[*opcode].cycles;
}

uint8_t exec_pop(uint8_t *opcode) {
    uint8_t cycles;
    OPCODE instruction = opcode_table[*opcode];
    switch (*opcode) {
        case 0xc1:
            registers.C = read_mem(registers.SP);
            registers.SP++;
            registers.B = read_mem(registers.SP);
            registers.SP++;
            break;
        case 0xd1:
            registers.E = read_mem(registers.SP);
            registers.SP++;
            registers.D = read_mem(registers.SP);
            registers.SP++;
            break;
        case 0xe1:
            registers.L = read_mem(registers.SP);
            registers.SP++;
            registers.H = read_mem(registers.SP);
            registers.SP++;
            break;
        case 0xf1:
            registers.F = read_mem(registers.SP);
            registers.SP++;
            registers.A = read_mem(registers.SP);
            registers.SP++;
            break;
        default:
            return 0;
    }
    cycles = instruction.cycles;
    registers.PC += instruction.bytes;

    return cycles;
}

uint8_t exec_push(uint8_t *opcode) {
    uint8_t cycles;
    OPCODE instruction = opcode_table[*opcode];
    switch (*opcode) {
        case 0xc5:
            registers.SP--;
            write_mem(registers.SP, registers.B);
            registers.SP --;
            write_mem(registers.SP, registers.C);
            break;
        case 0xd5:
            registers.SP--;
            write_mem(registers.SP, registers.D);
            registers.SP --;
            write_mem(registers.SP, registers.E);
            break;
        case 0xe5:
            registers.SP--;
            write_mem(registers.SP, registers.H);
            registers.SP --;
            write_mem(registers.SP, registers.L);
            break;
        case 0xf5:
            registers.SP--;
            write_mem(registers.SP, registers.A);
            registers.SP --;
            write_mem(registers.SP, registers.F);
            break;
        default:
            return 0;
    }

    cycles = instruction.cycles;
    registers.PC += instruction.bytes;

    return cycles;
}

uint8_t exec_rst(uint8_t *opcode) {
    uint8_t cycles;
    OPCODE instruction = opcode_table[*opcode];
    uint16_t target;

    uint16_t pc = registers.PC + instruction.bytes;
    switch (*opcode) {
        case 0xc7:
            target = 0x00c7;
            break;
        case 0xcf:
            target = 0x0008;
            break;
        case 0xd7:
            target = 0x0010;
            break;
        case 0xdf:
            target = 0x0018;
            break;
        case 0xe7:
            target = 0x0020;
            break;
        case 0xef:
            target = 0x0028;
            break;
        case 0xf7:
            target = 0x0030;
            break;
        case 0xff:
            target = 0x0038;
            break;
        default:
            return 0;
    }

    registers.SP--;
    write_mem(registers.SP, (uint8_t) (pc >> 8));
    registers.SP--;
    write_mem(registers.SP, (uint8_t) pc);


    registers.PC = target;

    cycles = instruction.cycles;
    return cycles;
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