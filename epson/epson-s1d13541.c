/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2014 Plastic Logic Limited

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
 * epson-s1d13541.c -- Epson EPDC S1D13541
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include "epson/epson-s1d13541.h"
#include "epson/epson-s1d135xx.h"
#include <pl/epdc.h>
#include <stdlib.h>
#include "assert.h"

#if S1D135XX_INTERIM
#include "epson-cmd.h"
extern struct s1d135xx *g_s1d13541_hack;
#endif

#define LOG_TAG "s1d13541"
#include "utils.h"

#define S1D13541_PROD_CODE              0x0053
#define S1D13541_STATUS_HRDY            (1 << 13)
#define S1D13541_INTERNAL_CLOCK_ENABLE  (1 << 7)
#define S1D13541_I2C_CLOCK_DIV          7 /* Maybe 4 ? */
#define S1D13541_PROT_KEY_1             0x5678 /* ToDo: add to s1d135xx_data */
#define S1D13541_PROT_KEY_2             0x1234
#define S1D13541_TEMP_SENSOR_CONTROL    (1 << 14)
#define S1D13541_TEMP_SENSOR_EXTERNAL   (1 << 6)
#define S1D13541_AUTO_TEMP_JUDGE_EN     (1 << 2)

enum s1d13541_reg {
	S1D13541_REG_CLOCK_CONFIG          = 0x0010,
	S1D13541_REG_I2C_CLOCK             = 0x001A,
	S1D13541_REG_PROT_KEY_1            = 0x042C,
	S1D13541_REG_PROT_KEY_2            = 0x042E,
	S1D13541_REG_FRAME_DATA_LENGTH     = 0x0400,
	S1D13541_REG_LINE_DATA_LENGTH      = 0x0406,
	S1D13541_REG_WF_DECODER_BYPASS     = 0x0420,
};

static const struct pl_wfid s1d13541_wf_table[] = {
	{ wf_refresh,      1 },
	{ wf_delta,        3 },
	{ wf_delta_mono,   2 },
	{ wf_refresh_mono, 4 },
	{ wf_init,         0 },
	{ NULL,           -1 }
};

/* -- private functions -- */

static int s1d13541_init_clocks(struct s1d135xx *p);

/* -- pl_epdc interface -- */

static int s1d13541_set_temp_mode(struct pl_epdc *epdc,
				  enum pl_epdc_temp_mode mode)
{
	struct s1d135xx *p = epdc->data;
	uint16_t reg;

	if (mode == epdc->temp_mode)
		return 0;

	reg = s1d135xx_read_reg(p, S1D135XX_REG_PERIPH_CONFIG);
	/* ToDo: when do we set this bit back? */
	reg &= S1D13541_TEMP_SENSOR_CONTROL;

	switch (mode) {
	case PL_EPDC_TEMP_MANUAL:
		break;
	case PL_EPDC_TEMP_EXTERNAL:
		reg &= ~S1D13541_TEMP_SENSOR_EXTERNAL;
		break;
	case PL_EPDC_TEMP_INTERNAL:
		reg |= S1D13541_TEMP_SENSOR_EXTERNAL;
		break;
	default:
		abort_msg("Invalid temperature mode");
	}

	s1d135xx_write_reg(p, S1D135XX_REG_PERIPH_CONFIG, reg);

	/* Configure the controller to automatically update the waveform table
	 * after each temperature measurement.  */
	reg = s1d135xx_read_reg(p, S1D13541_REG_WF_DECODER_BYPASS);
	reg |= S1D13541_AUTO_TEMP_JUDGE_EN;
	s1d135xx_write_reg(p, reg, S1D13541_REG_WF_DECODER_BYPASS);

	epdc->temp_mode = mode;

	return 0;
}

/* -- initialisation -- */

int epson_epdc_init_s1d13541(struct pl_epdc *epdc)
{
	struct s1d135xx *p = epdc->data;
	uint16_t prod_code;

	p->hrdy_mask = S1D13541_STATUS_HRDY;
	p->hrdy_result = S1D13541_STATUS_HRDY;

	s1d135xx_hard_reset(p);

	if (s1d135xx_soft_reset(p))
		return -1;

	if (s1d13541_init_clocks(p))
		return -1;

	prod_code = s1d135xx_read_reg(p, S1D135XX_REG_REV_CODE);

	LOG("Product code: 0x%04X", prod_code);

	if (prod_code != S1D13541_PROD_CODE) {
		LOG("Invalid product code");
		return -1;
	}

	LOG("load_init_code");

	if (s1d135xx_load_init_code(p)) {
		LOG("Failed to load init code");
		return -1;
	}

	s1d135xx_write_reg(p, S1D13541_REG_PROT_KEY_1, S1D13541_PROT_KEY_1);
	s1d135xx_write_reg(p, S1D13541_REG_PROT_KEY_2, S1D13541_PROT_KEY_2);

	if (s1d135xx_wait_idle(p))
		return -1;

	if (s1d135xx_init_gate_drv(p))
		return -1;

	if (s1d135xx_wait_dspe_trig(p))
		return -1;

	epdc->wf_table = s1d13541_wf_table;
	epdc->xres = s1d135xx_read_reg(p, S1D13541_REG_LINE_DATA_LENGTH);
	epdc->yres = s1d135xx_read_reg(p, S1D13541_REG_FRAME_DATA_LENGTH);
	epdc->set_temp_mode = s1d13541_set_temp_mode;

	LOG("Ready %dx%d", epdc->xres, epdc->yres);

#if S1D135XX_INTERIM
	g_s1d13541_hack = p;
	epson_set_idle_mask(0x2000, 0x2000);
#endif

	return 0;
}

/* ----------------------------------------------------------------------------
 * private functions
 */

static int s1d13541_init_clocks(struct s1d135xx *p)
{
	s1d135xx_write_reg(p, S1D13541_REG_I2C_CLOCK,
			   S1D13541_I2C_CLOCK_DIV);
	s1d135xx_write_reg(p, S1D13541_REG_CLOCK_CONFIG,
			   S1D13541_INTERNAL_CLOCK_ENABLE);

	return s1d135xx_wait_idle(p);
}
