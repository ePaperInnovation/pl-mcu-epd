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
#include <string.h>
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
#include "config.h"
#include "utils.h"

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
#define I2C_EEPROM_PLWF_DATA 0x54

#define LOG_TAG "hbz6"

/* Run regional update sequence of standard full-screen slideshow */
#define USE_REGION_SLIDESHOW 1

#if USE_REGION_SLIDESHOW
static const char SLIDES_PATH[] = "img/slides.txt";
static const char SEP[] = ", ";
#endif

static struct tps65185_info *pmic_info;
static struct i2c_adapter *i2c;
static struct s1d135xx *epson;
static struct vcom_cal *vcom_calibration;
static struct vcom_info vcom_data;
static struct i2c_eeprom *psu_eeprom;
static struct eeprom_data *psu_data;
static struct i2c_eeprom *plwf_eeprom;
static struct plwf_data *plwf_data;

static void check_temperature(struct s1d135xx *epson);
#if USE_REGION_SLIDESHOW
static int run_region_slideshow(struct s1d135xx *epson);
#else
static int run_std_slideshow(struct s1d135xx *epson);
#endif

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
	LOG("Powering up");
	gpio_set_value(B_HWSW_CTRL, false);
	gpio_set_value(B_PMIC_EN, true);

	do {
		mdelay(1);
	} while (!gpio_get_value(B_POK));

	gpio_set_value(B_HWSW_CTRL, true);

	return 0;
}

/* Board specific power down control */
static int power_down(void)
{
	gpio_set_value(B_HWSW_CTRL, false);
	gpio_set_value(B_PMIC_EN, false);
	LOG("Powered down");

	return 0;
}

/* Initialise the Hummingbird Z[6|7].x platform */
int plat_hbZn_init(const char *platform_path, int i2c_on_epson)
{
	int ret = 0;
	short previous;
	int vcom;
	screen_t prev_screen;

	LOG("HB Z6/7 platform initialisation");

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
		return -1;

#if !CONFIG_PSU_ONLY
	/* initialise the Epson controller */
	check(s1d13541_init_start(EPSON_CS_0, &prev_screen, &epson) == 0);
	check(s1d13541_init_prodcode(epson) == 0);
	check(s1d13541_init_clock(epson) == 0);
	check(s1d13541_init_initcode(epson) == 0);
	check(s1d13541_init_pwrstate(epson) == 0);
	check(s1d13541_init_keycode(epson) == 0);

#endif

	/* initialise the i2c interface as required */
	if (i2c_on_epson)
		check(epson_i2c_init(epson, &i2c) == 0);
	else
		check(msp430_i2c_init(0, &i2c) == 0);

#if !CONFIG_PSU_ONLY
#if CONFIG_WF_ON_SD_CARD
	check(s1d13541_init_waveform_sd(epson) == 0);
#else
	eeprom_init(i2c, I2C_EEPROM_PLWF_DATA, EEPROM_24AA256, &plwf_eeprom);
	plwf_data_init(&plwf_data);
	check(!s1d13541_init_waveform_eeprom(epson, plwf_eeprom, plwf_data));
	plwf_data_free(&plwf_data);
#endif /* WAVEFORM_ON_SD_CARD */
	check(s1d13541_init_gateclr(epson) == 0);
	check(s1d13541_init_end(epson, prev_screen) == 0);
#endif
	/* intialise the psu EEPROM */
	eeprom_init(i2c, I2C_EEPROM_PSU_DATA, EEPROM_24LC014, &psu_eeprom);

	/* read the psu calibration data and ready it for use */
	psu_data_init(&psu_data);
	psu_data_read(psu_eeprom, psu_data);
	if (psu_data_get_vcom_data(psu_data, &vcom_data) == 0) {
		vcom_init(&vcom_data, VCOM_VGSWING, &vcom_calibration);
	} else {
		LOG("Using power supply defaults");
#if CONFIG_PSU_WRITE_DEFAULTS
		LOG("Writing default psu data");
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
		/* always power down first in case we last left HV on */
		power_down();
		power_up();
	}
#endif

	/* initialise display */
	epson_fill_buffer(0x0030, false, epson->yres, epson->xres, 0xff);
	s1d13541_init_display(epson);
	power_up();
	s1d13541_update_display(epson, WVF_INIT);
	s1d13541_wait_update_end(epson);
	power_down();

	/* run the slideshow */

#if USE_REGION_SLIDESHOW
	ret = run_region_slideshow(epson);
#else
	ret = run_std_slideshow(epson);
#endif

	s1d135xx_deselect(epson, previous);

	return ret;
}

#if USE_REGION_SLIDESHOW

static int cmd_image(struct s1d135xx *epson, const char *line)
{
	struct slideshow_item item;

	if (slideshow_parse_item(line, &item))
		return -1;

	if (slideshow_load_image_area(&item, "img", 0x0030, false))
		return -1;

	return 0;
}

static int cmd_update(struct s1d135xx *epson, const char *line)
{
	char waveform[16];
	struct area a;
	int delay_ms;
	const char *opt;
	int len;
	int stat = 0;
	int wfid;

	opt = line;
	len = parser_read_str(opt, SEP, waveform, sizeof(waveform));

	if (len <= 0)
		return -1;

	opt += len;
	len = parser_read_area(opt, SEP, &a);

	if (len <= 0)
		return -1;

	opt += len;
	len = parser_read_int(opt, SEP, &delay_ms);

	if (len < 0)
		return -1;

	wfid = s1d135xx_get_wfid(waveform);

	if (wfid < 0) {
		LOG("Invalid waveform name: %s", waveform);
		return -1;
	}

	s1d13541_update_display_area(epson, wfid, &a);
	mdelay(delay_ms);

	return stat;
}

static int cmd_power(struct s1d135xx *epson, const char *line)
{
	char on_off[4];

	if (parser_read_str(line, SEP, on_off, sizeof(on_off)) < 0)
		return -1;

	if (!strcmp(on_off, "on")) {
		check_temperature(epson);
		power_up();
	} else if (!strcmp(on_off, "off")) {
		s1d13541_wait_update_end(epson);
		power_down();
	} else {
		LOG("Invalid on/off value: %s", on_off);
		return -1;
	}

	return 0;
}

static int run_region_slideshow(struct s1d135xx *epson)
{
	FIL slides;
	int stat;

	if (f_open(&slides, SLIDES_PATH, FA_READ) != FR_OK) {
		LOG("Failed to open slideshow text file [%s]", SLIDES_PATH);
		return -1;
	}

	stat = 0;

	while (!stat) {
		struct cmd {
			const char *name;
			int (*func)(struct s1d135xx *epson, const char *str);
		};
		static const struct cmd cmd_table[] = {
			{ "update", cmd_update },
			{ "power", cmd_power },
			{ "image", cmd_image },
			{ NULL, NULL }
		};
		const struct cmd *cmd;
		char line[81];
		char cmd_name[16];
		int len;

		stat = parser_read_file_line(&slides, line, sizeof(line));

		if (stat < 0) {
			LOG("Failed to read line from %s", SLIDES_PATH);
			break;
		}

		if (!stat) {
			f_lseek(&slides, 0);
			continue;
		}

		stat = 0;

		if (line[0] == '#')
			continue;

		len = parser_read_str(line, SEP, cmd_name, sizeof(cmd_name));

		if (len < 0) {
			LOG("Failed to read command");
			stat = -1;
			break;
		}

		for (cmd = cmd_table; cmd->name != NULL; ++cmd) {
			if (!strcmp(cmd->name, cmd_name)) {
				stat = cmd->func(epson, (line + len));
				break;
			}
		}

		if (cmd->name == NULL) {
			LOG("Invalid command: %s", cmd_name);
			stat = -1;
			break;
		}
	}

	f_close(&slides);

	return stat;
}
#else
static int show_image(const char *image, void *arg)
{
	struct s1d135xx *epson = arg;

	slideshow_load_image(image, 0x0030, false);
	check_temperature(epson);
	power_up();
	s1d13541_update_display(epson, WVF_REFRESH);
	s1d13541_wait_update_end(epson);
	power_down();

	return 0;
}

static int run_std_slideshow(struct s1d135xx *epson)
{
	int run = 1;

	while (run)
		slideshow_run("img", show_image, epson);

	return 0;
}
#endif

static void check_temperature(struct s1d135xx *epson)
{
	u8 needs_update;

	/* Ask Epson to determine if waveform needs reloading */
	s1d13541_measure_temperature(epson, &needs_update);

	if (needs_update) {
#if CONFIG_WF_ON_SD_CARD
		s1d13541_send_waveform();
#else
		eeprom_init(i2c, I2C_EEPROM_PLWF_DATA, EEPROM_24AA256,
			    &plwf_eeprom);
		plwf_data_init(&plwf_data);
		s1d13541_send_waveform_eeprom(epson, plwf_eeprom, plwf_data);
		plwf_data_free(&plwf_data);
#endif /* WAVEFORM_ON_SD_CARD */
	}
}
