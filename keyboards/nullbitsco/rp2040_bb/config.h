/*
Copyright 2015 Jun Wako <wakojun@gmail.com>

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

#pragma once

#include "config_common.h"

/* key matrix size */
#define MATRIX_COLS 2
#define MATRIX_ROWS 3

/* COL2ROW, ROW2COL */
#define DIODE_DIRECTION COL2ROW

/* matrix pin definitions */
#define MATRIX_COL_PINS { GP4, GP5 }
#define MATRIX_ROW_PINS { GP6, GP7, GP8 }

/* RP2040 definitions */
#define RP2040_FLASH_W25X10CL
#define RMKB_SIO_DRIVER SIOD0
#define RMKB_SIO_TX_PIN GP0
#define RMKB_SIO_RX_PIN GP1

/* Optional SMT LED pins */
#define RGB_DI_PIN GP19
#define RGBLED_NUM 8
#define RGBLIGHT_DEFAULT_VAL 64
#define RGBLIGHT_DEFAULT_MODE 10
#define RGBLIGHT_EFFECT_BREATHING
#define RGBLIGHT_EFFECT_RAINBOW_MOOD
#define RGBLIGHT_EFFECT_RAINBOW_SWIRL
#define RGBLIGHT_EFFECT_SNAKE
#define RGBLIGHT_EFFECT_KNIGHT
#define RGBLIGHT_EFFECT_CHRISTMAS
#define RGBLIGHT_EFFECT_STATIC_GRADIENT
#define RGBLIGHT_EFFECT_RGB_TEST
#define RGBLIGHT_EFFECT_ALTERNATING
#define RGBLIGHT_EFFECT_TWINKLE
#define RGBLIGHT_SLEEP

/* Optional encoder pins */
#define ENCODERS_PAD_A { GP23 }
#define ENCODERS_PAD_B { GP20 }

/* Optional I2C pins (needed for OLED) */
#define I2C1_SDA_PIN GP2
#define I2C1_SCL_PIN GP3
#define I2C_DRIVER I2CD2

/* Debug configs */
// #define DEBUG_MATRIX_SCAN_RATE
// #define KEYBOARD_REMOTE
