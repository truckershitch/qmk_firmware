/*
Copyright 2021 Jay Greco

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H

// place overrides here
void keyboard_post_init_kb(void) {
    #ifdef CONSOLE_ENABLE
    // debug_enable = true;
    debug_matrix = true;
    #endif
    keyboard_post_init_user();
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    // If console is enabled, it will print the matrix position and status of each key pressed
    #ifdef CONSOLE_ENABLE
    dprintf("kc: 0x%04X, col: %u, row: %u, pressed: %c, time: %u\n", keycode, record->event.key.col, record->event.key.row, record->event.pressed ? '1' : '0', record->event.time);
    #endif

    if (!process_record_user(keycode, record)) return false;

    return true;
}
