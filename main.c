/*
 * Copyright (C) 2013 Plastic Logic Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
/*
 * main.c -- main entry point for code.
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include "assert.h"
#include "types.h"
#include "main.h"
#include "FatFs/ff.h"
#include "plat-hbz13.h"
#include "plat-cuckoo.h"
#include "plat-hbz6.h"
#include "plat-raven.h"
#include "plat-ruddock2.h"

/* Select one of the platforms below, and if necessary a display type
 */
#define PLAT_CUCKOO				0	// 524 controller, 10.7" Type-4
#define PLAT_RAVEN				0	// 524 controller, 10.7" Type-11

/* These platforms can drive a range of small displays as defined below */
#define	PLAT_Z13				0	// 541 controller, small display
#define PLAT_Z6					0 	// 541 controller, small display
//#define	CONFIG_DISPLAY_TYPE		"0:/Type-16"
//#define	CONFIG_DISPLAY_TYPE		"0:/Type-18"

#define PLAT_Z7					0 	// 541 controller, small display
//#define	CONFIG_DISPLAY_TYPE		"0:/Type-19"

#define	CONFIG_I2C_ON_EPSON		0

// global file system information used by FatFs
static FATFS Sd_Card;

static int sdcard_init(void)
{
	f_chdrive(0);
	return (f_mount(0,&Sd_Card) == FR_OK ? 0 : -EACCES);
}

int app_main(void)
{
	/* initialise the Ruddock2 motherboard */
	ruddock2_init();

	/* ready the SD card support */
	check(sdcard_init() == 0);

	/* initialise the desired platform */
#if PLAT_CUCKOO
	plat_cuckoo_init();
#elif PLAT_Z13
	plat_hbz13_init(CONFIG_DISPLAY_TYPE, CONFIG_I2C_ON_EPSON);
#elif PLAT_Z6 || PLAT_Z7
	plat_hbZn_init(CONFIG_DISPLAY_TYPE, CONFIG_I2C_ON_EPSON);
#elif PLAT_RAVEN
	plat_raven_init();
#else
#error No hardware platform/display type selected
#endif

	/* Do not return from app_main */
	do {
		udelay(1);
	} while(1);
}

