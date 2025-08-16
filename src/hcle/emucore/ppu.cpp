#include "ppu.hpp"
#include "cpu.hpp"
#include "nes.hpp"
#include "mapper.hpp"

#include <cstring>

static constexpr uint8_t PALETTE_COLORS[0x8][0x40][0x3] = {
    0x54, 0x54, 0x54, 0x00, 0x1E, 0x74, 0x08, 0x10, 0x90, 0x30, 0x00, 0x88, 0x44, 0x00, 0x64, 0x5C,
    0x00, 0x30, 0x54, 0x04, 0x00, 0x3C, 0x18, 0x00, 0x20, 0x2A, 0x00, 0x08, 0x3A, 0x00, 0x00, 0x40,
    0x00, 0x00, 0x3C, 0x00, 0x00, 0x32, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x98, 0x96, 0x98, 0x08, 0x4C, 0xC4, 0x30, 0x32, 0xEC, 0x5C, 0x1E, 0xE4, 0x88, 0x14, 0xB0, 0xA0,
    0x14, 0x64, 0x98, 0x22, 0x20, 0x78, 0x3C, 0x00, 0x54, 0x5A, 0x00, 0x28, 0x72, 0x00, 0x08, 0x7C,
    0x00, 0x00, 0x76, 0x28, 0x00, 0x66, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xEC, 0xEE, 0xEC, 0x4C, 0x9A, 0xEC, 0x78, 0x7C, 0xEC, 0xB0, 0x62, 0xEC, 0xE4, 0x54, 0xEC, 0xEC,
    0x58, 0xB4, 0xEC, 0x6A, 0x64, 0xD4, 0x88, 0x20, 0xA0, 0xAA, 0x00, 0x74, 0xC4, 0x00, 0x4C, 0xD0,
    0x20, 0x38, 0xCC, 0x6C, 0x38, 0xB4, 0xCC, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xEC, 0xEE, 0xEC, 0xA8, 0xCC, 0xEC, 0xBC, 0xBC, 0xEC, 0xD4, 0xB2, 0xEC, 0xEC, 0xAE, 0xEC, 0xEC,
    0xAE, 0xD4, 0xEC, 0xB4, 0xB0, 0xE4, 0xC4, 0x90, 0xCC, 0xD2, 0x78, 0xB4, 0xDE, 0x78, 0xA8, 0xE2,
    0x90, 0x98, 0xE2, 0xB4, 0xA0, 0xD6, 0xE4, 0xA0, 0xA2, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x5C, 0x4B, 0x4B, 0x00, 0x1B, 0x68, 0x08, 0x0E, 0x81, 0x34, 0x00, 0x7A, 0x4A, 0x00, 0x5A, 0x65,
    0x00, 0x2B, 0x5C, 0x03, 0x00, 0x42, 0x15, 0x00, 0x23, 0x25, 0x00, 0x08, 0x34, 0x00, 0x00, 0x39,
    0x00, 0x00, 0x36, 0x00, 0x00, 0x2D, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xA7, 0x87, 0x88, 0x08, 0x44, 0xB0, 0x34, 0x2D, 0xD4, 0x65, 0x1B, 0xCD, 0x95, 0x12, 0x9E, 0xB0,
    0x12, 0x5A, 0xA7, 0x1E, 0x1C, 0x84, 0x36, 0x00, 0x5C, 0x51, 0x00, 0x2C, 0x66, 0x00, 0x08, 0x6F,
    0x00, 0x00, 0x6A, 0x24, 0x00, 0x5B, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xD6, 0xD4, 0x53, 0x8A, 0xD4, 0x84, 0x6F, 0xD4, 0xC1, 0x58, 0xD4, 0xFA, 0x4B, 0xD4, 0xFF,
    0x4F, 0xA2, 0xFF, 0x5F, 0x5A, 0xE9, 0x7A, 0x1C, 0xB0, 0x99, 0x00, 0x7F, 0xB0, 0x00, 0x53, 0xBB,
    0x1C, 0x3D, 0xB7, 0x61, 0x3D, 0xA2, 0xB7, 0x42, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xD6, 0xD4, 0xB8, 0xB7, 0xD4, 0xCE, 0xA9, 0xD4, 0xE9, 0xA0, 0xD4, 0xFF, 0x9C, 0xD4, 0xFF,
    0x9C, 0xBE, 0xFF, 0xA2, 0x9E, 0xFA, 0xB0, 0x81, 0xE0, 0xBD, 0x6C, 0xC6, 0xC7, 0x6C, 0xB8, 0xCB,
    0x81, 0xA7, 0xCB, 0xA2, 0xB0, 0xC0, 0xCD, 0xB0, 0x91, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x4B, 0x5C, 0x4B, 0x00, 0x21, 0x68, 0x07, 0x11, 0x81, 0x2B, 0x00, 0x7A, 0x3D, 0x00, 0x5A, 0x52,
    0x00, 0x2B, 0x4B, 0x04, 0x00, 0x36, 0x1A, 0x00, 0x1C, 0x2E, 0x00, 0x07, 0x3F, 0x00, 0x00, 0x46,
    0x00, 0x00, 0x42, 0x00, 0x00, 0x37, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x88, 0xA5, 0x88, 0x07, 0x53, 0xB0, 0x2B, 0x37, 0xD4, 0x52, 0x21, 0xCD, 0x7A, 0x16, 0x9E, 0x90,
    0x16, 0x5A, 0x88, 0x25, 0x1C, 0x6C, 0x42, 0x00, 0x4B, 0x63, 0x00, 0x24, 0x7D, 0x00, 0x07, 0x88,
    0x00, 0x00, 0x81, 0x24, 0x00, 0x70, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xD4, 0xFF, 0xD4, 0x44, 0xA9, 0xD4, 0x6C, 0x88, 0xD4, 0x9E, 0x6B, 0xD4, 0xCD, 0x5C, 0xD4, 0xD4,
    0x60, 0xA2, 0xD4, 0x74, 0x5A, 0xBE, 0x95, 0x1C, 0x90, 0xBB, 0x00, 0x68, 0xD7, 0x00, 0x44, 0xE4,
    0x1C, 0x32, 0xE0, 0x61, 0x32, 0xC6, 0xB7, 0x36, 0x42, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xD4, 0xFF, 0xD4, 0x97, 0xE0, 0xD4, 0xA9, 0xCE, 0xD4, 0xBE, 0xC3, 0xD4, 0xD4, 0xBF, 0xD4, 0xD4,
    0xBF, 0xBE, 0xD4, 0xC6, 0x9E, 0xCD, 0xD7, 0x81, 0xB7, 0xE7, 0x6C, 0xA2, 0xF4, 0x6C, 0x97, 0xF8,
    0x81, 0x88, 0xF8, 0xA2, 0x90, 0xEB, 0xCD, 0x90, 0xB2, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x53, 0x53, 0x44, 0x00, 0x1D, 0x5D, 0x07, 0x0F, 0x74, 0x2F, 0x00, 0x6E, 0x43, 0x00, 0x51, 0x5B,
    0x00, 0x26, 0x53, 0x03, 0x00, 0x3B, 0x17, 0x00, 0x1F, 0x29, 0x00, 0x07, 0x39, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x3B, 0x00, 0x00, 0x31, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x96, 0x94, 0x7B, 0x07, 0x4B, 0x9E, 0x2F, 0x31, 0xBF, 0x5B, 0x1D, 0xB8, 0x86, 0x13, 0x8E, 0x9E,
    0x13, 0x51, 0x96, 0x21, 0x19, 0x76, 0x3B, 0x00, 0x53, 0x59, 0x00, 0x27, 0x70, 0x00, 0x07, 0x7A,
    0x00, 0x00, 0x74, 0x20, 0x00, 0x64, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xE9, 0xEB, 0xBF, 0x4B, 0x98, 0xBF, 0x76, 0x7A, 0xBF, 0xAE, 0x61, 0xBF, 0xE1, 0x53, 0xBF, 0xE9,
    0x57, 0x91, 0xE9, 0x68, 0x51, 0xD1, 0x86, 0x19, 0x9E, 0xA8, 0x00, 0x72, 0xC2, 0x00, 0x4B, 0xCD,
    0x19, 0x37, 0xC9, 0x57, 0x37, 0xB2, 0xA5, 0x3B, 0x3B, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xE9, 0xEB, 0xBF, 0xA6, 0xC9, 0xBF, 0xBA, 0xBA, 0xBF, 0xD1, 0xB0, 0xBF, 0xE9, 0xAC, 0xBF, 0xE9,
    0xAC, 0xAB, 0xE9, 0xB2, 0x8E, 0xE1, 0xC2, 0x74, 0xC9, 0xCF, 0x61, 0xB2, 0xDB, 0x61, 0xA6, 0xDF,
    0x74, 0x96, 0xDF, 0x91, 0x9E, 0xD3, 0xB8, 0x9E, 0xA0, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x4B, 0x4B, 0x5C, 0x00, 0x1B, 0x7F, 0x07, 0x0E, 0x9E, 0x2B, 0x00, 0x95, 0x3D, 0x00, 0x6E, 0x52,
    0x00, 0x34, 0x4B, 0x03, 0x00, 0x36, 0x15, 0x00, 0x1C, 0x25, 0x00, 0x07, 0x34, 0x00, 0x00, 0x39,
    0x00, 0x00, 0x36, 0x00, 0x00, 0x2D, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x88, 0x87, 0xA7, 0x07, 0x44, 0xD7, 0x2B, 0x2D, 0xFF, 0x52, 0x1B, 0xFA, 0x7A, 0x12, 0xC1, 0x90,
    0x12, 0x6E, 0x88, 0x1E, 0x23, 0x6C, 0x36, 0x00, 0x4B, 0x51, 0x00, 0x24, 0x66, 0x00, 0x07, 0x6F,
    0x00, 0x00, 0x6A, 0x2C, 0x00, 0x5B, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xD4, 0xD6, 0xFF, 0x44, 0x8A, 0xFF, 0x6C, 0x6F, 0xFF, 0x9E, 0x58, 0xFF, 0xCD, 0x4B, 0xFF, 0xD4,
    0x4F, 0xC6, 0xD4, 0x5F, 0x6E, 0xBE, 0x7A, 0x23, 0x90, 0x99, 0x00, 0x68, 0xB0, 0x00, 0x44, 0xBB,
    0x23, 0x32, 0xB7, 0x76, 0x32, 0xA2, 0xE0, 0x36, 0x36, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xD4, 0xD6, 0xFF, 0x97, 0xB7, 0xFF, 0xA9, 0xA9, 0xFF, 0xBE, 0xA0, 0xFF, 0xD4, 0x9C, 0xFF, 0xD4,
    0x9C, 0xE9, 0xD4, 0xA2, 0xC1, 0xCD, 0xB0, 0x9E, 0xB7, 0xBD, 0x84, 0xA2, 0xC7, 0x84, 0x97, 0xCB,
    0x9E, 0x88, 0xCB, 0xC6, 0x90, 0xC0, 0xFA, 0x90, 0x91, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x53, 0x44, 0x53, 0x00, 0x18, 0x72, 0x07, 0x0C, 0x8E, 0x2F, 0x00, 0x86, 0x43, 0x00, 0x63, 0x5B,
    0x00, 0x2F, 0x53, 0x03, 0x00, 0x3B, 0x13, 0x00, 0x1F, 0x22, 0x00, 0x07, 0x2E, 0x00, 0x00, 0x33,
    0x00, 0x00, 0x30, 0x00, 0x00, 0x28, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x96, 0x79, 0x96, 0x07, 0x3D, 0xC2, 0x2F, 0x28, 0xE9, 0x5B, 0x18, 0xE1, 0x86, 0x10, 0xAE, 0x9E,
    0x10, 0x63, 0x96, 0x1B, 0x1F, 0x76, 0x30, 0x00, 0x53, 0x48, 0x00, 0x27, 0x5C, 0x00, 0x07, 0x64,
    0x00, 0x00, 0x5F, 0x27, 0x00, 0x52, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xE9, 0xC0, 0xE9, 0x4B, 0x7C, 0xE9, 0x76, 0x64, 0xE9, 0xAE, 0x4F, 0xE9, 0xE1, 0x44, 0xE9, 0xE9,
    0x47, 0xB2, 0xE9, 0x55, 0x63, 0xD1, 0x6E, 0x1F, 0x9E, 0x89, 0x00, 0x72, 0x9E, 0x00, 0x4B, 0xA8,
    0x1F, 0x37, 0xA5, 0x6A, 0x37, 0x91, 0xC9, 0x3B, 0x30, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xE9, 0xC0, 0xE9, 0xA6, 0xA5, 0xE9, 0xBA, 0x98, 0xE9, 0xD1, 0x90, 0xE9, 0xE9, 0x8C, 0xE9, 0xE9,
    0x8C, 0xD1, 0xE9, 0x91, 0xAE, 0xE1, 0x9E, 0x8E, 0xC9, 0xAA, 0x76, 0xB2, 0xB3, 0x76, 0xA6, 0xB7,
    0x8E, 0x96, 0xB7, 0xB2, 0x9E, 0xAD, 0xE1, 0x9E, 0x83, 0x9E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x44, 0x53, 0x53, 0x00, 0x1D, 0x72, 0x06, 0x0F, 0x8E, 0x26, 0x00, 0x86, 0x37, 0x00, 0x63, 0x4A,
    0x00, 0x2F, 0x44, 0x03, 0x00, 0x30, 0x17, 0x00, 0x19, 0x29, 0x00, 0x06, 0x39, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x3B, 0x00, 0x00, 0x31, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7B, 0x94, 0x96, 0x06, 0x4B, 0xC2, 0x26, 0x31, 0xE9, 0x4A, 0x1D, 0xE1, 0x6E, 0x13, 0xAE, 0x81,
    0x13, 0x63, 0x7B, 0x21, 0x1F, 0x61, 0x3B, 0x00, 0x44, 0x59, 0x00, 0x20, 0x70, 0x00, 0x06, 0x7A,
    0x00, 0x00, 0x74, 0x27, 0x00, 0x64, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xBF, 0xEB, 0xE9, 0x3D, 0x98, 0xE9, 0x61, 0x7A, 0xE9, 0x8E, 0x61, 0xE9, 0xB8, 0x53, 0xE9, 0xBF,
    0x57, 0xB2, 0xBF, 0x68, 0x63, 0xAB, 0x86, 0x1F, 0x81, 0xA8, 0x00, 0x5D, 0xC2, 0x00, 0x3D, 0xCD,
    0x1F, 0x2D, 0xC9, 0x6A, 0x2D, 0xB2, 0xC9, 0x30, 0x3B, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xBF, 0xEB, 0xE9, 0x88, 0xC9, 0xE9, 0x98, 0xBA, 0xE9, 0xAB, 0xB0, 0xE9, 0xBF, 0xAC, 0xE9, 0xBF,
    0xAC, 0xD1, 0xBF, 0xB2, 0xAE, 0xB8, 0xC2, 0x8E, 0xA5, 0xCF, 0x76, 0x91, 0xDB, 0x76, 0x88, 0xDF,
    0x8E, 0x7B, 0xDF, 0xB2, 0x81, 0xD3, 0xE1, 0x81, 0xA0, 0x9E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x4A, 0x4A, 0x4A, 0x00, 0x1A, 0x67, 0x07, 0x0E, 0x80, 0x2A, 0x00, 0x79, 0x3C, 0x00, 0x59, 0x51,
    0x00, 0x2A, 0x4A, 0x03, 0x00, 0x35, 0x15, 0x00, 0x1C, 0x25, 0x00, 0x07, 0x33, 0x00, 0x00, 0x39,
    0x00, 0x00, 0x35, 0x00, 0x00, 0x2C, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x87, 0x85, 0x87, 0x07, 0x43, 0xAE, 0x2A, 0x2C, 0xD2, 0x51, 0x1A, 0xCB, 0x79, 0x11, 0x9C, 0x8E,
    0x11, 0x59, 0x87, 0x1E, 0x1C, 0x6A, 0x35, 0x00, 0x4A, 0x50, 0x00, 0x23, 0x65, 0x00, 0x07, 0x6E,
    0x00, 0x00, 0x69, 0x23, 0x00, 0x5A, 0x6A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xD2, 0xD4, 0xD2, 0x43, 0x89, 0xD2, 0x6A, 0x6E, 0xD2, 0x9C, 0x57, 0xD2, 0xCB, 0x4A, 0xD2, 0xD2,
    0x4E, 0xA0, 0xD2, 0x5E, 0x59, 0xBC, 0x79, 0x1C, 0x8E, 0x97, 0x00, 0x67, 0xAE, 0x00, 0x43, 0xB9,
    0x1C, 0x31, 0xB5, 0x60, 0x31, 0xA0, 0xB5, 0x35, 0x35, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xD2, 0xD4, 0xD2, 0x95, 0xB5, 0xD2, 0xA7, 0xA7, 0xD2, 0xBC, 0x9E, 0xD2, 0xD2, 0x9B, 0xD2, 0xD2,
    0x9B, 0xBC, 0xD2, 0xA0, 0x9C, 0xCB, 0xAE, 0x80, 0xB5, 0xBB, 0x6A, 0xA0, 0xC5, 0x6A, 0x95, 0xC9,
    0x80, 0x87, 0xC9, 0xA0, 0x8E, 0xBE, 0xCB, 0x8E, 0x90, 0x8E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// This table is generated once by applying the luminance formula to PALETTE_COLORS.
// Y = 0.299*R + 0.587*G + 0.114*B
static constexpr uint8_t GRAYSCALE_PALETTE_LOOKUP[8][64] = {
    {84, 25, 30, 75, 91, 87, 72, 49, 31, 25, 34, 40, 38, 30, 38, 0, 150, 69, 69, 111, 143, 143, 122, 78, 56, 56, 75, 84, 79, 62, 78, 0, 236, 143, 132, 169, 203, 203, 182, 137, 102, 102, 137, 150, 137, 111, 137, 0, 236, 191, 178, 203, 224, 224, 211, 182, 146, 146, 169, 178, 165, 146, 165, 0},
    {84, 25, 30, 75, 91, 87, 72, 49, 31, 25, 34, 40, 38, 30, 38, 0, 150, 69, 69, 111, 143, 143, 122, 78, 56, 56, 75, 84, 79, 62, 78, 0, 236, 143, 132, 169, 203, 203, 182, 137, 102, 102, 137, 150, 137, 111, 137, 0, 236, 191, 178, 203, 224, 224, 211, 182, 146, 146, 169, 178, 165, 146, 165, 0},
    {84, 25, 30, 75, 91, 87, 72, 49, 31, 25, 34, 40, 38, 30, 38, 0, 150, 69, 69, 111, 143, 143, 122, 78, 56, 56, 75, 84, 79, 62, 78, 0, 236, 143, 132, 169, 203, 203, 182, 137, 102, 102, 137, 150, 137, 111, 137, 0, 236, 191, 178, 203, 224, 224, 211, 182, 146, 146, 169, 178, 165, 146, 165, 0},
    {84, 25, 30, 75, 91, 87, 72, 49, 31, 25, 34, 40, 38, 30, 38, 0, 150, 69, 69, 111, 143, 143, 122, 78, 56, 56, 75, 84, 79, 62, 78, 0, 236, 143, 132, 169, 203, 203, 182, 137, 102, 102, 137, 150, 137, 111, 137, 0, 236, 191, 178, 203, 224, 224, 211, 182, 146, 146, 169, 178, 165, 146, 165, 0},
    {84, 25, 30, 75, 91, 87, 72, 49, 31, 25, 34, 40, 38, 30, 38, 0, 150, 69, 69, 111, 143, 143, 122, 78, 56, 56, 75, 84, 79, 62, 78, 0, 236, 143, 132, 169, 203, 203, 182, 137, 102, 102, 137, 150, 137, 111, 137, 0, 236, 191, 178, 203, 224, 224, 211, 182, 146, 146, 169, 178, 165, 146, 165, 0},
    {84, 25, 30, 75, 91, 87, 72, 49, 31, 25, 34, 40, 38, 30, 38, 0, 150, 69, 69, 111, 143, 143, 122, 78, 56, 56, 75, 84, 79, 62, 78, 0, 236, 143, 132, 169, 203, 203, 182, 137, 102, 102, 137, 150, 137, 111, 137, 0, 236, 191, 178, 203, 224, 224, 211, 182, 146, 146, 169, 178, 165, 146, 165, 0},
    {84, 25, 30, 75, 91, 87, 72, 49, 31, 25, 34, 40, 38, 30, 38, 0, 150, 69, 69, 111, 143, 143, 122, 78, 56, 56, 75, 84, 79, 62, 78, 0, 236, 143, 132, 169, 203, 203, 182, 137, 102, 102, 137, 150, 137, 111, 137, 0, 236, 191, 178, 203, 224, 224, 211, 182, 146, 146, 169, 178, 165, 146, 165, 0},
    {84, 25, 30, 75, 91, 87, 72, 49, 31, 25, 34, 40, 38, 30, 38, 0, 150, 69, 69, 111, 143, 143, 122, 78, 56, 56, 75, 84, 79, 62, 78, 0, 236, 143, 132, 169, 203, 203, 182, 137, 102, 102, 137, 150, 137, 111, 137, 0, 236, 191, 178, 203, 224, 224, 211, 182, 146, 146, 169, 178, 165, 146, 165, 0}};

static constexpr uint8_t REVERSE_BYTE_LOOKUP[256] = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};

static constexpr uint8_t DECAY_MASKS[] = {0x3F, 0xDF, 0xE0};

cynes::PPU::PPU(NES &nes)
    : _nes{nes},
      _frame_buffer{new uint8_t[0x2D000]}
{
    glob_state.current_x = 0x0000;
    glob_state.current_y = 0x0000;
    glob_state.rendering_enabled = false;
    glob_state.rendering_enabled_delayed = false;
    glob_state.prevent_vertical_blank = false;
    glob_state.control_increment_mode = false;
    glob_state.control_foreground_table = false;
    glob_state.control_background_table = false;
    glob_state.control_foreground_large = false;
    glob_state.control_interrupt_on_vertical_blank = false;
    glob_state.mask_grayscale_mode = false;
    glob_state.mask_render_background_left = false;
    glob_state.mask_render_foreground_left = false;
    glob_state.mask_render_background = false;
    glob_state.mask_render_foreground = false;
    glob_state.mask_color_emphasize = 0x00;
    glob_state.status_sprite_overflow = false;
    glob_state.status_sprite_zero_hit = false;
    glob_state.status_vertical_blank = false;

    glob_state.register_decay = 0x00;
    glob_state.ppu_latch_cycle = false;
    glob_state.latch_address = false;
    glob_state.register_t_ = 0x0000;
    glob_state.register_v = 0x0000;
    glob_state.delayed_register_v = 0x0000;
    glob_state.scroll_x = 0x00;
    glob_state.delay_data_read_counter = 0x00;
    glob_state.delay_data_write_counter = 0x00;
    glob_state.buffer_data = 0x00;

    glob_state.foreground_data_pointer = 0x00;
    glob_state.foreground_sprite_count = 0x00;
    glob_state.foreground_sprite_count_next = 0x00;
    glob_state.foreground_sprite_pointer = 0x00;
    glob_state.foreground_read_delay_counter = 0x00;
    glob_state.foreground_sprite_address = 0x0000;
    glob_state.foreground_sprite_zero_line = false;
    glob_state.foreground_sprite_zero_should = false;
    glob_state.foreground_sprite_zero_hit = false;
    glob_state.foreground_evaluation_step = SpriteEvaluationStep::LOAD_SECONDARY_OAM;

    std::memset(glob_state.clock_decays, 0x00, 0x3);
    std::memset(glob_state.background_data, 0x00, 0x4);
    std::memset(glob_state.background_shifter, 0x0000, 0x8);
    std::memset(glob_state.foreground_data, 0x00, 0x20);
    std::memset(glob_state.foreground_shifter, 0x00, 0x10);
    std::memset(glob_state.foreground_attributes, 0x00, 0x8);
    std::memset(glob_state.foreground_positions, 0x00, 0x8);

    std::memset(_palette_cache, 0, sizeof(_palette_cache));
}

void cynes::PPU::update_palette_cache()
{
    for (int i = 0; i < 32; ++i)
    {
        // The address mirroring logic from read_ppu is replicated here.
        uint16_t addr = 0x3F00 + i;
        if (addr >= 0x3F10 && (addr % 4) == 0)
        {
            addr -= 0x10;
        }
        _palette_cache[i] = _nes.read_ppu(addr);
    }
}

void cynes::PPU::setOutputModeGrayscale()
{
    _render_pixel = &PPU::render_pixel_gray;
    // Clear buffer of garbage data as grayscale will only overwrite first third
    if (_frame_buffer)
    {
        std::memset(_frame_buffer.get(), 0, 0x2D000);
    }
}

void cynes::PPU::power()
{
    glob_state.current_y = 0xFF00;
    glob_state.current_x = 0xFF00;

    glob_state.rendering_enabled = false;
    glob_state.rendering_enabled_delayed = false;
    glob_state.prevent_vertical_blank = false;

    glob_state.control_increment_mode = false;
    glob_state.control_foreground_table = false;
    glob_state.control_background_table = false;
    glob_state.control_foreground_large = false;
    glob_state.control_interrupt_on_vertical_blank = false;

    glob_state.mask_grayscale_mode = false;
    glob_state.mask_render_background_left = false;
    glob_state.mask_render_foreground_left = false;
    glob_state.mask_render_background = false;
    glob_state.mask_render_foreground = false;

    glob_state.mask_color_emphasize = 0x00;

    glob_state.status_sprite_overflow = true;
    glob_state.status_sprite_zero_hit = false;
    glob_state.status_vertical_blank = true;

    glob_state.foreground_sprite_pointer = 0x00;

    glob_state.latch_address = false;
    glob_state.ppu_latch_cycle = false;

    glob_state.register_t_ = 0x0000;
    glob_state.register_v = 0x0000;
    glob_state.scroll_x = 0x00;

    glob_state.delay_data_write_counter = 0x00;
    glob_state.delay_data_read_counter = 0x00;
    glob_state.buffer_data = 0x00;
}

void cynes::PPU::reset()
{
    glob_state.current_y = 0xFF00;
    glob_state.current_x = 0xFF00;

    glob_state.rendering_enabled = false;
    glob_state.rendering_enabled_delayed = false;
    glob_state.prevent_vertical_blank = false;

    glob_state.control_increment_mode = false;
    glob_state.control_foreground_table = false;
    glob_state.control_background_table = false;
    glob_state.control_foreground_large = false;
    glob_state.control_interrupt_on_vertical_blank = false;

    glob_state.mask_grayscale_mode = false;
    glob_state.mask_render_background_left = false;
    glob_state.mask_render_foreground_left = false;
    glob_state.mask_render_background = false;
    glob_state.mask_render_foreground = false;

    glob_state.mask_color_emphasize = 0x00;

    glob_state.latch_address = false;
    glob_state.ppu_latch_cycle = false;

    glob_state.register_t_ = 0x0000;
    glob_state.register_v = 0x0000;
    glob_state.scroll_x = 0x00;

    glob_state.delay_data_write_counter = 0x00;
    glob_state.delay_data_read_counter = 0x00;
    glob_state.buffer_data = 0x00;
}

void cynes::PPU::render_pixel_rgb(size_t pixel_offset, uint8_t color_index)
{
    memcpy(_frame_buffer.get() + pixel_offset * 3, PALETTE_COLORS[glob_state.mask_color_emphasize][color_index], 3);
}

void cynes::PPU::render_pixel_gray(size_t pixel_offset, uint8_t color_index)
{
    _frame_buffer.get()[pixel_offset] = GRAYSCALE_PALETTE_LOOKUP[glob_state.mask_color_emphasize][color_index];
}

void cynes::PPU::tick()
{
    // --- OPTIMIZATION: Update the palette cache at the start of the visible frame ---
    if (glob_state.current_y == 0 && glob_state.current_x == 0 && glob_state.rendering_enabled)
    {
        update_palette_cache();
    }
    if (glob_state.current_x > 339)
    {
        glob_state.current_x = 0;

        if (++glob_state.current_y > 261)
        {
            glob_state.current_y = 0;
            glob_state.foreground_sprite_count = 0;

            glob_state.ppu_latch_cycle = !glob_state.ppu_latch_cycle;

            for (int k = 0; k < 3; k++)
            {
                if (glob_state.clock_decays[k] > 0 && --glob_state.clock_decays[k] == 0)
                {
                    if (glob_state.clock_decays[k] > 0 && --glob_state.clock_decays[k] == 0)
                    {
                        glob_state.register_decay &= DECAY_MASKS[k];
                    }
                }
            }
        }

        reset_foreground_data();

        if (glob_state.current_y == 261)
        {
            glob_state.status_sprite_overflow = false;
            glob_state.status_sprite_zero_hit = false;

            memset(glob_state.foreground_shifter, 0x00, 0x10);
        }
    }
    else
    {
        glob_state.current_x++;

        if (glob_state.current_y < 240)
        {
            if (glob_state.current_x < 257 || (glob_state.current_x >= 321 && glob_state.current_x < 337))
            {
                load_background_shifters();
            }

            if (glob_state.current_x == 256)
            {
                increment_scroll_y();
            }
            else if (glob_state.current_x == 257)
            {
                reset_scroll_x();
            }

            if (glob_state.current_x >= 2 && glob_state.current_x < 257)
            {
                update_foreground_shifter();
            }

            if (glob_state.current_x < 65)
            {
                clear_foreground_data();
            }
            else if (glob_state.current_x < 257)
            {
                fetch_foreground_data();
            }
            else if (glob_state.current_x < 321)
            {
                load_foreground_shifter();
            }

            if (glob_state.current_x > 0 && glob_state.current_x < 257 && glob_state.current_y < 240)
            {
                uint8_t color_index = _palette_cache[blend_colors()];
                (this->*_render_pixel)((glob_state.current_y << 8) + glob_state.current_x - 1, color_index);
            }
        }
        else if (glob_state.current_y == 240 && glob_state.current_x == 1)
        {
            _nes.read_ppu(glob_state.register_v);
        }
        else if (glob_state.current_y == 261)
        {
            if (glob_state.current_x == 1)
            {
                glob_state.status_vertical_blank = false;

                _nes.cpu.set_non_maskable_interrupt(false);
            }

            if (glob_state.current_x < 257 || (glob_state.current_x >= 321 && glob_state.current_x < 337))
            {
                load_background_shifters();
            }

            if (glob_state.current_x == 256)
            {
                increment_scroll_y();
            }
            else if (glob_state.current_x == 257)
            {
                reset_scroll_x();
            }
            else if (glob_state.current_x >= 280 && glob_state.current_x < 305)
            {
                reset_scroll_y();
            }

            if (glob_state.current_x > 1)
            {
                if (glob_state.current_x < 257)
                {
                    update_foreground_shifter();
                }
                else if (glob_state.current_x < 321)
                {
                    load_foreground_shifter();
                }
            }

            if (glob_state.rendering_enabled && (glob_state.current_x == 337 || glob_state.current_x == 339))
            {
                _nes.read_ppu(0x2000 | (glob_state.register_v & 0x0FFF));

                if (glob_state.current_x == 339 && glob_state.ppu_latch_cycle)
                {
                    glob_state.current_x = 340;
                }
            }
        }
        else if (glob_state.current_x == 1 && glob_state.current_y == 241)
        {
            if (!glob_state.prevent_vertical_blank)
            {
                glob_state.status_vertical_blank = true;

                if (glob_state.control_interrupt_on_vertical_blank)
                {
                    _nes.cpu.set_non_maskable_interrupt(true);
                }
            }

            glob_state.prevent_vertical_blank = false;
            glob_state.frame_ready = true;
        }
    }

    if (glob_state.rendering_enabled_delayed != glob_state.rendering_enabled)
    {
        glob_state.rendering_enabled_delayed = glob_state.rendering_enabled;

        if (glob_state.current_y < 240 || glob_state.current_y == 261)
        {
            if (!glob_state.rendering_enabled_delayed)
            {
                _nes.read_ppu(glob_state.register_v);

                if (glob_state.current_x >= 65 && glob_state.current_x <= 256)
                {
                    glob_state.foreground_sprite_pointer++;
                }
            }
        }
    }

    if (glob_state.delay_data_write_counter > 0 && --glob_state.delay_data_write_counter == 0)
    {
        glob_state.register_v = glob_state.delayed_register_v;
        glob_state.register_t_ = glob_state.register_v;

        if ((glob_state.current_y >= 240 && glob_state.current_y != 261) || !glob_state.rendering_enabled)
        {
            _nes.read_ppu(glob_state.register_v);
        }
    }

    if (glob_state.delay_data_read_counter > 0)
    {
        glob_state.delay_data_read_counter--;
    }

    _nes.get_mapper().tick();
}

void cynes::PPU::write(uint8_t address, uint8_t value)
{
    memset(glob_state.clock_decays, DECAY_PERIOD, 3);

    glob_state.register_decay = value;

    switch (static_cast<Register>(address))
    {
    case Register::PPU_CTRL:
    {
        glob_state.register_t_ &= 0xF3FF;
        glob_state.register_t_ |= (value & 0x03) << 10;

        glob_state.control_increment_mode = value & 0x04;
        glob_state.control_foreground_table = value & 0x08;
        glob_state.control_background_table = value & 0x10;
        glob_state.control_foreground_large = value & 0x20;
        glob_state.control_interrupt_on_vertical_blank = value & 0x80;

        if (!glob_state.control_interrupt_on_vertical_blank)
        {
            _nes.cpu.set_non_maskable_interrupt(false);
        }
        else if (glob_state.status_vertical_blank)
        {
            _nes.cpu.set_non_maskable_interrupt(true);
        }

        break;
    }

    case Register::PPU_MASK:
    {
        glob_state.mask_grayscale_mode = value & 0x01;
        glob_state.mask_render_background_left = value & 0x02;
        glob_state.mask_render_foreground_left = value & 0x04;
        glob_state.mask_render_background = value & 0x08;
        glob_state.mask_render_foreground = value & 0x10;
        glob_state.mask_color_emphasize = value >> 5;

        glob_state.rendering_enabled = glob_state.mask_render_background || glob_state.mask_render_foreground;
        break;
    }

    case Register::OAM_ADDR:
    {
        glob_state.foreground_sprite_pointer = value;

        break;
    }

    case Register::OAM_DATA:
    {
        if ((glob_state.current_y >= 240 && glob_state.current_y != 261) || !glob_state.rendering_enabled)
        {
            if ((glob_state.foreground_sprite_pointer & 0x03) == 0x02)
            {
                value &= 0xE3;
            }

            _nes.write_oam(glob_state.foreground_sprite_pointer++, value);
        }
        else
        {
            glob_state.foreground_sprite_pointer += 4;
        }

        break;
    }

    case Register::PPU_SCROLL:
    {
        if (!glob_state.latch_address)
        {
            glob_state.scroll_x = value & 0x07;

            glob_state.register_t_ &= 0xFFE0;
            glob_state.register_t_ |= value >> 3;
        }
        else
        {
            glob_state.register_t_ &= 0x8C1F;

            glob_state.register_t_ |= (value & 0xF8) << 2;
            glob_state.register_t_ |= (value & 0x07) << 12;
        }

        glob_state.latch_address = !glob_state.latch_address;

        break;
    }

    case Register::PPU_ADDR:
    {
        if (!glob_state.latch_address)
        {
            glob_state.register_t_ &= 0x00FF;
            glob_state.register_t_ |= value << 8;
        }
        else
        {
            glob_state.register_t_ &= 0xFF00;
            glob_state.register_t_ |= value;

            glob_state.delay_data_write_counter = 3;
            glob_state.delayed_register_v = glob_state.register_t_;
        }

        glob_state.latch_address = !glob_state.latch_address;

        break;
    }

    case Register::PPU_DATA:
    {
        if ((glob_state.register_v & 0x3FFF) >= 0x3F00)
        {
            _nes.write_ppu(glob_state.register_v, value);
        }
        else
        {
            if ((glob_state.current_y >= 240 && glob_state.current_y != 261) || !glob_state.rendering_enabled)
            {
                _nes.write_ppu(glob_state.register_v, value);
            }
            else
            {
                _nes.write_ppu(glob_state.register_v, glob_state.register_v & 0xFF);
            }
        }

        if ((glob_state.current_y >= 240 && glob_state.current_y != 261) || !glob_state.rendering_enabled)
        {
            glob_state.register_v += glob_state.control_increment_mode ? 32 : 1;
            glob_state.register_v &= 0x7FFF;

            _nes.read_ppu(glob_state.register_v);
        }
        else
        {
            increment_scroll_x();
            increment_scroll_y();
        }

        break;
    }

    default:
        break;
    }
}

uint8_t cynes::PPU::read(uint8_t address)
{
    switch (static_cast<Register>(address))
    {
    case Register::PPU_STATUS:
    {
        memset(glob_state.clock_decays, DECAY_PERIOD, 2);

        glob_state.latch_address = false;

        glob_state.register_decay &= 0x1F;
        glob_state.register_decay |= glob_state.status_sprite_overflow << 5;
        glob_state.register_decay |= glob_state.status_sprite_zero_hit << 6;
        glob_state.register_decay |= glob_state.status_vertical_blank << 7;

        glob_state.status_vertical_blank = false;
        _nes.cpu.set_non_maskable_interrupt(false);

        if (glob_state.current_y == 241 && glob_state.current_x == 0)
        {
            glob_state.prevent_vertical_blank = true;
        }

        break;
    }

    case Register::OAM_DATA:
    {
        memset(glob_state.clock_decays, DECAY_PERIOD, 3);

        glob_state.register_decay = _nes.read_oam(glob_state.foreground_sprite_pointer);

        break;
    }

    case Register::PPU_DATA:
    {
        if (glob_state.delay_data_read_counter == 0)
        {
            uint8_t value = _nes.read_ppu(glob_state.register_v);

            if ((glob_state.register_v & 0x3FFF) >= 0x3F00)
            {
                glob_state.register_decay &= 0xC0;
                glob_state.register_decay |= value & 0x3F;

                glob_state.clock_decays[0] = glob_state.clock_decays[2] = DECAY_PERIOD;

                glob_state.buffer_data = _nes.read_ppu(glob_state.register_v - 0x1000);
            }
            else
            {
                glob_state.register_decay = glob_state.buffer_data;
                glob_state.buffer_data = value;

                memset(glob_state.clock_decays, DECAY_PERIOD, 3);
            }

            if ((glob_state.current_y >= 240 && glob_state.current_y != 261) || !glob_state.rendering_enabled)
            {
                glob_state.register_v += glob_state.control_increment_mode ? 32 : 1;
                glob_state.register_v &= 0x7FFF;

                _nes.read_ppu(glob_state.register_v);
            }
            else
            {
                increment_scroll_x();
                increment_scroll_y();
            }

            glob_state.delay_data_read_counter = 6;
        }

        break;
    }

    default:
        break;
    }

    return glob_state.register_decay;
}

const uint8_t *cynes::PPU::get_frame_buffer() const
{
    return _frame_buffer.get();
}

bool cynes::PPU::is_frame_ready()
{
    bool frame_ready = glob_state.frame_ready;
    glob_state.frame_ready = false;

    return frame_ready;
}

void cynes::PPU::increment_scroll_x()
{
    if (glob_state.mask_render_background || glob_state.mask_render_foreground)
    {
        if ((glob_state.register_v & 0x001F) == 0x1F)
        {
            glob_state.register_v &= 0xFFE0;
            glob_state.register_v ^= 0x0400;
        }
        else
        {
            glob_state.register_v++;
        }
    }
}

void cynes::PPU::increment_scroll_y()
{
    if (glob_state.mask_render_background || glob_state.mask_render_foreground)
    {
        if ((glob_state.register_v & 0x7000) != 0x7000)
        {
            glob_state.register_v += 0x1000;
        }
        else
        {
            glob_state.register_v &= 0x8FFF;

            uint8_t coarse_y = (glob_state.register_v & 0x03E0) >> 5;

            if (coarse_y == 0x1D)
            {
                coarse_y = 0;
                glob_state.register_v ^= 0x0800;
            }
            else if (((glob_state.register_v >> 5) & 0x1F) == 0x1F)
            {
                coarse_y = 0;
            }
            else
            {
                coarse_y++;
            }

            glob_state.register_v &= 0xFC1F;
            glob_state.register_v |= coarse_y << 5;
        }
    }
}

void cynes::PPU::reset_scroll_x()
{
    if (glob_state.mask_render_background || glob_state.mask_render_foreground)
    {
        glob_state.register_v &= 0xFBE0;
        glob_state.register_v |= glob_state.register_t_ & 0x041F;
    }
}

void cynes::PPU::reset_scroll_y()
{
    if (glob_state.mask_render_background || glob_state.mask_render_foreground)
    {
        glob_state.register_v &= 0x841F;
        glob_state.register_v |= glob_state.register_t_ & 0x7BE0;
    }
}

void cynes::PPU::load_background_shifters()
{
    update_background_shifters();

    if (glob_state.rendering_enabled)
    {
        switch (glob_state.current_x & 0x07)
        {
        case 0x1:
        {
            glob_state.background_shifter[0] = (glob_state.background_shifter[0] & 0xFF00) | glob_state.background_data[2];
            glob_state.background_shifter[1] = (glob_state.background_shifter[1] & 0xFF00) | glob_state.background_data[3];

            if (glob_state.background_data[1] & 0x01)
            {
                glob_state.background_shifter[2] = (glob_state.background_shifter[2] & 0xFF00) | 0xFF;
            }
            else
            {
                glob_state.background_shifter[2] = (glob_state.background_shifter[2] & 0xFF00);
            }

            if (glob_state.background_data[1] & 0x02)
            {
                glob_state.background_shifter[3] = (glob_state.background_shifter[3] & 0xFF00) | 0xFF;
            }
            else
            {
                glob_state.background_shifter[3] = (glob_state.background_shifter[3] & 0xFF00);
            }

            uint16_t address = 0x2000;
            address |= glob_state.register_v & 0x0FFF;

            glob_state.background_data[0] = _nes.read_ppu(address);

            break;
        }

        case 0x3:
        {
            uint16_t address = 0x23C0;
            address |= glob_state.register_v & 0x0C00;
            address |= (glob_state.register_v >> 4) & 0x38;
            address |= (glob_state.register_v >> 2) & 0x07;

            glob_state.background_data[1] = _nes.read_ppu(address);

            if (glob_state.register_v & 0x0040)
            {
                glob_state.background_data[1] >>= 4;
            }

            if (glob_state.register_v & 0x0002)
            {
                glob_state.background_data[1] >>= 2;
            }

            glob_state.background_data[1] &= 0x03;

            break;
        }

        case 0x5:
        {
            uint16_t address = glob_state.control_background_table << 12;
            address |= glob_state.background_data[0] << 4;
            address |= glob_state.register_v >> 12;

            glob_state.background_data[2] = _nes.read_ppu(address);

            break;
        }

        case 0x7:
        {
            uint16_t address = glob_state.control_background_table << 12;
            address |= glob_state.background_data[0] << 4;
            address |= glob_state.register_v >> 12;
            address += 0x8;

            glob_state.background_data[3] = _nes.read_ppu(address);

            break;
        }

        case 0x0:
            increment_scroll_x();
            break;
        }
    }
}

void cynes::PPU::update_background_shifters()
{
    if (glob_state.mask_render_background || glob_state.mask_render_foreground)
    {
        glob_state.background_shifter[0] <<= 1;
        glob_state.background_shifter[1] <<= 1;
        glob_state.background_shifter[2] <<= 1;
        glob_state.background_shifter[3] <<= 1;
    }
}

void cynes::PPU::reset_foreground_data()
{
    glob_state.foreground_sprite_count_next = glob_state.foreground_sprite_count;

    glob_state.foreground_data_pointer = 0;
    glob_state.foreground_sprite_count = 0;
    glob_state.foreground_evaluation_step = SpriteEvaluationStep::LOAD_SECONDARY_OAM;
    glob_state.foreground_sprite_zero_line = glob_state.foreground_sprite_zero_should;
    glob_state.foreground_sprite_zero_should = false;
    glob_state.foreground_sprite_zero_hit = false;
}

void cynes::PPU::clear_foreground_data()
{
    if (glob_state.current_x & 0x01)
    {
        glob_state.foreground_data[glob_state.foreground_data_pointer++] = 0xFF;

        glob_state.foreground_data_pointer &= 0x1F;
    }
}

void cynes::PPU::fetch_foreground_data()
{
    if (glob_state.current_x % 2 == 0 && glob_state.rendering_enabled)
    {
        uint8_t sprite_size = glob_state.control_foreground_large ? 16 : 8;

        switch (glob_state.foreground_evaluation_step)
        {
        case SpriteEvaluationStep::LOAD_SECONDARY_OAM:
        {
            uint8_t sprite_data = _nes.read_oam(glob_state.foreground_sprite_pointer);

            glob_state.foreground_data[glob_state.foreground_sprite_count * 4 + (glob_state.foreground_sprite_pointer & 0x03)] = sprite_data;

            if (!(glob_state.foreground_sprite_pointer & 0x3))
            {
                int16_t offset_y = int16_t(glob_state.current_y) - int16_t(sprite_data);

                if (offset_y >= 0 && offset_y < sprite_size)
                {
                    if (!glob_state.foreground_sprite_pointer++)
                    {
                        glob_state.foreground_sprite_zero_should = true;
                    }
                }
                else
                {
                    glob_state.foreground_sprite_pointer += 4;

                    if (!glob_state.foreground_sprite_pointer)
                    {
                        glob_state.foreground_evaluation_step = SpriteEvaluationStep::IDLE;
                    }
                    else if (glob_state.foreground_sprite_count == 8)
                    {
                        glob_state.foreground_evaluation_step = SpriteEvaluationStep::INCREMENT_POINTER;
                    }
                }
            }
            else if (!(++glob_state.foreground_sprite_pointer & 0x03))
            {
                glob_state.foreground_sprite_count++;

                if (!glob_state.foreground_sprite_pointer)
                {
                    glob_state.foreground_evaluation_step = SpriteEvaluationStep::IDLE;
                }
                else if (glob_state.foreground_sprite_count == 8)
                {
                    glob_state.foreground_evaluation_step = SpriteEvaluationStep::INCREMENT_POINTER;
                }
            }

            break;
        }

        case SpriteEvaluationStep::INCREMENT_POINTER:
        {
            if (glob_state.foreground_read_delay_counter)
            {
                glob_state.foreground_read_delay_counter--;
            }
            else
            {
                int16_t offset_y = int16_t(glob_state.current_y) - int16_t(_nes.read_oam(glob_state.foreground_sprite_pointer));

                if (offset_y >= 0 && offset_y < sprite_size)
                {
                    glob_state.status_sprite_overflow = true;

                    glob_state.foreground_sprite_pointer++;
                    glob_state.foreground_read_delay_counter = 3;
                }
                else
                {
                    uint8_t low = (glob_state.foreground_sprite_pointer + 1) & 0x03;

                    glob_state.foreground_sprite_pointer += 0x04;
                    glob_state.foreground_sprite_pointer &= 0xFC;

                    if (!glob_state.foreground_sprite_pointer)
                    {
                        glob_state.foreground_evaluation_step = SpriteEvaluationStep::IDLE;
                    }

                    glob_state.foreground_sprite_pointer |= low;
                }
            }

            break;
        }

        default:
            glob_state.foreground_sprite_pointer = 0;
        }
    }
}

void cynes::PPU::load_foreground_shifter()
{
    if (glob_state.rendering_enabled)
    {
        glob_state.foreground_sprite_pointer = 0;

        if (glob_state.current_x == 257)
        {
            glob_state.foreground_data_pointer = 0;
        }

        switch (glob_state.current_x & 0x7)
        {
        case 0x1:
        {
            uint16_t address = 0x2000;
            address |= glob_state.register_v & 0x0FFF;

            _nes.read_ppu(address);

            break;
        }

        case 0x3:
        {
            uint16_t address = 0x23C0;
            address |= glob_state.register_v & 0x0C00;
            address |= (glob_state.register_v >> 4) & 0x38;
            address |= (glob_state.register_v >> 2) & 0x07;

            _nes.read_ppu(address);

            break;
        }

        case 0x5:
        {
            uint8_t sprite_index = glob_state.foreground_data[glob_state.foreground_data_pointer * 4 + 1];
            uint8_t sprite_attribute = glob_state.foreground_data[glob_state.foreground_data_pointer * 4 + 2];

            uint8_t offset = 0x00;

            if (glob_state.foreground_data_pointer < glob_state.foreground_sprite_count)
            {
                offset = glob_state.current_y - glob_state.foreground_data[glob_state.foreground_data_pointer * 4];
            }

            glob_state.foreground_sprite_address = 0x0000;

            if (glob_state.control_foreground_large)
            {
                glob_state.foreground_sprite_address = (sprite_index & 0x01) << 12;

                if (sprite_attribute & 0x80)
                {
                    if (offset < 8)
                    {
                        glob_state.foreground_sprite_address |= ((sprite_index & 0xFE) + 1) << 4;
                    }
                    else
                    {
                        glob_state.foreground_sprite_address |= ((sprite_index & 0xFE)) << 4;
                    }
                }
                else
                {
                    if (offset < 8)
                    {
                        glob_state.foreground_sprite_address |= ((sprite_index & 0xFE)) << 4;
                    }
                    else
                    {
                        glob_state.foreground_sprite_address |= ((sprite_index & 0xFE) + 1) << 4;
                    }
                }
            }
            else
            {
                glob_state.foreground_sprite_address = glob_state.control_foreground_table << 12 | sprite_index << 4;
            }

            if (sprite_attribute & 0x80)
            {
                glob_state.foreground_sprite_address |= (7 - offset) & 0x07;
            }
            else
            {
                glob_state.foreground_sprite_address |= offset & 0x07;
            }

            uint8_t sprite_pattern_lsb_plane = _nes.read_ppu(glob_state.foreground_sprite_address);

            if (sprite_attribute & 0x40)
            {
                sprite_pattern_lsb_plane = REVERSE_BYTE_LOOKUP[sprite_pattern_lsb_plane];
            }

            glob_state.foreground_shifter[glob_state.foreground_data_pointer * 2] = sprite_pattern_lsb_plane;

            break;
        }

        case 0x7:
        {
            uint8_t sprite_pattern_msb_plane = _nes.read_ppu(glob_state.foreground_sprite_address + 8);

            if (glob_state.foreground_data[glob_state.foreground_data_pointer * 4 + 2] & 0x40)
            {
                sprite_pattern_msb_plane = REVERSE_BYTE_LOOKUP[sprite_pattern_msb_plane];
            }

            glob_state.foreground_shifter[glob_state.foreground_data_pointer * 2 + 1] = sprite_pattern_msb_plane;
            glob_state.foreground_positions[glob_state.foreground_data_pointer] = glob_state.foreground_data[glob_state.foreground_data_pointer * 4 + 3];
            glob_state.foreground_attributes[glob_state.foreground_data_pointer] = glob_state.foreground_data[glob_state.foreground_data_pointer * 4 + 2];

            glob_state.foreground_data_pointer++;

            break;
        }
        }
    }
}

void cynes::PPU::update_foreground_shifter()
{
    if (glob_state.mask_render_foreground)
    {
        for (uint8_t sprite = 0; sprite < glob_state.foreground_sprite_count_next; sprite++)
        {
            if (glob_state.foreground_positions[sprite] > 0)
            {
                glob_state.foreground_positions[sprite]--;
            }
            else
            {
                glob_state.foreground_shifter[sprite * 2] <<= 1;
                glob_state.foreground_shifter[sprite * 2 + 1] <<= 1;
            }
        }
    }
}

uint8_t cynes::PPU::blend_colors()
{
    if (!glob_state.rendering_enabled && (glob_state.register_v & 0x3FFF) >= 0x3F00)
    {
        return glob_state.register_v & 0x1F;
    }

    uint8_t background_pixel = 0x00;
    uint8_t background_palette = 0x00;

    if (glob_state.mask_render_background && (glob_state.current_x > 8 || glob_state.mask_render_background_left))
    {
        uint16_t bit_mask = 0x8000 >> glob_state.scroll_x;

        background_pixel = ((glob_state.background_shifter[0] & bit_mask) > 0) | (((glob_state.background_shifter[1] & bit_mask) > 0) << 1);
        background_palette = ((glob_state.background_shifter[2] & bit_mask) > 0) | (((glob_state.background_shifter[3] & bit_mask) > 0) << 1);
    }

    uint8_t foreground_pixel = 0x00;
    uint8_t foreground_palette = 0x00;
    uint8_t foreground_priority = 0x00;

    if (glob_state.mask_render_foreground && (glob_state.current_x > 8 || glob_state.mask_render_foreground_left))
    {
        glob_state.foreground_sprite_zero_hit = false;

        for (uint8_t sprite = 0; sprite < glob_state.foreground_sprite_count_next; sprite++)
        {
            if (glob_state.foreground_positions[sprite] == 0)
            {
                foreground_pixel = ((glob_state.foreground_shifter[sprite * 2] & 0x80) > 0) | (((glob_state.foreground_shifter[sprite * 2 + 1] & 0x80) > 0) << 1);
                foreground_palette = (glob_state.foreground_attributes[sprite] & 0x03) + 0x04;
                foreground_priority = (glob_state.foreground_attributes[sprite] & 0x20) == 0x00;

                if (foreground_pixel != 0)
                {
                    if (sprite == 0 && glob_state.current_x != 256)
                    {
                        glob_state.foreground_sprite_zero_hit = true;
                    }

                    break;
                }
            }
        }
    }

    uint8_t final_pixel = 0;
    uint8_t final_palette = 0;

    bool b_is_opaque = (background_pixel != 0);
    bool f_is_opaque = (foreground_pixel != 0);

    if (b_is_opaque && (!f_is_opaque || !foreground_priority))
    {
        final_pixel = background_pixel;
        final_palette = background_palette;
    }
    else if (f_is_opaque)
    {
        final_pixel = foreground_pixel;
        final_palette = foreground_palette;
    }

    if (b_is_opaque && f_is_opaque && glob_state.foreground_sprite_zero_hit && glob_state.foreground_sprite_zero_line && (glob_state.current_x > 8 || glob_state.mask_render_background_left || glob_state.mask_render_foreground_left))
    {
        glob_state.status_sprite_zero_hit = true;
    }

    final_pixel |= final_palette << 2;

    if (glob_state.mask_grayscale_mode)
    {
        final_pixel &= 0x30;
    }

    return final_pixel;
}
