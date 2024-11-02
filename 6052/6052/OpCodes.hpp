#ifndef OPCODES_H
#define OPCODES_H

#include <unordered_map>
#include <vector>

enum class AddressingMode;

struct OpCode {
    uint8_t code;
    const char* mnemonic;
    uint8_t len;
    uint8_t cycles;
    AddressingMode mode;

    OpCode(uint8_t c, const char* m, uint8_t l, uint8_t cy, AddressingMode mo)
        : code(c), mnemonic(m), len(l), cycles(cy), mode(mo) {}
};

extern const std::vector<OpCode> CPU_OPS_CODES;
extern const std::unordered_map<uint8_t, const OpCode*> OPCODES_MAP;

#endif
