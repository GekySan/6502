#ifndef CPU_HPP
#define CPU_HPP

#include "Bus.hpp"

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>


enum class AddressingMode {
    Implied, // Aussi appelé "Implicit"
    Accumulator,
    Immediate,
    ZeroPage,
    ZeroPage_X,
    ZeroPage_Y,
    Relative,
    Absolute,
    Absolute_X,
    Absolute_Y,
    Indirect,
    Indirect_X, // Aussi appelé "Indexed Indirect"
    Indirect_Y, // Aussi appelé "Indirect Indexed"
};

class CPU {
public:
    explicit CPU(Bus& bus_ref);

    void reset();
    void load(const std::vector<uint8_t>& program);
    void load_and_run(const std::vector<uint8_t>& program);
    void run(int max_cycles = -1);
    void run_with_callback(std::function<void(CPU&)> callback, int max_cycles = -1);

    uint8_t mem_read(uint16_t addr) const;
    void mem_write(uint16_t addr, uint8_t data);
    bool is_cpu_running() const;

    uint8_t register_a;
    uint8_t register_x;
    uint8_t register_y;
    uint8_t status;
    uint16_t program_counter;
    uint8_t stack_pointer;

private:
    Bus& bus;

    bool is_running;

    uint16_t mem_read_u16(uint16_t addr) const;
    void mem_write_u16(uint16_t addr, uint16_t data);

    uint16_t get_operand_address(AddressingMode mode) const;

    void update_zero_and_negative_flags(uint8_t result);
    void update_negative_flags(uint8_t result);
    void set_register_a(uint8_t value);
    void set_carry_flag();
    void add_to_register_a(uint8_t data);
    void compare(AddressingMode mode, uint8_t compare_with);


    void ADC(AddressingMode mode);
    void AND(AddressingMode mode);
    void ASL(AddressingMode mode);
    void Branch(bool condition);
    void BIT(AddressingMode mode);
    void CLC();
    void CMP(AddressingMode mode);
    void CPX(AddressingMode mode);
    void CPY(AddressingMode mode);
    void DEC(AddressingMode mode);
    void DEX();
    void DEY();
    void EOR(AddressingMode mode);
    void INC(AddressingMode mode);
    void INX();
    void INY();
    void JMP(AddressingMode mode);
    void JSR(AddressingMode mode);
    void LDA(AddressingMode mode);
    void LDX(AddressingMode mode);
    void LDY(AddressingMode mode);
    void LSR(AddressingMode mode);
    void ORA(AddressingMode mode);
    void PHP();
    void PLA();
    void PLP();
    void ROL(AddressingMode mode);
    void ROR(AddressingMode mode);
    void SBC(AddressingMode mode);
    void STA(AddressingMode mode);
    void STX(AddressingMode mode);
    void STY(AddressingMode mode);
    void TAX();
    void TAY();
    void TSX();
    void TXA();
    void TXS();
    void TYA();
    
    uint8_t stack_pop();
    uint16_t stack_pop_u16();
    void stack_push(uint8_t data);
    void stack_push_u16(uint16_t data);
};

#endif
