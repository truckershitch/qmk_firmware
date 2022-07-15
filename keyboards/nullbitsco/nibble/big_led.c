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
#include "big_led.h"

static rgb_led big_led = { 0, 0, 0, 0, 5, 10 };

void set_big_LED_rgb(uint8_t r_mode, uint8_t g_mode, uint8_t b_mode) {
    set_big_LED_r(r_mode);
    set_big_LED_g(g_mode);
    set_big_LED_b(b_mode);
}

void set_big_LED_r(uint8_t mode) {
    switch(mode) {
        case LED_ON:
            setPinOutput(BIG_LED_R_PIN);
            writePin(BIG_LED_R_PIN, GPIO_STATE_HIGH);
        break;

        case LED_OFF:
            setPinOutput(BIG_LED_R_PIN);
            writePin(BIG_LED_R_PIN, GPIO_STATE_LOW);
        break;

        default:
        break;
    }
}

void set_big_LED_g(uint8_t mode) {
    switch(mode) {
        case LED_ON:
            setPinOutput(BIG_LED_G_PIN);
            writePin(BIG_LED_G_PIN, GPIO_STATE_HIGH);
        break;

        case LED_OFF:
            setPinOutput(BIG_LED_G_PIN);
            writePin(BIG_LED_G_PIN, GPIO_STATE_LOW);
        break;

        default:
        break;
    }
}

void set_big_LED_b(uint8_t mode) {
    switch(mode) {
        case LED_ON:
            setPinOutput(BIG_LED_B_PIN);
            writePin(BIG_LED_B_PIN, GPIO_STATE_HIGH);
        break;

        case LED_OFF:
            setPinOutput(BIG_LED_B_PIN);
            writePin(BIG_LED_B_PIN, GPIO_STATE_LOW);
        break;

        default:
        break;
    }
}

inline void matrix_scan_big_LED(void) {
    big_led.r_pwm++;
    big_led.g_pwm++;
    big_led.b_pwm++;

    big_led.r_pwm < big_led.r ? set_big_LED_r(LED_ON) : set_big_LED_r(LED_OFF);
    big_led.g_pwm < big_led.g ? set_big_LED_g(LED_ON) : set_big_LED_g(LED_OFF);
    big_led.b_pwm < big_led.b ? set_big_LED_b(LED_ON) : set_big_LED_b(LED_OFF);
}

void set_big_LED_rgb_pwm(uint8_t r_pwm, uint8_t g_pwm, uint8_t b_pwm) {
    big_led.r = r_pwm;
    big_led.g = g_pwm;
    big_led.b = b_pwm;
}

void set_big_LED_r_pwm(uint8_t pwm) {
    big_led.r = pwm;
}

void set_big_LED_g_pwm(uint8_t pwm) {
    big_led.g = pwm;
}

void set_big_LED_b_pwm(uint8_t pwm) {
    big_led.b = pwm;
}
