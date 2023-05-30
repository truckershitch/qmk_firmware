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

uint8_t
 msg[UART_MSG_LEN],
 msg_idx = 0;

#if defined(__AVR_ATmega32U4__)
#include "uart.h"

#elif defined(MCU_RP)
#include "quantum.h"
#include "usb_util.h"
#include <hal.h>

// NOTE: RP2040 uart driver is not ready, so these must be implemented w/ SIO
typedef SIODriver QMKSerialDriver;
typedef SIOConfig QMKSerialConfig;

/* USART in 8E2 config with RX and TX FIFOs enabled. */
static QMKSerialConfig serial_config = {
    .baud = (SERIAL_UART_BAUD),
    .UARTLCR_H = UART_UARTLCR_H_WLEN_8BITS | UART_UARTLCR_H_PEN | UART_UARTLCR_H_STP2 | UART_UARTLCR_H_FEN,
    .UARTCR = 0U,
    .UARTIFLS = UART_UARTIFLS_RXIFLSEL_1_8F | UART_UARTIFLS_TXIFLSEL_1_8E,
    .UARTDMACR = 0U
};
static QMKSerialDriver* serial_driver = (QMKSerialDriver*)&RMKB_SIO_DRIVER;

void uart_init(uint32_t baud) {
  // Set GPIO pins
  palSetLineMode(RMKB_SIO_TX_PIN, PAL_MODE_ALTERNATE_UART);
  palSetLineMode(RMKB_SIO_RX_PIN, PAL_MODE_ALTERNATE_UART);

  // Init SIO driver
  sioStart(serial_driver, &serial_config);
}

void uart_transmit(const uint8_t* data, uint16_t length) {
  chnWriteTimeout(serial_driver, data, (size_t)length, TIME_MS2I(20));
}

bool uart_available(void) { 
  return !sioIsRXEmptyX(serial_driver);
}

uint8_t uart_read(void) {
  return (uint8_t)sioGetX(serial_driver);
}

#else
// Platform is not supported yet
void matrix_init_remote_kb(void) {}
void process_record_remote_kb(uint16_t keycode, keyrecord_t *record) {}
void matrix_scan_remote_kb(void) {}

#endif

#if defined(KEYBOARD_HOST)
static bool is_host(void) { return true; }

#elif defined(KEYBOARD_REMOTE)
static bool is_host(void) { return false; }

#else
static bool is_host(void) {
  static bool init = false;
  static bool is_host = false;

  if (!init) {
    #if defined(__AVR_ATmega32U4__)
    // true if VBUS is present, false otherwise.
    USBCON |= (1 << OTGPADE); //enables VBUS pad
    _delay_us(10);
    is_host = (USBSTA & (1<<VBUS));  //checks state of VBUS

    #else //MCU_RP
    for (uint16_t i = 0; i < (200); i++) {
        // This will return true if a USB connection has been established
        if (usb_connected_state()) {
            is_host = true;
            break;
        }
        wait_ms(10);
    }
    #endif
  dprintf("host: %s\n",  is_host ? "true" : "false");
  }
  init = true;
  return is_host;
}

#endif

static uint8_t chksum8(const unsigned char *buf, size_t len) {
  unsigned int sum;
  for (sum = 0 ; len != 0 ; len--)
    sum += *(buf++);
  return (uint8_t)sum;
}

static void send_msg(uint16_t keycode, bool pressed) {
  msg[IDX_PREAMBLE] = UART_PREAMBLE;
  msg[IDX_KCLSB] = (keycode & 0xFF);
  msg[IDX_KCMSB] = (keycode >> 8) & 0xFF;
  msg[IDX_PRESSED] = pressed;
  msg[IDX_CHECKSUM] = chksum8(msg, UART_MSG_LEN-1);
  uart_transmit(msg, UART_MSG_LEN);
}

static inline void print_message_buffer(void) {
  for (int i=0; i<UART_MSG_LEN; i++) {
    dprintf("msg[%u]: 0x%02X\n", i, msg[i]);
  }
}

static void process_uart(void) {
  uint8_t chksum = chksum8(msg, UART_MSG_LEN-1);
  if (msg[IDX_PREAMBLE] != UART_PREAMBLE || msg[IDX_CHECKSUM] != chksum) {
     dprintf("UART checksum mismatch!\n");
     print_message_buffer();
     dprintf("calc checksum: 0x%02X\n", chksum);
  } else {
    uint16_t keycode = (uint16_t)msg[IDX_KCLSB] | ((uint16_t)msg[IDX_KCMSB] << 8);
    bool pressed = (bool)msg[IDX_PRESSED];
    if (IS_RM_KC(keycode)) {
      keyrecord_t record;
      record.event.pressed = pressed;
      if (pressed) dprintf("Remote macro: press [%u]\n", keycode);
      else dprintf("Remote macro: release [%u]\n", keycode);
      process_record_user(keycode, &record);
    } else {
      if (pressed) {
        dprintf("Remote: press [%u]\n", keycode);
        register_code(keycode);
    } else {
        dprintf("Remote: release [%u]\n", keycode);
        unregister_code(keycode);
      }
    }
  }
}

static void get_msg(void) {
  while (uart_available()) {
    msg[msg_idx] = uart_read();
    dprintf("idx: %u, recv: 0x%02X\n", msg_idx, msg[msg_idx]);
    if (msg_idx == 0 && (msg[msg_idx] != UART_PREAMBLE)) {
      dprintf("Byte sync error!\n");
      msg_idx = 0;
    } else if (msg_idx == (UART_MSG_LEN-1)) {
      process_uart();
      msg_idx = 0;
      break;
    } else {
      msg_idx++;
    }
  }
}

static void handle_host_incoming(void) {
  get_msg();
}

static void handle_host_outgoing(void) {
  // for future reverse link use
}

static void handle_remote_incoming(void) {
  // for future reverse link use
}

static void handle_remote_outgoing(uint16_t keycode, keyrecord_t *record) {
  if (IS_HID_KC(keycode) || IS_RM_KC(keycode)) {
    dprintf("Remote: send [%u]\n", keycode);
    send_msg(keycode, record->event.pressed);
  }
}

// Public functions
void matrix_init_remote_kb(void) {
  uart_init(SERIAL_UART_BAUD);
}

void process_record_remote_kb(uint16_t keycode, keyrecord_t *record) {
  if (is_host()) handle_host_outgoing();
  else handle_remote_outgoing(keycode, record);
}

void matrix_scan_remote_kb(void) {
  if (is_host()) handle_host_incoming();
  else handle_remote_incoming();
}
