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
 * plat-cuckoo.c -- Platform file for the 524, Type4 10.7" display electronics
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 * Uses 524 controller - With Hynix SDRAM
 * Drives Type4 display only
 * Maxim 17135 PMIC - temperature sensor fitted
 * Epson Power pins control PSU power up/VCOM
 * Epson i2c bridge used to talk to PMIC/DAC
 * VCOM controlled by external MAX5820 DAC
 *
 */

#include <pl/i2c.h>
#include <stdio.h>
#include "platform.h"
#include "types.h"
#include "assert.h"
#include "FatFs/ff.h"
#include "msp430-gpio.h"
#include "vcom.h"
#include "epson/epson-cmd.h"
#include "epson/S1D13524.h"
#include "epson/epson-i2c.h"
#include "pmic-max17135.h"
#include "dac-5820.h"
#include "epson/epson-utils.h"
#include "slideshow.h"

#define	EPSON_CS_0		GPIO(3,6)

/* i2c addresses for MAXIM PMIC and DAC */
#define I2C_DAC_ADDR	0x39
#define I2C_PMIC_ADDR	0x48

#define CONFIG_TEMP_SENSE_MODE	1

static int show_image(const char *image, void *arg);

static struct pl_i2c i2c;
static struct dac5820_info *dac_info;
static struct max17135_info *pmic_info;
static struct s1d135xx *epson;
static struct vcom_cal vcom_calibration;

/* No PSU calibration data so always use defaults */
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

	dac5820_write(dac_info);
	dac5820_set_power(dac_info, true);

	/* use Epson power pins to power up */
	epson_power_up();

	return 0;
}

static int power_down(void)
{
	/* use Epson power pins to power down */
	epson_power_down();
	dac5820_set_power(dac_info, false);

	printk("Powered down\n");

	return 0;
}

/* Initialise the platform */
int plat_cuckoo_init(struct platform *plat)
{
	int done = 0;
	screen_t previous;
	int vcom;

	printk("Cuckoo platform initialisation\n");

	/* all file operations will be within the Type4 subtree */
	check(f_chdir("0:/Type4") == 0);

	/* read the display vcom */
	vcom = util_read_vcom();
	assert(vcom > 0);

	/* initialise the Epson interface */
	/* Note slower clock speed because of long cables in wiring harness. */
	epsonif_init(0, 2);

	/* define gpio's required for operation */
	check(gpio_request(EPSON_CS_0,	PIN_GPIO | PIN_OUTPUT | PIN_INIT_HIGH) == 0);

	/* initialise the Epson controller */
	check(s1d13524_init(EPSON_CS_0, &epson)==0);

	/* initialise the i2c interface on the epson */
	check(epson_i2c_init(epson, &i2c)==0);

	/* initialise the vcom calibration data */
	vcom_init(&vcom_calibration, &psu_calibration, VCOM_VGSWING);

	/* intialise the PMIC */
	max17135_init(&i2c, I2C_PMIC_ADDR, &pmic_info);
	max17135_configure(pmic_info, NULL, MAX17135_SEQ_0);
	max17135_temp_enable(pmic_info);

	/* intialise the DAC and pass it the vcom calibration data */
	dac5820_init(&i2c, I2C_DAC_ADDR, &dac_info);
	dac5820_configure(dac_info, &vcom_calibration);

	dac5820_set_voltage(dac_info, vcom);

	/* select the controller for future operations */
	check(s1d135xx_select(epson, &previous)==0);

	/* measure temperature manually */
	s1d13524_set_temperature_mode(epson, TEMP_MODE_MANUAL);

	/* clear the display */
	power_up();
	s1d13524_update_display(epson, s1d135xx_get_wfid(wf_init));
	power_down();

	/* run the slideshow */
	while(!done) {
		slideshow_run("img", show_image, NULL);
	}

	s1d135xx_deselect(epson, previous);

	return 0;
}

static int show_image(const char *image, void *arg)
{
#if CONFIG_TEMP_SENSE_MODE == 1
	short measured;

	max17135_temperature_measure(pmic_info, &measured);
	s1d13524_set_temperature(epson, measured);
	s1d13524_measure_temperature(epson);
#endif

	printk("Load: %s\n", image);
	slideshow_load_image(image, 0x0000, true);

	power_up();
	s1d13524_update_display(epson, s1d135xx_get_wfid(wf_refresh));
	power_down();

	return 0;
}

