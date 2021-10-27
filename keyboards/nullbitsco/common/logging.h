#pragma once

#define LERROR 0
#define LINFO  1
#define LDEBUG 2

#define DLEVEL LINFO //0: Error, 1: Info, 2: Debug

#define SERROR "ERROR"
#define SINFO  "INFO "
#define SDEBUG "DEBUG"

#if defined (CONSOLE_ENABLE)
  #define RMK_PRINTF(l, f_, ...) dprintf("%s %s: " f_, l, __func__, ##__VA_ARGS__)
  #define ERR_PRINTF(l, f_, ...) dprintf("%s %s:%d: " f_, l, __func__, __LINE__, ##__VA_ARGS__)
#endif 

#if defined(CONSOLE_ENABLE)
  #define PRINT(...) dprintf(__VA_ARGS__); 
#else 
  #define PRINT(...)
#endif // ALL DLEVELS

#if defined(CONSOLE_ENABLE) && DLEVEL >= 0
  #define ERROR(...) ERR_PRINTF(SERROR, __VA_ARGS__); 
#else 
  #define ERROR(...)
#endif // DLEVEL 0+

#if defined(CONSOLE_ENABLE) && DLEVEL >= 1
  #define INFO(...) RMK_PRINTF(SINFO, __VA_ARGS__); 
#else 
  #define INFO(...)
#endif // DLEVEL 1+

#if defined(CONSOLE_ENABLE) && DLEVEL >= 2
  #define DEBUG(...) RMK_PRINTF(SDEBUG, __VA_ARGS__); 
#else 
  #define DEBUG(...)
#endif // DLEVEL 2+