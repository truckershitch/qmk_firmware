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
Remote keyboard is an experimental feature that allows for connecting another
keyboard, macropad, numpad, or accessory without requiring an additional USB connection.
The "remote keyboard" forwards its keystrokes using UART serial over TRRS. Dynamic VUSB 
detect allows the keyboard automatically switch to host or remote mode depending on
which is connected to the USB port.

Possible functionality includes the ability to send data from the host to the remote using
a reverse link, allowing for LED sync, configuration, and more data sharing between devices.
This will require a new communication protocol, as the current one is limited.
*/

#include "remote_kb.h"
#include "uart.h"

// V2
remote_kb_config rm_config;

//TODO: static void pack_message_type_length(void); ???
static uint8_t chksum8(const unsigned char *buf, size_t len);

static void print_message_buffer_v2(uint8_t *msg_buffer, uint16_t len) {
  PRINT("index     [%03d]: [", len);
  // Print index first
  for (uint16_t i=0; i<len; i++) {
    PRINT(" %03d ", i);
  }
  PRINT("]\n");

  PRINT("msg_buffer[%03d]: [", len);
  // Print index first
  for (uint16_t i=0; i<len; i++) {
    PRINT(" %03X ", msg_buffer[i]);
  }
  PRINT("]\n");
}

// Unpack message from a remote_kb_message to a raw buffer
//TODO: add bounds checking
static bool unpack_remote_kb_message(remote_kb_message msg, uint8_t *msg_buffer, uint16_t max_len) {
  // uint16_t fail_index = 0;
  
  // Set preamble
  msg_buffer[IDX_MESSAGE_PREAMBLE] = RMKB_MSG_PREAMBLE;

  // Set message type and length
  uint8_t tmp = ((msg.message_type & 0xF) << 4) | (msg.message_length & 0xF);
  DEBUG("type: %01X length: %01X\n", msg.message_type, msg.message_length);
  DEBUG("MESSAGE_TYPE_LENGTH: %02X\n", tmp);
  msg_buffer[IDX_MESSAGE_TYPE_LENGTH] = tmp; //todo: clean up?

  // Copy payload from msg into message_buffer, starting at the right offset
  memcpy(&msg_buffer[IDX_MESSAGE_PAYLOAD], msg.message_payload, msg.message_length);

  // Set checksum
  uint16_t IDX_MESSAGE_CHECKSUM = IDX_MESSAGE_PAYLOAD + msg.message_length;
  // if (IDX_MESSAGE_CHECKSUM >= max_len) goto fail;

  uint8_t checksum = chksum8(msg_buffer, IDX_MESSAGE_CHECKSUM);
  DEBUG("IDX_MESSAGE_CHECKSUM: %d\n", IDX_MESSAGE_CHECKSUM);
  DEBUG("checksum: %02X\n", checksum);
  msg_buffer[IDX_MESSAGE_CHECKSUM] = checksum;

  #if DLEVEL >= 2
  print_message_buffer_v2(msg_buffer, max_len);
  #endif
  
  return true;

  // fail:
  // dprintf("buffer error at index: %d!", )
  // return false;
}

static remote_kb_message pack_remote_kb_message(uint8_t *msg_buffer) {
  remote_kb_message msg;
  msg.message_type = msg_buffer[IDX_MESSAGE_TYPE_LENGTH] >> 4;
  msg.message_length = msg_buffer[IDX_MESSAGE_TYPE_LENGTH] & 0xF;
  msg.message_payload = &msg_buffer[IDX_MESSAGE_PAYLOAD];

  DEBUG("message_type: %01X message_length: %01X\n", msg.message_type, msg.message_length);

  return msg;
}

// Send a packed message. Wrap this for each message type.
static void send_msg_v2_core(remote_kb_message msg) {
  // Determine size of message
  uint16_t msg_buffer_size = MSG_LEN_REMOTE_KB_MSG + msg.message_length - 1;
  DEBUG("msg_buffer_size: %d\n", msg_buffer_size);

  uint8_t msg_buffer[msg_buffer_size];
  if (unpack_remote_kb_message(msg, msg_buffer, msg_buffer_size)) {
    DEBUG("uart_putchar:");
    #if DLEVEL >= 2
    for (int i=0; i<msg_buffer_size; i++) {
      PRINT(" %02X", msg_buffer[i]);
    }
    PRINT("\n");
    #endif
    for (int i=0; i<msg_buffer_size; i++) {
      uart_putchar(msg_buffer[i]);
    }
  }
}

static void handle_handshake(handshake_message hs_msg) {
  DEBUG("hs_protocol_ver: %d hs_message_sender: %s\n", hs_msg.hs_protocol_ver, hs_msg.hs_message_sender ? "host" : "remote");
  if ((hs_msg.hs_protocol_ver == REMOTE_KB_PROTOCOL_VER) && (hs_msg.hs_message_sender != rm_config.host)) {
    INFO("Handshake accepted from %s\n", hs_msg.hs_message_sender ? "host" : "remote");
    rm_config.protocol_ver = REMOTE_KB_PROTOCOL_VER;
    rm_config.connected = true;
  }
}

//TODO: buffer bounds check?
static void unpack_handshake_message(handshake_message hs_msg, uint8_t *hs_msg_buffer) {
  DEBUG("hs_msg protocol_ver: %01X sender: %01X\n", hs_msg.hs_protocol_ver, hs_msg.hs_message_sender);
  hs_msg_buffer[0] = (hs_msg.hs_protocol_ver & 0x7F) << 1;
  hs_msg_buffer[0] |= (hs_msg.hs_message_sender & 0x01); 
}

static handshake_message pack_handshake_message(uint8_t *hs_msg_buffer) {
  handshake_message hs_msg;
  hs_msg.hs_protocol_ver = (hs_msg_buffer[0] >> 1) & 0x7F;
  hs_msg.hs_message_sender = (hs_msg_buffer[0] & 0x01);
  DEBUG("hs_msg protocol_ver: %01X sender: %01X\n", hs_msg.hs_protocol_ver, hs_msg.hs_message_sender);
  return hs_msg;
}

// TODO: buffer bounds check?
static void unpack_key_event_message(key_event_message key_msg, uint8_t *key_msg_buffer) {
  DEBUG("key_msg keycode: %02X pressed: %02X\n", key_msg.keycode, key_msg.pressed);
  key_msg_buffer[0] = (key_msg.keycode & 0xFF);
  key_msg_buffer[1] = (key_msg.keycode >> 8) & 0xFF;
  key_msg_buffer[2] = key_msg.pressed;
}

// static key_event_message pack_key_event_message(uint8_t *key_msg_buffer) {
//   key_event_message key_msg;
//   key_msg.keycode = key_msg_buffer[0] | (key_msg_buffer[1] << 8);
//   key_msg.pressed = key_msg_buffer[2];
//   DEBUG("key_msg keycode: %02X pressed: %02X\n", key_msg.keycode, key_msg.pressed);
//   return key_msg;
// }

// TODO: convert payload to raw buffer? worth?
// OR: could convert to pass pointer to struct, and then do the unpacking in
// send_msg_v2_core based on message type
static void send_handshake_core(bool sender) {
  handshake_message hs_msg;
  hs_msg.hs_protocol_ver = REMOTE_KB_PROTOCOL_VER;
  hs_msg.hs_message_sender = sender;

  uint8_t hs_msg_buffer[MSG_LEN_HS] = {0};
  unpack_handshake_message(hs_msg, hs_msg_buffer);

  remote_kb_message rmkb_msg;
  rmkb_msg.message_type = MSG_HANDSHAKE;
  rmkb_msg.message_length = MSG_LEN_HS;
  rmkb_msg.message_payload = hs_msg_buffer;

  send_msg_v2_core(rmkb_msg);
}

static void send_keyevent_msg_v2(key_event_message ke_msg) {
  uint8_t ke_msg_buffer[MSG_LEN_KEY_EVENT] = {0};
  unpack_key_event_message(ke_msg, ke_msg_buffer);

  remote_kb_message rmkb_msg;
  rmkb_msg.message_type = MSG_KEY_EVENT;
  rmkb_msg.message_length = MSG_LEN_KEY_EVENT;
  rmkb_msg.message_payload = ke_msg_buffer;

  send_msg_v2_core(rmkb_msg);
}

//TODO: rename raw msg to include buffer
static void parse_message_v2(uint8_t *raw_msg) {
  uint8_t message_type = raw_msg[IDX_MESSAGE_TYPE_LENGTH] >> 4;
  uint8_t message_length = raw_msg[IDX_MESSAGE_TYPE_LENGTH] & 0xF;
  DEBUG("message_type: %01X message_length: %01X\n", message_type, message_length);

  uint16_t IDX_MESSAGE_CHECKSUM = IDX_MESSAGE_PAYLOAD + message_length;
  uint8_t checksum = chksum8(raw_msg, IDX_MESSAGE_CHECKSUM);
  DEBUG("IDX_MESSAGE_CHECKSUM: %d\n", IDX_MESSAGE_CHECKSUM);
  DEBUG("checksum: %02X\n", checksum);

  // No point in packing if the checksum is wrong.
  if (checksum != raw_msg[IDX_MESSAGE_CHECKSUM]) {
    DEBUG("checksum mismatch!\n");
    return;
  }

  remote_kb_message rmkb_msg = pack_remote_kb_message(raw_msg);

  switch (rmkb_msg.message_type) {
    case MSG_HANDSHAKE: {
      DEBUG("msg type: MSG_HANDSHAKE\n");
      handshake_message hs_msg = pack_handshake_message(rmkb_msg.message_payload);
      handle_handshake(hs_msg);
    } break;
    case MSG_KEY_EVENT:
      DEBUG("msg type: MSG_KEY_EVENT\n");
      break;
    default:
      DEBUG("Unknown message type: %d\n", message_type);
      break;
  }
}

// -------------- Private functions -------------- //

static bool vbus_detect(void) {
  #if defined(__AVR_ATmega32U4__)
    //returns true if VBUS is present, false otherwise.
    USBCON |= (1 << OTGPADE); //enables VBUS pad
    _delay_us(10);
    return (USBSTA & (1<<VBUS));  //checks state of VBUS
  #else
    #error vbus_detect is not implemented for this architecure!
  #endif
}

static uint8_t chksum8(const unsigned char *buf, size_t len) {
  unsigned int sum;
  for (sum = 0 ; len != 0 ; len--)
    sum += *(buf++);
  return (uint8_t)sum;
}

// V1 send message function
static void send_keyevent_msg_v1(key_event_message ke_msg) {
  if (IS_HID_KC(ke_msg.keycode) || IS_RM_KC(ke_msg.keycode)) {
    uint8_t raw_msg[UART_MSG_LEN];

    INFO("Remote: send [%u]\n", ke_msg.keycode);
    raw_msg[IDX_PREAMBLE] = UART_PREAMBLE;
    raw_msg[IDX_KCLSB] = (ke_msg.keycode & 0xFF);
    raw_msg[IDX_KCMSB] = (ke_msg.keycode >> 8) & 0xFF;
    raw_msg[IDX_PRESSED] = ke_msg.pressed;
    raw_msg[IDX_CHECKSUM] = chksum8(raw_msg, UART_MSG_LEN-1);

    for (int i=0; i<UART_MSG_LEN; i++) {
      uart_putchar(raw_msg[i]);
    }
  }
}

static void parse_message_v1(uint8_t *raw_msg) {
  uint8_t chksum = chksum8(raw_msg, UART_MSG_LEN-1);
  if (raw_msg[IDX_PREAMBLE] != UART_PREAMBLE || raw_msg[IDX_CHECKSUM] != chksum) {
     ERROR("UART checksum mismatch!\n");
     print_message_buffer_v2(raw_msg, UART_MSG_LEN);
     ERROR("calc checksum: %02X\n", chksum);
  } else {
    uint16_t keycode = (uint16_t)raw_msg[IDX_KCLSB] | ((uint16_t)raw_msg[IDX_KCMSB] << 8);
    bool pressed = (bool)raw_msg[IDX_PRESSED];
    if (IS_RM_KC(keycode)) {
      keyrecord_t record;
      record.event.pressed = pressed;
      if (pressed) {
        INFO("Remote macro: press [%u]\n", keycode);
        } else {
        INFO("Remote macro: release [%u]\n", keycode);
        }
      process_record_user(keycode, &record);
    } else {
      if (pressed) {
        INFO("Remote: press [%u]\n", keycode);
        register_code(keycode);
    } else {
        INFO("Remote: release [%u]\n", keycode);
        unregister_code(keycode);
      }
    }
  }
}

//TODO: buffer boudns checks!
static void get_remote_kb_message(void) {
  if (!uart_available()) return;
  
  uint16_t start_time = timer_read();
  uint8_t raw_msg[RMKB_MSG_BUFFSIZE] = {0};
  uint16_t timer_val[RMKB_MSG_BUFFSIZE] = {0};
  uint8_t raw_msg_idx = 0;

  //TODO: remove timer code and iteration counting
  static uint8_t iteration = 0;
  DEBUG("i: %d start time: %u\n", iteration++, start_time);

  while (uart_available()) {
    raw_msg[raw_msg_idx] = uart_getchar();
    timer_val[raw_msg_idx] = timer_read();  

    //TODO: this is gross, clean it up (broken anyways)
    if (raw_msg_idx == 0 && (raw_msg[raw_msg_idx] != UART_PREAMBLE && raw_msg[raw_msg_idx] != RMKB_MSG_PREAMBLE )) {
      ERROR("Byte sync error!\n");
      ERROR("start time: %u\n",start_time);
      for (int i=0; i<RMKB_MSG_BUFFSIZE; i++) {
        ERROR("idx: %d, recv: 0x%02X, time: %u\n", i, raw_msg[i], timer_val[i]);
      }
      raw_msg_idx = 0;
    } else {
      raw_msg_idx++;
    }
  }
  for (int i=0; i<RMKB_MSG_BUFFSIZE; i++) {
    DEBUG("idx: %d, recv: 0x%02X, time: %u\n", i, raw_msg[i], timer_val[i]);
  }

  if ((raw_msg[IDX_PREAMBLE] == RMKB_MSG_PREAMBLE) || (rm_config.protocol_ver == 2 && rm_config.connected)) {
    parse_message_v2(raw_msg);
  } else {
    parse_message_v1(raw_msg);
  }

  raw_msg_idx = 0;
}

// TODO: pointless right now
static void handle_incoming_messages(void) {
  get_remote_kb_message();
}

static void remote_kb_send_keycode(key_event_message ke_msg) {
  // Only send if not the host
  if (!rm_config.host) {
    if (rm_config.protocol_ver == 2 && rm_config.connected) {
      send_keyevent_msg_v2(ke_msg);
    } else {
      send_keyevent_msg_v1(ke_msg);
    }
  }
}

static void host_detect(void) {
  rm_config.host = vbus_detect();
  
  #if defined (KEYBOARD_HOST)
  rm_config.host = true;
  #elif defined(KEYBOARD_REMOTE)
  rm_config.host = false;
  #endif
}

static void print_status(void) {
  if (timer_elapsed(rm_config.status_timer) >= STATUS_TIMEOUT_MS) {
    INFO("connected: %s\n", rm_config.connected ? "true" : "false");
    INFO("host: %s\n", rm_config.host ? "true" : "false");
    INFO("protocol_ver: %d\n", rm_config.protocol_ver);
    rm_config.status_timer = timer_read();
  }
}

static void remote_kb_send_handshake(void) {
  if (timer_elapsed(rm_config.handshake_timer) >= HANDSHAKE_TIMEOUT_MS) {
    INFO("Sending RMKB HS\n");
    send_handshake_core(rm_config.host);
    rm_config.handshake_timer = timer_read();
  }
  return;
}

// -------------- Public functions -------------- //

void matrix_init_remote_kb(void) {
  uart_init(SERIAL_UART_BAUD);
  host_detect();
  rm_config.protocol_ver = 1;
}

// Send keystrokes from local to remote
void process_record_remote_kb(uint16_t keycode, keyrecord_t *record) {
  key_event_message ke_msg;
  ke_msg.keycode = keycode;
  ke_msg.pressed = record->event.pressed;
  remote_kb_send_keycode(ke_msg);
}

// Receive keystrokes from remote to local
void matrix_scan_remote_kb(void) {
  print_status(); //DEBUG

  if (!rm_config.connected) {
    remote_kb_send_handshake();
  }

  handle_incoming_messages();
}
