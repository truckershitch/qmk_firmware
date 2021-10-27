
#pragma once

#define KEYBOARD_HOST // Force host mode

#ifdef MATRIX_ROW_PINS
    #undef MATRIX_ROW_PINS
    #define MATRIX_ROW_PINS { D4, C6, D7, F4, B4 }
#endif

#ifdef MATRIX_COL_MUX_PINS
    #undef MATRIX_COL_MUX_PINS
    #define MATRIX_COL_MUX_PINS { F7, F6, F5, F0 }
#endif

#ifdef RGB_DI_PIN
    #undef RGB_DI_PIN
    #define RGB_DI_PIN B2
#endif

#ifdef RGBLED_NUM
    #undef RGBLED_NUM
    #define RGBLED_NUM 5
#endif

#ifdef ENCODERS_PAD_A
    #undef ENCODERS_PAD_A
    #define ENCODERS_PAD_A { B3 }
#endif

#ifdef ENCODERS_PAD_B
    #undef ENCODERS_PAD_B
    #define ENCODERS_PAD_B { B1 }
#endif
