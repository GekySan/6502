#include "Bus.hpp"

#include <iostream>

#define RAM_START 0x0000
#define RAM_MIRRORS_END 0x1FFF
#define PPU_REGISTERS_START 0x2000
#define PPU_REGISTERS_MIRRORS_END 0x3FFF

Bus::Bus() {
    memory.fill(0);
}

void Bus::load_program(const std::vector<uint8_t>& program, uint16_t start_addr) {
    std::copy(program.begin(), program.end(), memory.begin() + start_addr);
}

uint8_t Bus::mem_read(uint16_t addr) const {
    if (addr >= RAM_START && addr <= RAM_MIRRORS_END) {
        uint16_t mirror_down_addr = addr & 0x07FF;
        return memory[mirror_down_addr];
    }
    /*
    else if (addr >= PPU_REGISTERS_START && addr <= PPU_REGISTERS_MIRRORS_END) {
        uint16_t mirror_down_addr = addr & 0x2007;
        std::cerr << "Accès aux registres PPU non implémenté !" << std::endl;
        return 0;
    }*/
    else {
        return memory[addr];
    }
}

void Bus::mem_write(uint16_t addr, uint8_t data) {
    if (addr >= RAM_START && addr <= RAM_MIRRORS_END) {
        uint16_t mirror_down_addr = addr & 0x07FF;
        memory[mirror_down_addr] = data;
    }/*
    else if (addr >= PPU_REGISTERS_START && addr <= PPU_REGISTERS_MIRRORS_END) {
        uint16_t mirror_down_addr = addr & 0x2007;
        std::cerr << "Accès en écriture aux registres PPU non implémenté !" << std::endl;
    }*/
    else {
        memory[addr] = data;
    }
}

uint16_t Bus::mem_read_u16(uint16_t addr) const {
    uint8_t lo = mem_read(addr);
    uint8_t hi = mem_read(addr + 1);
    return (hi << 8) | lo;
}

void Bus::mem_write_u16(uint16_t addr, uint16_t data) {
    uint8_t hi = data >> 8;
    uint8_t lo = data & 0xFF;
    mem_write(addr, lo);
    mem_write(addr + 1, hi);
}
