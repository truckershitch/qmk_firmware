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
#include <stdint.h>
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
                  KC_NUM,  KC_PAUS, KC_PAST, KC_HOME,
                  KC_P7,   KC_P8,   KC_P9,   KC_PPLS,
        KC_MUTE,  KC_P4,   KC_P5,   KC_P6,   _______,
        MO(_FN1), KC_P1,   KC_P2,   KC_P3,   KC_PENT,
        KC_BSPC,  _______, _______, _______, _______,

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

char *coarse_txt[] = {"Jog  1   ", "Jog  5   ", "Jog  10  ", "Jog  20  ", "Jog  50  ", "Jog  100 ", "Jog  200 ", "Jog  500 "};
char *fine_txt[] =   {"Jog  0.01", "Jog  0.02", "Jog  0.05", "Jog  0.10", "Jog  0.20", "Jog  0.50", "Jog  1.0 "};

uint8_t coarse_keys[8] = {KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N}; // coarse movement keys (HID keycode 10 to 17)
uint8_t fine_keys[7]   = {KC_A, KC_B, KC_C, KC_D, KC_E, KC_F, KC_G}; // fine movement keys (HID keycode 4 to 10)

int coarse_max = sizeof(coarse_keys) / sizeof(coarse_keys[0]) - 1; // index of last element in coarse_keys
int fine_max = sizeof(fine_keys) / sizeof(fine_keys[0]) - 1; // index of last element in fine_keys

uint8_t *jog_keys;
char **jog_keys_txt;
int jog_keys_max, jog_prev, jog_next;
int *jog_keys_pos, *jog_other_keys_pos;

bool jog_is_coarse = true;

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (jog_is_coarse) {
        jog_keys = coarse_keys;
        jog_keys_txt = coarse_txt;
        jog_keys_max = coarse_max;
        jog_keys_pos = &coarse_pos;
        jog_other_keys_pos = &fine_pos;
    }
    else { // jog_is_coarse == false
        jog_keys = fine_keys;
        jog_keys_txt = fine_txt;
        jog_keys_max = fine_max;
        jog_keys_pos = &fine_pos;
        jog_other_keys_pos = &coarse_pos;
    }

    if (clockwise) {
        if (*jog_keys_pos == jog_keys_max || *jog_keys_pos == -1) { // far end of list or initial position
            jog_next = 0;
            *jog_keys_pos = 0;
        }
        else {
            jog_next = ++*jog_keys_pos;
        }

        tap_code(*(jog_keys + jog_next));
#ifdef OLED_ENABLE
        oled_write(*(jog_keys_txt + jog_next), false);
#endif
    }
    else { // counterclockwise
        if (*jog_keys_pos <= 0) { // near end of list or initial position
            jog_prev = jog_keys_max;
            *jog_keys_pos = jog_keys_max;
        }
        else {
            jog_prev = --*jog_keys_pos;
        }

        tap_code(*(jog_keys + jog_prev));
#ifdef OLED_ENABLE
        oled_write(*(jog_keys_txt + jog_prev), false);
#endif
    }

    *jog_other_keys_pos = -1; // reset other keys to initial position

    return false;
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_MUTE:
            if (record->event.pressed) {
                // Do something when pressed
            } else {
                // Do something else when release
                jog_is_coarse = !jog_is_coarse; // toggle coarse/fine jogging
            }
            return false; // Skip all further processing of this key
    }
    return true;
};
