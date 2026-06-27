#include "debug.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Global debug mode flag — defaults to off; set to true by --debug flag in main */
bool g_debug_mode = false;
bool g_instr_log = false;

static FILE *g_debug_log = NULL;
static char serial_line_buf[256];
static size_t serial_line_len = 0;

void pretty_print_instr(const decoded_instr *instr, memory *mem, uint16_t pc);
void close_debug_log(void);

static void serial_check_line(void)
{
    if (strstr(serial_line_buf, "Passed") != NULL) {
        close_debug_log();
        exit(0);
    }

    if (strstr(serial_line_buf, "Failed") != NULL) {
        close_debug_log();
        exit(1);
    }
}

void init_debug_log(void)
{
    g_debug_log = fopen(DEBUG_LOG_PATH, "w");
    serial_line_len = 0;
}

void close_debug_log(void)
{
    if (g_debug_log != NULL) {
        fclose(g_debug_log);
        g_debug_log = NULL;
    }
}

void debug_log_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fflush(stdout);
}

void instr_log_printf(const char *fmt, ...)
{
    if (g_debug_log == NULL) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(g_debug_log, fmt, args);
    va_end(args);
    fflush(g_debug_log);
}

void debug_log_instruction(uint16_t pc, const decoded_instr *instr, memory *mem)
{
    if (g_debug_log == NULL) {
        return;
    }

    instr_log_printf("%04X: ", pc);
    pretty_print_instr(instr, mem, pc);
    instr_log_printf("\n");
}

void serial_transmit_byte(uint8_t byte)
{
    putchar((char)byte);
    fflush(stdout);

    if (byte == '\n') {
        serial_line_buf[serial_line_len] = '\0';
        serial_check_line();
        serial_line_len = 0;
        return;
    }

    if (byte == '\r') {
        return;
    }

    if (serial_line_len + 1 < sizeof(serial_line_buf)) {
        serial_line_buf[serial_line_len++] = (char)byte;
    }
}

void bad_assert(const char *expr, const char *file, const char *func, int line)
{
    fprintf(stderr, "ASSERTION FAILED %s at %s:%s:%d\n", expr, file, func, line);
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
    if (g_debug_log == NULL) {
        return;
    }

    const instr_operands *ops = &instr->ops;

    switch (instr->id)
    {
    case INSTR_NOP:
        fprintf(g_debug_log, "NOP");
        break;
    case INSTR_INC_R8:
        fprintf(g_debug_log,"INC %s", r8_names[ops->r8]);
        break;
    case INSTR_DEC_R8:
        fprintf(g_debug_log,"DEC %s", r8_names[ops->r8]);
        break;
    case INSTR_INC_R16:
        fprintf(g_debug_log,"INC %s", r16_names[ops->r16]);
        break;
    case INSTR_DEC_R16:
        fprintf(g_debug_log,"DEC %s", r16_names[ops->r16]);
        break;
    case INSTR_ADD_HL_R16:
        fprintf(g_debug_log,"ADD HL,%s", r16_names[ops->r16]);
        break;
    case INSTR_LD_R16_IMM16:
        {
            uint16_t val = read_memory16(mem, pc + 1);
            fprintf(g_debug_log,"LD %s,$%04X", r16_names[ops->r16], val);
        }
        break;
    case INSTR_LD_R8_IMM8:
        {
            uint8_t val = read_memory8(mem, pc + 1);
            fprintf(g_debug_log,"LD %s,$%02X", r8_names[ops->r8], val);
        }
        break;
    case INSTR_LD_R16MEM_A:
        fprintf(g_debug_log,"LD (%s),A", r16mem_names[ops->r16]);
        break;
    case INSTR_LD_A_R16MEM:
        fprintf(g_debug_log,"LD A,(%s)", r16mem_names[ops->r16]);
        break;
    case INSTR_LD_IMM16_SP:
        {
            uint16_t addr = read_memory16(mem, pc + 1);
            fprintf(g_debug_log,"LD ($%04X),SP", addr);
        }
        break;
    case INSTR_RLCA: fprintf(g_debug_log,"RLCA"); break;
    case INSTR_RRCA: fprintf(g_debug_log,"RRCA"); break;
    case INSTR_RLA:  fprintf(g_debug_log,"RLA"); break;
    case INSTR_RRA:  fprintf(g_debug_log,"RRA"); break;
    case INSTR_DAA:  fprintf(g_debug_log,"DAA"); break;
    case INSTR_CPL:  fprintf(g_debug_log,"CPL"); break;
    case INSTR_SCF:  fprintf(g_debug_log,"SCF"); break;
    case INSTR_CCF:  fprintf(g_debug_log,"CCF"); break;

    case INSTR_JR_IMM8:
        {
            int8_t offset = (int8_t)read_memory8(mem, pc + 1);
            uint16_t target = pc + 2 + offset;
            fprintf(g_debug_log,"JR $%04X", target);
        }
        break;
    case INSTR_JR_COND_IMM8:
        {
            int8_t offset = (int8_t)read_memory8(mem, pc + 1);
            uint16_t target = pc + 2 + offset;
            fprintf(g_debug_log,"JR %s,$%04X", cond_names[ops->cond], target);
        }
        break;
    case INSTR_STOP:
        fprintf(g_debug_log,"STOP");
        break;
    case INSTR_LD_R8_R8:
        fprintf(g_debug_log,"LD %s,%s", r8_names[ops->r8_dest], r8_names[ops->r8_src]);
        break;
    case INSTR_HALT:
        fprintf(g_debug_log,"HALT");
        break;

    case INSTR_ADD_A_R8:
    case INSTR_ADC_A_R8:
    case INSTR_SUB_A_R8:
    case INSTR_SBC_A_R8:
    case INSTR_AND_A_R8:
    case INSTR_XOR_A_R8:
    case INSTR_OR_A_R8:
    case INSTR_CP_A_R8:
        fprintf(g_debug_log,"%s A,%s", alu_op_names[ops->alu_op], r8_names[ops->r8]);
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
            fprintf(g_debug_log,"%s A,$%02X", alu_op_names[idx], val);
        }
        break;

    case INSTR_CB_RLC: fprintf(g_debug_log,"RLC %s", r8_names[ops->r8]); break;
    case INSTR_CB_RRC: fprintf(g_debug_log,"RRC %s", r8_names[ops->r8]); break;
    case INSTR_CB_RL:  fprintf(g_debug_log,"RL %s", r8_names[ops->r8]); break;
    case INSTR_CB_RR:  fprintf(g_debug_log,"RR %s", r8_names[ops->r8]); break;
    case INSTR_CB_SLA: fprintf(g_debug_log,"SLA %s", r8_names[ops->r8]); break;
    case INSTR_CB_SRA: fprintf(g_debug_log,"SRA %s", r8_names[ops->r8]); break;
    case INSTR_CB_SWAP: fprintf(g_debug_log,"SWAP %s", r8_names[ops->r8]); break;
    case INSTR_CB_SRL: fprintf(g_debug_log,"SRL %s", r8_names[ops->r8]); break;
    case INSTR_CB_BIT: fprintf(g_debug_log,"BIT %d,%s", ops->bit, r8_names[ops->r8]); break;
    case INSTR_CB_RES: fprintf(g_debug_log,"RES %d,%s", ops->bit, r8_names[ops->r8]); break;
    case INSTR_CB_SET: fprintf(g_debug_log,"SET %d,%s", ops->bit, r8_names[ops->r8]); break;

    case INSTR_RET_NZ: fprintf(g_debug_log,"RET NZ"); break;
    case INSTR_RET_Z:  fprintf(g_debug_log,"RET Z"); break;
    case INSTR_RET_NC: fprintf(g_debug_log,"RET NC"); break;
    case INSTR_RET_C:  fprintf(g_debug_log,"RET C"); break;
    case INSTR_RET:    fprintf(g_debug_log,"RET"); break;
    case INSTR_RETI:   fprintf(g_debug_log,"RETI"); break;

    case INSTR_JP_NZ:
    case INSTR_JP_Z:
    case INSTR_JP_NC:
    case INSTR_JP_C:
        {
            uint16_t target = read_memory16(mem, pc + 1);
            fprintf(g_debug_log,"JP %s,$%04X", cond_names[ops->cond], target);
        }
        break;
    case INSTR_JP:
        {
            uint16_t target = read_memory16(mem, pc + 1);
            fprintf(g_debug_log,"JP $%04X", target);
        }
        break;
    case INSTR_JP_HL:
        fprintf(g_debug_log,"JP HL");
        break;

    case INSTR_CALL_NZ:
    case INSTR_CALL_Z:
    case INSTR_CALL_NC:
    case INSTR_CALL_C:
        {
            uint16_t target = read_memory16(mem, pc + 1);
            fprintf(g_debug_log,"CALL %s,$%04X", cond_names[ops->cond], target);
        }
        break;
    case INSTR_CALL:
        {
            uint16_t target = read_memory16(mem, pc + 1);
            fprintf(g_debug_log,"CALL $%04X", target);
        }
        break;

    case INSTR_RST:
        {
            static const uint16_t rst_addrs[] = {0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38};
            fprintf(g_debug_log,"RST $%02X", (unsigned)rst_addrs[ops->tgt3]);
        }
        break;

    case INSTR_POP:
        fprintf(g_debug_log,"POP %s", r16stk_names[ops->r16stk]);
        break;
    case INSTR_PUSH:
        fprintf(g_debug_log,"PUSH %s", r16stk_names[ops->r16stk]);
        break;

    case INSTR_LDH_C_A:
        fprintf(g_debug_log,"LDH (C),A");
        break;
    case INSTR_LDH_IMM8_A:
        {
            uint8_t addr = read_memory8(mem, pc + 1);
            fprintf(g_debug_log,"LDH ($%02X),A", addr);
        }
        break;
    case INSTR_LD_IMM16_A:
        {
            uint16_t addr = read_memory16(mem, pc + 1);
            fprintf(g_debug_log,"LD ($%04X),A", addr);
        }
        break;
    case INSTR_LDH_A_C:
        fprintf(g_debug_log,"LDH A,(C)");
        break;
    case INSTR_LDH_A_IMM8:
        {
            uint8_t addr = read_memory8(mem, pc + 1);
            fprintf(g_debug_log,"LDH A,($%02X)", addr);
        }
        break;
    case INSTR_LD_A_IMM16:
        {
            uint16_t addr = read_memory16(mem, pc + 1);
            fprintf(g_debug_log,"LD A,($%04X)", addr);
        }
        break;

    case INSTR_ADD_SP_IMM8:
        {
            int8_t val = (int8_t)read_memory8(mem, pc + 1);
            fprintf(g_debug_log,"ADD SP,$%02X", (uint8_t)val);
        }
        break;
    case INSTR_LD_HL_SP_PLUS_IMM8:
        {
            int8_t val = (int8_t)read_memory8(mem, pc + 1);
            fprintf(g_debug_log,"LD HL,SP+$%02X", (uint8_t)val);
        }
        break;
    case INSTR_LD_SP_HL:
        fprintf(g_debug_log,"LD SP,HL");
        break;

    case INSTR_DI:
        fprintf(g_debug_log,"DI");
        break;
    case INSTR_EI:
        fprintf(g_debug_log,"EI");
        break;

    default:
        fprintf(g_debug_log,"??? (%d)", (int)instr->id);
        break;
    }
}