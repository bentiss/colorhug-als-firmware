/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2011-2015 Richard Hughes <richard@hughsie.com>
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
 *
 * Additionally, some constants and code snippets have been taken from
 * freely available datasheets which are:
 *
 * Copyright (C) Microchip Technology, Inc.
 */

#include "ColorHug.h"
#include "HardwareProfile.h"
#include "usb_config.h"
#include "ch-common.h"
#include "ch-flash.h"

#include <delays.h>
#include <USB/usb.h>
#include <USB/usb_common.h>
#include <USB/usb_device.h>
#include <USB/usb_function_hid.h>

/**
 * ISRCode:
 **/
void interrupt
ISRCode(void)
{
}

static uint16_t		SensorIntegralTime = 0xffff;
static ChFreqScale	multiplier_old = CH_FREQ_SCALE_0;

/* this is used to map the firmware to a hardware version */
static const char flash_id[] = CH_FIRMWARE_ID_TOKEN;

/* USB idle support */
static uint8_t		idle_command = 0x00;
static uint16_t		idle_counter = 0x00;

/* USB buffers */
static uint8_t TxBuffer[CH_USB_HID_EP_SIZE];
USB_HANDLE		USBOutHandle = 0;
USB_HANDLE		USBInHandle = 0;

/**
 * CHugTakeReadingRaw:
 *
 * The TAOS3200 sensor with the external IR filter gives the following rough
 * outputs with red selected at 100%:
 *
 *   Frequency   |   Condition
 * --------------|-------------------
 *    10Hz       | Aperture covered
 *    160Hz      | TFT backlight on, but masked to black
 *    1.24KHz    | 100 white at 170cd/m2
 **/
static uint32_t
CHugTakeReadingRaw (uint32_t integral_time)
{
	const uint8_t abs_scale[] = {  5, 5, 7, 6 }; /* red, white, blue, green */
	uint32_t i;
	uint32_t number_edges = 0;
	uint32_t value;
	uint8_t ra_tmp = PORTA;

	/* wait for the output to change so we start on a new pulse
	 * rising edge, which means more accurate black readings */
	for (i = 0; i < integral_time; i++) {
		if (ra_tmp != PORTA) {
			/* ___      ____
			 *    |____|    |___
			 *
			 *         ^- START HERE
			 */
			if (PORTAbits.RA4 == 1)
				break;
			ra_tmp = PORTA;
		}
	}

	/* we got no change */
	if (i == integral_time)
		return 0;

	/* count how many times we get a rising edge */
	for (i = 0; i < integral_time; i++) {
		if (ra_tmp != PORTA) {
			if (PORTAbits.RA4 == 1) {
				number_edges++;
				/* overflow */
				if (number_edges == 0)
					return UINT32_MAX;
			}
			ra_tmp = PORTA;
		}
	}

	/* scale it according to the datasheet */
	ra_tmp = CHugGetColorSelect();
	value = number_edges * abs_scale[ra_tmp];

	/* overflow */
	if (value < number_edges)
		return UINT32_MAX;

	return number_edges;
}

/**
 * CHugDeviceIdle:
 **/
static void
CHugDeviceIdle(void)
{
	switch (idle_command) {
	case CH_CMD_RESET:
		RESET();
		break;
	}
	idle_command = 0x00;
}

/**
 * ProcessIO:
 **/
static void
ProcessIO(void)
{
	uint32_t reading;

	/* User Application USB tasks */
	if ((USBDeviceState < CONFIGURED_STATE) ||
	    (USBSuspendControl == 1))
		return;

	/* prevent to send too many readings */
	if (idle_counter++ != 0x00)
		return;

	if(!HIDTxHandleBusy(USBInHandle)) {
		/* clear for debugging */
		memset (TxBuffer, 0xff, sizeof (TxBuffer));

		CHugSetMultiplier(CH_FREQ_SCALE_100);

		reading = CHugTakeReadingRaw(SensorIntegralTime);
		memcpy (&TxBuffer[CH_BUFFER_OUTPUT_DATA],
			(const void *) &reading,
			sizeof(uint32_t));

		TxBuffer[CH_BUFFER_OUTPUT_RETVAL] = 01;
		TxBuffer[CH_BUFFER_OUTPUT_CMD] = 01;
		USBInHandle = HIDTxPacket(HID_EP,
					  (BYTE*)&TxBuffer[0],
					  CH_USB_HID_EP_SIZE);
		CHugSetMultiplier(CH_FREQ_SCALE_0);
	}
}

/**
 * USER_USB_CALLBACK_EVENT_HANDLER:
 * @event: the type of event
 * @pdata: pointer to the event data
 * @size: size of the event data
 *
 * This function is called from the USB stack to
 * notify a user application that a USB event
 * occured.  This callback is in interrupt context
 * when the USB_INTERRUPT option is selected.
 **/
BOOL
USER_USB_CALLBACK_EVENT_HANDLER(int event, void *pdata, WORD size)
{
	switch(event) {
	case EVENT_TRANSFER:
		break;
	case EVENT_SUSPEND:
		/* need to reduce power to < 2.5mA, so power down sensor */
		multiplier_old = CHugGetMultiplier();
		CHugSetMultiplier(CH_FREQ_SCALE_0);

		/* power down LEDs */
		CHugSetLEDs(0);
		break;
	case EVENT_RESUME:
		/* restore full power mode */
		CHugSetMultiplier(multiplier_old);
		break;
	case EVENT_CONFIGURED:
		/* enable the HID endpoint */
		USBEnableEndpoint(HID_EP,
				  USB_IN_ENABLED|
				  USB_HANDSHAKE_ENABLED|
				  USB_DISALLOW_SETUP);
		break;
	case EVENT_EP0_REQUEST:
		USBCheckHIDRequest();
		break;
	case EVENT_TRANSFER_TERMINATED:
		break;
	default:
		break;
	}
	return TRUE;
}

/**
 * main:
 **/
void
main(void)
{
	uint8_t i;

	/* The USB module will be enabled if the bootloader has booted,
	 * so we soft-detach from the host. */
	if(UCONbits.USBEN == 1) {
		UCONbits.SUSPND = 0;
		UCON = 0;
		Delay10KTCYx(0xff);
	}

	/* set some defaults to power down the sensor */
	CHugSetLEDs(0);
	CHugSetColorSelect(CH_COLOR_SELECT_WHITE);
	CHugSetMultiplier(CH_FREQ_SCALE_0);

	/* Initializes USB module SFRs and firmware variables to known states */
	USBDeviceInit();
	USBDeviceAttach();

	/* do the welcome flash */
	for (i = 0; i < 3; i++) {
		CHugSetLEDs(1);
		Delay10KTCYx(0x5f);
		CHugSetLEDs(0);
		Delay10KTCYx(0x5f);
	}

	/* convince the compiler it's actually used */
	if (flash_id[0] == '\0')
		CHugFatalError(CH_ERROR_WRONG_UNLOCK_CODE);

	while(1) {

		/* clear watchdog */
		CLRWDT();

		/* check bus status and service USB interrupts */
		USBDeviceTasks();

		ProcessIO();
	}
}
