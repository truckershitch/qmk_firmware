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
#include "nb_logging.h"
#include <string.h>

typedef struct remote_kb_config {
  bool connected : 1;             // Set after getting a HS packet
  bool host : 1;                  // Set after reading VBUS (or defines)
  uint8_t protocol_ver : 6;       // Counterparty's protocol ver
  uint16_t handshake_timer;       // Timer for throttling HS packets
  uint16_t status_timer;          // Timer for printing status
  uint32_t rgb_state;             // Current RGB state
} remote_kb_config;

typedef struct message_header_t {
  uint8_t message_type : 4;       // remote_kb_message_type
  uint8_t message_length : 4;     // Message payload length
} message_header_t;

typedef struct handshake_data_t {
  uint8_t hs_protocol_ver : 7;    // Protocol version
  uint8_t hs_message_sender : 1;  // Message sender (HOST or REMOTE)
} handshake_data_t;

typedef struct key_event_data_t {
  uint16_t keycode;               // Keycode
  uint8_t pressed;                // Pressed or not
} key_event_data_t;

typedef struct rgb_data_t {
  uint32_t rgb_state;             // RGB state
} rgb_data_t;

typedef struct handshake_message_t {
  message_header_t header;        // Message header
  handshake_data_t data;          // Handshake data
} handshake_message_t;

typedef struct key_event_message_t {
  message_header_t header;        // Message header
  key_event_data_t data;          // Key event data
} key_event_message_t;

typedef struct rgb_event_message_t {
  message_header_t header;        // Message header
  rgb_data_t data;                // RGB data
} rgb_event_message_t;

#define REMOTE_KB_PROTOCOL_VER 2
#define SERIAL_UART_BAUD 153600 // Low error rate for 32u4 @ 16MHz

#define MSG_SENDER_HOST 1
#define MSG_SENDER_REMOTE 0
#define PRESSED true
#define RELEASED false

// Protocol V2
#define MSG_PREAMBLE_V2 0x68
#define RMKB_MSG_BUFFSIZE 8
#define HANDSHAKE_TIMEOUT_MS 1000
#define DEFAULT_TAP_DELAY 3

// Message types
enum remote_kb_message_type {
  MSG_BASE = 0,              // Message 0 is always reserved
  MSG_HANDSHAKE,             // Handshake message
  MSG_KEY_EVENT,             // Key event message
  MSG_RGB_SYNC,              // RGB sync message
  NUM_MSGS,                  // Not a message
};

// Message indexes
enum remote_kb_message_idx {
  IDX_MESSAGE_PREAMBLE = 0,  // Message preamble
  IDX_MESSAGE_TYPE_LENGTH,   // Message type & length
  IDX_MESSAGE_PAYLOAD,       // Message payload
  NUM_IDX,                   // Not an index
};

// Message size in bytes (data only, not header)
enum remote_kb_message_length {
  MSG_LEN_HEADER = 2,         // Message header length
  MSG_LEN_HS = 1,             // Handshake message length
  MSG_LEN_KEY_EVENT = 3,      // Key event message length
  MSG_LEN_RGB_EVENT = 4,      // RGB sync message length
  MSG_LEN_REMOTE_KB_MSG = 4,  // Remote keyboard message length
  MSG_LEN_CHECKSUM = 1,       // Message checksum length
};

// Protocol V1
#define MSG_PREAMBLE_V1 0x69

// Message indexes
enum remote_kb_v1_message_idx {
  IDX_PREAMBLE_V1 = 0,        // Message preamble
  IDX_KCLSB,                  // Keycode LSB
  IDX_KCMSB,                  // Keycode MSB
  IDX_PRESSED,                // Pressed or not 
  IDX_CHECKSUM,               // Checksum
  MSG_LEN_V1,                 // Not an index
};

// -------------- Public functions -------------- //
// V2
void
  tap_code_remote_kb(uint16_t keycode),
  register_code_remote_kb(uint16_t keycode),
  unregister_code_remote_kb(uint16_t keycode);

// V1
void
  matrix_init_remote_kb(void),
  process_record_remote_kb(uint16_t keycode, keyrecord_t *record),
  matrix_scan_remote_kb(void);