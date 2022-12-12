/*
Copyright 2021 Jay Greco

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H

enum layer_names {
    _BASE,
    _VIA1,
    _VIA2,
    _VIA3
};

bool macro_test_mode = false;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

[_BASE] = LAYOUT(
    KC_F1, KC_F2, KC_F3,
    KC_F4, KC_F5, KC_F6
),

[_VIA1] = LAYOUT(
    _______, _______, _______,
    _______, _______, _______
),

[_VIA2] = LAYOUT(
    _______, _______, _______,
    _______, _______, _______
),

[_VIA3] = LAYOUT(
    _______, _______, _______,
    _______, _______, _______
)

};

void matrix_init_user(void) {
    set_scramble_LED(LED_OFF);
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (clockwise) {
        tap_code(KC_VOLU);
    } else {
        tap_code(KC_VOLD);
    }
    return true;
}

#define id_macro_test_mode 0x04
#define id_set_debug_mode  0x05
void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    uint8_t command = data[0];
    uint8_t mode = data[1];
    dprintf("command: %u mode: %u\n", command, mode);
    switch (command) {
        case id_set_keyboard_value:
            if (mode == id_macro_test_mode) {
                dprintf("Setting MACRO test mode!\n");
                macro_test_mode = true;
            }
            if (mode == id_set_debug_mode) {
                debug_enable = true;
                dprintf("Debug mode enabled\n");
                macro_test_mode = true;
            }
        break;

        default:
        break;
    }
}

bool process_record_via_override(uint16_t keycode, keyrecord_t *record) {
    if (!macro_test_mode) return true;

    if (record->event.pressed) {
        if (keycode >= MACRO00 && keycode <= MACRO15) {
            //Convert from 0x5F12 --> 0x3A
            uint8_t id = keycode - MACRO00; 
            dprintf("M%d\n", id);
            tap_code(id + 0x3A);
        }
    }
    return false;
}
