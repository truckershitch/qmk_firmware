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
  {15,0,0},
  {15,1,0},
  {15,2,0},
  {15,3,0},
  {15,4,0},
  {15,5,0},
  {15,6,0},
  {15,7,0},
  {15,8,0},
  {15,9,0},
  {15,10,0},
  {15,11,0},
  {15,12,0},
  {15,13,0},
  {15,14,0},
  {15,15,0},
  {14,15,0},
  {13,15,0},
  {12,15,0},
  {11,15,0},
  {10,15,0},
  {9,15,0},
  {8,15,0},
  {7,15,0},
  {6,15,0},
  {5,15,0},
  {4,15,0},
  {3,15,0},
  {2,15,0},
  {1,15,0},
  {0,15,0},
  {0,15,1},
  {0,15,2},
  {0,15,3},
  {0,15,4},
  {0,15,5},
  {0,15,6},
  {0,15,7},
  {0,15,8},
  {0,15,9},
  {0,15,10},
  {0,15,11},
  {0,15,12},
  {0,15,13},
  {0,15,14},
  {0,15,15},
  {0,14,15},
  {0,13,15},
  {0,12,15},
  {0,11,15},
  {0,10,15},
  {0,9,15},
  {0,8,15},
  {0,7,15},
  {0,6,15},
  {0,5,15},
  {0,4,15},
  {0,3,15},
  {0,2,15},
  {0,1,15},
  {0,0,15},
  {1,0,15},
  {2,0,15},
  {3,0,15},
  {4,0,15},
  {5,0,15},
  {6,0,15},
  {7,0,15},
  {8,0,15},
  {9,0,15},
  {10,0,15},
  {11,0,15},
  {12,0,15},
  {13,0,15},
  {14,0,15},
  {15,0,15},
  {15,0,14},
  {15,0,13},
  {15,0,12},
  {15,0,11},
  {15,0,10},
  {15,0,9},
  {15,0,8},
  {15,0,7},
  {15,0,6},
  {15,0,5},
  {15,0,4},
  {15,0,3},
  {15,0,2},
  {15,0,1}
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

void matrix_scan_user(void) {
    matrix_scan_scramble_LED();

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
