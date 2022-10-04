// Copyright 2022 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// Left side (front)
#define D3 0U
#define D2 1U
//      GND
//      GND
#define D1 2U
#define D0 3U
#define D4 4U
#define C6 5U
#define D7 6U
#define E6 7U
#define B4 8U
#define B5 9U

// Right side (front)
//      RAW
//      GND
//      RESET
//      VCC
#define F4 29U
#define F5 28U
#define F6 27U
#define F7 26U
#define B1 22U
#define B3 20U
#define B2 23U
#define B6 21U

// LEDs (Mapped to R and G channel of the Bit-C PRO's RGB led)
// NOTE: changed for this branch specifically, so that it uses the TX and RX leds instead.
#define D5 14U
#define B0 11U
