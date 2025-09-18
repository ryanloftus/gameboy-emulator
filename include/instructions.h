#ifndef _INSTRUCTIONS_H_
#define _INSTRUCTIONS_H_

#include <stdint.h>
#include <stddef.h>

#include "cpu.h"

typedef struct
{
    uint8_t bitmask;
    uint8_t pattern;
    uint8_t cycles;
    uint8_t bytes;
    void (*exec)(virtual_cpu *, uint8_t opcode);
} Instruction;

extern const Instruction block_zero_instructions[];
extern const size_t block_zero_instructions_count;

#endif
