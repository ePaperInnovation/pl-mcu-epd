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
 *          Guillaume Tucker <guillaume.tucker@plasticlogic.com>
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

#include <pl/platform.h>
#include <pl/gpio.h>
#include <pl/i2c.h>
#include <pl/hwinfo.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "assert.h"
#include "msp430-gpio.h"
#include "msp430-spi.h"
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
#include "config.h"
#include "plat-hbz6.h"

#define LOG_TAG "hbz6"
#include "utils.h"

#define CONFIG_PLAT_RUDDOCK2	1
#if CONFIG_PLAT_RUDDOCK2
#define B_HWSW_CTRL     MSP430_GPIO(1,2)
#define B_POK           MSP430_GPIO(1,0)
#define B_PMIC_EN       MSP430_GPIO(1,1)
#define EPSON_CS_0      MSP430_GPIO(3,6)
#define EPSON_CLK_EN    MSP430_GPIO(1,6)
#define EPSON_3V3_EN    MSP430_GPIO(1,7)
#endif

/* 1 to cycle power supplies only, no display update (testing only) */
#define	CONFIG_PSU_ONLY 0

/* I2C addresses */
#define I2C_PMIC_ADDR        0x68
#define I2C_PLWF_EEPROM_ADDR 0x54

static const char SLIDES_PATH[] = "img/slides.txt";
static const char SEP[] = ", ";

static struct platform *g_plat;
static struct pl_i2c g_epson_i2c;
static struct pl_i2c *g_i2c;
static struct tps65185_info *pmic_info;
static struct s1d135xx *epson;
static struct vcom_cal vcom_calibration;
static struct i2c_eeprom plwf_eeprom = {
	NULL, I2C_PLWF_EEPROM_ADDR, EEPROM_24AA256,
};
static struct plwf_data plwf_data;

static void check_temperature(struct s1d135xx *epson);
static int run_region_slideshow(struct s1d135xx *epson);
static int run_std_slideshow(struct s1d135xx *epson);

#if CONFIG_HW_INFO_DEFAULT
/* Fallback VCOM calibration data if EEPROM fails */
static const struct pl_hw_vcom_info def_vcom_info = {
	.dac_x1 = 127,
	.dac_y1 = 4172,
	.dac_x2 = 381,
	.dac_y2 = 12490,
	.vgpos_mv = 25080,
	.vgneg_mv = -32300,
	.swing_ideal = 56886,
};
#endif

/* Board specific power up control */
static int power_up(void)
{
	g_plat->gpio.set(B_HWSW_CTRL, false);
	g_plat->gpio.set(B_PMIC_EN, true);

	do {
		mdelay(1);
	} while (!g_plat->gpio.get(B_POK));

	g_plat->gpio.set(B_HWSW_CTRL, true);

	return 0;
}

/* Board specific power down control */
static int power_down(void)
{
	g_plat->gpio.set(B_HWSW_CTRL, false);
	g_plat->gpio.set(B_PMIC_EN, false);

	return 0;
}

#if CONFIG_DEMO_POWERMODES
/* Board specific power state calls*/
/* Off */
static int pwrstate_off_mode(void)
{
	s1d13541_pwrstate_sleep(epson);
	/* Turn off epson clock and 3v3 */
	g_plat->gpio.set(EPSON_CLK_EN, false);
	g_plat->gpio.set(EPSON_3V3_EN, false);
	g_plat->gpio.set(EPSON_CS_0, false);
	return 0;
}

/* Sleep */
static int pwrstate_sleep_mode(void)
{
	s1d13541_pwrstate_sleep(epson);
	/* Turn off epson clock */
	g_plat->gpio.set(EPSON_CLK_EN, false);

	return 0;
}

/* Standby */
static int pwrstate_standby_mode(void)
{
	/* Enable epson 3v3 clock */
	g_plat->gpio.set(EPSON_CLK_EN, true);
	s1d13541_pwrstate_standby(epson);

	return 0;
}

/* Run */
static int pwrstate_run_mode(void)
{
	/* Enable epson clock */
	g_plat->gpio.set(EPSON_CLK_EN, true);
	s1d13541_pwrstate_run(epson);

	return 0;
}

static int pwrstate_poweron_mode(void)
{
	/* Re-enable power, clock and chip select */
	g_plat->gpio.set(EPSON_3V3_EN, true);
	g_plat->gpio.set(EPSON_CLK_EN, true);
	g_plat->gpio.set(EPSON_CS_0, true);

	/* Need to re-init epson chip */
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
	LOG("Loading display data from SD card");
	if (s1d13541_send_waveform())
		abort_msg("Failed to load waveform from SD card");
	/* read the display vcom */
	vcom = util_read_vcom();
	assert(vcom > 0);
#elif CONFIG_DISP_DATA_EEPROM_SD
	if (wf_from_eeprom()) {
		LOG("Failed to load waveform from EEPROM");
		LOG("Loading display data from SD card");
		if (s1d13541_send_waveform())
			abort_msg("Failed to load waveform from SD card");
		/* read the display vcom */
		vcom = util_read_vcom();
		assert(vcom > 0);
	} else {
		vcom = plwf_data.info.vcom;
	}
#elif CONFIG_DISP_DATA_SD_EEPROM
	LOG("Loading display data from SD card");
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
return 0;
}

/* Cycle through available power modes */
static int powerdemo_run(void)
{
	const int wfid_refresh = s1d135xx_get_wfid(wf_refresh);

	/* Set RUN mode and update display */
	pwrstate_run_mode();

	slideshow_load_image("img/01_n.pgm", 0x0030, false);
	power_up();
	s1d13541_update_display(epson, wfid_refresh);
	s1d13541_wait_update_end(epson);
	power_down();
	msleep(2000);

	/* Set SLEEP mode */
	printk("Setting SLEEP mode\n");
	pwrstate_sleep_mode();
	msleep(2000);

	/* Set STANDBY mode */
	printk("Setting STANDBY mode\n");
	pwrstate_standby_mode();
	msleep(2000);

	/* Set back to RUN mode, update display with previous image */
	pwrstate_run_mode();
	power_up();
	s1d13541_update_display(epson, wfid_refresh);
	s1d13541_wait_update_end(epson);
	power_down();

	/* Power down the epson chip, wait for 2s then power back up */
	pwrstate_off_mode();
	msleep(2000);
	pwrstate_poweron_mode();
	msleep(1000);

	return 0;
}
#endif /* CONFIG_DEMO_POWERMODES */

#if !CONFIG_DISP_DATA_SD_ONLY
static int wf_from_eeprom()
{
	LOG("Loading display data from EEPROM");

	if (plwf_data_init(&plwf_data, &plwf_eeprom)) {
		LOG("Failed to initialise display data");
		return -1;
	}

	if (plwf_load_wf(&plwf_data, &plwf_eeprom, epson, S1D13541_WF_ADDR)) {
		LOG("Failed to load waveform from EEPROM");
		return -1;
	}

	return 0;
}
#endif

/* Initialise the Hummingbird Z[6|7].x platform */
int plat_hbZn_init(struct platform *plat, const char *platform_path,
		   int i2c_on_epson)
{
	static const struct pl_gpio_config gpios[] = {
		{ B_HWSW_CTRL,  PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
		{ B_PMIC_EN,    PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
		{ B_POK,        PL_GPIO_INPUT                   },
		{ EPSON_CS_0,   PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
		{ EPSON_3V3_EN, PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
		{ EPSON_CLK_EN, PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	};
	struct pl_hw_info pl_hw_info;
	int ret = 0;
	short previous;
	int vcom;
	screen_t prev_screen;

	LOG("HB Z6/7 platform initialisation");

	g_plat = plat;

	if (f_chdir(platform_path) != FR_OK)
		abort_msg("Failed to find platform directory");

	/* initialise the Epson interface */
	epsonif_init(&plat->gpio, 0, 1);

	s1d135xx_set_wfid_table(EPDC_S1D13541);

	/* define gpio's required for operation */
	if (pl_gpio_config_list(&plat->gpio, gpios, ARRAY_SIZE(gpios)))
		return -1;

#if !CONFIG_PSU_ONLY
	/* initialise the Epson controller */
	check(s1d13541_early_init(EPSON_CS_0, &prev_screen, &epson) == 0);
	check(s1d13541_init_clock(epson) == 0);
	check(s1d13541_init_initcode(epson) == 0);
	check(s1d13541_init_pwrstate(epson) == 0);
	check(s1d13541_init_keycode(epson) == 0);
#endif

	/* initialise the i2c interface as required */
	if (i2c_on_epson) {
		if (epson_i2c_init(epson, &g_epson_i2c))
			return -1;
		g_i2c = &g_epson_i2c;
	} else {
		g_i2c = &g_plat->host_i2c;
	}

	plwf_eeprom.i2c = g_i2c;

#if !CONFIG_PSU_ONLY

#if CONFIG_DISP_DATA_EEPROM_ONLY
	if (wf_from_eeprom())
		abort_msg("Failed to load waveform from EEPROM");
	vcom = plwf_data.info.vcom;
#elif CONFIG_DISP_DATA_SD_ONLY
	LOG("Loading display data from SD card");
	if (s1d13541_send_waveform())
		abort_msg("Failed to load waveform from SD card");
	/* read the display vcom */
	vcom = util_read_vcom();
	assert(vcom > 0);
#elif CONFIG_DISP_DATA_EEPROM_SD
	if (wf_from_eeprom()) {
		LOG("Failed to load waveform from EEPROM");
		LOG("Loading display data from SD card");
		if (s1d13541_send_waveform())
			abort_msg("Failed to load waveform from SD card");
		/* read the display vcom */
		vcom = util_read_vcom();
		assert(vcom > 0);
	} else {
		vcom = plwf_data.info.vcom;
	}
#elif CONFIG_DISP_DATA_SD_EEPROM
	LOG("Loading display data from SD card");
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
#endif

#if CONFIG_HW_INFO_EEPROM
	/* read the psu calibration data and ready it for use */
	if (pl_hw_info_init(&pl_hw_info, &g_plat->hw_eeprom)) {
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

	/* initialise the VCOM */
	vcom_init(&vcom_calibration, &pl_hw_info.vcom);

	/* select the controller for future operations */
	s1d135xx_select(epson, &previous);

	/* measure temperature using internal sensor */
	s1d13541_set_temperature_mode(epson, TEMP_MODE_INTERNAL);

	/* initialise the PMIC and pass it the vcom calibration data */
	tps65185_init(g_i2c, I2C_PMIC_ADDR, &pmic_info);
	tps65185_configure(pmic_info, &vcom_calibration);
	tps65185_set_vcom_voltage(pmic_info, vcom);

	plat_s1d13541_init_display(epson);
	plat_s1d13541_slideshow(epson);

	s1d135xx_deselect(epson, previous);

	return ret;
}

int plat_s1d13541_init_display(struct s1d135xx *epson)
{
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
	s1d13541_update_display(epson, s1d135xx_get_wfid(wf_init));
	s1d13541_wait_update_end(epson);
	power_down();
	return 0;
}

int plat_s1d13541_slideshow(struct s1d135xx *epson)
{
	int ret = 0;
#if CONFIG_DEMO_POWERMODES
	/* run the power states demo */
	while (1)
		powerdemo_run();
#else
	/* run the slideshow */
	if (is_file_present(SLIDES_PATH))
		ret = run_region_slideshow(epson);
	else
		ret = run_std_slideshow(epson);
#endif

	return ret;
}

static int cmd_update(struct s1d135xx *epson, const char *line)
{
	char waveform[16];
	struct area area;
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
	len = parser_read_area(opt, SEP, &area);

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

	s1d13541_update_display_area(epson, wfid, &area);
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

static int cmd_fill(struct s1d135xx *epson, const char *line)
{
	struct area area;
	const char *opt;
	int len;
	int gl;

	opt = line;
	len = parser_read_area(opt, SEP, &area);

	if (len <= 0)
		return -1;

	opt += len;
	len = parser_read_int(opt, SEP, &gl);

	if (len < 0)
		return -1;

	if ((gl > 15) || (gl < 0)) {
		LOG("Invalid grey level value: %d", gl);
		return -1;
	}

	gl |= (gl << 4);
	epson_fill_area(0x0030, false, &area, gl);

	return 0;
}

static int cmd_image(struct s1d135xx *epson, const char *line)
{
	struct slideshow_item item;

	if (slideshow_parse_item(line, &item))
		return -1;

	if (slideshow_load_image_area(&item, "img", 0x0030, false))
		return -1;

	return 0;
}

static int cmd_sleep(struct s1d135xx *epson, const char *line)
{
	int sleep_ms;
	int len;

	len = parser_read_int(line, SEP, &sleep_ms);

	if (len < 0)
		return -1;

	if (sleep_ms < 0) {
		LOG("Invalid sleep duration: %d", sleep_ms);
		return -1;
	}

	msleep(sleep_ms);

	return 0;
}

#define SLIDES_LOG(msg, ...)					\
	LOG("[%s:%4lu] "msg, SLIDES_PATH, lno, ##__VA_ARGS__)

static int run_region_slideshow(struct s1d135xx *epson)
{
	FIL slides;
	int stat;
	unsigned long lno;

	LOG("Running sequence from %s", SLIDES_PATH);

	if (f_open(&slides, SLIDES_PATH, FA_READ) != FR_OK) {
		LOG("Failed to open slideshow text file [%s]", SLIDES_PATH);
		return -1;
	}

	stat = 0;
	lno = 0;

	while (!stat) {
		struct cmd {
			const char *name;
			int (*func)(struct s1d135xx *epson, const char *str);
		};
		static const struct cmd cmd_table[] = {
			{ "update", cmd_update },
			{ "power", cmd_power },
			{ "fill", cmd_fill },
			{ "image", cmd_image },
			{ "sleep", cmd_sleep },
			{ NULL, NULL }
		};
		const struct cmd *cmd;
		char line[81];
		char cmd_name[16];
		int len;

		++lno;
		stat = parser_read_file_line(&slides, line, sizeof(line));

		if (stat < 0) {
			SLIDES_LOG("Failed to read line");
			break;
		}

		if (!stat) {
			f_lseek(&slides, 0);
			lno = 0;
			continue;
		}

		stat = 0;

		if ((line[0] == '\0') || (line[0] == '#'))
			continue;

		len = parser_read_str(line, SEP, cmd_name, sizeof(cmd_name));

		if (len < 0) {
			SLIDES_LOG("Failed to read command");
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
			SLIDES_LOG("Invalid command");
			stat = -1;
			break;
		}
	}

	f_close(&slides);

	return stat;
}

#undef SLIDES_LOG

static int show_image(const char *image, void *arg)
{
	struct s1d135xx *epson = arg;

	if (slideshow_load_image(image, 0x0030, false))
		return -1;

	check_temperature(epson);
	power_up();
	s1d13541_update_display(epson, s1d135xx_get_wfid(wf_refresh));
	s1d13541_wait_update_end(epson);
	power_down();

	return 0;
}

int run_std_slideshow(struct s1d135xx *epson)
{
	int run = 1;

	LOG("Running standard slideshow");

	while (run)
		slideshow_run("img", show_image, epson);

	return 0;
}

static void check_temperature(struct s1d135xx *epson)
{
	u8 needs_update;

	s1d13541_measure_temperature(epson, &needs_update);

	if (!needs_update)
		return;

#if CONFIG_DISP_DATA_EEPROM_ONLY
	if (plwf_load_wf(&plwf_data, &plwf_eeprom, epson, S1D13541_WF_ADDR))
		abort_msg("Failed to reload waveform from EEPROM");
#elif CONFIG_DISP_DATA_SD_ONLY
	if (s1d13541_send_waveform())
		abort_msg("Failed to reload waveform from SD card");
#elif CONFIG_DISP_DATA_EEPROM_SD
	if (plwf_load_wf(&plwf_data, &plwf_eeprom, epson, S1D13541_WF_ADDR)) {
		LOG("Failed to reload waveform from EEPROM, trying SD card");
		if (s1d13541_send_waveform())
			abort_msg("Failed to reload waveform from SD card");
	}
#elif CONFIG_DISP_DATA_SD_EEPROM
	if (s1d13541_send_waveform()) {
		LOG("Failed to reload waveform from SD card, trying EEPROM");
		if (plwf_load_wf(&plwf_data, &plwf_eeprom, epson,
				 S1D13541_WF_ADDR))
			abort_msg("Failed to reload waveform from EEPROM");
	}
#endif
}
