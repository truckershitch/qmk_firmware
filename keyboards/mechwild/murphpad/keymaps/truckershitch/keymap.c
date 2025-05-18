/* Copyright 2021 Kyle McCreery
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

// Defines names for use in layer keycodes and the keymap
enum layer_names {
    _BASE,
    _FN1,
    _FN2,
    _FN3
};


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Base */
    [_BASE] = LAYOUT(
                  KC_F1,   KC_F2,   KC_F3,   KC_F4,
                  KC_NUM,  KC_PAUS, KC_U,    KC_HOME,
                  _______, KC_UP,   KC_G,    KC_PPLS,
        KC_MUTE,  KC_LEFT, KC_5,    KC_RGHT, _______,
        MO(_FN1), _______, KC_DOWN, _______, KC_PENT,
        KC_BSPC,  KC_P0,   _______, KC_PDOT, _______,

                  KC_F5,   KC_F6,   KC_F7

    ),
    [_FN1] = LAYOUT(
                 _______,  _______, _______, _______,
                 _______,  _______, _______, _______,
                 RGB_HUD,  RGB_SPI, RGB_HUI, _______,
        _______, RGB_RMOD, RGB_TOG, RGB_MOD, _______,
        _______, RGB_VAD,  RGB_SPD, RGB_VAI,  KC_F12,
        _______, RGB_SAD,  _______, RGB_SAI, _______,

                 _______, _______, _______

    ),
    [_FN2] = LAYOUT(
                  _______, _______, _______, _______,
                  _______, _______, _______, _______,
                  _______, _______, _______, _______,
        _______,  _______, _______, _______, _______,
        _______,  _______, _______, _______, _______,
        _______,  _______, _______, _______, _______,

                  _______, _______, _______

    ),
    [_FN3] = LAYOUT(
                  _______, _______, _______, _______,
                  _______, _______, _______, _______,
                  _______, _______, _______, _______,
        _______,  _______, _______, _______, _______,
        _______,  _______, _______, _______, _______,
        _______,  _______, _______, _______, _______,

                  _______, _______, _______

    )
};

static int coarse_pos = -1;
static int fine_pos = -1;

bool encoder_update_user(uint8_t index, bool clockwise) {
    char *coarse_txt[] = {"Jog  1   ", "Jog  5   ", "Jog  10  ", "Jog  20  ", "Jog  50  ", "Jog  100 ", "Jog  200 ", "Jog  500 "};
    char *fine_txt[] =   {"Jog  0.01", "Jog  0.02", "Jog  0.05", "Jog  0.10", "Jog  0.20", "Jog  0.50", "Jog  1.0 "};

    uint8_t coarse_keys[8] = {KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N}; // coarse movement keys (HID keycode 10 to 17)
    uint8_t fine_keys[7]   = {KC_A, KC_B, KC_C, KC_D, KC_E, KC_F, KC_G}; // fine movement keys (HID keycode 4 to 10)

    int coarse_len = sizeof(coarse_keys) / sizeof(coarse_keys[0]); // number of elements in coarse_keys
    int fine_len = sizeof(fine_keys) / sizeof(fine_keys[0]); // number of elements in fine_keys

    uint8_t current_layer = get_highest_layer(layer_state);

    uint8_t *jog_keys;
    char **jog_keys_txt;
    int jog_keys_len, jog_prev, jog_next;
    int *jog_keys_pos, *jog_other_keys_pos;

    switch(current_layer) {
    case _BASE:
        jog_keys = coarse_keys;
        jog_keys_txt = coarse_txt;
        jog_keys_len = coarse_len;
        jog_keys_pos = &coarse_pos;
        jog_other_keys_pos = &fine_pos;
        break;

    case _FN1:
        jog_keys = fine_keys;
        jog_keys_txt = fine_txt;
        jog_keys_len = fine_len;
        jog_keys_pos = &fine_pos;
        jog_other_keys_pos = &coarse_pos;
        break;

    default:
        jog_keys = coarse_keys;
        jog_keys_txt = coarse_txt;
        jog_keys_len = coarse_len;
        jog_keys_pos = &coarse_pos;
        jog_other_keys_pos = &fine_pos;
        break;
    }

    if (clockwise) {
        if (*jog_keys_pos == jog_keys_len - 1 || *jog_keys_pos == -1) { // far end of list or initial position
            jog_next = 0;
            *jog_keys_pos = 0;
        }
        else {
            jog_next = *jog_keys_pos + 1;
            *jog_keys_pos = *jog_keys_pos + 1;
        }

        tap_code(*(jog_keys + jog_next));
#ifdef OLED_ENABLE
        oled_write(*(jog_keys_txt + jog_next), false);
#endif
    }
    else { // counterclockwise
        if (*jog_keys_pos <= 0) { // near end of list or initial position
            jog_prev = jog_keys_len - 1;
            *jog_keys_pos = jog_keys_len - 1;
        }
        else {
            jog_prev = *jog_keys_pos - 1;
            *jog_keys_pos = *jog_keys_pos - 1;
        }

        tap_code(*(jog_keys + jog_prev));
#ifdef OLED_ENABLE
        oled_write(*(jog_keys_txt + jog_prev), false);
#endif
    }

    *jog_other_keys_pos = -1; // reset other keys to initial position

    return false;
};

#ifdef OLED_ENABLE
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    case KC_F1:
        if (record->event.pressed) {
            oled_write("Jog  1  ", false);
        }
    case KC_F2:
        if (record->event.pressed) {
            oled_write("Jog  5  ", false);
        }
    case KC_F3:
        if (record->event.pressed) {
            oled_write("Jog  20 ", false);
        }
    case KC_F4:
        if (record->event.pressed) {
            oled_write("Jog  100", false);
        }
    }
    return true;
};
#endif
