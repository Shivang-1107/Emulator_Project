#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <string>
#include <cstdint>
#include <vector>

struct CartridgeHeader {
    std::string title;
    uint8_t cartridgeType;
    uint8_t romSizeCode;
    uint8_t ramSizeCode;
    uint8_t headerChecksum;
};

std::vector <uint8_t> loadROM(const std::string& filepath);
CartridgeHeader parseHeader(const std::vector<uint8_t>& romData);
uint8_t calculateChecksum(const std::vector<uint8_t>& romData);
int getROMSizeKB(uint8_t romSizeCode);
int getRAMSizeKB(uint8_t ramSizeCode);

#endif