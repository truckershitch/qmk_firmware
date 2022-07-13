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
#include "bitc_led.h"

#if defined(MCU_RP)
void set_bitc_LED_R(uint8_t mode) {
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
void set_bitc_LED_G(uint8_t mode) {
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
void set_bitc_LED_B(uint8_t mode) {
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

void set_bitc_LED_RGB(uint8_t r_mode, uint8_t g_mode, uint8_t b_mode) {
    set_bitc_LED_R(r_mode);
    set_bitc_LED_G(g_mode);
    set_bitc_LED_B(b_mode);
}

void set_bitc_LED(uint8_t mode) {
    set_bitc_LED_RGB(mode, mode, mode);
}

#else
void set_bitc_LED(uint8_t mode) {
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
