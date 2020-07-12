ARDUINO_LIBS = avr-i2c-slave avr_uart
ARDMK_VENDOR = archlinux-arduino
BOARD_TAG    = uno
MONITOR_PORT = /dev/ttyACM1
ifeq ($(IS_LAMP),yes)
MONITOR_PORT = /dev/ttyACM0
endif

USER_LIB_PATH = /home/johann/PERSO/AVR/libs/

CFLAGS = -g -Os -std=gnu99
ifeq ($(IS_LAMP),yes)
CFLAGS += -DIS_LAMP
endif
LDFLAGS = -Wl,-u,vfprintf -lprintf_flt

include /home/johann/PERSO/Arduino-Makefile/Arduino.mk
