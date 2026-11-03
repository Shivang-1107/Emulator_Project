#include "cpu.h"
#include <algorithm>

CPU::CPU(const std::vector<uint8_t>& rom) {
    std::copy_n(rom.begin(), std::min(rom.size(), std::size_t(0x8000)), memory.begin());
    reset();
}

void CPU::reset() {
    reg.setAF(0x01B0);
    reg.setBC(0x0013);
    reg.setDE(0x00D8);
    reg.setHL(0x014D);
    reg.sp = 0xFFFE;
    reg.pc = 0x0100;
    ime = false;
    imeDelay = 0;
    halted = stopped = haltBug = locked = false;
}

uint8_t CPU::read(uint16_t address) const { return memory[address]; }
void CPU::write(uint16_t address, uint8_t value) { memory[address] = value; }
bool CPU::interruptsEnabled() const { return ime; }
bool CPU::isHalted() const { return halted; }
bool CPU::isStopped() const { return stopped; }
bool CPU::isLocked() const { return locked; }
void CPU::wakeFromStop() { stopped = false; }

uint8_t CPU::fetch() {
    uint8_t value = read(reg.pc);
    if (haltBug) haltBug = false;
    else reg.pc++;
    return value;
}

uint16_t CPU::fetch16() {
    uint8_t low = fetch();
    uint8_t high = fetch();
    return (high << 8) | low;
}

void CPU::setFlag(uint8_t flag, bool on) {
    if (on) reg.f |= flag;
    else reg.f &= static_cast<uint8_t>(~flag);
    reg.f &= 0xF0;
}

uint8_t CPU::readRegister(int index) const {
    switch (index) {
        case 0: return reg.b;
        case 1: return reg.c;
        case 2: return reg.d;
        case 3: return reg.e;
        case 4: return reg.h;
        case 5: return reg.l;
        case 6: return read(reg.getHL());
        default: return reg.a;
    }
}

void CPU::writeRegister(int index, uint8_t value) {
    switch (index) {
        case 0: reg.b = value; break;
        case 1: reg.c = value; break;
        case 2: reg.d = value; break;
        case 3: reg.e = value; break;
        case 4: reg.h = value; break;
        case 5: reg.l = value; break;
        case 6: write(reg.getHL(), value); break;
        case 7: reg.a = value; break;
    }
}

uint16_t CPU::readPair(int index) const {
    switch (index) {
        case 0: return reg.getBC();
        case 1: return reg.getDE();
        case 2: return reg.getHL();
        default: return reg.sp;
    }
}

void CPU::writePair(int index, uint16_t value) {
    switch (index) {
        case 0: reg.setBC(value); break;
        case 1: reg.setDE(value); break;
        case 2: reg.setHL(value); break;
        case 3: reg.sp = value; break;
    }
}

bool CPU::condition(int index) const {
    switch (index) {
        case 0: return !(reg.f & FLAG_Z);
        case 1: return reg.f & FLAG_Z;
        case 2: return !(reg.f & FLAG_C);
        default: return reg.f & FLAG_C;
    }
}

void CPU::push(uint16_t value) {
    write(--reg.sp, value >> 8);
    write(--reg.sp, value & 0xFF);
}

uint16_t CPU::pop() {
    uint8_t low = read(reg.sp++);
    uint8_t high = read(reg.sp++);
    return (high << 8) | low;
}

void CPU::cp(uint8_t value) {
    setFlag(FLAG_Z, reg.a == value);
    setFlag(FLAG_N, true);
    setFlag(FLAG_H, (reg.a & 0x0F) < (value & 0x0F));
    setFlag(FLAG_C, reg.a < value);
}

void CPU::alu(int operation, uint8_t value) {
    int a = reg.a;
    int carry = (reg.f & FLAG_C) ? 1 : 0;
    int result = 0;
    switch (operation) {
        case 0:
        case 1:
            if (operation == 0) carry = 0;
            result = a + value + carry;
            reg.f = 0;
            setFlag(FLAG_H, (a & 15) + (value & 15) + carry > 15);
            setFlag(FLAG_C, result > 255);
            reg.a = static_cast<uint8_t>(result);
            break;
        case 2:
        case 3:
            if (operation == 2) carry = 0;
            result = a - value - carry;
            reg.f = FLAG_N;
            setFlag(FLAG_H, (a & 15) < (value & 15) + carry);
            setFlag(FLAG_C, a < value + carry);
            reg.a = static_cast<uint8_t>(result);
            break;
        case 4: reg.a &= value; reg.f = FLAG_H; break;
        case 5: reg.a ^= value; reg.f = 0; break;
        case 6: reg.a |= value; reg.f = 0; break;
        case 7: cp(value); return;
    }
    setFlag(FLAG_Z, reg.a == 0);
}

uint8_t CPU::rotate(int operation, uint8_t value) {
    int oldCarry = (reg.f & FLAG_C) ? 1 : 0;
    bool carry = false;
    uint8_t result = value;
    switch (operation) {
        case 0: carry = value & 0x80; result = (value << 1) | (value >> 7); break;
        case 1: carry = value & 1; result = (value >> 1) | (value << 7); break;
        case 2: carry = value & 0x80; result = (value << 1) | oldCarry; break;
        case 3: carry = value & 1; result = (value >> 1) | (oldCarry << 7); break;
        case 4: carry = value & 0x80; result = value << 1; break;
        case 5: carry = value & 1; result = (value >> 1) | (value & 0x80); break;
        case 6: result = (value << 4) | (value >> 4); break;
        case 7: carry = value & 1; result = value >> 1; break;
    }
    reg.f = 0;
    setFlag(FLAG_Z, result == 0);
    setFlag(FLAG_C, carry);
    return result;
}

int CPU::executeCB() {
    uint8_t opcode = fetch();
    int group = opcode >> 6;
    int operation = (opcode >> 3) & 7;
    int target = opcode & 7;
    uint8_t value = readRegister(target);
    if (group == 0) writeRegister(target, rotate(operation, value));
    else if (group == 1) {
        setFlag(FLAG_Z, (value & (1 << operation)) == 0);
        setFlag(FLAG_N, false);
        setFlag(FLAG_H, true);
    } else if (group == 2) {
        writeRegister(target, value & ~(1 << operation));
    } else {
        writeRegister(target, value | (1 << operation));
    }
    return target == 6 ? (group == 1 ? 12 : 16) : 8;
}

uint16_t CPU::addOffset(uint16_t value, uint8_t operand) {
    int offset = operand < 0x80 ? operand : int(operand) - 0x100;
    reg.f = 0;
    setFlag(FLAG_H, (value & 15) + (operand & 15) > 15);
    setFlag(FLAG_C, (value & 255) + operand > 255);
    return static_cast<uint16_t>(value + offset);
}

int CPU::jr(bool take) {
    uint8_t operand = fetch();
    int offset = operand < 0x80 ? operand : int(operand) - 0x100;
    if (!take) return 8;
    reg.pc = static_cast<uint16_t>(reg.pc + offset);
    return 12;
}

int CPU::step() {
    if (locked) return -1;
    if (stopped) return 0;
    if (halted) {
        if ((read(0xFFFF) & read(0xFF0F) & 0x1F) == 0) return 4;
        halted = false;
    }
    int cycles = execute(fetch());
    if (imeDelay > 0 && --imeDelay == 0) ime = true;
    reg.f &= 0xF0;
    return cycles;
}

int CPU::execute(uint8_t opcode) {
    int target = (opcode >> 3) & 7;
    int source = opcode & 7;
    int pair = (opcode >> 4) & 3;
    if (opcode >= 0x40 && opcode <= 0x7F) {
        if (opcode == 0x76) {
            if (!ime && (read(0xFFFF) & read(0xFF0F) & 0x1F)) haltBug = true;
            else halted = true;
            return 4;
        }
        writeRegister(target, readRegister(source));
        return (target == 6 || source == 6) ? 8 : 4;
    }
    if (opcode >= 0x80 && opcode <= 0xBF) {
        alu(target, readRegister(source));
        return source == 6 ? 8 : 4;
    }
    if (opcode < 0x40) {
        if ((opcode & 7) == 4 || (opcode & 7) == 5) {
            uint8_t value = readRegister(target);
            bool decrement = (opcode & 7) == 5;
            uint8_t result = static_cast<uint8_t>(value + (decrement ? -1 : 1));
            setFlag(FLAG_Z, result == 0);
            setFlag(FLAG_N, decrement);
            setFlag(FLAG_H, decrement ? (value & 15) == 0 : (value & 15) == 15);
            writeRegister(target, result);
            return target == 6 ? 12 : 4;
        }
        if ((opcode & 7) == 6) {
            writeRegister(target, fetch());
            return target == 6 ? 12 : 8;
        }
        if ((opcode & 15) == 1) { writePair(pair, fetch16()); return 12; }
        if ((opcode & 15) == 3) { writePair(pair, readPair(pair) + 1); return 8; }
        if ((opcode & 15) == 11) { writePair(pair, readPair(pair) - 1); return 8; }
        if ((opcode & 15) == 9) {
            uint16_t hl = reg.getHL();
            uint16_t value = readPair(pair);
            setFlag(FLAG_N, false);
            setFlag(FLAG_H, (hl & 0xFFF) + (value & 0xFFF) > 0xFFF);
            setFlag(FLAG_C, int(hl) + value > 0xFFFF);
            reg.setHL(hl + value);
            return 8;
        }
        if ((opcode & 15) == 2 || (opcode & 15) == 10) {
            uint16_t address = pair < 2 ? readPair(pair) : reg.getHL();
            if (opcode & 8) reg.a = read(address);
            else write(address, reg.a);
            if (pair == 2) reg.setHL(address + 1);
            if (pair == 3) reg.setHL(address - 1);
            return 8;
        }
    }
    if (opcode >= 0xC0) {
        if ((opcode & 0xC7) == 0xC6) { alu(target, fetch()); return 8; }
        if ((opcode & 0xC7) == 0xC7) { push(reg.pc); reg.pc = opcode & 0x38; return 16; }
        if ((opcode & 0xCF) == 0xC1) {
            uint16_t value = pop();
            if (pair == 3) reg.setAF(value);
            else writePair(pair, value);
            return 12;
        }
        if ((opcode & 0xCF) == 0xC5) {
            push(pair == 3 ? reg.getAF() : readPair(pair));
            return 16;
        }
        if (opcode < 0xE0) {
            if ((opcode & 7) == 0) {
                if (!condition(target & 3)) return 8;
                reg.pc = pop();
                return 20;
            }
            if ((opcode & 7) == 2) {
                uint16_t address = fetch16();
                if (!condition(target & 3)) return 12;
                reg.pc = address;
                return 16;
            }
            if ((opcode & 7) == 4) {
                uint16_t address = fetch16();
                if (!condition(target & 3)) return 12;
                push(reg.pc);
                reg.pc = address;
                return 24;
            }
        }
    }
    switch (opcode) {
        case 0x00: return 4;
        case 0x08: {
            uint16_t address = fetch16();
            write(address, reg.sp & 0xFF);
            write(static_cast<uint16_t>(address + 1), reg.sp >> 8);
            return 20;
        }
        case 0x10: fetch(); stopped = true; return 4;
        case 0x18: return jr(true);
        case 0x20: case 0x28: case 0x30: case 0x38:
            return jr(condition((opcode >> 3) & 3));
        case 0x07: case 0x0F: case 0x17: case 0x1F:
            reg.a = rotate((opcode >> 3) & 3, reg.a);
            setFlag(FLAG_Z, false);
            return 4;
        case 0x27: {
            int correction = 0;
            bool carry = reg.f & FLAG_C;
            if (reg.f & FLAG_N) {
                if (reg.f & FLAG_H) correction |= 0x06;
                if (carry) correction |= 0x60;
                reg.a = static_cast<uint8_t>(reg.a - correction);
            } else {
                if ((reg.f & FLAG_H) || (reg.a & 15) > 9) correction |= 0x06;
                if (carry || reg.a > 0x99) { correction |= 0x60; carry = true; }
                reg.a = static_cast<uint8_t>(reg.a + correction);
            }
            setFlag(FLAG_Z, reg.a == 0);
            setFlag(FLAG_H, false);
            setFlag(FLAG_C, carry);
            return 4;
        }
        case 0x2F: reg.a = ~reg.a; setFlag(FLAG_N, true); setFlag(FLAG_H, true); return 4;
        case 0x37: setFlag(FLAG_N, false); setFlag(FLAG_H, false); setFlag(FLAG_C, true); return 4;
        case 0x3F: setFlag(FLAG_N, false); setFlag(FLAG_H, false); setFlag(FLAG_C, !(reg.f & FLAG_C)); return 4;
        case 0xC3: reg.pc = fetch16(); return 16;
        case 0xC9: reg.pc = pop(); return 16;
        case 0xCB: return executeCB();
        case 0xCD: {
            uint16_t address = fetch16();
            push(reg.pc);
            reg.pc = address;
            return 24;
        }
        case 0xD9: reg.pc = pop(); ime = true; imeDelay = 0; return 16;
        case 0xE0: write(0xFF00 | fetch(), reg.a); return 12;
        case 0xE2: write(0xFF00 | reg.c, reg.a); return 8;
        case 0xE8: reg.sp = addOffset(reg.sp, fetch()); return 16;
        case 0xE9: reg.pc = reg.getHL(); return 4;
        case 0xEA: { uint16_t address = fetch16(); write(address, reg.a); return 16; }
        case 0xF0: reg.a = read(0xFF00 | fetch()); return 12;
        case 0xF2: reg.a = read(0xFF00 | reg.c); return 8;
        case 0xF3: ime = false; imeDelay = 0; return 4;
        case 0xF8: reg.setHL(addOffset(reg.sp, fetch())); return 12;
        case 0xF9: reg.sp = reg.getHL(); return 8;
        case 0xFA: reg.a = read(fetch16()); return 16;
        case 0xFB: if (!ime && imeDelay == 0) imeDelay = 2; return 4;
        default: locked = true; return -1;
    }
}

