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

/*
Level-based logging for debugging and testing.

There are four levels: DEBUG, INFO, and ERROR.
Each log level prints the current function name.
In addition, the message level is printed to the console.

No log printing is enabled unless CONSOLE_ENABLE is defined.
*/

#pragma once

#define LOG_NONE 0
#define LOG_ERROR 1
#define LOG_INFO 2
#define LOG_DEBUG 3

// Set the log level here.
#define LOG_LEVEL LOG_ERROR

#define STR_ERROR "ERROR"
#define STR_INFO "INFO "
#define STR_DEBUG "DEBUG"

#if defined(CONSOLE_ENABLE)
#    define RMK_PRINTF(l, f_, ...) dprintf("%s %s: " f_, l, __func__, ##__VA_ARGS__)
#    define ERR_PRINTF(l, f_, ...) dprintf("%s %s:%d: " f_, l, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if defined(CONSOLE_ENABLE) && LOG_LEVEL >= LOG_NONE
#    define PRINT(...) dprintf(__VA_ARGS__);
#else
#    define PRINT(...)
#endif  // ALL LOG_LEVELS

#if defined(CONSOLE_ENABLE) && LOG_LEVEL >= LOG_ERROR
#    define ERROR(...) ERR_PRINTF(STR_ERROR, __VA_ARGS__);
#else
#    define ERROR(...)
#endif  // LOG_LEVEL LOG_ERROR+

#if defined(CONSOLE_ENABLE) && LOG_LEVEL >= LOG_INFO
#    define INFO(...) RMK_PRINTF(STR_INFO, __VA_ARGS__);
#else
#    define INFO(...)
#endif  // LOG_LEVEL LOG_INFO+

#if defined(CONSOLE_ENABLE) && LOG_LEVEL >= LOG_DEBUG
#    define DEBUG(...) RMK_PRINTF(STR_DEBUG, __VA_ARGS__);
#else
#    define DEBUG(...)
#endif  // LOG_LEVEL LOG_DEBUG+