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

## Phase 1 — CPU Core and Instruction Set

Implemented the Game Boy CPU core with registers, flags, instruction decoding,
and instruction cycle counts.

### What was added
- All legal base instructions and all 256 CB-prefixed operations
- Register and memory loads, arithmetic, logic, and comparisons
- Increments, decrements, rotations, shifts, and bit operations
- Stack operations, jumps, calls, returns, and restart instructions
- CPU control instructions: HALT, STOP, DI, EI, and RETI
- Temporary 64 KB writable memory for instruction execution and stack storage

### Verification completed
- Successfully built using CMake and Visual Studio 2026
- Loaded `Pokemon_Red.gb` and matched the expected cartridge information and checksum
- Executed five instructions with the expected register values, program counter
  values, and cycle counts: NOP, JP, CP, JR Z, and XOR A

This verifies the initial execution path. The complete instruction set has not
yet been tested.

### Current limitations
- Memory is a temporary flat buffer; the Game Boy memory map is not implemented
- Interrupt servicing is pending
- STOP wake-up still needs hardware integration
- Execution currently prints five CPU steps; games are not yet playable

### Next
Phase 2 — Memory and Bus/MMU: route CPU memory accesses through the Game Boy
memory map.