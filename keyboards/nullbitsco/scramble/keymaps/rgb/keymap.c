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

#define RGB_LUT_LEN 90
#define RGB_LUT_R 0
#define RGB_LUT_G 1
#define RGB_LUT_B 2
uint8_t rgb_lut_idx = 0;
static const uint8_t rgb_lut[RGB_LUT_LEN][3] = {
    {100,0,0},
    {100,6,0},
    {100,13,0},
    {100,20,0},
    {100,26,0},
    {100,33,0},
    {100,40,0},
    {100,46,0},
    {100,53,0},
    {100,60,0},
    {100,66,0},
    {100,73,0},
    {100,80,0},
    {100,86,0},
    {100,93,0},
    {100,100,0},
    {93,100,0},
    {86,100,0},
    {80,100,0},
    {73,100,0},
    {66,100,0},
    {60,100,0},
    {53,100,0},
    {46,100,0},
    {40,100,0},
    {33,100,0},
    {26,100,0},
    {20,100,0},
    {13,100,0},
    {6,100,0},
    {0,100,0},
    {0,100,6},
    {0,100,13},
    {0,100,20},
    {0,100,26},
    {0,100,33},
    {0,100,40},
    {0,100,46},
    {0,100,53},
    {0,100,60},
    {0,100,66},
    {0,100,73},
    {0,100,80},
    {0,100,86},
    {0,100,93},
    {0,100,100},
    {0,93,100},
    {0,86,100},
    {0,80,100},
    {0,73,100},
    {0,66,100},
    {0,60,100},
    {0,53,100},
    {0,46,100},
    {0,40,100},
    {0,33,100},
    {0,26,100},
    {0,20,100},
    {0,13,100},
    {0,6,100},
    {0,0,100},
    {6,0,100},
    {13,0,100},
    {20,0,100},
    {26,0,100},
    {33,0,100},
    {40,0,100},
    {46,0,100},
    {53,0,100},
    {60,0,100},
    {66,0,100},
    {73,0,100},
    {80,0,100},
    {86,0,100},
    {93,0,100},
    {100,0,100},
    {100,0,93},
    {100,0,86},
    {100,0,80},
    {100,0,73},
    {100,0,66},
    {100,0,60},
    {100,0,53},
    {100,0,46},
    {100,0,40},
    {100,0,33},
    {100,0,26},
    {100,0,20},
    {100,0,13},
    {100,0,6},
};

enum layer_names {
    _BASE,
    _VIA1,
    _VIA2,
    _VIA3
};

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

// bool led_update_user(led_t led_state) {
//     if (led_state.caps_lock) {
//         set_scramble_LED(LED_ON);
//     } else {
//         set_scramble_LED(LED_OFF);
//     }
//     return true;
// }

void matrix_scan_user(void) {
    static uint16_t pwm_timer;

    if (timer_elapsed(pwm_timer) > 50) {
        pwm_timer = timer_read();
        if (rgb_lut_idx++ >= RGB_LUT_LEN-1) rgb_lut_idx = 0;
        set_scramble_LED_rgb_pwm(rgb_lut[rgb_lut_idx][RGB_LUT_R], rgb_lut[rgb_lut_idx][RGB_LUT_G], rgb_lut[rgb_lut_idx][RGB_LUT_B]);
    }
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (clockwise) {
        tap_code(KC_VOLU);
    } else {
        tap_code(KC_VOLD);
    }
    return true;
}
