# SM83 instruction implementation checklist

Tracking progress against the [Pan Docs CPU instruction set](https://gbdev.io/pandocs/CPU_Instruction_Set.html).

**Legend**

| Mark | Meaning |
|------|---------|
| `[x]` | Implemented and dispatched (handler runs for matching opcodes) |
| `[~]` | Partial: stub, missing from decode table, or known correctness gaps |
| `[ ]` | Not implemented |

Parameterized families share one handler per row on Pan Docs; sub-bullets list operand variants (one checkbox = entire family done when the generic handler works).

**Last reviewed:** Cycle tracking added — `virtual_cpu.cycles` accumulates per-instruction cycle counts, with conditional branches adding +1 cycle when taken.

---

## Block 0

Opcodes with bits 7–6 = `00`.

### Control

- [x] `nop`

### Loads (16-bit immediate and memory)

- [x] `ld r16, imm16` — bc, de, hl, sp (4)
- [x] `ld [r16mem], a` — bc, de, hl+, hl- (4)
- [x] `ld a, [r16mem]` — bc, de, hl+, hl- (4)
- [x] `ld [imm16], sp`

### 16-bit inc/dec and add

- [x] `inc r16` — bc, de, hl, sp (4)
- [x] `dec r16` — bc, de, hl, sp (4)
- [x] `add hl, r16` — bc, de, hl, sp (4)

### 8-bit inc/dec

- [x] `inc r8` — b, c, d, e, h, l, [hl], a (8)
- [x] `dec r8` — b, c, d, e, h, l, [hl], a (8)

### 8-bit load immediate

- [x] `ld r8, imm8` — b, c, d, e, h, l, [hl], a (8)

### Rotates and flags on A (single encodings)

- [x] `rlca`
- [x] `rrca`
- [x] `rla`
- [x] `rra`
- [x] `daa`
- [x] `cpl`
- [x] `scf`
- [x] `ccf`

### Jumps (relative)

- [x] `jr imm8` — decode/cycles/bytes set in block-0 table; handler reads signed offset and updates PC
- [x] `jr cond, imm8` — nz, z, nc, c (4); handler checks condition flag and jumps if met; adds +1 cycle when taken (3 taken / 2 untaken)

### Power

- [~] `stop` — 2-byte instruction; handler stubbed, not in block-0 table

---

## Block 1: 8-bit register-to-register loads

Opcodes with bits 7–6 = `01`.

- [x] `ld r8, r8` — 63 encodings (8×8 minus `ld [hl],[hl]`)
- [ ] `halt` — opcode `0x76` (`ld [hl], [hl]` encoding); detected but not implemented

---

## Block 2: 8-bit arithmetic (A and r8)

Opcodes with bits 7–6 = `10`.

- [x] `add a, r8` — b, c, d, e, h, l, [hl], a (8)
- [x] `adc a, r8` — (8)
- [x] `sub a, r8` — (8)
- [x] `sbc a, r8` — (8)
- [x] `and a, r8` — (8)
- [x] `xor a, r8` — (8)
- [x] `or a, r8` — (8)
- [x] `cp a, r8` — (8)

**All block 2 ALU operations verified:** flag behavior matches RGBDS `gbz80.7` specification for all 8 operations. DAA subtraction path correctly preserves carry flag.

---

## Block 3

Opcodes with bits 7–6 = `11`. Entire block currently unimplemented (`execute_block_three_instruction` stub).

### 8-bit ALU with immediate

- [x] `add a, imm8`
- [x] `adc a, imm8`
- [x] `sub a, imm8`
- [x] `sbc a, imm8`
- [x] `and a, imm8`
- [x] `xor a, imm8`
- [x] `or a, imm8`
- [x] `cp a, imm8`

### Returns and conditional returns

- [ ] `ret cond` — nz, z, nc, c (4)
- [ ] `ret`
- [ ] `reti`

### Jumps and calls

- [ ] `jp cond, imm16` — nz, z, nc, c (4)
- [ ] `jp imm16`
- [ ] `jp hl`
- [ ] `call cond, imm16` — nz, z, nc, c (4)
- [ ] `call imm16`

### Restart

- [ ] `rst tgt3` — $00, $08, $10, $18, $20, $28, $30, $38 (8)

### Stack (16-bit push/pop)

- [ ] `pop r16stk` — bc, de, hl, af (4)
- [ ] `push r16stk` — bc, de, hl, af (4)

### CB prefix dispatch

- [x] Prefix `0xCB` — enters CB instruction table (see below)

### High RAM and absolute loads

- [ ] `ldh [c], a`
- [ ] `ldh [imm8], a`
- [ ] `ld [imm16], a`
- [ ] `ldh a, [c]`
- [ ] `ldh a, [imm8]`
- [ ] `ld a, [imm16]`

### SP/HL arithmetic

- [ ] `add sp, imm8`
- [ ] `ld hl, sp + imm8`
- [ ] `ld sp, hl`

### Interrupt enable

- [ ] `di`
- [ ] `ei`

### Invalid opcodes (hardware lock-up)

Do not implement as normal instructions; document behavior if encountered:

- [x] Behavior defined for: `$D3`, `$DB`, `$DD`, `$E3`, `$E4`, `$EB`, `$EC`, `$ED`, `$F4`, `$FC`, `$FD`

---

## $CB prefix instructions

Fetched after opcode `0xCB`; second byte selects operation. All rotate/shift families implemented.

### Rotates and shifts

- [x] `rlc r8` — b, c, d, e, h, l, [hl], a (8)
- [x] `rrc r8` — (8)
- [x] `rl r8` — (8)
- [x] `rr r8` — (8)
- [x] `sla r8` — (8)
- [x] `sra r8` — (8)
- [x] `swap r8` — (8)
- [x] `srl r8` — (8)

### Bit test / reset / set

- [ ] `bit b3, r8` — 8 bit indices × 8 registers (64)
- [ ] `res b3, r8` — (64)
- [ ] `set b3, r8` — (64)

---

## CPU timing (cycle tracking)

- [x] `virtual_cpu.cycles` field added — accumulated in `fetch_execute()` after each instruction
- [x] Base cycles per instruction defined in `instr_cycles[]` table in `decode.c`
- [x] Conditional branch (`jr cond, imm8`) adds +1 extra cycle when the branch is taken (3 taken / 2 untaken)
- [ ] Conditional `jp`, `call`, `ret` — need extra cycle handling when implemented
- [ ] `ret`/`reti` — add +1 cycle (4 total per Pan Docs)

---

## Summary

| Section | Families | Done | Partial | Not started |
|---------|----------|------|---------|-------------|
| Block 0 | 22 | 21 | 1 | 0 |
| Block 1 | 2 | 1 | 0 | 1 |
| Block 2 | 8 | 8 | 0 | 0 |
| Block 3 | 28 | 8 | 0 | 20 |
| CB prefix | 11 | 8 | 0 | 3 |
| **Total (families)** | **71** | **46** | **1** | **24** |

*Partial block-0 items need decode-table entries and real handlers, not just stub functions. `jr imm8` and `jr cond, imm8` are both fully implemented.*

---

## References

- [CPU Instruction Set — Pan Docs](https://gbdev.io/pandocs/CPU_Instruction_Set.html)
- [Opcode tables (optables)](https://gbdev.io/gb-opcodes/optables)
- [gbz80(7) — per-instruction descriptions](https://rgbds.gbdev.io/docs/gbz80.7)