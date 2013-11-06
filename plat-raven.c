/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013 Plastic Logic Limited

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
 * plat-raven.c -- Platform file for the Raven 10.7" display electronics
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 * Uses 524 controller - With Micron SD RAM
 * Uses MAXIM PMIC - no temperature sensor fitted
 * Drives Type-11 display only
 * Epson Power pins control PSU power up/VCOM
 * Epson i2c bridge used to talk to PMIC/DAC
 * Has LM75 Temperature sensor
 * Has no ADC or VCOM DAC
 * VCOM controlled by VCOM DAC in PMIC
 * Has 5 Epson gpio bits brought out on test pads
 *
 */

#include "platform.h"
#include <stdio.h>
#include "types.h"
#include "assert.h"
#include "FatFs/ff.h"
#include "msp430-gpio.h"
#include "i2c.h"
#include "vcom.h"
#include "i2c-eeprom.h"
#include "psu-data.h"
#include "epson/epson-cmd.h"
#include "epson/S1D13524.h"
#include "epson/epson-i2c.h"
#include "pmic-max17135.h"
#include "temp-lm75.h"
#include "epson/epson-utils.h"
#include "slideshow.h"

#define	EPSON_CS_0		GPIO(3,6)

/* i2c addresses of Maxim PMIC, PSU data EEPROM and temp sensor */
#define I2C_PMIC_ADDR		0x48
#define	I2C_EEPROM_PSU_DATA	0x50
#define	I2C_TEMP_SENSOR		0x49

/* 0 no temperature sensing
 * 1 manual temperature sending
 * 2 automatic temperature sensing
 */
#define CONFIG_TEMP_SENSE_MODE		1
#define	CONFIG_PSU_WRITE_DEFAULTS	0

static int show_image(char *image, void *arg);

static struct i2c_adapter *i2c;
static struct max17135_info *pmic_info;
static struct s1d135xx *epson;
static struct lm75_info *lm75_info;
static struct vcom_cal *vcom_calibration;
static struct i2c_eeprom *eeprom;
static struct eeprom_data *psu_data;
static struct vcom_info vcom_data;

/* Fallback VCOM calibration data if PSU EEPROM corrupt */
static struct vcom_info psu_calibration = {
	.dac_x1 = 63,
	.dac_y1 = 4586,
	.dac_x2 = 189,
	.dac_y2 = 9800,
	.vgpos_mv = 27770,
	.vgneg_mv = -41520,
};
#define VCOM_VGSWING 70000

static int power_up(void)
{
	printk("Powering up\n");

	// use Epson power pins to power up
	epson_power_up();

	return 0;
}

static int power_down(void)
{
	// use Epson power pins to power down
	epson_power_down();

	printk("Powered down\n");

	return 0;
}

/* Initialise the platform */
int plat_raven_init(void)
{
	int done = 0;
	screen_t previous;
	int vcom;

	printk("Raven platform initialisation\n");

	/* all file operations will be within the Type-11 subtree */
	check(f_chdir("0:/Type-11") == 0);

	/* read the display vcom */
	vcom = util_read_vcom();
	assert(vcom > 0);

	/* initialise the Epson interface */
	epsonif_init(0, 1);

	/* define gpio's required for operation */
	check(gpio_request(EPSON_CS_0,	PIN_GPIO | PIN_OUTPUT | PIN_INIT_HIGH) == 0);

	/* initialise the Epson controller */
	check(s1d13524_init(EPSON_CS_0, &epson)==0);

	/* initialise the i2c interface on the epson */
	check(epson_i2c_init(epson, &i2c)==0);

	/* intialise the psu EEPROM */
	eeprom_init(i2c, I2C_EEPROM_PSU_DATA, EEPROM_24LC014, &eeprom);

	/* read the calibration data and ready it for use */
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

	/* measure temperature manually */
	s1d13524_set_temperature_mode(epson, TEMP_MODE_MANUAL);

	/* intialise the PMIC and pass it the vcom calibration data */
	max17135_init(i2c, I2C_PMIC_ADDR, &pmic_info);
	max17135_configure(pmic_info, vcom_calibration, MAX17135_SEQ_1);
	max17135_set_vcom_voltage(pmic_info, vcom);

	/* initialise the i2c temperature sensor */
	lm75_init(i2c, I2C_TEMP_SENSOR, &lm75_info);

	power_up();
	s1d13524_update_display(epson, 0);
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
#if CONFIG_TEMP_SENSE_MODE == 1
	short measured;

	lm75_temperature_measure(lm75_info, &measured);
	s1d13524_set_temperature(epson, measured);
	s1d13524_measure_temperature(epson);
#endif

	printk("Load: %s\n", image);
	slideshow_load_image(image, 0x0000, true);

	power_up();
	s1d13524_update_display(epson, 2);
	power_down();

	return 0;
}
