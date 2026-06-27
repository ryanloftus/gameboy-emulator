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

uint8_t joypad_read_p1(const memory *mem)
{
    uint8_t select = mem->raw[JOYPAD_REG_ADDR] & 0x30;
    uint8_t dpad = mem->joypad_buttons & 0x0F;
    uint8_t action = (mem->joypad_buttons >> 4) & 0x0F;
    uint8_t nibble;

    if (select == 0x30) {
        /* Neither row selected — all buttons read as released. */
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
        /* D-pad row selected. */
        nibble = (uint8_t)(~dpad & 0x0F);
    } else {
        /* Action row selected. */
        nibble = (uint8_t)(~action & 0x0F);
    }

    return (uint8_t)(0xC0 | select | nibble);
}

void init_joypad(joypad *joypad, memory *mem)
{
    joypad->mem = mem;
    mem->raw[JOYPAD_REG_ADDR] = 0x30;
    mem->joypad_buttons = 0;
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
    uint8_t prev = mem->joypad_buttons;

    if (event->type == SDL_KEYDOWN) {
        if ((prev & button) == 0) {
            mem->io_registers[IF_REG_ADDR & 0xFF] |= 0x10;
        }
        mem->joypad_buttons = (uint8_t)(prev | button);
    } else {
        mem->joypad_buttons = (uint8_t)(prev & ~button);
    }
}
