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

#include "quantum.h"

#define LED_ON          2
#define LED_DIM         1
#define LED_OFF         0

#define GPIO_STATE_LOW  0
#define GPIO_STATE_HIGH 1

#if defined(MCU_RP)
#define PIN_LED_R       GP18
#define PIN_LED_G       GP19
#define PIN_LED_B       GP20
#else // MCU_AVR
#define PIN_LED         B2
#endif // defined(MCU_RP)

void set_scramble_LED(uint8_t mode);
#if defined(MCU_RP)
void set_scramble_LED_R(uint8_t mode);
void set_scramble_LED_G(uint8_t mode);
void set_scramble_LED_B(uint8_t mode);
void set_scramble_LED_RGB(uint8_t r_mode, uint8_t g_mode, uint8_t b_mode);
#endif // MCU_RP
