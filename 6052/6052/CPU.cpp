#include "CPU.hpp"
#include "OpCodes.hpp"

#include <chrono>
#include <iostream>
#include <thread>

#define STACK 0x0100
#define STACK_RESET 0xFD

CPU::CPU(Bus& bus_ref) : bus(bus_ref), is_running(true) {
    reset();
}

void CPU::reset() {
    register_a = 0;
    register_x = 0;
    register_y = 0;
    stack_pointer = STACK_RESET;
    status = 0x24;
    program_counter = mem_read_u16(0xFFFC);
    is_running = true;
}

void CPU::load(const std::vector<uint8_t>& program) {
    bus.load_program(program, 0x0600);
    mem_write_u16(0xFFFC, 0x0600);
}

void CPU::run(int max_cycles) {
    run_with_callback([](CPU&) {}, max_cycles);
}

void CPU::load_and_run(const std::vector<uint8_t>& program) {
    load(program);
    reset();
    run();
}

void CPU::run_with_callback(std::function<void(CPU&)> callback, int max_cycles) {
    int cycles = 0;
    while (true) {
        uint8_t code = mem_read(program_counter++);
        uint16_t program_counter_state = program_counter;
        const OpCode* opcode = OPCODES_MAP.at(code);

        switch (code) {
        case 0x69:
        case 0x65:
        case 0x75:
        case 0x6D:
        case 0x7D:
        case 0x79:
        case 0x61:
        case 0x71:
            ADC(opcode->mode);
            break;

        case 0x29:
        case 0x25:
        case 0x35:
        case 0x2D:
        case 0x3D:
        case 0x39:
        case 0x21:
        case 0x31:
            AND(opcode->mode);
            break;

        case 0x0A:
        case 0x06:
        case 0x16:
        case 0x0E:
        case 0x1E:
            ASL(opcode->mode);
            break;

            // BCC
        case 0x90:
            Branch(!(status & 0x01));
            break;

            // BCS
        case 0xB0:
            Branch(status & 0x01);
            break;

            // BEQ
        case 0xF0:
            Branch(status & 0x02);
            break;

        case 0x24:
        case 0x2C:
            BIT(opcode->mode);
            break;

            // BMI
        case 0x30:
            Branch(status & 0x80);
            break;

            // BNE
        case 0xD0:
            Branch(!(status & 0x02));
            break;

            // BPL
        case 0x10:
            Branch(!(status & 0x80));
            break;

            // BRK
        case 0x00:
            is_running = false;
            return;

            // BVC
        case 0x50:
            Branch(!(status & 0x40));
            break;

            // BVS
        case 0x70:
            Branch(status & 0x40);
            break;

        case 0x18:
            CLC();
            break;

            // CLD
        case 0xD8:
            status &= ~0x08;
            break;

            // CLI
        case 0x58:
            status &= ~0x04;
            break;

            // CLV
        case 0xB8:
            status &= ~0x40;
            break;

        case 0xC9:
        case 0xC5:
        case 0xD5:
        case 0xCD:
        case 0xDD:
        case 0xD9:
        case 0xC1:
        case 0xD1:
            CMP(opcode->mode);
            break;

        case 0xE0:
        case 0xE4:
        case 0xEC:
            CPX(opcode->mode);
            break;

        case 0xC0:
        case 0xC4:
        case 0xCC:
            CPY(opcode->mode);
            break;

        case 0xC6:
        case 0xD6:
        case 0xCE:
        case 0xDE:
            DEC(opcode->mode);
            break;

        case 0xCA:
            DEX();
            break;

        case 0x88:
            DEY();
            break;

        case 0x49:
        case 0x45:
        case 0x55:
        case 0x4D:
        case 0x5D:
        case 0x59:
        case 0x41:
        case 0x51:
            EOR(opcode->mode);
            break;

        case 0xE6:
        case 0xF6:
        case 0xEE:
        case 0xFE:
            INC(opcode->mode);
            break;

        case 0xE8:
            INX();
            break;

        case 0xC8:
            INY();
            break;

        case 0x4C:
        case 0x6C:
            JMP(opcode->mode);
            break;

        case 0x20:
            JSR(opcode->mode);
            break;

        case 0xA9:
        case 0xA5:
        case 0xB5:
        case 0xAD:
        case 0xBD:
        case 0xB9:
        case 0xA1:
        case 0xB1:
            LDA(opcode->mode);
            break;

        case 0xA2:
        case 0xA6:
        case 0xB6:
        case 0xAE:
        case 0xBE:
            LDX(opcode->mode);
            break;

        case 0xA0:
        case 0xA4:
        case 0xB4:
        case 0xAC:
        case 0xBC:
            LDY(opcode->mode);
            break;

        case 0x4A:
        case 0x46:
        case 0x56:
        case 0x4E:
        case 0x5E:
            LSR(opcode->mode);
            break;

            // NOP
        case 0xEA:
            break;

        case 0x09:
        case 0x05:
        case 0x15:
        case 0x0D:
        case 0x1D:
        case 0x19:
        case 0x01:
        case 0x11:
            ORA(opcode->mode);
            break;

            // PHA
        case 0x48:
            stack_push(register_a);
            break;

        case 0x08:
            PHP();
            break;

        case 0x68:
            PLA();
            break;

        case 0x28:
            PLP();
            break;

        case 0x2A:
        case 0x26:
        case 0x36:
        case 0x2E:
        case 0x3E:
            ROL(opcode->mode);
            break;

        case 0x6A:
        case 0x66:
        case 0x76:
        case 0x6E:
        case 0x7E:
            ROR(opcode->mode);
            break;

            // RTI
        case 0x40:
            status = stack_pop();
            status &= ~0x10;
            program_counter = stack_pop_u16();
            break;

            // RTS
        case 0x60:
            program_counter = stack_pop_u16() + 1;
            break;

        case 0xE9:
        case 0xE5:
        case 0xF5:
        case 0xED:
        case 0xFD:
        case 0xF9:
        case 0xE1:
        case 0xF1:
            SBC(opcode->mode);
            break;

            // SEC
        case 0x38:
            set_carry_flag();
            break;

            // SED
        case 0xF8:
            status |= 0x08;
            break;

            // SEI
        case 0x78:
            status |= 0x04;
            break;

        case 0x85:
        case 0x95:
        case 0x8D:
        case 0x9D:
        case 0x99:
        case 0x81:
        case 0x91:
            STA(opcode->mode);
            break;

        case 0x86:
        case 0x96:
        case 0x8E:
            STX(opcode->mode);
            break;

        case 0x84:
        case 0x94:
        case 0x8C:
            STY(opcode->mode);
            break;

        case 0xAA:
            TAX();
            break;

        case 0xA8:
            TAY();
            break;

        case 0xBA:
            TSX();
            break;

        case 0x8A:
            TXA();
            break;

        case 0x9A:
            TXS();
            break;

        case 0x98:
            TYA();
            break;

        default:
            std::cerr << "Opcode non implémenté: 0x" << std::hex << static_cast<int>(code) << std::endl; // Ne devrait pas arriver
            exit(1);
        }


        if (program_counter_state == program_counter) {
            program_counter += (opcode->len - 1);
        }

        cycles += opcode->cycles;

        if (max_cycles > 0 && cycles >= max_cycles) {
            break;
        }

        callback(*this);
    }
}

uint8_t CPU::mem_read(uint16_t addr) const {
    return bus.mem_read(addr);
}

void CPU::mem_write(uint16_t addr, uint8_t data) {
    bus.mem_write(addr, data);
}

uint16_t CPU::mem_read_u16(uint16_t addr) const {
    return bus.mem_read_u16(addr);
}

void CPU::mem_write_u16(uint16_t addr, uint16_t data) {
    bus.mem_write_u16(addr, data);
}

bool CPU::is_cpu_running() const {
    return is_running;
}


uint16_t CPU::get_operand_address(AddressingMode mode) const {
    switch (mode) {
    case AddressingMode::Implied:
        return 0;
    case AddressingMode::Accumulator:
        return 0;
    case AddressingMode::Immediate:
        return program_counter;
    case AddressingMode::ZeroPage:
        return mem_read(program_counter);
    case AddressingMode::ZeroPage_X:
        return (mem_read(program_counter) + register_x) & 0xFF;
    case AddressingMode::ZeroPage_Y:
        return (mem_read(program_counter) + register_y) & 0xFF;
    case AddressingMode::Relative: {
        int8_t offset = mem_read(program_counter);
        return program_counter + 1 + offset;
    }
    case AddressingMode::Absolute:
        return mem_read_u16(program_counter);
    case AddressingMode::Absolute_X:
        return mem_read_u16(program_counter) + register_x;
    case AddressingMode::Absolute_Y:
        return mem_read_u16(program_counter) + register_y;
    case AddressingMode::Indirect: {
        uint16_t addr = mem_read_u16(program_counter);
        uint16_t lo = mem_read(addr);
        uint16_t hi = mem_read((addr & 0xFF00) | ((addr + 1) & 0xFF));
        return (hi << 8) | lo;
    }
    case AddressingMode::Indirect_X: {
        uint8_t ptr = mem_read(program_counter) + register_x;
        uint16_t lo = mem_read(ptr & 0xFF);
        uint16_t hi = mem_read((ptr + 1) & 0xFF);
        return (hi << 8) | lo;
    }
    case AddressingMode::Indirect_Y: {
        uint8_t ptr = mem_read(program_counter);
        uint16_t lo = mem_read(ptr & 0xFF);
        uint16_t hi = mem_read((ptr + 1) & 0xFF);
        return ((hi << 8) | lo) + register_y;
    }
    default:
        std::cerr << "Mode d'adressage non implémenté" << std::endl; // Non plus
        return 0;
    }
}

void CPU::update_zero_and_negative_flags(uint8_t result) {
    if (result == 0) {
        status |= 0x02;
    }
    else {
        status &= ~0x02;
    }

    if (result & 0x80) {
        status |= 0x80;
    }
    else {
        status &= ~0x80;
    }
}

void CPU::update_negative_flags(uint8_t result) {
    if (result & 0x80) {
        status |= 0x80;
    }
    else {
        status &= ~0x80;
    }
}

void CPU::set_register_a(uint8_t value) {
    register_a = value;
    update_zero_and_negative_flags(register_a);
}

void CPU::set_carry_flag() {
    status |= 0x01;
}

void CPU::add_to_register_a(uint8_t data) {
    uint16_t sum = register_a + data + (status & 0x01);
    if (sum > 0xFF) {
        set_carry_flag();
    }
    else {
        CLC();
    }

    if (~(register_a ^ data) & (register_a ^ sum) & 0x80) {
        status |= 0x40;
    }
    else {
        status &= ~0x40;
    }

    register_a = sum & 0xFF;
    update_zero_and_negative_flags(register_a);
}

void CPU::stack_push(uint8_t data) {
    mem_write(STACK + stack_pointer, data);
    stack_pointer--;
}

uint8_t CPU::stack_pop() {
    stack_pointer++;
    return mem_read(STACK + stack_pointer);
}

void CPU::stack_push_u16(uint16_t data) {
    uint8_t hi = data >> 8;
    uint8_t lo = data & 0xFF;
    stack_push(hi);
    stack_push(lo);
}

uint16_t CPU::stack_pop_u16() {
    uint8_t lo = stack_pop();
    uint8_t hi = stack_pop();
    return (hi << 8) | lo;
}


void CPU::ADC(AddressingMode mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    add_to_register_a(data);
}

void CPU::AND(AddressingMode mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    set_register_a(register_a & data);
}

void CPU::ASL(AddressingMode mode) {
    if (mode == AddressingMode::Accumulator) {
        if (register_a & 0x80) {
            set_carry_flag();
        }
        else {
            CLC();
        }
        register_a <<= 1;
        update_zero_and_negative_flags(register_a);
    }
    else {
        uint16_t addr = get_operand_address(mode);
        uint8_t data = mem_read(addr);
        if (data & 0x80) {
            set_carry_flag();
        }
        else {
            CLC();
        }
        data <<= 1;
        mem_write(addr, data);
        update_zero_and_negative_flags(data);
    }
}


void CPU::Branch(bool condition) {
    if (condition) {
        int8_t offset = static_cast<int8_t>(mem_read(program_counter));
        program_counter++;
        program_counter += offset;
    }
    else {
        program_counter++;
    }
}

void CPU::BIT(AddressingMode mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    uint8_t result = register_a & data;
    if (result == 0) {
        status |= 0x02;
    }
    else {
        status &= ~0x02;
    }
    status = (status & 0x3F) | (data & 0xC0);
}

void CPU::CLC() {
    status &= ~0x01;
}

void CPU::CMP(AddressingMode mode) {
    compare(mode, register_a);
}

void CPU::compare(AddressingMode mode, uint8_t compare_with) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    uint8_t result = compare_with - data;
    if (compare_with >= data) {
        set_carry_flag();
    }
    else {
        CLC();
    }
    update_zero_and_negative_flags(result);
}

void CPU::CPX(AddressingMode mode) {
    compare(mode, register_x);
}

void CPU::CPY(AddressingMode mode) {
    compare(mode, register_y);
}

void CPU::DEC(AddressingMode mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    data--;
    mem_write(addr, data);
    update_zero_and_negative_flags(data);
}

void CPU::DEX() {
    register_x--;
    update_zero_and_negative_flags(register_x);
}

void CPU::DEY() {
    register_y--;
    update_zero_and_negative_flags(register_y);
}

void CPU::EOR(AddressingMode mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    set_register_a(register_a ^ data);
}

void CPU::INC(AddressingMode mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    data++;
    mem_write(addr, data);
    update_zero_and_negative_flags(data);
}

void CPU::INX() {
    register_x++;
    update_zero_and_negative_flags(register_x);
}

void CPU::INY() {
    register_y++;
    update_zero_and_negative_flags(register_y);
}

void CPU::JMP(AddressingMode mode) {
    uint16_t target;

    if (mode == AddressingMode::Indirect) {
        uint16_t addr = mem_read_u16(program_counter);

        if ((addr & 0x00FF) == 0x00FF) {
            uint8_t lo = mem_read(addr);
            uint8_t hi = mem_read(addr & 0xFF00);
            target = (hi << 8) | lo;
        }
        else {
            target = mem_read_u16(addr);
        }
    }
    else if (mode == AddressingMode::Absolute) {
        target = mem_read_u16(program_counter);
    }
    else {
        std::cerr << "Mode d'adressage non supporté pour JMP" << std::endl; // Non plus
        return;
    }

    program_counter = target;
}

void CPU::JSR(AddressingMode mode) {
    stack_push_u16(program_counter + 1);
    uint16_t target = get_operand_address(mode);
    program_counter = target;
}

void CPU::LDA(AddressingMode mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    set_register_a(data);
}

void CPU::LDX(AddressingMode mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    register_x = data;
    update_zero_and_negative_flags(register_x);
}

void CPU::LDY(AddressingMode mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    register_y = data;
    update_zero_and_negative_flags(register_y);
}

void CPU::LSR(AddressingMode mode) {
    if (mode == AddressingMode::Accumulator) {
        if (register_a & 0x01) {
            set_carry_flag();
        }
        else {
            CLC();
        }
        register_a >>= 1;
        update_zero_and_negative_flags(register_a);
    }
    else {
        uint16_t addr = get_operand_address(mode);
        uint8_t data = mem_read(addr);
        if (data & 0x01) {
            set_carry_flag();
        }
        else {
            CLC();
        }
        data >>= 1;
        mem_write(addr, data);
        update_zero_and_negative_flags(data);
    }
}

void CPU::ORA(AddressingMode mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    set_register_a(register_a | data);
}

void CPU::PHP() {
    stack_push(status | 0x10);
}

void CPU::PLA() {
    register_a = stack_pop();
    update_zero_and_negative_flags(register_a);
}

void CPU::PLP() {
    status = stack_pop();
    status &= ~0x10;
}

void CPU::ROL(AddressingMode mode) {
    uint8_t carry = (status & 0x01);

    if (mode == AddressingMode::Accumulator) {
        if (register_a & 0x80) {
            set_carry_flag();
        }
        else {
            CLC();
        }
        register_a = (register_a << 1) | carry;
        update_zero_and_negative_flags(register_a);
    }
    else {
        uint16_t addr = get_operand_address(mode);
        uint8_t data = mem_read(addr);
        if (data & 0x80) {
            set_carry_flag();
        }
        else {
            CLC();
        }
        data = (data << 1) | carry;
        mem_write(addr, data);
        update_zero_and_negative_flags(data);
    }
}

void CPU::ROR(AddressingMode mode) {
    uint8_t carry = (status & 0x01) << 7;

    if (mode == AddressingMode::Accumulator) {
        if (register_a & 0x01) {
            set_carry_flag();
        }
        else {
            CLC();
        }
        register_a = (register_a >> 1) | carry;
        update_zero_and_negative_flags(register_a);
    }
    else {
        uint16_t addr = get_operand_address(mode);
        uint8_t data = mem_read(addr);
        if (data & 0x01) {
            set_carry_flag();
        }
        else {
            CLC();
        }
        data = (data >> 1) | carry;
        mem_write(addr, data);
        update_zero_and_negative_flags(data);
    }
}

void CPU::SBC(AddressingMode mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    add_to_register_a(~data);
}

void CPU::STA(AddressingMode mode) {
    uint16_t addr = get_operand_address(mode);
    mem_write(addr, register_a);
}

void CPU::STX(AddressingMode mode) {
    uint16_t addr = get_operand_address(mode);
    mem_write(addr, register_x);
}

void CPU::STY(AddressingMode mode) {
    uint16_t addr = get_operand_address(mode);
    mem_write(addr, register_y);
}

void CPU::TAX() {
    register_x = register_a;
    update_zero_and_negative_flags(register_x);
}

void CPU::TAY() {
    register_y = register_a;
    update_zero_and_negative_flags(register_y);
}

void CPU::TSX() {
    register_x = stack_pointer;
    update_zero_and_negative_flags(register_x);
}

void CPU::TXA() {
    register_a = register_x;
    update_zero_and_negative_flags(register_a);
}

void CPU::TXS() {
    stack_pointer = register_x;
}

void CPU::TYA() {
    register_a = register_y;
    update_zero_and_negative_flags(register_a);
}

