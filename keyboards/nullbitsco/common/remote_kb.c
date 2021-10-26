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
*/

#include "remote_kb.h"
#include "uart.h"

// V2
remote_kb_config rm_config;

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

static void handle_handshake(handshake_data_t hs_data) {
  DEBUG("hs_protocol_ver: %d hs_message_sender: %s\n", hs_data.hs_protocol_ver, hs_data.hs_message_sender ? "host" : "remote");
  if ((hs_data.hs_protocol_ver == REMOTE_KB_PROTOCOL_VER) && (hs_data.hs_message_sender != rm_config.host)) {
    INFO("accepted from %s\n", hs_data.hs_message_sender ? "host" : "remote");
    rm_config.protocol_ver = REMOTE_KB_PROTOCOL_VER;
    rm_config.connected = true;
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
  
  // ke_msg_buffer[0] = (ke_msg.data.keycode & 0xFF);
  // ke_msg_buffer[1] = (ke_msg.data.keycode >> 8) & 0xFF;
  // ke_msg_buffer[2] = ke_msg.data.pressed;

  // uint16_t keycode = (uint16_t)raw_msg[IDX_KCLSB] | ((uint16_t)raw_msg[IDX_KCMSB] << 8);
  // bool pressed = (bool)raw_msg[IDX_PRESSED];

  key_event_data.keycode = (uint16_t)key_event_msg_buffer[0];
  key_event_data.keycode |= ((uint16_t)key_event_msg_buffer[1] << 8);
  key_event_data.pressed = key_event_msg_buffer[2];
  DEBUG("key_event_data keycode: %04X pressed: %01X\n", key_event_data.keycode, key_event_data.pressed);
  return key_event_data;
}

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
  #if DLEVEL >= 2
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
  #if DLEVEL >= 2
  for (int i=0; i<HS_MSG_SIZE; i++) {
    PRINT(" %02X", hs_msg_buffer[i]);
  }
  PRINT("\n");
  #endif

  for (int i=0; i<HS_MSG_SIZE; i++) {
    uart_putchar(hs_msg_buffer[i]);
  }
}

// TODO: bounds checking, including SAFE_RANGE and 0x00-0xFF
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

//TODO: rename raw msg to include buffer?
static void parse_message_v2(uint8_t *raw_msg) {
  //TODO: pack to message_header_t struct? (cleaner way to do this?)
  uint8_t message_type = raw_msg[IDX_MESSAGE_TYPE_LENGTH] >> 4;
  uint8_t message_length = raw_msg[IDX_MESSAGE_TYPE_LENGTH] & 0xF;
  DEBUG("message_type: %01X message_length: %01X\n", message_type, message_length);

  uint16_t IDX_MESSAGE_CHECKSUM = IDX_MESSAGE_PAYLOAD + message_length;
  uint8_t checksum = chksum8(raw_msg, IDX_MESSAGE_CHECKSUM);
  DEBUG("IDX_MESSAGE_CHECKSUM: %d\n", IDX_MESSAGE_CHECKSUM);
  DEBUG("checksum: %02X\n", checksum);

  // No point in packing if the checksum is wrong. Right?
  if (checksum != raw_msg[IDX_MESSAGE_CHECKSUM]) {
    DEBUG("checksum mismatch!\n");
    return;
  }

  switch (message_type) {
    case MSG_HANDSHAKE: {
      DEBUG("msg type: MSG_HANDSHAKE\n");
      // Only send a handshake back if the message is from the host.
      if (!rm_config.host) {
        wait_ms(1);
        send_handshake();
      }
      //TODO: better to do in place?
      handshake_data_t hs_data = pack_handshake_message(&raw_msg[IDX_MESSAGE_PAYLOAD]);
      handle_handshake(hs_data);
    } break;
    case MSG_KEY_EVENT:
      DEBUG("msg type: MSG_KEY_EVENT\n");
      pack_key_event_message(&raw_msg[IDX_MESSAGE_PAYLOAD]);
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
static void send_keyevent_msg_v1(key_event_data_t ke_data) {
  if (IS_HID_KC(ke_data.keycode) || IS_RM_KC(ke_data.keycode)) {
    uint8_t raw_msg[UART_MSG_LEN];

    INFO("Remote: send [%u]\n", ke_data.keycode);
    raw_msg[IDX_PREAMBLE] = UART_PREAMBLE;
    raw_msg[IDX_KCLSB] = (ke_data.keycode & 0xFF);
    raw_msg[IDX_KCMSB] = (ke_data.keycode >> 8) & 0xFF;
    raw_msg[IDX_PRESSED] = ke_data.pressed;
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
static inline void get_message(void) {
  if (!uart_available()) return;
  
  uint16_t start_time = timer_read();

  // TODO: testing to see if we are racing
  /*
  yep. so, either leave delay, OR 
  perhaps it makes more sense to change
  the architecture to read packets as 
  they come in, and then read a certain
  number of bytes at a time, and 
  timeout if we don't get a full packet.
  */
  wait_ms(1);

  uint8_t raw_msg[RMKB_MSG_BUFFSIZE] = {0};
  uint16_t timer_val[RMKB_MSG_BUFFSIZE] = {0};
  uint8_t raw_msg_idx = 0;
  uint8_t bytes_read = 0;

  while (uart_available()) {
    raw_msg[raw_msg_idx] = uart_getchar();
    timer_val[raw_msg_idx] = timer_read();  

    if (raw_msg_idx == 0) {
      if (raw_msg[raw_msg_idx] != UART_PREAMBLE && raw_msg[raw_msg_idx] != RMKB_MSG_PREAMBLE ) {
        wait_ms(20);
        ERROR("Byte sync error!\n");
        ERROR("start time: %u, bytes read: %u\n", start_time, bytes_read);
        for (int i=0; i<RMKB_MSG_BUFFSIZE; i++) {
          ERROR("idx: %d, recv: 0x%02X, time: %u\n", i, raw_msg[i], timer_val[i]);
        }
        wait_ms(20);
        raw_msg_idx = 0;
      } else {
        raw_msg_idx++;
        bytes_read++;
        
        if (raw_msg_idx == RMKB_MSG_BUFFSIZE) {
          ERROR("Buffer overflow!\n");
          raw_msg_idx = 0;
        }
      }
    } else {
      raw_msg_idx++;
      bytes_read++;
      if (raw_msg_idx == RMKB_MSG_BUFFSIZE) {
        ERROR("Buffer overflow!\n");
        raw_msg_idx = 0;
      }
    }
  }
  #if DLEVEL >=2
  uint16_t end_time = timer_read();
  wait_ms(20);
  DEBUG("start time: %u, end time: %u, bytes read: %u\n", start_time, end_time, bytes_read);
  for (int i=0; i<RMKB_MSG_BUFFSIZE; i++) {
    DEBUG("idx: %d, recv: 0x%02X, time: %u\n", i, raw_msg[i], timer_val[i]);
  }
  wait_ms(20);
  #endif

  if ((raw_msg[IDX_PREAMBLE] == RMKB_MSG_PREAMBLE) || (rm_config.connected && rm_config.protocol_ver == 2)) {
    parse_message_v2(raw_msg);
  } else {
    parse_message_v1(raw_msg);
  }

  raw_msg_idx = 0;
}

// TODO: pointless right now
static void handle_incoming_messages(void) {
  get_message();
}

static void remote_kb_send_keycode(key_event_data_t ke_data) {
  // Only send if not the host
  if (!rm_config.host) {
    if (rm_config.protocol_ver == 2 && rm_config.connected) {
      send_key_event(ke_data);
    } else {
      send_keyevent_msg_v1(ke_data);
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

// static void print_status(void) {
//   if (timer_elapsed(rm_config.status_timer) >= STATUS_TIMEOUT_MS) {
//     INFO("connected: %s\n", rm_config.connected ? "true" : "false");
//     INFO("host: %s\n", rm_config.host ? "true" : "false");
//     INFO("protocol_ver: %d\n", rm_config.protocol_ver);
//     rm_config.status_timer = timer_read();
//   }
// }

//TODO: do this every x seconds to re-establish connection
//TODO: set connected=false first in case of disconnect? worth? idk
static void remote_kb_send_handshake(void) {
  static int hs_count = 0;
  if (timer_elapsed(rm_config.handshake_timer) >= HANDSHAKE_TIMEOUT_MS) {
    hs_count++;
    INFO("Sending HS %d\n", hs_count);
    send_handshake();
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
  key_event_data_t ke_data;
  ke_data.keycode = keycode;
  ke_data.pressed = record->event.pressed;
  remote_kb_send_keycode(ke_data);
}

// Receive keystrokes from remote to local
void matrix_scan_remote_kb(void) {
  // print_status(); //DEBUG

  // Only send handshakes if host
  if (!rm_config.connected && rm_config.host) {
    remote_kb_send_handshake();
  }

  handle_incoming_messages();
}
