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

Additional functionality includes the ability to send data from the host to the remote using
a reverse link, allowing for LED sync, configuration, and more data sharing between devices.
This is implemented in the V2 protocol.

TODO: Add RGB animation sync for the remote keyboard.
TODO: Add periodic handshake to ensure the remote keyboard is still connected.
*/

#include "remote_kb.h"
#include "uart.h"

// V2
remote_kb_config rm_config;

static uint8_t checksum8(const unsigned char *buf, size_t len);
static void send_key_event(key_event_data_t ke_data);
static void send_rgb(void);
static void send_handshake(void);

// -------------- Private functions -------------- //

static void print_message_buffer(uint8_t *msg_buffer, uint16_t len) {
  PRINT("index     [%03d]: [", len);
  for (uint16_t i=0; i<len; i++) {
    PRINT(" %03d ", i);
  }
  PRINT("]\n");

  PRINT("msg_buffer[%03d]: [", len);
  for (uint16_t i=0; i<len; i++) {
    PRINT(" %03X ", msg_buffer[i]);
  }
  PRINT("]\n");
}

static void remote_kb_print_status(void) {
  #if LOG_LEVEL >= LOG_INFO
  PRINT("---------------- RMKB STATUS ----------------\n");
  INFO("connected: %s\n", rm_config.connected ? "true" : "false");
  INFO("host: %s\n", rm_config.host ? "true" : "false");
  INFO("protocol_ver: %d\n", rm_config.protocol_ver);
  INFO("rgb_state: %08lX\n", rm_config.rgb_state);
  PRINT("---------------------------------------------\n");
  #endif
}


// ---------- Final activity handlers ----------- //
// Handle handshake messages
static void handle_handshake(handshake_data_t hs_data) {
  DEBUG("hs_protocol_ver: %d hs_message_sender: %s\n", hs_data.hs_protocol_ver, hs_data.hs_message_sender ? "host" : "remote");
  if ((hs_data.hs_protocol_ver == REMOTE_KB_PROTOCOL_VER) && (hs_data.hs_message_sender != rm_config.host)) {
    INFO("accepted from %s\n", hs_data.hs_message_sender ? "host" : "remote");
    rm_config.protocol_ver = REMOTE_KB_PROTOCOL_VER;
    rm_config.connected = true;
  }
  if (rm_config.connected) {
      // Only send a handshake back if the message is from the host
      if (!rm_config.host) {
        wait_ms(1);
        send_handshake();
      } else {
        // Send the initial RGB configuration on response from remote
        wait_ms(1);
        send_rgb();
      }
  }
  remote_kb_print_status();
}

// Handle key event messages
static void handle_key_event(key_event_data_t key_event_data) {
  if (IS_ANY(key_event_data.keycode)) {
    if (key_event_data.pressed) {
      INFO("remote: press [0x%02X]\n", key_event_data.keycode);
      register_code(key_event_data.keycode);
    } else {
      INFO("remote: release [0x%02X]\n", key_event_data.keycode);
      unregister_code(key_event_data.keycode);
    }
  }
}

// Handle RGB event messages
static void handle_rgb(rgb_data_t rgb_data) {
  // Only update if not host
  if (rm_config.host) return;
  if (rgb_data.rgb_state != rm_config.rgb_state) {
    rm_config.rgb_state = rgb_data.rgb_state;
    rgblight_update_dword(rgb_data.rgb_state);
      INFO("updated from host\n");
  }
}

// TODO: kill these
// -------------- Message packing --------------- //

 // Key event message packing
static key_event_data_t pack_key_event_message(uint8_t *key_event_msg_buffer) {
  key_event_data_t key_event_data;
  key_event_data.keycode = (uint16_t)key_event_msg_buffer[0];
  key_event_data.keycode |= ((uint16_t)key_event_msg_buffer[1] << 8);
  key_event_data.pressed = key_event_msg_buffer[2];
  DEBUG("key_event_data keycode: %04X pressed: %01X\n", key_event_data.keycode, key_event_data.pressed);
  return key_event_data;
}

// RGB event message packing
static rgb_data_t pack_rgb_message(uint8_t *rgb_msg_buffer) {
  rgb_data_t rgb_data;
  rgb_data.rgb_state = (uint32_t)rgb_msg_buffer[0];
  rgb_data.rgb_state |= ((uint32_t)rgb_msg_buffer[1] << 8);
  rgb_data.rgb_state |= ((uint32_t)rgb_msg_buffer[2] << 16);
  rgb_data.rgb_state |= ((uint32_t)rgb_msg_buffer[3] << 24);
  DEBUG("rgb_data rgb_state: %08lX\n", rgb_data.rgb_state);
  return rgb_data;
}

// ------------ Core event senders -------------- //
// Checksum calculation
static uint8_t checksum8(const unsigned char *buf, size_t len) {
  unsigned int sum;
  for (sum = 0 ; len != 0 ; len--)
    sum += *(buf++);
  return (uint8_t)sum;
}

// Uart send
static inline void uart_putmsg(uint8_t *msg_buffer, uint8_t message_size) {
  #if LOG_LEVEL >= LOG_DEBUG
    DEBUG("uart_putmsg:\n");
    print_message_buffer(ke_msg_buffer, message_size);
  #endif

  for (int i=0; i<message_size; i++) {
    uart_putchar(msg_buffer[i]);
  }
}

// V1 send message function
static inline void send_keyevent_msg_v1(key_event_data_t ke_data) {
  if (IS_ANY(ke_data.keycode)) {
    uint8_t raw_msg_buf[MSG_LEN_V1];

    INFO("remote: send [%u]\n", ke_data.keycode);
    raw_msg_buf[IDX_PREAMBLE_V1] = MSG_PREAMBLE_V1;
    raw_msg_buf[IDX_KCLSB] = (ke_data.keycode & 0xFF);
    raw_msg_buf[IDX_KCMSB] = (ke_data.keycode >> 8) & 0xFF;
    raw_msg_buf[IDX_PRESSED] = ke_data.pressed;
    raw_msg_buf[IDX_CHECKSUM] = checksum8(raw_msg_buf, MSG_LEN_V1-1);

    for (int i=0; i<MSG_LEN_V1; i++) {
      uart_putchar(raw_msg_buf[i]);
    }
  }
}

// TODO: why do it this way? why not use the same send function for 
// every message and just pass the type/length (or some encapsulation thereof)
// Just add the message header to the message before it's sent.
// The header size never changes, only what's in it.
// Send key event message
static inline void send_key_event_core(key_event_message_t ke_msg) {
  const uint8_t message_size = MSG_LEN_HEADER + MSG_LEN_KEY_EVENT + MSG_LEN_CHECKSUM;

  uint8_t ke_msg_buffer[message_size];
  ke_msg_buffer[0] = MSG_PREAMBLE_V2;
  ke_msg_buffer[1] = (ke_msg.header.message_type & 0xF) << 4;
  ke_msg_buffer[1] |= (ke_msg.header.message_length & 0xF);

  ke_msg_buffer[2] = (ke_msg.data.keycode & 0xFF);
  ke_msg_buffer[3] = (ke_msg.data.keycode >> 8) & 0xFF;
  ke_msg_buffer[4] = ke_msg.data.pressed;
  ke_msg_buffer[5] = checksum8(ke_msg_buffer, message_size-1);

  uart_putmsg(ke_msg_buffer, message_size);
}

// Send RGB event message
static inline void send_rgb_core(rgb_event_message_t rgb_msg) {
  const uint8_t message_size = MSG_LEN_HEADER + MSG_LEN_RGB_EVENT + MSG_LEN_CHECKSUM;

  uint8_t rgb_msg_buffer[message_size];
  rgb_msg_buffer[0] = MSG_PREAMBLE_V2;
  rgb_msg_buffer[1] = (rgb_msg.header.message_type & 0xF) << 4;
  rgb_msg_buffer[1] |= (rgb_msg.header.message_length & 0xF);

  rgb_msg_buffer[2] = (rgb_msg.data.rgb_state & 0xFF);        //LSB
  rgb_msg_buffer[3] = (rgb_msg.data.rgb_state >> 8) & 0xFF;
  rgb_msg_buffer[4] = (rgb_msg.data.rgb_state >> 16) & 0xFF;
  rgb_msg_buffer[5] = (rgb_msg.data.rgb_state >> 24) & 0xFF;  //MSB
  rgb_msg_buffer[6] = checksum8(rgb_msg_buffer, message_size-1);

  uart_putmsg(rgb_msg_buffer, message_size);
}

// TODO: it's silly to have two levels of abstraction here
// What does this buy? Find a way to reduce it to one.
// Either merge the two levels of send between message types
// or use a common send function.
// ------------ High-level senders -------------- //
// Send key event message
static void send_key_event(key_event_data_t ke_data) {
  INFO("send_key_event kc: %04X, pressed: %u\n", ke_data.keycode, ke_data.pressed);

  key_event_message_t ke_msg;

  ke_msg.header.message_type = MSG_KEY_EVENT;
  ke_msg.header.message_length = MSG_LEN_KEY_EVENT;
  ke_msg.data = ke_data;

  send_key_event_core(ke_msg);
}

// Send handshake message
static void send_handshake(void){
  message_header_t msg_header;
  msg_header.message_type = MSG_HANDSHAKE;
  msg_header.message_length = MSG_LEN_HS;

  handshake_data_t hs_msg;
  hs_msg.hs_protocol_ver = REMOTE_KB_PROTOCOL_VER;
  hs_msg.hs_message_sender = rm_config.host;

  // TODO: can save on computing this by changing how the lengths are used?
  // There should be no reason this has to be done.
  // The length can probably be inferred or something with sizeof()
  // or defined in the header file with the total length
  const uint8_t message_size = MSG_LEN_HEADER + MSG_LEN_HS + MSG_LEN_CHECKSUM;

  uint8_t hs_msg_buffer[message_size];
  hs_msg_buffer[0] = MSG_PREAMBLE_V2;
  hs_msg_buffer[1] = msg_header.raw;
  hs_msg_buffer[2] = hs_msg.raw;
  hs_msg_buffer[3] = checksum8(hs_msg_buffer, message_size-1);

  uart_putmsg(hs_msg_buffer, message_size);
}

// Send RGB event message
static void send_rgb(void) {
  INFO("sending rgb to remote\n");

  rgb_event_message_t rgb_msg;

  rgb_msg.header.message_type = MSG_RGB_SYNC;
  rgb_msg.header.message_length = MSG_LEN_RGB_EVENT;
  rgb_msg.data.rgb_state = rm_config.rgb_state;

  send_rgb_core(rgb_msg);
}

// -------------- Message parsing --------------- //
// Generic message parser
static void parse_message(uint8_t *raw_msg_buf) {
  message_header_t msg_header = (message_header_t)raw_msg_buf[IDX_MESSAGE_TYPE_LENGTH];

  //TODO: if changing message_size change this too
  uint16_t idx_checksum = IDX_MESSAGE_PAYLOAD + msg_header.message_length;
  uint8_t checksum = checksum8(raw_msg_buf, idx_checksum);
  DEBUG("idx_checksum: %d\n", idx_checksum);
  DEBUG("checksum: %02X\n", checksum);

  if (checksum != raw_msg_buf[idx_checksum]) {
    ERROR("checksum mismatch!\n");
    return;
  }

  switch (msg_header.message_type) {
    case MSG_HANDSHAKE: {
      DEBUG("msg type: MSG_HANDSHAKE\n");
      handshake_data_t hs_data = (handshake_data_t)raw_msg_buf[IDX_MESSAGE_PAYLOAD];
      handle_handshake(hs_data);
    } break;

    case MSG_KEY_EVENT:
      DEBUG("msg type: MSG_KEY_EVENT\n");
      key_event_data_t ke_data = pack_key_event_message(&raw_msg_buf[IDX_MESSAGE_PAYLOAD]);
      handle_key_event(ke_data);
      break;
      
    case MSG_RGB_SYNC:
      DEBUG("msg type: MSG_RGB_SYNC\n");
      rgb_data_t rgb_data = pack_rgb_message(&raw_msg_buf[IDX_MESSAGE_PAYLOAD]);
      handle_rgb(rgb_data);
      break;

    default:
      DEBUG("unknown message type: %d\n", msg_header.message_type);
      break;
  }
}

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

static inline void parse_message_v1(uint8_t *raw_msg_buf) {
  uint8_t chksum = checksum8(raw_msg_buf, MSG_LEN_V1-1);
  if (raw_msg_buf[IDX_MESSAGE_PREAMBLE] != MSG_PREAMBLE_V1 || raw_msg_buf[IDX_CHECKSUM] != chksum) {
     ERROR("UART checksum mismatch!\n");
     print_message_buffer(raw_msg_buf, MSG_LEN_V1);
     ERROR("calc checksum: %02X\n", chksum);
  } else {
    key_event_data_t ke_data = pack_key_event_message(&raw_msg_buf[IDX_KCLSB]);
    handle_key_event(ke_data);
  }
}

// --------------- Top-level RMKB --------------- //
// Future: this will miss messages if they come in too quickly
// Consider changing to a timeout-based read
static inline void remote_kb_get_message(void) {
  if (!uart_available()) return;
  
  // Wait just a bit for the rest of the message to arrive
  wait_ms(1);

  uint8_t raw_msg_buf[RMKB_MSG_BUFFSIZE] = {0};
  uint8_t buf_idx = 0;

  while (uart_available()) {
    // Check for buffer overflow
    if (buf_idx >= RMKB_MSG_BUFFSIZE) {
      ERROR("buffer overflow!\n");
      buf_idx = 0;
    }

    // Get a byte and put it in the buffer
    raw_msg_buf[buf_idx] = uart_getchar();

    // Check for preamble
    // If this is the first byte, and it's not the preamble,
    // then the message is invalid
    if (buf_idx == 0) {
      if (raw_msg_buf[buf_idx] != MSG_PREAMBLE_V1 && raw_msg_buf[buf_idx] != MSG_PREAMBLE_V2 ) {
        buf_idx = 0;
        ERROR("byte sync error!\n");
        for (int i=0; i<RMKB_MSG_BUFFSIZE; i++) {
          ERROR("idx: %d, recv: 0x%02X\n", i, raw_msg_buf[i]);
        }
      } else {
        buf_idx++;
      }
    } else {
      buf_idx++;
    }
  }

  #if LOG_LEVEL >= LOG_DEBUG
  DEBUG("raw_msg_buf:\n");
  print_message_buffer(raw_msg_buf, RMKB_MSG_BUFFSIZE);
  #endif

  // Determine which protocol version the message is using, and parse it accordingly
  if (raw_msg_buf[IDX_MESSAGE_PREAMBLE] == MSG_PREAMBLE_V2) {
    parse_message(raw_msg_buf);
  } else {
    parse_message_v1(raw_msg_buf);
  }
}

// Initialize RGB
static void remote_kb_rgb_init(void) {
  rm_config.rgb_state = eeconfig_read_rgblight();
}

// Update RGB if the configuration has changed
static inline void remote_kb_update_rgb(void) {
  if (!rm_config.host) return;
  if (rm_config.protocol_ver != REMOTE_KB_PROTOCOL_VER || !rm_config.connected) return;

  // Only update if the host has changed
  uint32_t rgb_state = rgblight_read_dword();
  if (rgb_state ^ rm_config.rgb_state) {
    DEBUG("RGB state changed: %08lX\n", rgb_state);
    rm_config.rgb_state = rgb_state;
    send_rgb();
  }
}

// Send a key event to the host
static void remote_kb_send_keyevent(key_event_data_t ke_data) {
  // Only send if not the host
  if (rm_config.host) return;

  if (rm_config.protocol_ver == REMOTE_KB_PROTOCOL_VER && rm_config.connected) {
    send_key_event(ke_data);
  } else {
    send_keyevent_msg_v1(ke_data);
  }
}

// Detect if this is a host or remote device
static void remote_kb_host_detect(void) {
  rm_config.host = vbus_detect();
  #if defined (KEYBOARD_HOST)
  rm_config.host = true;
  #elif defined(KEYBOARD_REMOTE)
  rm_config.host = false;
  #endif
}

// Send a remote keyboard handshake 
static void remote_kb_send_handshake(void) {
  if (timer_elapsed(rm_config.handshake_timer) >= HANDSHAKE_TIMEOUT_MS) {
    INFO("sending handshake\n");
    send_handshake();
    rm_config.handshake_timer = timer_read();
  }
  return;
}

// -------------- Public functions -------------- //
// V2
// Tap code wrapper to handle encoder events
void tap_code_remote_kb(uint16_t keycode) {
  tap_code(keycode);
  remote_kb_send_keyevent((key_event_data_t){keycode, PRESSED});
  wait_ms((TAP_CODE_DELAY ? TAP_CODE_DELAY : DEFAULT_TAP_DELAY));
  remote_kb_send_keyevent((key_event_data_t){keycode, RELEASED});
}

// Register code wrapper to handle encoder events
void register_code_remote_kb(uint16_t keycode) {
  register_code(keycode);
  remote_kb_send_keyevent((key_event_data_t){keycode, PRESSED});
}

// Unregister code wrapper to handle encoder events
void unregister_code_remote_kb(uint16_t keycode) {
  unregister_code(keycode);
  remote_kb_send_keyevent((key_event_data_t){keycode, RELEASED});
}

// V1
void matrix_init_remote_kb(void) {
  uart_init(SERIAL_UART_BAUD);
  remote_kb_host_detect();
  remote_kb_rgb_init();
}

// Send keystrokes from local to remote
void process_record_remote_kb(uint16_t keycode, keyrecord_t *record) {
  remote_kb_send_keyevent((key_event_data_t){keycode, record->event.pressed});
}

// Receive keystrokes from remote to local
void matrix_scan_remote_kb(void) {
  if (rm_config.host && !rm_config.connected) {
      remote_kb_send_handshake();
  }
  remote_kb_get_message();
  remote_kb_update_rgb();
}
