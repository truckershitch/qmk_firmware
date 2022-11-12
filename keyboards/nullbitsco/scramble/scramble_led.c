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
#include "scramble_led.h"

#if defined(MCU_RP)
static scramble_led_rgb_t scramble_led = { 0, 0, 0, 0, 5, 10 };

void set_scramble_LED_r(uint8_t mode) {
    switch(mode) {
        case LED_ON:
            setPinOutput(PIN_LED_R);
            writePin(PIN_LED_R, GPIO_STATE_LOW);
        break;

        case LED_DIM:
            setPinInputLow(PIN_LED_R);
        break;

        case LED_OFF:
            setPinOutput(PIN_LED_R);
            writePin(PIN_LED_R, GPIO_STATE_HIGH);
        break;

        default:
        break;
    }
}
void set_scramble_LED_g(uint8_t mode) {
    switch(mode) {
        case LED_ON:
            setPinOutput(PIN_LED_G);
            writePin(PIN_LED_G, GPIO_STATE_LOW);
        break;

        case LED_DIM:
            setPinInputLow(PIN_LED_G);
        break;

        case LED_OFF:
            setPinOutput(PIN_LED_G);
            writePin(PIN_LED_G, GPIO_STATE_HIGH);
        break;

        default:
        break;
    }
}
void set_scramble_LED_b(uint8_t mode) {
    switch(mode) {
        case LED_ON:
            setPinOutput(PIN_LED_B);
            writePin(PIN_LED_B, GPIO_STATE_LOW);
        break;

        case LED_DIM:
            setPinInputLow(PIN_LED_B);
        break;

        case LED_OFF:
            setPinOutput(PIN_LED_B);
            writePin(PIN_LED_B, GPIO_STATE_HIGH);
        break;

        default:
        break;
    }
}

void set_scramble_LED_rgb(uint8_t r_mode, uint8_t g_mode, uint8_t b_mode) {
    set_scramble_LED_r(r_mode);
    set_scramble_LED_g(g_mode);
    set_scramble_LED_b(b_mode);
}

void set_scramble_LED(uint8_t mode) {
    set_scramble_LED_rgb(mode, mode, mode);
}

inline void matrix_scan_scramble_LED(void) {
    scramble_led.r_pwm++;
    scramble_led.g_pwm++;
    scramble_led.b_pwm++;

    scramble_led.r_pwm < scramble_led.r ? set_scramble_LED_r(LED_ON) : set_scramble_LED_r(LED_OFF);
    scramble_led.g_pwm < scramble_led.g ? set_scramble_LED_g(LED_ON) : set_scramble_LED_g(LED_OFF);
    scramble_led.b_pwm < scramble_led.b ? set_scramble_LED_b(LED_ON) : set_scramble_LED_b(LED_OFF);
}

void set_scramble_LED_rgb_pwm(uint8_t r_pwm, uint8_t g_pwm, uint8_t b_pwm) {
    scramble_led.r = r_pwm;
    scramble_led.g = g_pwm;
    scramble_led.b = b_pwm;
}

void set_scramble_LED_r_pwm(uint8_t pwm) {
    scramble_led.r = pwm;
}

void set_scramble_LED_g_pwm(uint8_t pwm) {
    scramble_led.g = pwm;
}

void set_scramble_LED_b_pwm(uint8_t pwm) {
    scramble_led.b = pwm;
}

#else
void set_scramble_LED(uint8_t mode) {
    switch(mode) {
        case LED_ON:
            setPinOutput(PIN_LED);
            writePin(PIN_LED, GPIO_STATE_HIGH);
        break;

        case LED_DIM:
            setPinInput(PIN_LED);
        break;

        case LED_OFF:
            setPinOutput(PIN_LED);
            writePin(PIN_LED, GPIO_STATE_LOW);
        break;

        default:
        break;
    }
}
#endif // MCU_RP