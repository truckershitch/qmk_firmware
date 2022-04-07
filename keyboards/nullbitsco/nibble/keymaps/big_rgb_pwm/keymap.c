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
#include "big_led.h"

enum layer_names {
  _MA,
  _FN
};

enum custom_keycodes {
  KC_CUST = SAFE_RANGE,
};

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

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [_MA] = LAYOUT_ansi(
               _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,  _______,  _______,  _______, _______,  _______,
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
    _______, _______, _______, _______,                   _______,                   _______, _______, _______, _______,          _______, _______
  ),
  [_FN] = LAYOUT_ansi(
               _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,  _______,  _______,  _______, _______,  _______,
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
    _______, _______, _______, _______,                   _______,                   _______, _______, _______, _______,          _______, _______
  ),

};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  // Send keystrokes to host keyboard, if connected (see readme)
  // process_record_remote_kb(keycode, record);
  switch(keycode) {
    case KC_CUST: //custom macro
      if (record->event.pressed) {
      }
    break;

    case RM_1: //remote macro 1
      if (record->event.pressed) {
      }
    break;

    case RM_2: //remote macro 2
      if (record->event.pressed) {
      }
    break;

    case RM_3: //remote macro 3
      if (record->event.pressed) {
      }
    break;

    case RM_4: //remote macro 4
      if (record->event.pressed) {
      }
    break;

  }
return true;
}

// RGB config, for changing RGB settings on non-VIA firmwares
void change_RGB(bool clockwise) {
    bool shift = get_mods() & MOD_MASK_SHIFT;
    bool alt = get_mods() & MOD_MASK_ALT;
    bool ctrl = get_mods() & MOD_MASK_CTRL;

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

bool encoder_update_user(uint8_t index, bool clockwise) {
  if (layer_state_is(1)) {
    //change RGB settings
    change_RGB(clockwise);
  }
  else {
    if (clockwise) {
      tap_code(KC_VOLU);
  } else {
      tap_code(KC_VOLD);
    }
  }
    return true;
}

// void matrix_init_user(void) {
//   set_big_LED_rgb_pwm(1,1,1);
// }

void matrix_scan_user(void) {
  matrix_scan_big_LED();
  
  static uint16_t pwm_timer;

  if (timer_elapsed(pwm_timer) > 50) {
    pwm_timer = timer_read();
    if (rgb_lut_idx++ >= RGB_LUT_LEN-1) rgb_lut_idx = 0;
    set_big_LED_rgb_pwm(rgb_lut[rgb_lut_idx][RGB_LUT_R], rgb_lut[rgb_lut_idx][RGB_LUT_G], rgb_lut[rgb_lut_idx][RGB_LUT_B]);
  }
}

// Use Big LED to show status
// bool led_update_user(led_t led_state) {
//     set_big_LED_r_pwm(led_state.caps_lock ? LED_ON : LED_OFF);
//     set_big_LED_g_pwm(led_state.num_lock ? LED_ON : LED_OFF);
//     set_big_LED_b_pwm(led_state.scroll_lock ? LED_ON : LED_OFF);
//     return true;
// }