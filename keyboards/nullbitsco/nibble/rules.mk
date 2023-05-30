# Build Options
#   change yes to no to disable
#
BOOTMAGIC_ENABLE = no       # Enable Bootmagic Lite
MOUSEKEY_ENABLE = no        # Mouse keys
EXTRAKEY_ENABLE = yes       # Audio control and System control
CONSOLE_ENABLE = no         # Console for debug
COMMAND_ENABLE = no         # Commands for debug and configuration
NKRO_ENABLE = yes           # Enable N-Key Rollover
BACKLIGHT_ENABLE = no       # Enable keyboard backlight functionality
RGBLIGHT_ENABLE = yes       # Enable keyboard RGB underglow
AUDIO_ENABLE = no           # Audio output
ENCODER_ENABLE = yes        # Use rotary encoder
CUSTOM_MATRIX = lite        # Lite custom matrix

DEFAULT_FOLDER = nullbitsco/nibble/avr

# Project specific files
SRC += matrix.c \
       common/nb_leds.c \
       common/remote_kb.c
