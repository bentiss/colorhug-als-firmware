/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2015 Benjamin Tissoires <benjamin.tissoires@gmail.com>
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
 * This code is based on <kernel-tree>/samples/hidraw/hid-example.c,
 * copyrighted as such:
 *
 * Copyright (c) 2010 Alan Ott <alan@signal11.us>
 * Copyright (c) 2010 Signal 11 Software
 *
 */

// compile with: gcc -Wall -o hid-example hid-example.c

/* Linux */
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>

/*
 * Ugly hack to work around failing compilation on systems that don't
 * yet populate new version of hidraw.h to userspace.
 */
#ifndef HIDIOCSFEATURE
#warning Please have your distro update the userspace kernel headers
#define HIDIOCSFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x06, len)
#define HIDIOCGFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x07, len)
#endif

/* Unix */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

/* C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "../ColorHug.h"

const char *bus_str(int bus);

int main(int argc, char **argv)
{
	int fd;
	int i, res, desc_size = 0;
	char buf[256];
	struct hidraw_report_descriptor rpt_desc;
	struct hidraw_devinfo info;
	char *device = "/dev/hidraw0";
	struct sensor_settings sensor_settings;
	struct system_settings system_settings;
	char *feature;

	if (argc > 1)
		device = argv[1];

	/* Open the Device with non-blocking reads. In real life,
	   don't use a hard coded path; use libudev instead. */
	fd = open(device, O_RDWR|O_NONBLOCK);

	if (fd < 0) {
		perror("Unable to open device");
		return 1;
	}

	memset(&rpt_desc, 0x0, sizeof(rpt_desc));
	memset(&info, 0x0, sizeof(info));
	memset(buf, 0x0, sizeof(buf));

	/* Get Report Descriptor Size */
	res = ioctl(fd, HIDIOCGRDESCSIZE, &desc_size);
	if (res < 0)
		perror("HIDIOCGRDESCSIZE");
	else
		printf("Report Descriptor Size: %d\n", desc_size);

	/* Get Report Descriptor */
	rpt_desc.size = desc_size;
	res = ioctl(fd, HIDIOCGRDESC, &rpt_desc);
	if (res < 0) {
		perror("HIDIOCGRDESC");
	} else {
		printf("Report Descriptor:\n");
		for (i = 0; i < rpt_desc.size; i++)
			printf("%hhx ", rpt_desc.value[i]);
		puts("\n");
	}

	/* Get Raw Name */
	res = ioctl(fd, HIDIOCGRAWNAME(256), buf);
	if (res < 0)
		perror("HIDIOCGRAWNAME");
	else
		printf("Raw Name: %s\n", buf);

	/* Get Physical Location */
	res = ioctl(fd, HIDIOCGRAWPHYS(256), buf);
	if (res < 0)
		perror("HIDIOCGRAWPHYS");
	else
		printf("Raw Phys: %s\n", buf);

	/* Get Raw Info */
	res = ioctl(fd, HIDIOCGRAWINFO, &info);
	if (res < 0) {
		perror("HIDIOCGRAWINFO");
	} else {
		printf("Raw Info:\n");
		printf("\tbustype: %d (%s)\n",
			info.bustype, bus_str(info.bustype));
		printf("\tvendor: 0x%04hx\n", info.vendor);
		printf("\tproduct: 0x%04hx\n", info.product);
	}

	/* Get Feature */
	sensor_settings.report_id = CH_REPORT_SENSOR_SETTINGS;
	res = ioctl(fd, HIDIOCGFEATURE(sizeof(sensor_settings)), &sensor_settings);
	if (res < 0) {
		perror("HIDIOCGFEATURE");
	} else {
		printf("ioctl HIDIOCGFEATURE returned: %d\n", res);
		printf("Report data (not containing the report number):\n\t");
		feature = (char *)&sensor_settings;
		for (i = 0; i < res; i++)
			printf("%hhx ", feature[i]);
		puts("\n");
	}

	/* invert the LED setting */
	sensor_settings.LEDs_state = !sensor_settings.LEDs_state;

	/* Set Feature */
	res = ioctl(fd, HIDIOCSFEATURE(sizeof(sensor_settings)), &sensor_settings);
	if (res < 0)
		perror("HIDIOCSFEATURE");
	else
		printf("ioctl HIDIOCGFEATURE returned: %d\n", res);

	if (argc > 2) {
		/* Get Feature */
		system_settings.report_id = CH_REPORT_SYSTEM_SETTINGS;
		res = ioctl(fd, HIDIOCGFEATURE(sizeof(system_settings)), &system_settings);
		if (res < 0) {
			perror("HIDIOCGFEATURE");
		} else {
			printf("ioctl HIDIOCGFEATURE returned: %d\n", res);
			printf("Report data (not containing the report number):\n\t");
			feature = (char *)&system_settings;
			for (i = 0; i < res; i++)
				printf("%hhx ", feature[i]);
			puts("\n");
		}

		/* set the Reset setting */
		system_settings.reset = 0x01;

		/* set the Flas success flag */
//		system_settings.flash_flag = 0x01;

		/* Set Feature */
		res = ioctl(fd, HIDIOCSFEATURE(sizeof(system_settings)), &system_settings);
		if (res < 0)
			perror("HIDIOCSFEATURE");
		else
			printf("ioctl HIDIOCGFEATURE returned: %d\n", res);
	}

	close(fd);
	return 0;
}

const char *
bus_str(int bus)
{
	switch (bus) {
	case BUS_USB:
		return "USB";
		break;
	case BUS_HIL:
		return "HIL";
		break;
	case BUS_BLUETOOTH:
		return "Bluetooth";
		break;
	case BUS_VIRTUAL:
		return "Virtual";
		break;
	default:
		return "Other";
		break;
	}
}
