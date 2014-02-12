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
 * plat-hbz6.c -- Plastic Logic Hummingbird Z6 adapter
 *
 * Authors: Andrew Cox <andrew.cox@plasticlogic.com>
 *          Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <pl/platform.h>
#include <pl/i2c.h>
#include <pl/hwinfo.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "assert.h"
#include "msp430-gpio.h"
#include "vcom.h"
#include "msp430-i2c.h"
#include "FatFs/ff.h"
#include "pmic-tps65185.h"
#include "pmic-max17135.h"
#include "epson/S1D13541.h"
#include "epson/S1D13524.h"
#include "epson/epson-i2c.h"
#include "epson/epson-utils.h"
#include "epson/epson-if.h"
#include "plat-epson.h"
#include "temp-lm75.h"
#include "slideshow.h"
#include "i2c-eeprom.h"
#include "config.h"

#define LOG_TAG "plat-epson"
#include "utils.h"

/* Set 1 to cycle power supplies only, no display update (testing only) */
#define	CONFIG_PSU_ONLY 0

/* I2C addresses */
#define I2C_MAX17135_PMIC_ADDR  0x48
#define I2C_TPS65185_PMIC_ADDR 0x68
#define	I2C_TEMP_SENSOR		   0x49
#define I2C_PSU_EEPROM_ADDR    0x50
#define I2C_PLWF_EEPROM_ADDR   0x54

static struct tps65185_info *tps65185_pmic_info;
static struct max17135_info *max17135_pmic_info;
static struct pl_i2c i2c;
static struct s1d135xx *epson;
static struct vcom_cal vcom_calibration;
static struct lm75_info *lm75_info;
static struct i2c_eeprom plwf_eeprom = {
	NULL, I2C_PLWF_EEPROM_ADDR, EEPROM_24AA256,
};
static struct plwf_data plwf_data;

#if !CONFIG_DISP_DATA_SD_ONLY
static int wf_init_eeprom()
{
	LOG("Loading display data from EEPROM");

	if (plwf_data_init(&plwf_data, &plwf_eeprom)) {
		LOG("Failed to initialise display data");
		return -1;
	}

	return 0;
}

static int wf_from_eeprom()
{
	int ret = 0;
	LOG("Loading waveform from EEPROM");

	if (plwf_load_wf(&plwf_data, &plwf_eeprom, epson, S1D13541_WF_ADDR)) {
		abort_msg("Failed to load waveform from EEPROM");
		ret = -1;
	}

	return ret;
}
#endif

int plat_epson_init(struct platform *plat,
		    const struct pl_hw_info *pl_hw_info,
		    const struct epson_gpio_config *gpio_epson)
{
	static const char platform_path[] = "0:/";
	char full_path[10];
	int ret = 0;
	screen_t prev_screen;
	int vcom;

#if CONFIG_DISP_DATA_SD_EEPROM
	check(f_chdir(CONFIG_DISPLAY_TYPE) == FR_OK);
#endif

#if !CONFIG_DISP_DATA_SD_ONLY
	/* ToDo: use "guest" I2C instead */
	plwf_eeprom.i2c = &plat->host_i2c;
#endif

	/* initialise the Epson interface */
	epsonif_init(&plat->gpio, gpio_epson, 0, 1);

	/* Set the waveform id table for the current controller */
	s1d135xx_set_wfid_table(pl_hw_info->board.epdc_ref);

	/* initialise the Epson controller */
	switch (pl_hw_info->board.epdc_ref) {
	case EPDC_NONE:
		break;
	case EPDC_S1D13524:
		/* TODO: Proper display eeprom support for Raven */
		check(f_chdir("0:/Type11") == FR_OK);
		check(s1d13524_init(gpio_epson->cs0, &epson)==0);
		break;
	case EPDC_S1D13541:
#if !CONFIG_PSU_ONLY
		/* Epson S1D13541 controller early init */
		check(s1d13541_early_init(gpio_epson->cs0, &prev_screen, &epson) == 0);
#endif
		break;
	default:
		break;
	}

	switch(pl_hw_info->board.i2c_mode) {
	case I2C_MODE_HOST: /* MSP430 */
		/* No action taken, i2c already initialised for msp430 */
		break;
	case I2C_MODE_DISP: /* Epson */
	case I2C_MODE_S1D13524: /* Epson */
		//check(epson_i2c_init(epson, &i2c) == 0);
		break;
	case I2C_MODE_SC18IS6XX:
	case I2C_MODE_NONE:
	default:
		break;
	}

	/* Determine which display is connected, check appropriate path exists
	 * on the SD card in order to load the Epson initialistaion binary. */

#if CONFIG_DISP_DATA_EEPROM_ONLY
		if (wf_init_eeprom())
			abort_msg("Failed to load display data from EEPROM");
		else {
			sprintf(full_path, "%s%s", platform_path, plwf_data.info.panel_type);
			check(f_chdir(full_path) == FR_OK);
		}
#elif CONFIG_DISP_DATA_SD_ONLY
		LOG("Loading display data from SD card");
		check(f_chdir(CONFIG_DISPLAY_TYPE) == FR_OK);
#elif CONFIG_DISP_DATA_EEPROM_SD
	if (wf_init_eeprom()) {
		LOG("Failed to load display data from EEPROM");
		check(f_chdir(CONFIG_DISPLAY_TYPE) == FR_OK);
	} else {
		sprintf(full_path, "%s%s", platform_path,
			plwf_data.info.panel_type);
		check(f_chdir(full_path) == FR_OK);
	}
#elif CONFIG_DISP_DATA_SD_EEPROM
	LOG("Loading display data from SD card");
	if (f_chdir(CONFIG_DISPLAY_TYPE) != FR_OK) {
		LOG("Failed to load waveform from SD card");
		if (wf_init_eeprom())
			abort_msg("Failed to load display data from EEPROM");
		sprintf(full_path, "%s%s", platform_path,
			plwf_data.info.panel_type);
		check(f_chdir(full_path) == FR_OK);
	}
#else
#error "No valid display data mode set"
#endif

	if (pl_hw_info->board.epdc_ref == EPDC_S1D13541) {
#if !CONFIG_PSU_ONLY
		check(s1d13541_early_init_end(epson, prev_screen) == 0);
		check(s1d13541_init_start(gpio_epson->cs0, &prev_screen, epson) == 0);
		check(s1d13541_init_prodcode(epson) == 0);
		check(s1d13541_init_clock(epson) == 0);
		check(s1d13541_init_initcode(epson) == 0);
		check(s1d13541_init_pwrstate(epson) == 0);
		check(s1d13541_init_keycode(epson) == 0);

#if CONFIG_DISP_DATA_EEPROM_ONLY
		if (wf_from_eeprom())
			abort_msg("Failed to load waveform from EEPROM");
		vcom = plwf_data.info.vcom;
#elif CONFIG_DISP_DATA_SD_ONLY
		LOG("Loading waveform from SD card");
		if (s1d13541_send_waveform())
			abort_msg("Failed to load waveform from SD card");
		/* read the display vcom */
		vcom = util_read_vcom();
		assert(vcom > 0);
#elif CONFIG_DISP_DATA_EEPROM_SD
		if (wf_from_eeprom()) {
			LOG("Failed to load waveform from EEPROM");
			LOG("Loading waveform from SD card");
			if (s1d13541_send_waveform())
				abort_msg("Failed to load waveform from SD card");
			/* read the display vcom */
			vcom = util_read_vcom();
			assert(vcom > 0);
		} else {
			vcom = plwf_data.info.vcom;
		}
#elif CONFIG_DISP_DATA_SD_EEPROM
		LOG("Loading waveform from SD card");
		if (s1d13541_send_waveform()) {
			LOG("Failed to load waveform from SD card");
			if (wf_from_eeprom())
				abort_msg("Failed to load waveform from EEPROM");
			vcom = plwf_data.info.vcom;
		} else {
			/* read the display vcom */
			vcom = util_read_vcom();
			assert(vcom > 0);
		}
#endif

		check(s1d13541_init_gateclr(epson) == 0);
		check(s1d13541_init_end(epson, prev_screen) == 0);
#endif /* !CONFIG_PSU_ONLY */
	}

#if 0
	/* read the psu calibration data and ready it for use */
	if (&pl_hw_info == NULL) {
#if CONFIG_HW_INFO_DEFAULT
		LOG("WARNING: Using hard-coded default VCOM PSU values");
		pl_hw_info->version = PL_HW_INFO_VERSION;
		memcpy(&pl_hw_info->vcom, &def_vcom_info,
		       sizeof pl_hw_info->vcom);
#else
		abort_msg("Failed to initialise VCOM PSU data from EEPROM");
#endif
	}
#endif /* 0 */

	/* initialise the VCOM */
	vcom_init(&vcom_calibration, &pl_hw_info->vcom);

	/* select the controller for future operations */
	s1d135xx_select(epson, &prev_screen);

	if (pl_hw_info->board.epdc_ref == EPDC_S1D13524) {
		s1d13524_set_temperature_mode(epson, TEMP_MODE_MANUAL);
	}
	else if (pl_hw_info->board.epdc_ref == EPDC_S1D13541) {
		s1d13541_set_temperature_mode(epson, TEMP_MODE_INTERNAL);
	}
	else {
		LOG("No EPDC specified...");
		ret = -1;
	}

	switch (pl_hw_info->board.hv_pmic) {
	case HV_PMIC_MAX17135:
		max17135_init(&plat->host_i2c, I2C_MAX17135_PMIC_ADDR,
			      &max17135_pmic_info);
		max17135_configure(max17135_pmic_info, &vcom_calibration,
				   MAX17135_SEQ_1);
		max17135_set_vcom_voltage(max17135_pmic_info, vcom);
		break;
	case HV_PMIC_TPS65185:
	/* ToDo: use "guest" I2C instead */
		tps65185_init(&plat->host_i2c, I2C_TPS65185_PMIC_ADDR,
			      &tps65185_pmic_info);
		tps65185_configure(tps65185_pmic_info, &vcom_calibration);
		tps65185_set_vcom_voltage(tps65185_pmic_info, vcom);
		break;
	case HV_PMIC_NONE:
	default:
		break;
	}

	/* initialise the i2c temperature sensor */
	switch (pl_hw_info->board.temp_sensor) {
	case TEMP_SENSOR_LM75:
		lm75_init(&plat->host_i2c, I2C_TEMP_SENSOR, &lm75_info);
		break;
	case TEMP_SENSOR_NONE:
	default:
		break;
	}

	if (pl_hw_info->board.epdc_ref == EPDC_S1D13524) {
		plat_s1d13524_init_display(epson);
		plat_s1d13524_slideshow(epson);
	}
	else if (pl_hw_info->board.epdc_ref == EPDC_S1D13541) {
		plat_s1d13541_init_display(epson);
		plat_s1d13541_slideshow(epson);
	}
	else
	{
		LOG("No EPDC specified...");
		ret = -1;
	}

	s1d135xx_deselect(epson, prev_screen);

	return ret;
}
