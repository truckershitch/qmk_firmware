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
This is implemented with the V2 protocol.

TODO: Add RGB animation sync for the remote keyboard.
TODO: Add periodic handshake to ensure the remote keyboard is still connected.
*/

#include "remote_kb.h"
#include "uart.h"

// V2
remote_kb_config rm_config;

static uint8_t chksum8(const unsigned char *buf, size_t len);
static void send_key_event(key_event_data_t ke_data);
static void send_rgb(void);
static void send_handshake(void);

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

static void remote_kb_print_status(void) {
  #if DLEVEL >= LINFO
  PRINT("---------------- RMKB STATUS ----------------\n");
  INFO("connected: %s\n", rm_config.connected ? "true" : "false");
  INFO("host: %s\n", rm_config.host ? "true" : "false");
  INFO("protocol_ver: %d\n", rm_config.protocol_ver);
  INFO("rgb_state: %08lX\n", rm_config.rgb_state);
  PRINT("---------------------------------------------\n");
  #endif
}

static void handle_handshake(handshake_data_t hs_data) {
  DEBUG("hs_protocol_ver: %d hs_message_sender: %s\n", hs_data.hs_protocol_ver, hs_data.hs_message_sender ? "host" : "remote");
  if ((hs_data.hs_protocol_ver == REMOTE_KB_PROTOCOL_VER) && (hs_data.hs_message_sender != rm_config.host)) {
    INFO("accepted from %s\n", hs_data.hs_message_sender ? "host" : "remote");
    rm_config.protocol_ver = REMOTE_KB_PROTOCOL_VER;
    rm_config.connected = true;
  }
  if (rm_config.connected) {
          // Only send a handshake back if the message is from the host.
      if (!rm_config.host) {
        wait_ms(1);
        send_handshake();
      } else {
        // Send the initial RGB configuration
        wait_ms(1);
        send_rgb();
      }
  }
  remote_kb_print_status();
}

static void handle_key_event(key_event_data_t key_event_data) {
  if (IS_ANY(key_event_data.keycode)) {
    if (key_event_data.pressed) {
      INFO("Remote: press [0x%02X]\n", key_event_data.keycode);
      register_code(key_event_data.keycode);
    } else {
      INFO("Remote: release [0x%02X]\n", key_event_data.keycode);
      unregister_code(key_event_data.keycode);
    }
  }
}

static void handle_rgb(rgb_data_t rgb_data) {
  // Only update if not host
  if (rm_config.host) return;
  if (rgb_data.rgb_state != rm_config.rgb_state) {
    rm_config.rgb_state = rgb_data.rgb_state;
    rgblight_update_dword(rgb_data.rgb_state);
      INFO("updated from host\n");
  }
}

static handshake_data_t pack_handshake_message(uint8_t *hs_msg_buffer) {
  handshake_data_t hs_data;

  hs_data.hs_protocol_ver = (hs_msg_buffer[0] >> 1) & 0x7F;
  hs_data.hs_message_sender = (hs_msg_buffer[0] & 0x01);
  DEBUG("hs_data protocol_ver: %01X sender: %01X\n", hs_data.hs_protocol_ver, hs_data.hs_message_sender);
  return hs_data;
}

static key_event_data_t pack_key_event_message(uint8_t *key_event_msg_buffer) {
  key_event_data_t key_event_data;

  key_event_data.keycode = (uint16_t)key_event_msg_buffer[0];
  key_event_data.keycode |= ((uint16_t)key_event_msg_buffer[1] << 8);
  key_event_data.pressed = key_event_msg_buffer[2];
  DEBUG("key_event_data keycode: %04X pressed: %01X\n", key_event_data.keycode, key_event_data.pressed);
  return key_event_data;
}

static rgb_data_t pack_rgb_message(uint8_t *rgb_msg_buffer) {
  rgb_data_t rgb_data;

  rgb_data.rgb_state = (uint32_t)rgb_msg_buffer[0];
  rgb_data.rgb_state |= ((uint32_t)rgb_msg_buffer[1] << 8);
  rgb_data.rgb_state |= ((uint32_t)rgb_msg_buffer[2] << 16);
  rgb_data.rgb_state |= ((uint32_t)rgb_msg_buffer[3] << 24);
  DEBUG("rgb_data rgb_state: %08lX\n", rgb_data.rgb_state);
  return rgb_data;
}

// TODO: combine shared code
static void send_key_event_core(key_event_message_t ke_msg) {
  #define KE_MSG_SIZE 6

  uint8_t ke_msg_buffer[KE_MSG_SIZE] = {0};
  ke_msg_buffer[0] = RMKB_MSG_PREAMBLE;
  ke_msg_buffer[1] = (ke_msg.header.message_type & 0xF) << 4;
  ke_msg_buffer[1] |= (ke_msg.header.message_length & 0xF);
  ke_msg_buffer[2] = (ke_msg.data.keycode & 0xFF);
  ke_msg_buffer[3] = (ke_msg.data.keycode >> 8) & 0xFF;
  ke_msg_buffer[4] = ke_msg.data.pressed;
  ke_msg_buffer[5] = chksum8(ke_msg_buffer, KE_MSG_SIZE-1);

  DEBUG("uart_putchar:");
  #if DLEVEL >= LDEBUG
  for (int i=0; i<KE_MSG_SIZE; i++) {
    PRINT(" %02X", ke_msg_buffer[i]);
  }
  PRINT("\n");
  #endif

  for (int i=0; i<KE_MSG_SIZE; i++) {
    uart_putchar(ke_msg_buffer[i]);
  }
}

static void send_handshake_core(handshake_message_t hs_msg) {
  #define HS_MSG_SIZE 4

  uint8_t hs_msg_buffer[HS_MSG_SIZE] = {0};
  hs_msg_buffer[0] = RMKB_MSG_PREAMBLE;
  hs_msg_buffer[1] = (hs_msg.header.message_type & 0xF) << 4;
  hs_msg_buffer[1] |= (hs_msg.header.message_length & 0xF);
  hs_msg_buffer[2] = (hs_msg.data.hs_protocol_ver & 0x7F) << 1;
  hs_msg_buffer[2] |= (hs_msg.data.hs_message_sender & 0x01);
  hs_msg_buffer[3] = chksum8(hs_msg_buffer, HS_MSG_SIZE-1);

  DEBUG("uart_putchar:");
  #if DLEVEL >= LDEBUG
  for (int i=0; i<HS_MSG_SIZE; i++) {
    PRINT(" %02X", hs_msg_buffer[i]);
  }
  PRINT("\n");
  #endif

  for (int i=0; i<HS_MSG_SIZE; i++) {
    uart_putchar(hs_msg_buffer[i]);
  }
}

static void send_rgb_core(rgb_event_message_t rgb_msg) {
  #define RGB_MSG_SIZE 7

  uint8_t rgb_msg_buffer[RGB_MSG_SIZE] = {0};
  rgb_msg_buffer[0] = RMKB_MSG_PREAMBLE;
  rgb_msg_buffer[1] = (rgb_msg.header.message_type & 0xF) << 4;
  rgb_msg_buffer[1] |= (rgb_msg.header.message_length & 0xF);
  rgb_msg_buffer[2] = (rgb_msg.data.rgb_state & 0xFF);        //LSB
  rgb_msg_buffer[3] = (rgb_msg.data.rgb_state >> 8) & 0xFF;
  rgb_msg_buffer[4] = (rgb_msg.data.rgb_state >> 16) & 0xFF;
  rgb_msg_buffer[5] = (rgb_msg.data.rgb_state >> 24) & 0xFF;  //MSB
  rgb_msg_buffer[6] = chksum8(rgb_msg_buffer, RGB_MSG_SIZE-1);

  DEBUG("uart_putchar:");
  #if DLEVEL >= LDEBUG
  for (int i=0; i<RGB_MSG_SIZE; i++) {
    PRINT(" %02X", rgb_msg_buffer[i]);
  }
  PRINT("\n");
  #endif

  for (int i=0; i<RGB_MSG_SIZE; i++) {
    uart_putchar(rgb_msg_buffer[i]);
  }
}

static void send_key_event(key_event_data_t ke_data) {
  INFO("send_key_event kc: %04X, pressed: %u\n", ke_data.keycode, ke_data.pressed);

  key_event_message_t ke_msg;

  ke_msg.header.message_type = MSG_KEY_EVENT;
  ke_msg.header.message_length = MSG_LEN_KEY_EVENT;
  ke_msg.data = ke_data;

  send_key_event_core(ke_msg);
}

static void send_handshake(void){
  handshake_message_t hs_msg;

  hs_msg.header.message_type = MSG_HANDSHAKE;
  hs_msg.header.message_length = MSG_LEN_HS;
  hs_msg.data.hs_protocol_ver = REMOTE_KB_PROTOCOL_VER;
  hs_msg.data.hs_message_sender = rm_config.host;

  send_handshake_core(hs_msg);
}

static void send_rgb(void) {
  INFO("Sending rgb\n");

  rgb_event_message_t rgb_msg;

  rgb_msg.header.message_type = MSG_RGB_SYNC;
  rgb_msg.header.message_length = MSG_LEN_RGB_EVENT;
  rgb_msg.data.rgb_state = rm_config.rgb_state;

  send_rgb_core(rgb_msg);
}

//TODO: rename raw msg to include buffer?
static void parse_message_v2(uint8_t *raw_msg_buf) {
  //TODO: pack to message_header_t struct? (cleaner way to do this?)
  uint8_t message_type = raw_msg_buf[IDX_MESSAGE_TYPE_LENGTH] >> 4;
  uint8_t message_length = raw_msg_buf[IDX_MESSAGE_TYPE_LENGTH] & 0xF;
  DEBUG("message_type: %01X message_length: %01X\n", message_type, message_length);

  uint16_t IDX_MESSAGE_CHECKSUM = IDX_MESSAGE_PAYLOAD + message_length;
  uint8_t checksum = chksum8(raw_msg_buf, IDX_MESSAGE_CHECKSUM);
  DEBUG("IDX_MESSAGE_CHECKSUM: %d\n", IDX_MESSAGE_CHECKSUM);
  DEBUG("checksum: %02X\n", checksum);

  // No point in packing if the checksum is wrong. Right?
  if (checksum != raw_msg_buf[IDX_MESSAGE_CHECKSUM]) {
    DEBUG("checksum mismatch!\n");
    return;
  }

  switch (message_type) {
    case MSG_HANDSHAKE: {
      DEBUG("msg type: MSG_HANDSHAKE\n");
      handshake_data_t hs_data = pack_handshake_message(&raw_msg_buf[IDX_MESSAGE_PAYLOAD]);
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
      DEBUG("Unknown message type: %d\n", message_type);
      break;
  }
}

static void remote_kb_rgb_init(void) {
  rm_config.rgb_state = eeconfig_read_rgblight();
}

static void remote_kb_update_rgb(void) {
  // Only update if the host
  if (!rm_config.host) return;

  uint32_t rgb_state = rgblight_read_dword();
  if (rgb_state ^ rm_config.rgb_state) {
    DEBUG("RGB state changed: %08lX\n", rgb_state);
    rm_config.rgb_state = rgb_state;
    send_rgb();
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
static void send_keyevent_msg_v1(key_event_data_t ke_data) {
  if (IS_ANY(ke_data.keycode)) {
    uint8_t raw_msg_buf[UART_MSG_LEN];

    INFO("Remote: send [%u]\n", ke_data.keycode);
    raw_msg_buf[IDX_PREAMBLE] = UART_PREAMBLE;
    raw_msg_buf[IDX_KCLSB] = (ke_data.keycode & 0xFF);
    raw_msg_buf[IDX_KCMSB] = (ke_data.keycode >> 8) & 0xFF;
    raw_msg_buf[IDX_PRESSED] = ke_data.pressed;
    raw_msg_buf[IDX_CHECKSUM] = chksum8(raw_msg_buf, UART_MSG_LEN-1);

    for (int i=0; i<UART_MSG_LEN; i++) {
      uart_putchar(raw_msg_buf[i]);
    }
  }
}

static void parse_message_v1(uint8_t *raw_msg_buf) {
  uint8_t chksum = chksum8(raw_msg_buf, UART_MSG_LEN-1);
  if (raw_msg_buf[IDX_PREAMBLE] != UART_PREAMBLE || raw_msg_buf[IDX_CHECKSUM] != chksum) {
     ERROR("UART checksum mismatch!\n");
     print_message_buffer_v2(raw_msg_buf, UART_MSG_LEN);
     ERROR("calc checksum: %02X\n", chksum);
  } else {
    key_event_data_t ke_data = pack_key_event_message(&raw_msg_buf[IDX_KCLSB]);
    handle_key_event(ke_data);
  }
}

//TODO: buffer boudns checks!
//todo: stop reading after full message?
static inline void remote_kb_get_message(void) {
  if (!uart_available()) return;
  
  // Wait just a bit for the rest of the message to arrive
  wait_ms(1);

  uint8_t raw_msg_buf[RMKB_MSG_BUFFSIZE] = {0};
  uint8_t buf_idx = 0;
  uint8_t bytes_read = 0;

  while (uart_available()) {
    raw_msg_buf[buf_idx] = uart_getchar();

    if (buf_idx == 0) {
      if (raw_msg_buf[buf_idx] != UART_PREAMBLE && raw_msg_buf[buf_idx] != RMKB_MSG_PREAMBLE ) {
        ERROR("Byte sync error!\n");
        ERROR("bytes read: %u\n", bytes_read);
        for (int i=0; i<RMKB_MSG_BUFFSIZE; i++) {
          ERROR("idx: %d, recv: 0x%02X\n", i, raw_msg_buf[i]);
        }
        buf_idx = 0;
      } else {
        buf_idx++;
        bytes_read++;
        
        if (buf_idx == RMKB_MSG_BUFFSIZE) {
          ERROR("Buffer overflow!\n");
          buf_idx = 0;
        }
      }
    } else {
      buf_idx++;
      bytes_read++;
      if (buf_idx == RMKB_MSG_BUFFSIZE) {
        ERROR("Buffer overflow!\n");
        buf_idx = 0;
      }
    }
  }
  #if DLEVEL >= LDEBUG
  DEBUG("bytes read: %u\n", bytes_read);
  for (int i=0; i<RMKB_MSG_BUFFSIZE; i++) {
    DEBUG("idx: %d, recv: 0x%02X\n", i, raw_msg_buf[i]);
  }
  #endif

  if ((raw_msg_buf[IDX_PREAMBLE] == RMKB_MSG_PREAMBLE) || (rm_config.connected && rm_config.protocol_ver == 2)) {
    parse_message_v2(raw_msg_buf);
  } else {
    parse_message_v1(raw_msg_buf);
  }

  buf_idx = 0;
}
// uint16_t keycode, keyrecord_t *record
static void remote_kb_send_keycode(key_event_data_t ke_data) {
  // Only send if not the host
  if (rm_config.host) return;

  if (rm_config.protocol_ver == 2 && rm_config.connected) {
    send_key_event(ke_data);
  } else {
    send_keyevent_msg_v1(ke_data);
  }
}

static void remote_kb_host_detect(void) {
  rm_config.host = vbus_detect();
  
  #if defined (KEYBOARD_HOST)
  rm_config.host = true;
  #elif defined(KEYBOARD_REMOTE)
  rm_config.host = false;
  #endif
}

static void remote_kb_send_handshake(void) {
  if (timer_elapsed(rm_config.handshake_timer) >= HANDSHAKE_TIMEOUT_MS) {
    INFO("Sending handshake\n");
    send_handshake();
    rm_config.handshake_timer = timer_read();
  }
  return;
}

// -------------- Public functions -------------- //
// V2
//TODO: remove remote_kb prefix?
void tap_code_remote_kb(uint16_t keycode) {
  tap_code(keycode);
  remote_kb_send_keycode((key_event_data_t){keycode, PRESSED});
  wait_ms((TAP_CODE_DELAY ? TAP_CODE_DELAY : DEFAULT_TAP_DELAY));
  remote_kb_send_keycode((key_event_data_t){keycode, RELEASED});
}

void register_code_remote_kb(uint16_t keycode) {
  register_code(keycode);
  remote_kb_send_keycode((key_event_data_t){keycode, PRESSED});
}

void unregister_code_remote_kb(uint16_t keycode) {
  unregister_code(keycode);
  remote_kb_send_keycode((key_event_data_t){keycode, RELEASED});
}

// V1
void matrix_init_remote_kb(void) {
  uart_init(SERIAL_UART_BAUD);
  remote_kb_host_detect();
  remote_kb_rgb_init();
}

// Send keystrokes from local to remote
void process_record_remote_kb(uint16_t keycode, keyrecord_t *record) {
  remote_kb_send_keycode((key_event_data_t){keycode, record->event.pressed});
}

// Receive keystrokes from remote to local
void matrix_scan_remote_kb(void) {
  if (rm_config.host && !rm_config.connected) {
      remote_kb_send_handshake();
  }
  remote_kb_get_message();
  remote_kb_update_rgb();
}
