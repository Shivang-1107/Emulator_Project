#include "cartridge.h"
#include <fstream>
#include <iostream>

std::vector<uint8_t> loadROM(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);

    if (!file) {
        std::cerr << "Error: could not open file " << filepath << std::endl;
        return {};
    }

    std::vector<uint8_t> buffer(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    return buffer;
}

CartridgeHeader parseHeader(const std::vector<uint8_t>& romData) {
    CartridgeHeader header;

    header.title = "";
    for (int i = 0x0134; i <= 0x0143; i++) {
        char c = static_cast<char>(romData[i]);
        if (c == '\0') break;
        header.title += c;
    }

    header.cartridgeType = romData[0x0147];
    header.romSizeCode   = romData[0x0148];
    header.ramSizeCode   = romData[0x0149];
    header.headerChecksum = romData[0x014D];

    return header;
}

uint8_t calculateChecksum(const std::vector<uint8_t>& romData) {
    int x = 0;
    for (int i = 0x0134; i <= 0x014C; i++) {
        x = x - romData[i] - 1;
    }
    return static_cast<uint8_t>(x & 0xFF);
}

int getROMSizeKB(uint8_t romSizeCode) {
    return 32 * (1 << romSizeCode);
}

int getRAMSizeKB(uint8_t ramSizeCode) {
    switch (ramSizeCode) {
        case 0: return 0;
        case 1: return 2;
        case 2: return 8;
        case 3: return 32;
        case 4: return 128;
        case 5: return 64;
        default: return -1;
    }
}