#ifndef _INSTRUCTIONS_H_
#define _INSTRUCTIONS_H_

#include "cpu.h"
#include "decode.h"

void execute_instruction(virtual_cpu *cpu, const decoded_instr *instr);

#endif
