// Copyright 2022 Jay Greco
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "common/remote_kb.h"

void keyboard_post_init_user(void) {
    debug_enable   = true;
    debug_matrix   = true;
    // debug_keyboard = true;
    // debug_mouse    = true;
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    process_record_remote_kb(keycode, record);
    if (!process_record_user(keycode, record)) return false;
    return true;
}

void matrix_init_kb(void) {
    matrix_init_remote_kb();
    matrix_init_user();
}

void matrix_scan_kb(void) {
    matrix_scan_remote_kb();
    matrix_scan_user();
}
