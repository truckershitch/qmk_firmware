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
#pragma once

#include "config_common.h"

/* USB Device descriptor parameter */
#define VENDOR_ID       0x6E61
#define PRODUCT_ID      0x6064
#define DEVICE_VER      0x0001
#define MANUFACTURER    nullbits
#define PRODUCT         TIDBIT

/* key matrix pins */
#define MATRIX_ROW_PINS { GP22, GP7, GP6, GP5, GP4 }
#define MATRIX_COL_PINS { NO_PIN, NO_PIN, GP29, GP28, GP27, GP26 }

/* Optional encoder pins */
#define ENCODERS_PAD_A { GP23, GP8, GP3, GP0 }
#define ENCODERS_PAD_B { GP20, GP9, GP2, GP1 }

/* Optional SMT LED pins */
#define RGB_DI_PIN GP21

/* RP2040-specific defines*/
#define RP2040_FLASH_W25X10CL
#define I2C1_SDA_PIN GP2
#define I2C1_SCL_PIN GP3
#define I2C_DRIVER I2CD2

