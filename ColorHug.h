/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2011-2012 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef COLOR_HUG_H
#define COLOR_HUG_H

#include <stdint.h>
#include <stdbool.h>

/* device constants */
#define	CH_USB_VID				0x273f
#define	CH_USB_PID_BOOTLOADER			0x1006
#define	CH_USB_PID_FIRMWARE			0x1017
#define	CH_USB_CONFIG				0x0001
#define	CH_USB_INTERFACE			0x0000
#define	CH_USB_HID_EP				0x0001
#define	CH_USB_HID_EP_IN			(CH_USB_HID_EP | 0x80)
#define	CH_USB_HID_EP_OUT			(CH_USB_HID_EP | 0x00)
#define	CH_USB_HID_EP_SIZE			64

/* ensure this is incremented on each released build */
#define CH_VERSION_MAJOR			4
#define CH_VERSION_MINOR			0
#define CH_VERSION_MICRO			0

#define CH_REPORT_HID_SENSOR			0x01
#define CH_REPORT_SENSOR_SETTINGS		0x02
#define CH_REPORT_SYSTEM_SETTINGS		0x03

struct input_report {
	uint8_t report_id;
	uint8_t sensor_state;
	uint8_t sensor_event;
	uint32_t illuminance;
};

struct sensor_settings {
	uint8_t report_id;
	uint8_t color_select;
	uint8_t LEDs_state;
	uint8_t multiplier;
	uint16_t integral_time;
};

struct system_settings {
	uint8_t report_id;
	uint8_t hardware_version;
	uint16_t version_major;
	uint16_t version_minor;
	uint16_t version_micro;
	uint32_t serial;
	uint8_t reset;
	uint8_t flash_flag;
};

/**
 * CH_CMD_GET_COLOR_SELECT:
 *
 * Get the color select state.
 *
 * IN:  [1:cmd]
 * OUT: [1:retval][1:cmd][1:color_select]
 *
 * This command is only available in firmware mode.
 **/
#define	CH_CMD_GET_COLOR_SELECT			0x01

/**
 * CH_CMD_SET_COLOR_SELECT:
 *
 * Set the color select state.
 *
 * IN:  [1:cmd][1:color_select]
 * OUT: [1:retval][1:cmd]
 *
 * This command is only available in firmware mode.
 **/
#define	CH_CMD_SET_COLOR_SELECT			0x02

/**
 * CH_CMD_GET_MULTIPLIER:
 *
 * Gets the multiplier value.
 *
 * IN:  [1:cmd]
 * OUT: [1:retval][1:cmd][1:multiplier_value]
 *
 * This command is only available in firmware mode.
 **/
#define	CH_CMD_GET_MULTIPLIER			0x03

/**
 * CH_CMD_SET_MULTIPLIER:
 *
 * Sets the multiplier value.
 *
 * IN:  [1:cmd][1:multiplier_value]
 * OUT: [1:retval][1:cmd]
 *
 * This command is only available in firmware mode.
 **/
#define	CH_CMD_SET_MULTIPLIER			0x04

/**
 * CH_CMD_GET_INTEGRAL_TIME:
 *
 * Gets the integral time.
 *
 * IN:  [1:cmd]
 * OUT: [1:retval][1:cmd][2:integral_time]
 *
 * This command is only available in firmware mode.
 **/
#define	CH_CMD_GET_INTEGRAL_TIME		0x05

/**
 * CH_CMD_SET_INTEGRAL_TIME:
 *
 * Sets the integral time.
 *
 * IN:  [1:cmd][2:integral_time]
 * OUT: [1:retval][1:cmd]
 *
 * This command is only available in firmware mode.
 **/
#define	CH_CMD_SET_INTEGRAL_TIME		0x06

/**
 * CH_CMD_GET_FIRMWARE_VERSION:
 *
 * Gets the firmware version.
 *
 * IN:  [1:cmd]
 * OUT: [1:retval][1:cmd][2:major][2:minor][2:micro]
 *
 * This command is available in bootloader and firmware mode.
 **/
#define	CH_CMD_GET_FIRMWARE_VERSION		0x07

/**
 * CH_CMD_GET_SERIAL_NUMBER:
 *
 * Gets the device serial number.
 *
 * IN:  [1:cmd]
 * OUT: [1:retval][1:cmd][4:serial_number]
 *
 * This command is only available in firmware mode.
 **/
#define	CH_CMD_GET_SERIAL_NUMBER		0x0b

/**
 * CH_CMD_GET_LEDS:
 *
 * Get the LED state.
 *
 * IN:  [1:cmd]
 * OUT: [1:retval][1:cmd][1:led_state]
 *
 * This command is only available in firmware mode.
 **/
#define	CH_CMD_GET_LEDS				0x0d

/**
 * CH_CMD_SET_LEDS:
 *
 * Set the LED state. Using a @repeat value of anything other than
 * 0 will block the processor for the duration of the command.
 *
 * If @repeat is not 0, then the LEDs are reset to all off at the end
 * of the sequence.
 *
 * IN:  [1:cmd][1:led_state][1:repeat][1:on-time][1:off-time]
 * OUT: [1:retval][1:cmd]
 *
 * This command is only available in firmware mode.
 **/
#define	CH_CMD_SET_LEDS				0x0e

/**
 * CH_CMD_TAKE_READING_RAW:
 *
 * Take a raw reading.
 *
 * IN:  [1:cmd]
 * OUT: [1:retval][1:cmd][4:count]
 *
 * This command is only available in firmware mode.
 **/
#define	CH_CMD_TAKE_READING_RAW			0x21

/**
 * CH_CMD_RESET:
 *
 * Reset the processor.
 *
 * IN:  [1:cmd]
 * OUT: [1:retval][1:cmd] (but with success the device will disconnect)
 *
 * This command is available in bootloader and firmware mode.
 **/
#define	CH_CMD_RESET				0x24

/**
 * CH_CMD_READ_FLASH:
 *
 * Read in raw data from the flash memory.
 *
 * IN:  [1:cmd][2:address][1:length]
 * OUT: [1:retval][1:cmd][1:checksum][1-60:data]
 *
 * This command is only available in bootloader mode.
 **/
#define	CH_CMD_READ_FLASH			0x25

/**
 * CH_CMD_ERASE_FLASH:
 *
 * Erases flash memory before a write is done.
 * Erasing flash can only be done in 1k byte chunks and should be
 * aligned to 1k.
 *
 * IN:  [1:cmd][2:address][2:length]
 * OUT: [1:retval][1:cmd]
 *
 * This command is only available in bootloader mode.
 **/
#define	CH_CMD_ERASE_FLASH			0x29

/**
 * CH_CMD_WRITE_FLASH:
 *
 * Write raw data to the flash memory. You can only write aligned to
 * a 32 byte boundary, and you must flush any incomplete 64 byte block.
 *
 * IN:  [1:cmd][2:address][1:length][1:checksum][1-32:data]
 * OUT: [1:retval][1:cmd]
 *
 * This command is only available in bootloader mode.
 **/
#define	CH_CMD_WRITE_FLASH			0x26

/**
 * CH_CMD_BOOT_FLASH:
 *
 * Boot into to the flash memory.
 *
 * IN:  [1:cmd]
 * OUT: [1:retval][1:cmd]
 *
 * This command is only available in bootloader mode.
 **/
#define	CH_CMD_BOOT_FLASH			0x27

/**
 * CH_CMD_SET_FLASH_SUCCESS:
 *
 * Sets the result of the firmware flashing. The idea of this command
 * is that the flashing interaction is thus:
 *
 * 1.	Reset()			device goes to bootloader mode
 * 2.	SetFlashSuccess(FALSE)
 * 3.	WriteFlash($data)
 * 4.	ReadFlash($data)	to verify
 * 5.	BootFlash()		switch to program mode
 * 6.	SetFlashSuccess(TRUE)
 *
 * The idea is that we only set the success FALSE from the bootoloader
 * to indicate that on booting we should not boot into the program.
 * We can only set the success true from the *new* program code so as
 * to verify that the new program boots, and can accept HID commands.
 *
 * IN:  [1:cmd][1:success]
 * OUT: [1:retval][1:cmd]
 *
 * This command is available in bootloader and firmware mode, although
 * different values of @success are permitted in each.
 **/
#define	CH_CMD_SET_FLASH_SUCCESS		0x28

/**
 * CH_CMD_GET_HARDWARE_VERSION:
 *
 * Get the hardware version.
 *
 * The hardware versions are as follows:
 * 0x00		= Pre-production hardware
 * 0x01		= ColorHug
 * 0x02		= ColorHug2
 * 0x03		= ColorHug+
 * 0x04		= ColorHugALS
 * 0x05-0x0f	= Reserved for future use
 *
 * IN:  [1:cmd]
 * OUT: [1:retval][1:cmd][1:hw_version]
 *
 * This command is available in bootloader and firmware mode.
 **/
#define	CH_CMD_GET_HARDWARE_VERSION		0x30

/**
 * CH_CMD_SELF_TEST:
 *
 * Tests the device by trying to get a non-zero reading from each
 * color channel.
 *
 * IN:  [1:cmd]
 * OUT: [1:retval][1:cmd]
 *
 * This command is available in bootloader and firmware mode.
 **/
#define	CH_CMD_SELF_TEST			0x40

/* secret code, which is actually encoded as '84f40464' */
#define	CH_FIRMWARE_ID_TOKEN			"8f06"

/* input and output buffer offsets */
#define	CH_BUFFER_INPUT_CMD			0x00
#define	CH_BUFFER_INPUT_DATA			0x01
#define	CH_BUFFER_OUTPUT_RETVAL			0x00
#define	CH_BUFFER_OUTPUT_CMD			0x01
#define	CH_BUFFER_OUTPUT_DATA			0x02

/* where the custom firmware is stored */
#define CH_EEPROM_ADDR_RUNCODE			0x2000	/* bytes */
#define CH_EEPROM_ADDR_MAX			0x4000	/* bytes */

/* this is a whole seporate block at the end of the flash */
#define	CH_EEPROM_ADDR_FLASH_SUCCESS		0x3f80	/* bytes (in f/w) */

#define CH_COLOR_OFFSET_RED			0x00
#define CH_COLOR_OFFSET_GREEN			0x01
#define CH_COLOR_OFFSET_BLUE			0x02

/* flash constants */
#define	CH_FLASH_ERASE_BLOCK_SIZE		0x040	/* 64 */
#define	CH_FLASH_WRITE_BLOCK_SIZE		0x040	/* 64 */
#define	CH_FLASH_TRANSFER_BLOCK_SIZE		0x020	/* 32 */

/* which color to select */
typedef enum {
	CH_COLOR_SELECT_RED,
	CH_COLOR_SELECT_WHITE,
	CH_COLOR_SELECT_BLUE,
	CH_COLOR_SELECT_GREEN
} ChColorSelect;

/* which color to select */
typedef enum {
	CH_STATUS_LED_GREEN	= 1,
	CH_STATUS_LED_RED	= 2
} ChStatusLed;

/* what frequency divider to use */
typedef enum {
	CH_FREQ_SCALE_0,
	CH_FREQ_SCALE_20,
	CH_FREQ_SCALE_2,
	CH_FREQ_SCALE_100
} ChFreqScale;

typedef enum {
	CH_UNKNOWN_STATE = 0,
	CH_READY,
	CH_NOT_AVAILABLE,
	CH_NO_DATA_SEL,
	CH_INITIALIZING,
	CH_ACCESS_DENIED,
	CH_ERROR
} ChSensorState;

typedef enum {
	CH_UNKNOWN_EVENT = 0,
	CH_STATE_CHANGED,
	CH_PROPERTY_CHANGED,
	CH_DATA_UPDATED,
	CH_POLL_RESPONSE,
	CH_CHANGE_SENSITIVITY
} ChSensorEvent;

/* fatal error morse code */
typedef enum {
	CH_ERROR_NONE,
	CH_ERROR_UNKNOWN_CMD,
	CH_ERROR_WRONG_UNLOCK_CODE,
	CH_ERROR_NOT_IMPLEMENTED,
	CH_ERROR_UNDERFLOW_SENSOR,
	CH_ERROR_NO_SERIAL,
	CH_ERROR_WATCHDOG,
	CH_ERROR_INVALID_ADDRESS,
	CH_ERROR_INVALID_LENGTH,
	CH_ERROR_INVALID_CHECKSUM,
	CH_ERROR_INVALID_VALUE,
	CH_ERROR_UNKNOWN_CMD_FOR_BOOTLOADER,
	CH_ERROR_NO_CALIBRATION,
	CH_ERROR_OVERFLOW_MULTIPLY,
	CH_ERROR_OVERFLOW_ADDITION,
	CH_ERROR_OVERFLOW_SENSOR,
	CH_ERROR_OVERFLOW_STACK,
	CH_ERROR_DEVICE_DEACTIVATED,
	CH_ERROR_INCOMPLETE_REQUEST,
	CH_ERROR_SELF_TEST_SENSOR,
	CH_ERROR_SELF_TEST_RED,
	CH_ERROR_SELF_TEST_GREEN,
	CH_ERROR_SELF_TEST_BLUE,
	CH_ERROR_SELF_TEST_COLOR_SELECT,
	CH_ERROR_SELF_TEST_MULTIPLIER,
	CH_ERROR_INVALID_CALIBRATION,
	CH_ERROR_SELF_TEST_EEPROM = 35,
	CH_ERROR_LAST
} ChError;

#endif
