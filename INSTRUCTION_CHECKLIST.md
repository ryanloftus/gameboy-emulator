# SM83 instruction implementation checklist

Tracking progress against the [Pan Docs CPU instruction set](https://gbdev.io/pandocs/CPU_Instruction_Set.html).

**Legend**

| Mark | Meaning |
|------|---------|
| `[x]` | Implemented and dispatched (handler runs for matching opcodes) |
| `[~]` | Partial: stub, missing from decode table, or known correctness gaps |
| `[ ]` | Not implemented |

Parameterized families share one handler per row on Pan Docs; sub-bullets list operand variants (one checkbox = entire family done when the generic handler works).

**Last reviewed:** codebase state before decode/execute refactor.

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

- [~] `jr imm8` — handler stubbed, not in block-0 table
- [~] `jr cond, imm8` — nz, z, nc, c (4); handler stubbed, not in block-0 table

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

**Known gaps:** half-carry flag incomplete; subtraction flag not always cleared before non-sub ops.

---

## Block 3

Opcodes with bits 7–6 = `11`. Entire block currently unimplemented (`execute_block_three_instruction` stub).

### 8-bit ALU with immediate

- [ ] `add a, imm8`
- [ ] `adc a, imm8`
- [ ] `sub a, imm8`
- [ ] `sbc a, imm8`
- [ ] `and a, imm8`
- [ ] `xor a, imm8`
- [ ] `or a, imm8`
- [ ] `cp a, imm8`

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

- [ ] Prefix `0xCB` — enters CB instruction table (see below)

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

- [ ] Behavior defined for: `$D3`, `$DB`, `$DD`, `$E3`, `$E4`, `$EB`, `$EC`, `$ED`, `$F4`, `$FC`, `$FD`

---

## $CB prefix instructions

Fetched after opcode `0xCB`; second byte selects operation. None implemented.

### Rotates and shifts

- [ ] `rlc r8` — b, c, d, e, h, l, [hl], a (8)
- [ ] `rrc r8` — (8)
- [ ] `rl r8` — (8)
- [ ] `rr r8` — (8)
- [ ] `sla r8` — (8)
- [ ] `sra r8` — (8)
- [ ] `swap r8` — (8)
- [ ] `srl r8` — (8)

### Bit test / reset / set

- [ ] `bit b3, r8` — 8 bit indices × 8 registers (64)
- [ ] `res b3, r8` — (64)
- [ ] `set b3, r8` — (64)

---

## Summary

| Section | Families | Done | Partial | Not started |
|---------|----------|------|---------|-------------|
| Block 0 | 22 | 19 | 3 | 0 |
| Block 1 | 2 | 1 | 0 | 1 |
| Block 2 | 8 | 8 | 0 | 0 |
| Block 3 | 28 | 0 | 0 | 28 |
| CB prefix | 11 | 0 | 0 | 11 |
| **Total (families)** | **71** | **28** | **3** | **40** |

*Partial block-0 items need decode-table entries and real handlers, not just stub functions.*

---

## References

- [CPU Instruction Set — Pan Docs](https://gbdev.io/pandocs/CPU_Instruction_Set.html)
- [Opcode tables (optables)](https://gbdev.io/gb-opcodes/optables)
- [gbz80(7) — per-instruction descriptions](https://rgbds.gbdev.io/docs/gbz80.7)
