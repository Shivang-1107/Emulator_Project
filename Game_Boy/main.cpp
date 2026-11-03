#include <iostream>
#include "cartridge/cartridge.h"
#include "cpu/cpu.h"

int main() {
    std::cout << "Game Boy emulator starting..." << std::endl;

    std::string romPath = "tests/roms/Pokemon_Red.gb";
    std::vector<uint8_t> romData = loadROM(romPath);

    if (romData.empty()) {
        std::cerr << "Failed to load ROM." << std::endl;
        return 1;
    }

    std::cout << "Loaded ROM: " << romData.size() << " bytes" << std::endl;

    CartridgeHeader header = parseHeader(romData);
    std::cout << "Title: " << header.title << std::endl;
    std::cout << "Cartridge type: " << (int)header.cartridgeType << std::endl;
    std::cout << "ROM size code: " << (int)header.romSizeCode << std::endl;
    std::cout << "RAM size code: " << (int)header.ramSizeCode << std::endl;
    std::cout << "Header checksum: " << (int)header.headerChecksum << std::endl;

    uint8_t calculatedChecksum = calculateChecksum(romData);
    std::cout << "Calculated checksum: " << (int)calculatedChecksum << std::endl;

    if (calculatedChecksum == header.headerChecksum) {
        std::cout << "Checksum: PASS" << std::endl;
    }
    else {
        std::cout << "Checksum: FAIL" << std::endl;
    }

    std::cout << "ROM size: " << getROMSizeKB(header.romSizeCode) << " KB" << std::endl;
    std::cout << "RAM size: " << getRAMSizeKB(header.ramSizeCode) << " KB" << std::endl;

    CPU cpu(romData);

    std::cout << "Starting PC: 0x" << std::hex << cpu.reg.pc << std::dec << std::endl;

    for (int i = 0; i < 5; i++) {
        int cycles = cpu.step();
        if (cycles < 0) break;
            std::cout << "Step " << i + 1 << ": " << cycles << " cycles, PC = 0x"
            << std::hex << cpu.reg.pc
            << ", A = 0x" << (int)cpu.reg.a
            << ", F = 0x" << (int)cpu.reg.f << std::dec << std::endl;
    }

    return 0;
}