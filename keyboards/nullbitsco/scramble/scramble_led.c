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

void set_scramble_LED(uint8_t mode) {
    switch(mode) {
        case LED_ON:
            set_scramble_LED_rgb_pwm(65, 100, 95);
        break;

        case LED_DIM:
            set_scramble_LED_rgb_pwm(3, 9, 3);
        break;

        default: //LED_OFF
            set_scramble_LED_rgb_pwm(0, 0, 0);
        break;
    }
}

// SCRAMBLE LED configuration
pwm_led_t scramble_rgb = {
    {GP18, GP19, GP20},
    {&PWMD1, &PWMD1, &PWMD2},
    {0, 1, 0},
    PWM_OUTPUT_ACTIVE_LOW,
    0
};

// Internal PWM init (only runs once per PWM)
void _init_pwm(pwm_led_t* led) {
    // Only configure HW once
    if (!led->init_complete) {
        static PWMConfig pwmCFG = {
            .frequency = PWM_PWM_COUNTER_FREQUENCY,
            .period    = PWM_PWM_PERIOD,
        };

        // Set GPIO modes
        palSetPadMode(PAL_PORT(led->pin[RED]), PAL_PAD(led->pin[RED]), PWM_PAL_MODE);
        palSetPadMode(PAL_PORT(led->pin[GREEN]), PAL_PAD(led->pin[GREEN]), PWM_PAL_MODE);
        palSetPadMode(PAL_PORT(led->pin[BLUE]), PAL_PAD(led->pin[BLUE]), PWM_PAL_MODE);

        // Set pwm configuration
        pwmCFG.channels[0].mode = led->polarity;
        pwmCFG.channels[1].mode = led->polarity;

        // Start PWM
        pwmStart(led->driver[RED], &pwmCFG);
        pwmStart(led->driver[GREEN], &pwmCFG);
        pwmStart(led->driver[BLUE], &pwmCFG);

        // Set all channels to off after init
        pwmEnableChannel(led->driver[RED], led->channel[RED], PWM_PERCENTAGE_TO_WIDTH(led->driver[RED], 0));
        pwmEnableChannel(led->driver[GREEN], led->channel[GREEN], PWM_PERCENTAGE_TO_WIDTH(led->driver[GREEN], 0));
        pwmEnableChannel(led->driver[BLUE], led->channel[BLUE], PWM_PERCENTAGE_TO_WIDTH(led->driver[BLUE], 0));
        
        led->init_complete = 1;
    }
}

// Internal generic PWM setter
void _set_led_pwm(uint16_t pwm, pwm_led_t* led, uint8_t channel) {
    if (pwm > 100) pwm = 100;
    pwm *= 100;

    _init_pwm(led);
    pwmEnableChannel(led->driver[channel], led->channel[channel], PWM_PERCENTAGE_TO_WIDTH(led->driver[channel], pwm));
}

// SCRAMBLE LED setters
void set_scramble_LED_rgb_pwm(uint16_t r_pwm, uint16_t g_pwm, uint16_t b_pwm) {
    _set_led_pwm(r_pwm, &scramble_rgb, RED);
    _set_led_pwm(g_pwm, &scramble_rgb, GREEN);
    _set_led_pwm(b_pwm, &scramble_rgb, BLUE);
}

void set_scramble_LED_r_pwm(uint16_t pwm) {
    _set_led_pwm(pwm, &scramble_rgb, RED);
}

void set_scramble_LED_g_pwm(uint16_t pwm) {
    _set_led_pwm(pwm, &scramble_rgb, GREEN);
}

void set_scramble_LED_b_pwm(uint16_t pwm) {
    _set_led_pwm(pwm, &scramble_rgb, BLUE);
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