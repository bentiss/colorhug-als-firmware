/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2012 Richard Hughes <richard@hughsie.com>
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

// compile with: gcc -o ch-hid ch-hid.c -lhid -lusb-1.0

#include <hid.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../ColorHug.h"

#define	CH_USB_VID				0x273f
#define	CH_USB_PID_FIRMWARE			0x1017

int main (int argc, char *argv[])
{
	char buffer[64];
	struct sensor_settings sensor_settings;
	struct system_settings system_settings;
	HIDInterface* hid;
	HIDInterfaceMatcher matcher = { CH_USB_VID, CH_USB_PID_FIRMWARE, NULL, NULL, 0 };
	hid_return ret;
	int i;
	int iface_num = 0;
	int length;
	unsigned int timeout = 1000; /* ms */

	const int path_sensor_settings[] = {0x00200001, 0x00200041, 0xffc00002};
	const int path_system_settings[] = {0x00200001, 0x00200041, 0xffc00003};

	hid_set_debug (HID_DEBUG_ALL);
	hid_set_debug_stream (stderr);
	hid_set_usb_debug (0);

	ret = hid_init ();
	if (ret != HID_RET_SUCCESS) {
		fprintf (stderr, "hid_init failed with return code %d\n", ret);
		return 1;
	}

	hid = hid_new_HIDInterface ();
	if (hid == 0) {
		fprintf (stderr, "hid_new_HIDInterface () failed\n");
		return 1;
	}

	ret = hid_force_open (hid, iface_num, &matcher, 3);
	if (ret != HID_RET_SUCCESS) {
		fprintf (stderr, "hid_force_open failed with return code %d\n", ret);
		return 1;
	}

	ret = hid_write_identification (stdout, hid);
	if (ret != HID_RET_SUCCESS) {
		fprintf (stderr, "hid_write_identification failed with return code %d\n", ret);
		return 1;
	}

	ret = hid_dump_tree (stdout, hid);
	if (ret != HID_RET_SUCCESS) {
		fprintf (stderr, "hid_dump_tree failed with return code %d\n", ret);
		return 1;
	}

	/* read the sensor feature report */
	length = sizeof(sensor_settings);
	ret = hid_get_feature_report(hid, path_sensor_settings, 3, (char *)&sensor_settings, length);
	if (ret != HID_RET_SUCCESS) {
		printf ("hid_get_feature_report failed\n");
		return 1;
	}
	printf ("Sensor feature received from USB device: ");
	for (i=0; i < length; i++) {
		printf ("0x%02x, ", ((char *)&sensor_settings)[i]);
	}
	printf ("\n");

	/* switch the LED and send the feature back */
	sensor_settings.LEDs_state = !sensor_settings.LEDs_state;
	ret = hid_set_feature_report(hid, path_sensor_settings, 3, (char *)&sensor_settings, length);
	if (ret != HID_RET_SUCCESS) {
		printf ("hid_set_feature_report failed\n");
		return 1;
	}

	/* read one input report */
	length = 7;
	ret = hid_interrupt_read (hid, 0x01, buffer, length, timeout);
	if (ret != HID_RET_SUCCESS) {
		printf ("hid_interrupt_read failed\n");
		return 1;
	}
	printf ("Data received from USB device: ");
	for (i=0; i < length; i++) {
		printf ("0x%02x, ", buffer[i]);
	}
	printf ("\n");

	if (argc > 1) {
		/* read the system feature report */
		length = sizeof(struct system_settings);
		ret = hid_get_feature_report(hid, path_system_settings, 3, (char *)&system_settings, length);
		if (ret != HID_RET_SUCCESS) {
			printf ("hid_get_feature_report failed\n");
			return 1;
		}
		printf ("System feature received from USB device: ");
		for (i=0; i < length; i++) {
			printf ("0x%02x, ", ((char *)&system_settings)[i]);
		}
		printf ("\n");

		/* set the Reset setting */
		system_settings.reset = 0x01;

		/* set the Flash success flag */
//		system_settings.flash_flag = 0x01;

		ret = hid_set_feature_report(hid, path_system_settings, 3, (char *)&system_settings, length);
		if (ret != HID_RET_SUCCESS) {
			printf ("hid_set_feature_report failed\n");
			return 1;
		}
	}

	ret = hid_close (hid);
	if (ret != HID_RET_SUCCESS) {
		fprintf (stderr, "hid_close failed with return code %d\n", ret);
		return 1;
	}

	hid_delete_HIDInterface (&hid);

	ret = hid_cleanup ();
	if (ret != HID_RET_SUCCESS) {
		fprintf (stderr, "hid_cleanup failed with return code %d\n", ret);
		return 1;
	}

	return 0;
}
