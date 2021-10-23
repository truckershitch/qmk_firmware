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
#pragma once

#include "quantum.h"
#include <string.h>

enum remote_kb_message_type {
  MSG_BASE = 0, // Message 0 is always reserved
  MSG_HANDSHAKE,
  MSG_KEY_EVENT,
  MSG_ENCODER_EVENT,
  MSG_LED_SYNC,   // TODO
  MSG_OLED_SYNC,  // TODO
  NUM_MSGS, // Not a message
};

// Message size in bytes
enum remote_kb_message_size {
  MSG_LEN_HS = 1,
  MSG_LEN_KEY_EVENT = 3,
  MSG_LEN_ENC_EVENT = 2,
  MSG_LEN_REMOTE_KB_MSG = 4,
};

//__attribute__((packed))?
typedef struct remote_kb_config {
  bool connected : 1;         // Set after getting a HS packet
  bool host : 1;              // Set after reading VBUS (or defines)
  uint8_t protocol_ver : 6;   // Counterparty's protocol ver
  uint16_t handshake_timer;   // Timer for throttling HS packets
  uint16_t status_timer;      // Timer for printing status
} remote_kb_config;

typedef struct handshake_message {
  uint8_t hs_protocol_ver : 7;    // Protocol version
  uint8_t hs_message_sender : 1;  // Message sender (HOST or REMOTE)
} handshake_message;

typedef struct key_event_message {
  uint16_t keycode;       // Keycode
  uint8_t pressed;        // Pressed or not
} key_event_message;

typedef struct encoder_event_message {
  uint16_t keycode;       // Keycode
} encoder_event_message;

typedef struct remote_kb_message {
  uint8_t preamble;           // Magic number for sync TODO:remove?
  uint8_t message_type : 4;   // remote_kb_message_type
  uint8_t message_length : 4; // Message payload length
  uint8_t* message_payload;   // Pointer to payload buffer
  uint8_t message_checksum;   // 8bit checksum TODO: remove?
} remote_kb_message;

enum remote_kb_message_idx {
  IDX_MESSAGE_PREAMBLE = 0,
  IDX_MESSAGE_TYPE_LENGTH,
  IDX_MESSAGE_PAYLOAD,
  NUM_IDX,                // Not an index
};

#define REMOTE_KB_PROTOCOL_VER 2
#define SERIAL_UART_BAUD 153600 // Low error rate for 32u4 @ 16MHz

#define LERROR 0
#define LINFO  1
#define LDEBUG 2

#define DLEVEL LDEBUG //0: Error, 1: Info, 2: Debug
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

// Protocol V2

#define MSG_SENDER_HOST 1
#define MSG_SENDER_REMOTE 0
#define RMKB_MSG_PREAMBLE 0x68
#define RMKB_MSG_BUFFSIZE 8
#define HANDSHAKE_TIMEOUT_MS 5000
#define STATUS_TIMEOUT_MS 5000

// Protocol V1

#define UART_PREAMBLE 0x69
#define UART_MSG_LEN  5
#define UART_NULL     0

#define IDX_PREAMBLE  0
#define IDX_KCLSB     1
#define IDX_KCMSB     2
#define IDX_PRESSED   3
#define IDX_CHECKSUM  4

#define IS_HID_KC(x) ((x > 0) && (x < 0xFF))
#define IS_RM_KC(x) ((x >= RM_BASE) && (x <= 0xFFFF))


#define RM_BASE 0xFFFF-16
enum remote_macros {
  RM_1 = RM_BASE,
  RM_2,  RM_3,
  RM_4,  RM_5,
  RM_6,  RM_7,
  RM_8,  RM_9,
  RM_10, RM_11,
  RM_12, RM_13,
  RM_14, RM_15,
};


// Public functions
void
 matrix_init_remote_kb(void),
 process_record_remote_kb(uint16_t keycode, keyrecord_t *record),
 matrix_scan_remote_kb(void);