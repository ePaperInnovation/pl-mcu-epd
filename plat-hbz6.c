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
 * plat-hbz6.c -- Plastic Logic Hummingbird Z6 adapter
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 * Hummingbird HB Z6 platform intialisation. Can drive either:
 * Need to define
 * - i2c interface (either on CPU or the Epson controller)
 * - power sequence control pins
 * - epson controller
 * - TI Pmic - no temperature sensor fitted
 * - Uses internal display temperature sensor
 *
 */

#include "platform.h"
#include <stdio.h>
#include "types.h"
#include "assert.h"
#include "msp430-gpio.h"
#include "i2c.h"
#include "vcom.h"
#include "msp430-i2c.h"
#include "FatFs/ff.h"
#include "pmic-tps65185.h"
#include "epson/S1D13541.h"
#include "epson/epson-i2c.h"
#include "epson/epson-utils.h"
#include "epson/epson-if.h"
#include "slideshow.h"
#include "i2c-eeprom.h"
#include "psu-data.h"

#define CONFIG_PLAT_RUDDOCK2	1
#if CONFIG_PLAT_RUDDOCK2
#define B_HWSW_CTRL	GPIO(1,2)
#define B_POK		GPIO(1,0)
#define B_PMIC_EN	GPIO(1,1)
#define	EPSON_CS_0	GPIO(3,6)

#else
#endif


/* 1 to write default value to eeprom on read failure (testing only) */
#define	CONFIG_PSU_WRITE_DEFAULTS	0
/* 1 to cycle power supplies only, no display update (testing only) */
#define	CONFIG_PSU_ONLY				0

/* i2c addresses of TI PMIC and calibration data EEPROM */
#define I2C_PMIC_ADDR		0x68
#define	I2C_EEPROM_PSU_DATA	0x50

static struct tps65185_info *pmic_info;
static struct i2c_adapter *i2c;
static struct s1d135xx *epson;
static struct vcom_cal *vcom_calibration;
static struct vcom_info vcom_data;
static struct i2c_eeprom *eeprom;
static struct eeprom_data *psu_data;

static int show_image(char *image, void *arg);

/* Fallback VCOM calibration data if PSU EEPROM corrupt */
static struct vcom_info psu_calibration = {
	.dac_x1 = 127,
	.dac_y1 = 4172,
	.dac_x2 = 381,
	.dac_y2 = 12490,
	.vgpos_mv = 25080,
	.vgneg_mv = -32300,
};
#define VCOM_VGSWING 56886

/* Board specific power up control */
static int power_up(void)
{
	printk("Powering up\n");
	gpio_set_value(B_HWSW_CTRL, false);
	gpio_set_value(B_PMIC_EN, true);

	do {
		mdelay(1);
	} while (gpio_get_value(B_POK) == 0);

	gpio_set_value(B_HWSW_CTRL, true);

	return 0;
}

/* Board specific power down control */
static int power_down(void)
{
	gpio_set_value(B_HWSW_CTRL, false);
	gpio_set_value(B_PMIC_EN, false);

	printk("Powered down\n");

	return 0;
}

/* Initialise the Hummingbird Z[6|7].x platform */
int plat_hbZn_init(const char *platform_path, int i2c_on_epson)
{
	int done = 0;
	int ret = 0;
	short previous;
	int vcom;

	printk("HB Z6/7 platform initialisation\n");

	check(f_chdir(platform_path) == FR_OK);

	/* read the display vcom */
	vcom = util_read_vcom();
	assert(vcom > 0);

	/* initialise the Epson interface */
	epsonif_init(0, 1);

	/* define gpio's required for operation */
	ret |= gpio_request(B_HWSW_CTRL,PIN_GPIO | PIN_OUTPUT | PIN_INIT_LOW);
	ret |= gpio_request(B_PMIC_EN,	PIN_GPIO | PIN_OUTPUT | PIN_INIT_LOW);
	ret |= gpio_request(B_POK, 	  	PIN_GPIO | PIN_INPUT);
	ret |= gpio_request(EPSON_CS_0,	PIN_GPIO | PIN_OUTPUT | PIN_INIT_HIGH);

	if (ret)
		return -EBUSY;

#if !CONFIG_PSU_ONLY
	/* initialise the Epson controller */
	check(s1d13541_init(EPSON_CS_0, &epson) == 0);
#endif

	/* initialise the i2c interface as required */
	if (i2c_on_epson)
		check(epson_i2c_init(epson, &i2c) == 0);
	else
		check(msp430_i2c_init(0, &i2c) == 0);

	/* intialise the psu EEPROM */
	eeprom_init(i2c, I2C_EEPROM_PSU_DATA, EEPROM_24LC014, &eeprom);

	/* read the psu calibration data and ready it for use */
	psu_data_init(&psu_data);
	psu_data_read(eeprom, psu_data);
	if (psu_data_get_vcom_data(psu_data, &vcom_data) == 0) {
		vcom_init(&vcom_data, VCOM_VGSWING, &vcom_calibration);
	}
	else {
		printk("Using power supply defaults\n");
#if CONFIG_PSU_WRITE_DEFAULTS
		printk("Writing default psu data\n");
		psu_data_set_header_version(psu_data, 0);
		psu_data_set_board_info(psu_data, PSU_HB_Z6);
		psu_data_set_vcom_data(psu_data, &psu_calibration);
		psu_data_write(eeprom, psu_data);
#endif
		vcom_init(&psu_calibration, VCOM_VGSWING, &vcom_calibration);
	}
	psu_data_free(&psu_data);

	/* select the controller for future operations */
	s1d135xx_select(epson, &previous);

	/* measure temperature using internal sensor */
	s1d13541_set_temperature_mode(epson, TEMP_MODE_INTERNAL);

	/* initialise the PMIC and pass it the vcom calibration data */
	tps65185_init(i2c, I2C_PMIC_ADDR, &pmic_info);
	tps65185_configure(pmic_info, vcom_calibration);

	tps65185_set_vcom_voltage(pmic_info, vcom);

#if CONFIG_PSU_ONLY
	while (1) {
		/* always power down first incase we leave HV on */
		power_down();
		power_up();
	}
#endif

	/* initialise display */
	epson_fill_buffer(0x0030, false, epson->yres, epson->xres, 0xff);
	s1d13541_init_display(epson);
	power_up();
	s1d13541_update_display(epson, 0);
	power_down();

	/* run the slideshow */
	while(!done) {
		slideshow_run("img", show_image, NULL);
	}

	s1d135xx_deselect(epson, previous);

	return 0;
}

static int show_image(char *image, void *arg)
{
	u8 needs_update;

	/* Ask Epson to determine if waveform needs reloading */
	s1d13541_measure_temperature(epson, &needs_update);
	if (needs_update)
	{
		s1d13541_send_waveform();
	}

	printk("Load: %s\n", image);
	slideshow_load_image(image, 0x0030, false);

	power_up();
	s1d13541_update_display(epson, 0 /*1*/);
	power_down();

	return 0;
}

