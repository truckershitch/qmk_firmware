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

/* USB Device descriptor parameter */
#define VENDOR_ID       0x6E61
#define PRODUCT_ID      0x6069
#define DEVICE_VER      0x0001
#define MANUFACTURER    nullbits
#define PRODUCT         RP2040_BB_SPLIT

/* key matrix size */
#define MATRIX_COLS 2
#define MATRIX_ROWS 6

/* COL2ROW, ROW2COL */
#define DIODE_DIRECTION COL2ROW

/* matrix pin definitions */
#define MATRIX_COL_PINS { GP4, GP5 }
#define MATRIX_ROW_PINS { GP6, GP7, GP8 }

#define RP2040_FLASH_AT25SF128A
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED GP24
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 500U

/* Optional SMT LED pins */
#define RGB_DI_PIN GP19
#define RGBLED_NUM 16
#define RGBLED_SPLIT { 8, 8 }
#define RGBLIGHT_DEFAULT_VAL 64
#define RGBLIGHT_DEFAULT_MODE 10
#define RGBLIGHT_ANIMATIONS 

/* Optional encoder pins */
#define ENCODERS_PAD_A { GP23 }
#define ENCODERS_PAD_B { GP20 }

/* Optional I2C pins (needed for OLED) */
#define I2C1_SDA_PIN GP2
#define I2C1_SCL_PIN GP3
#define I2C_DRIVER I2CD2

/* Split configs */
#define	SPLIT_HAND_PIN_LOW_IS_LEFT
#define SPLIT_HAND_PIN GP9
#define SOFT_SERIAL_PIN GP26
#define SERIAL_PIO_USE_PIO1

/* Debug configs */
#define DEBUG_MATRIX_SCAN_RATE
#define TRANSIENT_EEPROM_SIZE 256
