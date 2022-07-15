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

/* Optional big LED pins */
#if defined(__AVR__)
#define BIG_LED_R_PIN D7
#define BIG_LED_G_PIN C6
#define BIG_LED_B_PIN D0
#else
#define BIG_LED_R_PIN GP6
#define BIG_LED_G_PIN GP5
#define BIG_LED_B_PIN GP3
#endif

#define LED_ON          2
#define LED_OFF         0

#define GPIO_STATE_LOW  0
#define GPIO_STATE_HIGH 1

typedef struct {
  uint8_t r : 4;
  uint8_t g : 4;
  uint8_t b : 4;
  uint8_t r_pwm : 4;
  uint8_t b_pwm : 4;
  uint8_t g_pwm : 4;
} rgb_led;

void
  set_big_LED_rgb(uint8_t r_mode, uint8_t g_mode, uint8_t b_mode),
  set_big_LED_r(uint8_t mode),
  set_big_LED_g(uint8_t mode),
  set_big_LED_b(uint8_t mode);

void
  matrix_scan_big_LED(void),
  set_big_LED_rgb_pwm(uint8_t r_pwm, uint8_t g_pwm, uint8_t b_pwm),
  set_big_LED_r_pwm(uint8_t pwm),
  set_big_LED_g_pwm(uint8_t pwm),
  set_big_LED_b_pwm(uint8_t pwm);
