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
#include <hal.h>

enum rgb_idx {
    RED = 0,
    GREEN,
    BLUE
};

typedef struct pwm_led_t {
    uint32_t pin[3];
    PWMDriver* driver[3];
    uint8_t channel[3];
    uint8_t polarity;
    uint8_t init_complete;
} pwm_led_t;

// Shared configuration
#define PWM_PAL_MODE (PAL_MODE_ALTERNATE_PWM | PAL_RP_PAD_DRIVE12 | PAL_RP_GPIO_OE)
#define PWM_PWM_COUNTER_FREQUENCY 1000000
#define PWM_PWM_PERIOD PWM_PWM_COUNTER_FREQUENCY / 2048

#else // MCU_AVR
#define PIN_LED B2
#endif

void set_scramble_LED(uint8_t mode);

#if defined(MCU_RP)
// RP2040 adds pwm control!
// PWM values are in percent, 0-100
void
    set_scramble_LED_rgb_pwm(uint16_t r_pwm, uint16_t g_pwm, uint16_t b_pwm),
    set_scramble_LED_r_pwm(uint16_t pwm),
    set_scramble_LED_g_pwm(uint16_t pwm),
    set_scramble_LED_b_pwm(uint16_t pwm);
#endif // MCU_RP
