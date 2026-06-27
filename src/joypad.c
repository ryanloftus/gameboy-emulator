#include "joypad.h"

#include <SDL.h>

static uint8_t map_key_to_button(SDL_Keycode key)
{
    switch (key) {
    case SDLK_UP:    return JOYP_UP;
    case SDLK_DOWN:  return JOYP_DOWN;
    case SDLK_LEFT:  return JOYP_LEFT;
    case SDLK_RIGHT: return JOYP_RIGHT;
    case SDLK_q:     return JOYP_A;
    case SDLK_w:     return JOYP_B;
    case SDLK_e:     return JOYP_START;
    case SDLK_r:     return JOYP_SELECT;
    default:         return 0;
    }
}

static uint8_t joypad_select_nibble(uint8_t select, uint8_t dpad, uint8_t action)
{
    uint8_t nibble;

    if (select == 0x30) {
        /* Neither row selected — all lines read high (released). */
        nibble = 0x0F;
    } else if ((select & 0x20) == 0 && (select & 0x10) == 0) {
        /* Both rows selected — shared column lines. */
        nibble = 0x0F;
        for (int i = 0; i < 4; i++) {
            if ((dpad & (uint8_t)(1u << i)) || (action & (uint8_t)(1u << i))) {
                nibble &= (uint8_t)~(1u << i);
            }
        }
    } else if ((select & 0x20) == 0) {
        /* Bit 5 clear — action row (JOYP_GET_BUTTONS / $10). */
        nibble = (uint8_t)(~action & 0x0F);
    } else if ((select & 0x10) == 0) {
        /* Bit 4 clear — d-pad row (JOYP_GET_CTRL_PAD / $20). */
        nibble = (uint8_t)(~dpad & 0x0F);
    } else {
        nibble = 0x0F;
    }

    return nibble;
}

static uint8_t joypad_current_lines(const memory *mem)
{
    uint8_t select = mem->raw[JOYPAD_REG_ADDR] & 0x30;
    uint8_t dpad = mem->joypad_buttons & 0x0F;
    uint8_t action = (mem->joypad_buttons >> 4) & 0x0F;
    return joypad_select_nibble(select, dpad, action);
}

static void joypad_signal_if_falling_edge(memory *mem, uint8_t old_lines, uint8_t new_lines)
{
    if ((old_lines & (uint8_t)~new_lines) != 0) {
        mem->io_registers[IF_REG_ADDR & 0xFF] |= 0x10;
    }
}

uint8_t joypad_read_p1(const memory *mem)
{
    uint8_t select = mem->raw[JOYPAD_REG_ADDR] & 0x30;
    uint8_t nibble = joypad_current_lines(mem);
    return (uint8_t)(0xC0 | select | nibble);
}

void init_joypad(joypad *joypad, memory *mem)
{
    joypad->mem = mem;
    mem->raw[JOYPAD_REG_ADDR] = 0x30;
    mem->joypad_buttons = 0;
}

void joypad_on_p1_write(memory *mem, uint8_t value)
{
    uint8_t old_lines = joypad_current_lines(mem);
    mem->raw[JOYPAD_REG_ADDR] = value & 0x30;
    uint8_t new_lines = joypad_current_lines(mem);
    joypad_signal_if_falling_edge(mem, old_lines, new_lines);
}

void update_joypad(joypad *joypad, const SDL_Event *event)
{
    if (event->type != SDL_KEYDOWN && event->type != SDL_KEYUP) {
        return;
    }

    uint8_t button = map_key_to_button(event->key.keysym.sym);
    if (button == 0) {
        return;
    }

    memory *mem = joypad->mem;
    uint8_t old_lines = joypad_current_lines(mem);
    uint8_t prev = mem->joypad_buttons;

    if (event->type == SDL_KEYDOWN) {
        mem->joypad_buttons = (uint8_t)(prev | button);
    } else {
        mem->joypad_buttons = (uint8_t)(prev & ~button);
    }

    joypad_signal_if_falling_edge(mem, old_lines, joypad_current_lines(mem));
}
