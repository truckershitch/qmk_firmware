# MCU name
MCU = RP2040
BOOTLOADER = rp2040

# Build Options
#   change yes to no to disable
#
BOOTMAGIC_ENABLE = no       # Enable Bootmagic Lite
MOUSEKEY_ENABLE = yes       # Mouse keys
EXTRAKEY_ENABLE = yes       # Audio control and System control
CONSOLE_ENABLE = yes        # Console for debug
COMMAND_ENABLE = no         # Commands for debug and configuration
NKRO_ENABLE = yes           # Enable N-Key Rollover
BACKLIGHT_ENABLE = no       # Enable keyboard backlight functionality

# Nullbits options
WS2812_DRIVER = vendor
OLED_ENABLE = yes
VIA_ENABLE = yes
ENCODER_ENABLE = yes
RGBLIGHT_ENABLE = yes
EEPROM_DRIVER = transient
AUDIO_ENABLE = no #not yet supported
