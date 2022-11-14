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

void init_pwm(void) {
    static uint8_t init_complete = 0;
    
    // Only configure HW once
    if (!init_complete) {
        static PWMConfig pwmCFG = {
            .frequency = PWM_PWM_COUNTER_FREQUENCY,
            .period    = PWM_PWM_PERIOD,
        };

        // Set GPIO modes
        palSetPadMode(PAL_PORT(PIN_LED_R), PAL_PAD(PIN_LED_R), PWM_PAL_MODE);
        palSetPadMode(PAL_PORT(PIN_LED_G), PAL_PAD(PIN_LED_G), PWM_PAL_MODE);
        palSetPadMode(PAL_PORT(PIN_LED_B), PAL_PAD(PIN_LED_B), PWM_PAL_MODE);

        // Set pwm configuration
        pwmCFG.channels[0].mode = PWM_OUTPUT_ACTIVE_LOW;
        pwmCFG.channels[1].mode = PWM_OUTPUT_ACTIVE_LOW;

        // Start PWM
        pwmStart(&PWM_R_DRIVER, &pwmCFG);
        pwmStart(&PWM_B_DRIVER, &pwmCFG);
        pwmStart(&PWM_G_DRIVER, &pwmCFG);

        // Set all channels to off after init
        pwmEnableChannel(&PWM_R_DRIVER, PWM_R_CHANNEL, PWM_PERCENTAGE_TO_WIDTH(&PWM_R_DRIVER, 0));
        pwmEnableChannel(&PWM_B_DRIVER, PWM_G_CHANNEL, PWM_PERCENTAGE_TO_WIDTH(&PWM_B_DRIVER, 0));
        pwmEnableChannel(&PWM_G_DRIVER, PWM_B_CHANNEL, PWM_PERCENTAGE_TO_WIDTH(&PWM_G_DRIVER, 0));
        
        init_complete = 1;
    }
}

void set_scramble_LED_rgb_pwm(uint16_t r_pwm, uint16_t g_pwm, uint16_t b_pwm) {
    set_scramble_LED_r_pwm(r_pwm);
    set_scramble_LED_g_pwm(g_pwm);
    set_scramble_LED_b_pwm(b_pwm);
}

void set_scramble_LED_r_pwm(uint16_t pwm) {
    if (pwm > 100) pwm = 100;
    pwm *= 100;

    init_pwm();
    pwmEnableChannel(&PWM_R_DRIVER, PWM_R_CHANNEL, PWM_PERCENTAGE_TO_WIDTH(&PWM_R_DRIVER, pwm));
}

void set_scramble_LED_g_pwm(uint16_t pwm) {
    if (pwm > 100) pwm = 100;
    pwm *= 100;

    init_pwm();
    pwmEnableChannel(&PWM_G_DRIVER, PWM_G_CHANNEL, PWM_PERCENTAGE_TO_WIDTH(&PWM_G_DRIVER, pwm));
}

void set_scramble_LED_b_pwm(uint16_t pwm) {
    if (pwm > 100) pwm = 100;
    pwm *= 100;

    init_pwm();
    pwmEnableChannel(&PWM_B_DRIVER, PWM_B_CHANNEL, PWM_PERCENTAGE_TO_WIDTH(&PWM_B_DRIVER, pwm));
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