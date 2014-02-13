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
 * plat-hbz13.c -- Plastic Logic Hummingbird Z1.3 adapter
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 * Hummingbird HB Z1.3 platform intiialisation. Can drive either:
 * Module-A: No i2c interface on Module so needs CPU i2c.
 * Module-B: Has i2c interface on Module no CPU i2c required.
 * Needs to define
 * - spi interface
 * - i2c interface (either on CPU or the Epson controller)
 * - power sequencer
 * - epson controller
 * Maxim 17135 PMIC - temperature sensor fitted
 * VCOM controlled by external MAX5820 DAC
 *
 */

#include <pl/platform.h>
#include <pl/gpio.h>
#include <pl/i2c.h>
#include <stdio.h>
#include "types.h"
#include "assert.h"
#include "msp430-gpio.h"
#include "msp430-i2c.h"
#include "FatFs/ff.h"
#include "vcom.h"
#include "pmic-max17135.h"
#include "dac-5820.h"
#include "epson/S1D13541.h"
#include "epson/epson-i2c.h"
#include "epson/epson-utils.h"
#include "epson/epson-if.h"
#include "slideshow.h"

/* i2c addresses Maxim PMIC and DAC */
#define I2C_PMIC_ADDR	0x48
#define I2C_DAC_ADDR	0x39

#define	CONFIG_PSU_ONLY	0	// set to 1 to cycle power supply only (no Epson access)

static struct platform *g_plat;
static struct pl_i2c g_epson_i2c;
static struct pl_i2c *g_i2c;
static struct dac5820_info *dac_info;
static struct max17135_info *pmic_info;
static struct s1d135xx *epson;
static struct vcom_cal vcom_calibration;

static int show_image(const char *image, void *arg);

/* No PSU calibration data so always use defaults */
static struct pl_hw_vcom_info def_vcom_info = {
	.dac_x1 = 63,
	.dac_y1 = 4586,
	.dac_x2 = 189,
	.dac_y2 = 9800,
	.vgpos_mv = 27770,
	.vgneg_mv = -41520,
	.swing_ideal = 700000,
};

static short measured;
static u8 needs_update;

/* Board specific power up control */
static int power_up(void)
{
#if 0
	printk("Powering up\n");
	g_plat->gpio.set(g_plat->hv_gpio->hvsw_ctrl, false);
	g_plat->gpio.set(g_plat->hv_gpio->pmic_en, true);

	do {
		mdelay(1);
	} while (!g_plat->gpio.get(g_plat->hv_gpio->pmic_pok));

	dac5820_write(dac_info);
	dac5820_set_power(dac_info, true);

	g_plat->gpio.set(g_plat->hv_gpio->hvsw_ctrl, true);
#endif

	return 0;
}

/* Board specific power down control */
static int power_down(void)
{
#if 0
	g_plat->gpio.set(g_plat->hv_gpio->hvsw_ctrl, false);
	dac5820_set_power(dac_info, false);
	g_plat->gpio.set(g_plat->hv_gpio->pmic_en, false);

	printk("Powered down\n");
#endif

	return 0;
}


/* Initialise the platform */
int plat_hbz13_init(struct platform *plat, const char *platform_path,
		    int i2c_on_epson)
{
	int done = 0;
	short previous;
	int vcom;
	screen_t prev_screen;

	printk("HB Z1.3 platform initialisation\n");

	g_plat = plat;

	if (f_chdir(platform_path) != FR_OK)
		abort_msg("Failed to find platform directory");

#if 0
	/* read the display vcom */
	vcom = util_read_vcom();
	assert(vcom > 0);
#endif

#if 0
	/* initialise the Epson interface */
	epsonif_init(&plat->gpio, 0, 1);
#endif

#if 0 /*!CONFIG_PSU_ONLY*/
	/* initialise the Epson controller */
	check(s1d13541_early_init(EPSON_CS_0, &prev_screen, &epson) == 0);
	check(s1d13541_early_init_end(epson, prev_screen) == 0);
	check(s1d13541_init_start(EPSON_CS_0, &prev_screen, epson) == 0);
	check(s1d13541_init_prodcode(epson) == 0);
	check(s1d13541_init_clock(epson) == 0);
	check(s1d13541_init_initcode(epson) == 0);
	check(s1d13541_init_pwrstate(epson) == 0);
	check(s1d13541_init_keycode(epson) == 0);
	check(s1d13541_send_waveform() == 0);
	check(s1d13541_init_gateclr(epson) == 0);
	check(s1d13541_init_end(epson, prev_screen) == 0);
#endif

	/* initialise the i2c interface as required */
	if (i2c_on_epson) {
		if (epson_i2c_init(epson, &g_epson_i2c))
			return -1;
		g_i2c = &g_epson_i2c;
	} else {
		g_i2c = &g_plat->host_i2c;
	}

	/* initialise the pmic */
	max17135_init(g_i2c, I2C_PMIC_ADDR, &pmic_info);
	max17135_configure(pmic_info, NULL,  MAX17135_SEQ_0);
	max17135_temp_enable(pmic_info);

	/* initialise the vcom calibration data */
	vcom_init(&vcom_calibration, &def_vcom_info);

	/* initialise the VCOM Dac and pass it the VCOM calibration data */
	dac5820_init(g_i2c, I2C_DAC_ADDR, &dac_info);
	dac5820_configure(dac_info, &vcom_calibration);
	dac5820_set_voltage(dac_info, vcom);

#if CONFIG_PSU_ONLY
	while (1) {
		// always power down incase we leave HV on
		power_down();
		power_up();
	}
#endif

	/* select the controller for future operations */
	s1d135xx_select(epson, &previous);

	/* measure temperature manually */
	s1d13541_set_temperature_mode(epson, TEMP_MODE_MANUAL);

	/* Fill display buffer with white */
	epson_fill_buffer(0x0030, false, epson->yres, epson->xres, 0xff);
	s1d13541_init_display(epson);
	power_up();
	s1d13541_update_display(epson, s1d135xx_get_wfid(wf_init));
	s1d13541_wait_update_end(epson);
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
	/* measure the temperature and pass it to the Epson */
	max17135_temperature_measure(pmic_info, &measured);
	s1d13541_set_temperature(epson, measured);

	/* Ask Epson to determine if waveform needs reloading */
	s1d13541_measure_temperature(epson, &needs_update);
	if (needs_update)
	{
		s1d13541_send_waveform();
	}

	printk("Load: %s\n", image);
	slideshow_load_image(image, 0x0030, false);

	power_up();
	s1d13541_update_display(epson, s1d135xx_get_wfid(wf_refresh));
	s1d13541_wait_update_end(epson);
	power_down();

	return 0;
}

