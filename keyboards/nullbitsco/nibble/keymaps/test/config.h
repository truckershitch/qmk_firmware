
#pragma once

#ifdef MATRIX_ROW_PINS
    #undef MATRIX_ROW_PINS
#endif

#ifdef MATRIX_COL_MUX_PINS
    #undef MATRIX_COL_MUX_PINS
#endif

#define MATRIX_ROW_PINS { D4, C6, D7, F4, B4 }
#define MATRIX_COL_MUX_PINS { F7, F6, F5, F0 }