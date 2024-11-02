#ifndef COLOR_HPP
#define COLOR_HPP

#include <cstdint>

struct Color {
    uint8_t r, g, b, a;
};

Color color(uint8_t byte);

#endif
