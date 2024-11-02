#ifndef BUS_HPP
#define BUS_HPP

#include <array>
#include <cstdint>
#include <vector>

class Bus {
public:
    Bus();

    uint8_t mem_read(uint16_t addr) const;
    void mem_write(uint16_t addr, uint8_t data);

    uint16_t mem_read_u16(uint16_t addr) const;
    void mem_write_u16(uint16_t addr, uint16_t data);

    void load_program(const std::vector<uint8_t>& program, uint16_t start_addr);

private:
    std::array<uint8_t, 0x10000> memory;
};

#endif
