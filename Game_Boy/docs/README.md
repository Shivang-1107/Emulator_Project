# Game Boy Emulator

## Phase 0

Set up the project skeleton and build pipeline, and built a ROM loader that reads a
Game Boy ROM file, parses its header, and verifies the header checksum.

### What it currently does
- Loads a `.gb` ROM file into memory as a raw byte buffer
- Parses the cartridge header: title, cartridge type, ROM size code, RAM size code,
  header checksum
- Converts ROM/RAM size codes into actual KB values
- Recalculates the header checksum and verifies it against the stored value
- Tested against `Tetris.gb` and `Pokemon_Red.gb` — checksum PASS on both

### Known limitations
- No CPU emulation yet — only header metadata is read, nothing is executed
- ROM path is hardcoded in `main.cpp`; no file picker yet
- Only tested against 2 ROMs so far

### Tools and environment
- CMake 4.3.5
- Visual Studio Community 2026, C++17
- Windows

### Next
Phase 1 — CPU Core: fetch-decode-execute loop, registers, flags, opcode tables.