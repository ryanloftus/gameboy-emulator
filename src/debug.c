#include "debug.h"

#include <stdlib.h>
#include <stdio.h>

/* Global debug mode flag — defaults to off; set to true by --debug flag in main */
bool g_debug_mode = false;

void bad_assert(const char *expr, const char *file, const char *func, int line)
{
    printf("ASSERTION FAILED %s at %s:%s:%d\n", expr, file, func, line);
    exit(1);
}

static const char *r8_names[] = { "B", "C", "D", "E", "H", "L", "(HL)", "A" };
static const char *r16_names[] = { "BC", "DE", "HL", "SP" };
static const char *r16mem_names[] = { "BC", "DE", "HL+", "HL-" };
static const char *r16stk_names[] = { "BC", "DE", "HL", "AF" };
static const char *cond_names[] = { "NZ", "Z", "NC", "C" };
static const char *alu_op_names[] = { "ADD", "ADC", "SUB", "SBC", "AND", "XOR", "OR", "CP" };

void pretty_print_instr(const decoded_instr *instr, memory *mem, uint16_t pc)
{
    const instr_operands *ops = &instr->ops;

    switch (instr->id)
    {
    case INSTR_NOP:
        printf("NOP");
        break;
    case INSTR_INC_R8:
        printf("INC %s", r8_names[ops->r8]);
        break;
    case INSTR_DEC_R8:
        printf("DEC %s", r8_names[ops->r8]);
        break;
    case INSTR_INC_R16:
        printf("INC %s", r16_names[ops->r16]);
        break;
    case INSTR_DEC_R16:
        printf("DEC %s", r16_names[ops->r16]);
        break;
    case INSTR_ADD_HL_R16:
        printf("ADD HL,%s", r16_names[ops->r16]);
        break;
    case INSTR_LD_R16_IMM16:
        {
            uint16_t val = read_memory16(mem, pc + 1);
            printf("LD %s,$%04X", r16_names[ops->r16], val);
        }
        break;
    case INSTR_LD_R8_IMM8:
        {
            uint8_t val = read_memory8(mem, pc + 1);
            printf("LD %s,$%02X", r8_names[ops->r8], val);
        }
        break;
    case INSTR_LD_R16MEM_A:
        printf("LD (%s),A", r16mem_names[ops->r16]);
        break;
    case INSTR_LD_A_R16MEM:
        printf("LD A,(%s)", r16mem_names[ops->r16]);
        break;
    case INSTR_LD_IMM16_SP:
        {
            uint16_t addr = read_memory16(mem, pc + 1);
            printf("LD ($%04X),SP", addr);
        }
        break;
    case INSTR_RLCA: printf("RLCA"); break;
    case INSTR_RRCA: printf("RRCA"); break;
    case INSTR_RLA:  printf("RLA"); break;
    case INSTR_RRA:  printf("RRA"); break;
    case INSTR_DAA:  printf("DAA"); break;
    case INSTR_CPL:  printf("CPL"); break;
    case INSTR_SCF:  printf("SCF"); break;
    case INSTR_CCF:  printf("CCF"); break;

    case INSTR_JR_IMM8:
        {
            int8_t offset = (int8_t)read_memory8(mem, pc + 1);
            uint16_t target = pc + 2 + offset;
            printf("JR $%04X", target);
        }
        break;
    case INSTR_JR_COND_IMM8:
        {
            int8_t offset = (int8_t)read_memory8(mem, pc + 1);
            uint16_t target = pc + 2 + offset;
            printf("JR %s,$%04X", cond_names[ops->cond], target);
        }
        break;
    case INSTR_STOP:
        printf("STOP");
        break;
    case INSTR_LD_R8_R8:
        printf("LD %s,%s", r8_names[ops->r8_dest], r8_names[ops->r8_src]);
        break;
    case INSTR_HALT:
        printf("HALT");
        break;

    case INSTR_ADD_A_R8:
    case INSTR_ADC_A_R8:
    case INSTR_SUB_A_R8:
    case INSTR_SBC_A_R8:
    case INSTR_AND_A_R8:
    case INSTR_XOR_A_R8:
    case INSTR_OR_A_R8:
    case INSTR_CP_A_R8:
        printf("%s A,%s", alu_op_names[ops->alu_op], r8_names[ops->r8]);
        break;

    case INSTR_ADD_A_IMM8:
    case INSTR_ADC_A_IMM8:
    case INSTR_SUB_A_IMM8:
    case INSTR_SBC_A_IMM8:
    case INSTR_AND_A_IMM8:
    case INSTR_XOR_A_IMM8:
    case INSTR_OR_A_IMM8:
    case INSTR_CP_A_IMM8:
        {
            uint8_t val = read_memory8(mem, pc + 1);
            /* alu_op is stored in ops struct for ALU imm instructions? No — it's not.
             * We compute it from the instr id itself. */
            static const instr_id alu_imm_ids[] = {
                INSTR_ADD_A_IMM8, INSTR_ADC_A_IMM8, INSTR_SUB_A_IMM8, INSTR_SBC_A_IMM8,
                INSTR_AND_A_IMM8, INSTR_XOR_A_IMM8, INSTR_OR_A_IMM8, INSTR_CP_A_IMM8
            };
            int idx = 0;
            for (int i = 0; i < 8; i++) {
                if (alu_imm_ids[i] == instr->id) { idx = i; break; }
            }
            printf("%s A,$%02X", alu_op_names[idx], val);
        }
        break;

    case INSTR_CB_RLC: printf("RLC %s", r8_names[ops->r8]); break;
    case INSTR_CB_RRC: printf("RRC %s", r8_names[ops->r8]); break;
    case INSTR_CB_RL:  printf("RL %s", r8_names[ops->r8]); break;
    case INSTR_CB_RR:  printf("RR %s", r8_names[ops->r8]); break;
    case INSTR_CB_SLA: printf("SLA %s", r8_names[ops->r8]); break;
    case INSTR_CB_SRA: printf("SRA %s", r8_names[ops->r8]); break;
    case INSTR_CB_SWAP: printf("SWAP %s", r8_names[ops->r8]); break;
    case INSTR_CB_SRL: printf("SRL %s", r8_names[ops->r8]); break;
    case INSTR_CB_BIT: printf("BIT %d,%s", ops->bit, r8_names[ops->r8]); break;
    case INSTR_CB_RES: printf("RES %d,%s", ops->bit, r8_names[ops->r8]); break;
    case INSTR_CB_SET: printf("SET %d,%s", ops->bit, r8_names[ops->r8]); break;

    case INSTR_RET_NZ: printf("RET NZ"); break;
    case INSTR_RET_Z:  printf("RET Z"); break;
    case INSTR_RET_NC: printf("RET NC"); break;
    case INSTR_RET_C:  printf("RET C"); break;
    case INSTR_RET:    printf("RET"); break;
    case INSTR_RETI:   printf("RETI"); break;

    case INSTR_JP_NZ:
    case INSTR_JP_Z:
    case INSTR_JP_NC:
    case INSTR_JP_C:
        {
            uint16_t target = read_memory16(mem, pc + 1);
            printf("JP %s,$%04X", cond_names[ops->cond], target);
        }
        break;
    case INSTR_JP:
        {
            uint16_t target = read_memory16(mem, pc + 1);
            printf("JP $%04X", target);
        }
        break;
    case INSTR_JP_HL:
        printf("JP HL");
        break;

    case INSTR_CALL_NZ:
    case INSTR_CALL_Z:
    case INSTR_CALL_NC:
    case INSTR_CALL_C:
        {
            uint16_t target = read_memory16(mem, pc + 1);
            printf("CALL %s,$%04X", cond_names[ops->cond], target);
        }
        break;
    case INSTR_CALL:
        {
            uint16_t target = read_memory16(mem, pc + 1);
            printf("CALL $%04X", target);
        }
        break;

    case INSTR_RST:
        {
            static const uint16_t rst_addrs[] = {0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38};
            printf("RST $%02X", (unsigned)rst_addrs[ops->tgt3]);
        }
        break;

    case INSTR_POP:
        printf("POP %s", r16stk_names[ops->r16stk]);
        break;
    case INSTR_PUSH:
        printf("PUSH %s", r16stk_names[ops->r16stk]);
        break;

    case INSTR_LDH_C_A:
        printf("LDH (C),A");
        break;
    case INSTR_LDH_IMM8_A:
        {
            uint8_t addr = read_memory8(mem, pc + 1);
            printf("LDH ($%02X),A", addr);
        }
        break;
    case INSTR_LD_IMM16_A:
        {
            uint16_t addr = read_memory16(mem, pc + 1);
            printf("LD ($%04X),A", addr);
        }
        break;
    case INSTR_LDH_A_C:
        printf("LDH A,(C)");
        break;
    case INSTR_LDH_A_IMM8:
        {
            uint8_t addr = read_memory8(mem, pc + 1);
            printf("LDH A,($%02X)", addr);
        }
        break;
    case INSTR_LD_A_IMM16:
        {
            uint16_t addr = read_memory16(mem, pc + 1);
            printf("LD A,($%04X)", addr);
        }
        break;

    case INSTR_ADD_SP_IMM8:
        {
            int8_t val = (int8_t)read_memory8(mem, pc + 1);
            printf("ADD SP,$%02X", (uint8_t)val);
        }
        break;
    case INSTR_LD_HL_SP_PLUS_IMM8:
        {
            int8_t val = (int8_t)read_memory8(mem, pc + 1);
            printf("LD HL,SP+$%02X", (uint8_t)val);
        }
        break;
    case INSTR_LD_SP_HL:
        printf("LD SP,HL");
        break;

    case INSTR_DI:
        printf("DI");
        break;
    case INSTR_EI:
        printf("EI");
        break;

    default:
        printf("??? (%d)", (int)instr->id);
        break;
    }
}