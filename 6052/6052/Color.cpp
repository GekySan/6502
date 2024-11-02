#include "Color.hpp"

Color color(uint8_t byte) {
    switch (byte) {
    case 0x0: return Color{ 0, 0, 0, 255 };          // Noir
    case 0x1: return Color{ 255, 255, 255, 255 };    // Blanc
    case 0x2: return Color{ 255, 0, 0, 255 };        // Rouge
    case 0x3: return Color{ 0, 255, 255, 255 };      // Cyan
    case 0x4: return Color{ 128, 0, 128, 255 };      // Violet
    case 0x5: return Color{ 0, 255, 0, 255 };        // Vert
    case 0x6: return Color{ 0, 0, 255, 255 };        // Bleu
    case 0x7: return Color{ 255, 255, 0, 255 };      // Jaune
    case 0x8: return Color{ 255, 165, 0, 255 };      // Orange
    case 0x9: return Color{ 165, 42, 42, 255 };      // Marron
    case 0xA: return Color{ 255, 69, 0, 255 };       // Rouge clair
    case 0xB: return Color{ 105, 105, 105, 255 };    // Gris foncé
    case 0xC: return Color{ 128, 128, 128, 255 };    // Gris
    case 0xD: return Color{ 144, 238, 144, 255 };    // Vert clair
    case 0xE: return Color{ 173, 216, 230, 255 };    // Bleu clair
    case 0xF: return Color{ 211, 211, 211, 255 };    // Gris clair
    default: return Color{ 0, 0, 0, 255 };           // Par défaut, Noir
    }
}
