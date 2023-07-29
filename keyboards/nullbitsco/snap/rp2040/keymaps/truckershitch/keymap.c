/* Copyright 2021 Jay Greco
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H
#include "oled_graphics.h"
#include "common/remote_kb.h"

// clang-format off
enum layers {
    _BASE = 0,
    _FUNC
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_ansi(
             KC_ESC,   KC_F1,   KC_F2,  KC_F3,  KC_F4,  KC_F5,  KC_F6,    KC_F7,  KC_F8,  KC_F9,   KC_F10,    KC_F11,   KC_F12,   KC_PSCR,  KC_DEL,
    KC_F13,  KC_GRV,   KC_1,    KC_2,   KC_3,   KC_4,   KC_5,   KC_6,     KC_7,   KC_8,   KC_9,    KC_0,      KC_MINS,  KC_EQL,        KC_BSPC,       KC_HOME,
    KC_F14,  KC_TAB,   KC_Q,    KC_W,   KC_E,   KC_R,           KC_T,     KC_Y,   KC_U,   KC_I,    KC_O,      KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,  KC_END,
    KC_F15,  KC_CAPS,  KC_A,    KC_S,   KC_D,   KC_F,           KC_G,     KC_H,   KC_J,   KC_K,    KC_L,      KC_SCLN,  KC_QUOT,  KC_ENT,             KC_PGUP,
    KC_F16,       KC_LSFT,      KC_Z,   KC_X,   KC_C,   KC_V,   KC_B,     KC_N,   KC_M,   KC_COMM, KC_DOT,    KC_SLSH,  KC_RSFT,            KC_UP,    KC_PGDN,
    KC_F17,  KC_LCTL,  KC_LGUI, KC_LALT,        MO(_FUNC),     KC_SPC,   KC_BSPC,                  KC_RALT,   KC_RGUI,  KC_RCTL,  KC_LEFT,  KC_DOWN,  KC_RGHT
  ),
    [_FUNC] = LAYOUT_ansi(
             QK_BOOT,  KC_NO,   KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,    KC_NO,  KC_NO,  KC_NO,   KC_NO,     KC_NO,    KC_NO,    KC_NO,    KC_INS,
    RGB_TOG, KC_NO,    KC_NO,   KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,    KC_NO,  KC_NO,  KC_NO,   KC_NO,     KC_NO,    KC_NO,         KC_NO,         KC_NO,
    KC_NO,   KC_NO,    KC_NO,   KC_NO,  KC_NO,  KC_NO,          KC_NO,    KC_NO,  KC_NO,  KC_NO,   KC_NO,     KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,
    KC_NO,   KC_NO,    KC_NO,   KC_NO,  KC_NO,  KC_NO,          KC_NO,    KC_NO,  KC_NO,  KC_NO,   KC_NO,     KC_NO,    KC_NO,    KC_NO,              KC_NO,
    KC_NO,        KC_NO,        KC_NO,  KC_NO,  KC_NO,  KC_NO,  KC_NO,    KC_NO,  KC_NO,  KC_NO,   KC_NO,     KC_NO,    KC_NO,              KC_NO,    KC_NO,
    KC_NO,   KC_NO,    KC_NO,   KC_NO,          KC_NO,          KC_NO,    KC_NO,                   KC_NO,     KC_NO,    KC_NO,    KC_MPRV,  KC_MPLY,  KC_MNXT
  ),
};
// clang-format on

// RGB config, for changing RGB settings on non-VIA firmware
#ifdef RGBLIGHT_ENABLE
void change_RGB(bool clockwise) {
    bool shift = get_mods() & MOD_MASK_SHIFT;
    bool alt   = get_mods() & MOD_MASK_ALT;
    bool ctrl  = get_mods() & MOD_MASK_CTRL;

#ifdef CONSOLE_ENABLE
    dprintf("Mods: %u\n", get_mods());
#endif

    if (clockwise) {
        if (alt) {
            rgblight_increase_hue();
        } else if (ctrl) {
            rgblight_increase_val();
        } else if (shift) {
            rgblight_increase_sat();
        } else {
            rgblight_step();
        }

    } else {
        if (alt) {
            rgblight_decrease_hue();
        } else if (ctrl) {
            rgblight_decrease_val();
        } else if (shift) {
            rgblight_decrease_sat();
        } else {
            rgblight_step_reverse();
        }
    }
}
#endif


#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [_BASE] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT) },
    [_FUNC] = { ENCODER_CCW_CW(RGB_RMOD, RGB_MOD), ENCODER_CCW_CW(KC_MPRV, KC_MNXT) },
};
#endif

// Decompress and write a precompressed bitmap frame to the OLED.
// Documentation and python compression script available at:
// https://github.com/nullbitsco/squeez-o
#ifdef USE_OLED_BITMAP_COMPRESSION
static void oled_write_compressed_P(const char* input_block_map, const char* input_block_list) {
    uint16_t block_index = 0;
    for (uint16_t i = 0; i < NUM_OLED_BYTES; i++) {
        uint8_t bit          = i % 8;
        uint8_t map_index    = i / 8;
        uint8_t _block_map   = (uint8_t)pgm_read_byte_near(input_block_map + map_index);
        uint8_t nonzero_byte = (_block_map & (1 << bit));
        if (nonzero_byte) {
            const char data = (const char)pgm_read_byte_near(input_block_list + block_index++);
            oled_write_raw_byte(data, i);
        } else {
            const char data = (const char)0x00;
            oled_write_raw_byte(data, i);
        }
    }
}
#endif

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    if (is_keyboard_left())
        return OLED_ROTATION_0;
    else
        return OLED_ROTATION_180;
}

static void render_nullbits_logo(void) {
#ifdef USE_OLED_BITMAP_COMPRESSION
    oled_write_compressed_P(nullbits_logo_block_map, nullbits_logo_bmp);
#else
    oled_write_raw_P(nullbits_logo, sizeof(nullbits_logo));
#endif
}

static void render_status(void) {
    oled_set_cursor(0, 0);
    oled_write_P(PSTR("SNAP75 "), false);
    oled_write_P(PSTR("Layer "), false);
    switch (get_highest_layer(layer_state)) {
        case _FUNC:
            oled_write_P(PSTR("FN1 "), false);
            break;
        default: // use BASE case as default
            oled_write_P(PSTR("Base"), false);
    }

    // Host Keyboard LED Status
    oled_set_cursor(0, 1);
    static uint8_t persistent_led_state = 0;
    uint8_t led_usb_state = host_keyboard_leds();

    // Only update if the LED state has changed
    // Otherwise, the OLED will not turn off if an LED is on.
    if (persistent_led_state != led_usb_state) {
        persistent_led_state = led_usb_state;

        oled_write_ln_P(PSTR(""), false);

        if (IS_LED_ON(led_usb_state, USB_LED_CAPS_LOCK)) {
            oled_set_cursor(0, 1);
            oled_write_P(PSTR("CAPS"), false);
        }

        if (IS_LED_ON(led_usb_state, USB_LED_NUM_LOCK)) {
            oled_set_cursor(5, 1);
            oled_write_P(PSTR("NUM"), true);
        }

        if (IS_LED_ON(led_usb_state, USB_LED_SCROLL_LOCK)) {
            oled_set_cursor(9, 1);
            oled_write_P(PSTR("SCR"), false);
        }
    }

    // WPM and max WPM
    oled_set_cursor(0, 2);
    oled_write_P(PSTR("WPM "), false);
    uint8_t current_wpm = get_current_wpm();
    oled_write(get_u8_str(current_wpm, '0'), true);

    oled_set_cursor(8, 2);
    oled_write_P(PSTR("MAX "), false);
    static uint8_t max_wpm;
    max_wpm = MAX(max_wpm, current_wpm);
    oled_write(get_u8_str(max_wpm, '0'), true);
}

bool oled_task_user(void) {
    if (is_keyboard_master()) {
        render_status();
    } else {
        render_nullbits_logo();
    }
    return true;
}
