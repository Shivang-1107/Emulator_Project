#ifndef CPU_H
#define CPU_H

#include <array>
#include <cstdint>
#include <vector>

struct Registers {
    uint8_t a, f;
    uint8_t b, c;
    uint8_t d, e;
    uint8_t h, l;

    uint16_t sp;
    uint16_t pc;

    uint16_t getBC() const { return (b << 8) | c; }
    void setBC(uint16_t value) {
        b = (value >> 8) & 0xFF;
        c = value & 0xFF;
    }
    uint16_t getDE() const { return (d << 8) | e; }
    void setDE(uint16_t value) {
        d = (value >> 8) & 0xFF;
        e = value & 0xFF;
    }
    uint16_t getHL() const { return (h << 8) | l; }
    void setHL(uint16_t value) {
        h = (value >> 8) & 0xFF;
        l = value & 0xFF;
    }
    uint16_t getAF() const { return (a << 8) | f; }
    void setAF(uint16_t value) {
        a = (value >> 8) & 0xFF;
        f = value & 0xF0;
    }
};

class CPU {
    private:
        std::array<uint8_t, 65536> memory{};
        bool ime = false;
        int imeDelay = 0;
        bool halted = false;
        bool stopped = false;
        bool haltBug = false;
        bool locked = false;
        uint8_t readRegister(int index) const;
        void writeRegister(int index, uint8_t value);
        uint16_t readPair(int index) const;
        void writePair(int index, uint16_t value);
        bool condition(int index) const;
        void push(uint16_t value);
        uint16_t pop();
        void alu(int operation, uint8_t value);
        uint8_t rotate(int operation, uint8_t value);
        int execute(uint8_t opcode);
        int executeCB();
        uint16_t addOffset(uint16_t value, uint8_t operand);
    public:
        Registers reg;
        CPU(const std::vector<uint8_t>& rom);
        void reset();
        uint8_t read(uint16_t address) const;
        void write(uint16_t address, uint8_t value);
        bool interruptsEnabled() const;
        bool isHalted() const;
        bool isStopped() const;
        bool isLocked() const;
        void wakeFromStop();
        uint8_t fetch();
        uint16_t fetch16();
        int step();
        static constexpr uint8_t FLAG_Z = 0x80;
        static constexpr uint8_t FLAG_N = 0x40;
        static constexpr uint8_t FLAG_H = 0x20;
        static constexpr uint8_t FLAG_C = 0x10;
        void setFlag(uint8_t flag, bool on);
        void cp(uint8_t value);
        int jr(bool condition);
};

#endif
