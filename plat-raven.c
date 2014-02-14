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
 * Drives Type11 display only
 * Epson Power pins control PSU power up/VCOM
 * Epson i2c bridge used to talk to PMIC/DAC
 * Has LM75 Temperature sensor
 * Has no ADC or VCOM DAC
 * VCOM controlled by VCOM DAC in PMIC
 * Has 5 Epson gpio bits brought out on test pads
 *
 */

#include <pl/platform.h>
#include <pl/gpio.h>
#include <pl/i2c.h>
#include <pl/hwinfo.h>
#include <stdio.h>
#include "types.h"
#include "assert.h"
#include "FatFs/ff.h"
#include "msp430-gpio.h"
#include "vcom.h"
#include "i2c-eeprom.h"
#include "epson/epson-cmd.h"
#include "epson/S1D13524.h"
#include "epson/epson-i2c.h"
#include "pmic-max17135.h"
#include "temp-lm75.h"
#include "epson/epson-utils.h"
#include "slideshow.h"
#include "utils.h"
#include "plat-raven.h"

#define LOG_TAG "raven"

#define	EPSON_CS_0              MSP430_GPIO(3,6)

/* i2c addresses of Maxim PMIC, PSU data EEPROM and temp sensor */
#define I2C_PMIC_ADDR		0x48
#define	I2C_TEMP_SENSOR		0x49

/* 0 no temperature sensing
 * 1 manual temperature sending
 * 2 automatic temperature sensing
 */
#define CONFIG_TEMP_SENSE_MODE		1
#define	CONFIG_PSU_WRITE_DEFAULTS	0

static int show_image(const char *image, void *arg);

static struct pl_i2c i2c;
static struct max17135_info *pmic_info;
static struct s1d135xx *epson;
static struct lm75_info *lm75_info;
static struct vcom_cal vcom_calibration;
static struct pl_hw_info pl_hw_info;

/* Fallback VCOM calibration data if PSU EEPROM corrupt */
static struct pl_hw_vcom_info def_vcom_info = {
	.dac_x1 = 63,
	.dac_y1 = 4586,
	.dac_x2 = 189,
	.dac_y2 = 9800,
	.vgpos_mv = 27770,
	.vgneg_mv = -41520,
	.swing_ideal = 70000,
};

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

#if 0
/* Initialise the platform */
int plat_raven_init(struct platform *plat)
{
	screen_t previous;
	int vcom;

	LOG("Raven platform initialisation\n");

	/* all file operations will be within the Type11 subtree */
	check(f_chdir("0:/Type11") == 0);

#if 0
	/* read the display vcom */
	vcom = util_read_vcom();
	assert(vcom > 0);
#endif

#if 0
	/* initialise the Epson interface */
	epsonif_init(&plat->gpio, 0, 1);
#endif

	s1d135xx_set_wfid_table(EPDC_S1D13524);

	/* define GPIOs required for operation */
	if (plat->gpio.config(EPSON_CS_0, PL_GPIO_OUTPUT | PL_GPIO_INIT_H))
		return -1;

	/* initialise the Epson controller */
	check(s1d13524_init(EPSON_CS_0, &epson)==0);

	/* initialise the i2c interface on the epson */
	check(epson_i2c_init(epson, &i2c)==0);

#if CONFIG_HW_INFO_EEPROM
	/* read the psu calibration data and ready it for use */
	if (pl_hw_info_init(&pl_hw_info, &plat->hw_eeprom)) {
#if CONFIG_HW_INFO_DEFAULT
		LOG("WARNING: Using hard-coded default VCOM PSU values");
		pl_hw_info.version = PL_HW_INFO_VERSION;
		memcpy(&pl_hw_info.vcom, &def_vcom_info,
		       sizeof pl_hw_info.vcom);
#else
		abort_msg("Failed to initialise VCOM PSU data from EEPROM");
#endif
	}
#else /* !CONFIG_HW_INFO_EEPROM */
	pl_hw_info.version = PL_HW_INFO_VERSION;
	memcpy(&pl_hw_info.vcom, &def_vcom_info, sizeof pl_hw_info.vcom);
#endif

	vcom_init(&vcom_calibration, &pl_hw_info.vcom);

	/* select the controller for future operations */
	s1d135xx_select(epson, &previous);

	/* measure temperature manually */
	s1d13524_set_temperature_mode(epson, TEMP_MODE_MANUAL);

	/* intialise the PMIC and pass it the vcom calibration data */
	max17135_init(&i2c, I2C_PMIC_ADDR, &pmic_info);
	max17135_configure(pmic_info, &vcom_calibration, MAX17135_SEQ_1);
	max17135_set_vcom_voltage(pmic_info, vcom);

	/* initialise the i2c temperature sensor */
	lm75_init(&i2c, I2C_TEMP_SENSOR, &lm75_info);

	plat_s1d13524_init_display(epson);
	plat_s1d13524_slideshow(epson);

	s1d135xx_deselect(epson, previous);

	return 0;
}
#endif /* 0 */

int plat_s1d13524_init_display(struct s1d135xx *epson)
{
	power_up();
	s1d13524_update_display(epson, s1d135xx_get_wfid(wf_init));
	power_down();
	return 0;
}

void plat_s1d13524_slideshow(struct s1d135xx *epson)
{
	/* run the slideshow */
	while(1) {
		slideshow_run("img", show_image, NULL);
	}
}

static int show_image(const char *image, void *arg)
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
	s1d13524_update_display(epson, s1d135xx_get_wfid(wf_refresh));
	power_down();

	return 0;
}

int plat_s1d12524_run_std_slideshow(struct s1d135xx *epson)
{
	int run = 1;

	LOG("Running standard S1D12524 slideshow");

	while (run)
		slideshow_run("img", show_image, epson);

	return 0;
}

