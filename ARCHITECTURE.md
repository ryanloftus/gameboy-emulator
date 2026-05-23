# Architecture and implementation status

This document describes how the Game Boy emulator is organized, what each source file does, and which hardware subsystems are implemented, in progress, or not started.

The project is a C11 emulator targeting a **little-endian** host. It uses **SDL2** for display output and **Unity** (vendored) for unit tests. CPU behavior is modeled after the [SM83 instruction set](https://gbdev.io/pandocs/CPU_Instruction_Set.html) documented on gbdev.io.

## High-level layout

```
┌─────────────┐     ┌──────────────┐     ┌─────────────┐
│   main.c    │────▶│  mmu (RAM)   │◀────│  cpu.c      │
│  SDL loop   │     │  64 KiB map  │     │ fetch/exec  │
└──────┬──────┘     └──────┬───────┘     └──────┬──────┘
       │                   │                    │
       │                   │         decode.c ◀──┤ decode
       │                   │                    ▼
       │                   │            instructions.c
       ▼                   │                 (execute)
┌─────────────┐            │
│   ppu.c     │────────────┘
│  render()   │
└─────────────┘
```

Today, **main** initializes memory and the CPU but does **not** run the CPU in its game loop—it only drives the PPU’s `render()` on a fixed ~60 Hz timer. The CPU is exercised primarily through **unit tests** that pass a raw `uint8_t *code` buffer instead of ROM in memory.

---

## Source files

### Application entry

| File | Role |
|------|------|
| `src/main.c` | Creates an SDL2 window (160×144, scaled 4×), allocates a 32-bit RGBA frame buffer, initializes `memory` and `virtual_cpu`, and runs an event loop. Each frame it calls `render()`, uploads pixels to an SDL texture, and presents. Window close exits the loop. |
| `Makefile` | Builds `gameboy` from all `src/*.c` with SDL2. Supports `BUILD=debug` (ASan/UBSan, `-g -O0`) vs release (`-O2`). Targets: `all`, `run`, `tests`, `test`, `clean`, `rebuild`. |
| `test.sh` | Runs `make tests` and executes `run_tests`. |

### CPU

| File | Role |
|------|------|
| `include/cpu.h` | Defines `virtual_cpu`: 8-bit registers (`a`–`l`, `f`), 16-bit pairs (`af`, `bc`, `de`, `hl`), `pc`, `sp`, plus pointers to `memory` and an optional `code` buffer used by tests. Declares `create_virtual_cpu` and `fetch_execute`. |
| `src/cpu.c` | Zeroes the CPU, wires `mem` and `code`. `fetch_execute` reads one opcode, calls `decode_opcode`, then `execute_instruction`, then advances `pc` by `decoded_instr.bytes`. Unmatched opcodes print a message. |

### Decode (opcode → instruction descriptor)

| File | Role |
|------|------|
| `include/decode.h` | `instr_id` enum, `instr_operands`, `decoded_instr`, and `decode_opcode`. |
| `src/decode.c` | Pure decode: block 0 pattern table, block 1/2 field extraction, metadata (`bytes`, `cycles`). No CPU mutation. |

### Instructions / execute (SM83)

| File | Role |
|------|------|
| `include/instructions.h` | Declares `execute_instruction(cpu, decoded_instr *)`. |
| `src/instructions.c` | Register accessors, flag/ALU helpers (gbz80-aligned), and per-instruction execute handlers selected by `instr_id`. |

**Pipeline:** `fetch` → `decode_opcode` → `execute_instruction` → `pc += bytes`.

**Decode model:** Opcodes are grouped into four blocks by bits 7–6:

- **Block 0** — Pattern table in `decode.c`: `NOP`, `INC`/`DEC` r8, `INC`/`DEC` r16, `ADD HL,rr`, `LD rr,d16`, `LD r,d8`, rotates on `A`, `DAA`, `CPL`, `SCF`, `CCF`, plus decode entries for stubbed `LD (rr),A`, `JR`, `STOP`, etc.
- **Block 1** — `LD r8,r8` (operands in `decoded_instr`); `HALT` (`0x76`) decoded but not executed.
- **Block 2** — `ADD`/`ADC`/`SUB`/`SBC`/`AND`/`XOR`/`OR`/`CP` with `A` and r8 operand index in `ops`.
- **Block 3** — `decode_opcode` returns false; not executed yet.

### Memory (MMU)

| File | Role |
|------|------|
| `include/mmu.h` | Defines the 64 KiB `memory` layout as a union: flat `raw[65536]` or named regions (ROM banks, VRAM, external RAM, work RAM, OAM, I/O, high RAM, IE). Address constants mirror the Game Boy map. Read/write helpers. |
| `src/mmu.c` | `init_memory` zeroes RAM. `read_memory8/16`, `write_memory8/16`, and `access_memory8` are direct index operations on `raw[]` with no banking, mirroring, or access rules yet. |

### Picture processing (PPU)

| File | Role |
|------|------|
| `include/ppu.h` | Declares `render(mem, frame_buffer, width, height)`. |
| `src/ppu.c` | Decodes 2bpp tiles from VRAM, reads background tile map at `0x9800`, applies scroll via `SCX`/`SCY` (`0xFF43`/`0xFF42`) and LCDC bit 4 for tile addressing mode. `render()` draws background, then calls empty `render_window` and `render_sprites`. Colors are a fixed four-shade grayscale, not BGP/OBP registers. |

### Debug

| File | Role |
|------|------|
| `include/debug.h` | `debug_assert` macro; failed asserts call `bad_assert` and `exit(1)`. |
| `src/debug.c` | Prints assertion location and terminates. |

### Tests

| File | Role |
|------|------|
| `tests/cpu_test_util.h` | Shared helpers: flag masks, `cpu_test_reset`, `cpu_test_run`, `assert_flags`. |
| `tests/test_cpu.c` | Legacy integration tests (regression suite). |
| `tests/test_cpu_block0.c` | Block 0 instruction tests (loads, inc/dec, rotates, flags). |
| `tests/test_cpu_block1.c` | `LD r8,r8` and `(HL)` memory tests. |
| `tests/test_cpu_block2.c` | ALU / `CP` tests with flag checks per gbz80. |
| `tests/test_decode.c` | Unit tests for `decode_opcode` (no CPU execution). |
| `tests/test_cpu_main.c` | Unity runner (`main`) for all test translation units. |
| `vendor/unity/` | Third-party Unity test framework (`unity.c`, `unity.h`, `unity_internals.h`). |

### Other

| File | Role |
|------|------|
| `README.md` | Short build/run/test instructions and link to gbdev pandocs. |
| `.gitignore` | Build artifacts, CMake cruft, binaries. |

---

## How the pieces connect

1. **Tests:** `cpu_test_reset(&cpu, mem, code)` → repeated `fetch_execute(&cpu)` (decode → execute → `pc += bytes`) → assertions on registers, flags, and `pc`. Opcodes are read from `code`, not from `mem->raw` unless `(HL)` is used.

2. **Main (current):** `init_memory` → `create_virtual_cpu(&cpu, &mem, NULL)` → SDL loop calls `render(&mem, frame_buffer, …)` only. With `code == NULL`, the CPU would not be safe to execute from `fetch_execute` as written.

3. **PPU:** Reads `mem->video_ram` and I/O bytes in `mem->raw` (e.g. `LCDC` at `0xFF40`) directly; it does not simulate LCD state machine timing or vblank interrupts.

---

## Game Boy subsystem status

Legend: **Implemented** — usable for intended purpose in-tree. **In progress** — partial code, stubs, or known gaps. **Not implemented** — absent or no meaningful code.

### CPU (SM83)

| Area | Status | Notes |
|------|--------|--------|
| Register file | **Implemented** | `virtual_cpu` matches expected layout; F register low nibble should be 0 on hardware (not enforced). |
| Block 0 (partial) | **In progress** | Many opcodes work via pattern table; `LD (rr),A`, `LD A,(rr)`, `LD (nn),SP`, `JR`, conditional `JR`, `STOP` are stubs (`printf` / TODO). |
| Block 1 (`LD`) | **In progress** | Generic `LD r8,r8` works; `HALT` not implemented. |
| Block 2 (ALU) | **Implemented** | Add/adc/sub/sbc/and/xor/or/cp with gbz80 flag rules; covered by unit tests. |
| Block 3 | **Not implemented** | Jumps, calls, returns, `RST`, immediate ALU, etc. |
| Decode / execute split | **Implemented** | `decode.c` + `execute_instruction`; PC advanced once in `cpu.c`. |
| Cycles / timing | **Not implemented** | `decoded_instr.cycles` populated in decode but unused; no M-cycle timing in the main loop. |
| Interrupts (CPU side) | **Not implemented** | No IME, no servicing of IF/IE. |
| `fetch_execute` integration | **In progress** | Works in tests with `code` buffer; not wired into `main`; uses `code` not cartridge bus. |

### Memory / cartridge

| Area | Status | Notes |
|------|--------|--------|
| Flat 64 KiB array | **Implemented** | Named regions in the struct for clarity. |
| ROM load from file | **Not implemented** | `make run ROM=…` is defined but `main` does not load a ROM. |
| MBC / bank switching | **Not implemented** | TODO in `mmu.c`. |
| Echo RAM (`0xE000`–`0xFDFF`) | **Not implemented** | Mirroring to work RAM; region is `echo_ram_ignore` in the struct. |
| Prohibited addresses | **Not implemented** | No asserts on `0xFEA0`–`0xFEFF`. |
| DMA | **Not implemented** | No OAM or HDMA. |

### PPU (graphics)

| Area | Status | Notes |
|------|--------|--------|
| Tile decode (2bpp) | **Implemented** | `populate_tiles` from VRAM. |
| Background | **In progress** | Scroll and tile map; fixed palette; no LCD disable/enable behavior. |
| Window layer | **Not implemented** | `render_window` is empty. |
| Sprites (OAM) | **Not implemented** | `render_sprites` is empty. |
| LCD modes / `LY` / STAT | **Not implemented** | No line timing or PPU state machine. |
| Palettes (BGP, OBP0/1) | **Not implemented** | Hardcoded `get_color` values. |

### Audio (APU)

| Area | Status | Notes |
|------|--------|--------|
| Square, wave, noise channels | **Not implemented** | No APU source files. |
| Sound mixing / output | **Not implemented** | SDL audio not used. |

### Timers

| Area | Status | Notes |
|------|--------|--------|
| DIV / TIMA / TMA / TAC | **Not implemented** | I/O space exists in the map but no timer logic. |

### Input

| Area | Status | Notes |
|------|--------|--------|
| Joypad (`0xFF00`) | **Not implemented** | No keyboard → joypad mapping. |

### Serial, other I/O

| Area | Status | Notes |
|------|--------|--------|
| Serial transfer | **Not implemented** | |
| Other I/O registers | **Not implemented** | Bytes can be stored in `raw` but hardware behavior is not simulated. |

### System integration

| Area | Status | Notes |
|------|--------|--------|
| Main emulation loop | **In progress** | SDL display loop only; CPU step not called. |
| Boot ROM | **Not implemented** | |
| Interrupt controller | **Not implemented** | `INTERRUPT_ENABLE_REGISTER` defined; no dispatch. |
| Save states / debugger UI | **Not implemented** | |

---

## Suggested reading order for new contributors

1. `include/cpu.h` and `include/mmu.h` — data structures.
2. `src/cpu.c` and `src/instructions.c` (block 1 and 2 first—they are simpler than block 0’s table).
3. `tests/test_cpu.c` — how the CPU is tested today.
4. `src/ppu.c` — background path used by `main`.
5. `src/main.c` — current runtime behavior vs. what a full loop would need (fetch CPU → advance timers → render on vblank, etc.).

## Build and test (reference)

```bash
make                  # release binary: ./gameboy
make BUILD=debug      # debug + sanitizers
./test.sh             # CPU unit tests
```

Requirements: SDL2, little-endian host (see `README.md`).
