#pragma once

#include_next <mcuconf.h>

#undef RP_I2C_USE_I2C1
#define RP_I2C_USE_I2C1 TRUE

#define RP_PWM_USE_PWM1 TRUE
#define RP_PWM_USE_PWM2 TRUE
// #define RP_PWM_IRQ_WRAP_NUMBER_PRIORITY     3 //wtf???
