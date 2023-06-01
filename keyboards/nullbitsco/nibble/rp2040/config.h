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

/*
 * Keyboard Matrix Assignments
 * The nibble uses a demultiplexer for the cols.
 * to free up more IOs for awesomeness!
 * See matrix.c for more details.
*/
#define MATRIX_ROW_PINS { GP22, GP20, GP23, GP21, GP4 }
#define MATRIX_COL_MUX_PINS { GP29, GP28, GP27, GP26 }
#define MATRIX_COL_PINS { }

/* RP2040-specific defines*/
#define RP2040_FLASH_W25X10CL
#define I2C1_SDA_PIN GP2
#define I2C1_SCL_PIN GP3
#define I2C_DRIVER I2CD1
#define RMKB_SIO_DRIVER SIOD0
#define RMKB_SIO_TX_PIN GP0
#define RMKB_SIO_RX_PIN GP1
