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
 * epson-s1d13524.c -- Epson EPDC S1D13524
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include "epson-s1d135xx.h"
#include <pl/epdc.h>
#include <stdlib.h>
#include <stdint.h>
#include "assert.h"

#define LOG_TAG "s1d13524"
#include "utils.h"

#define S1D13524_PROD_CODE              0x004F
#define S1D13524_STATUS_HRDY            (1 << 5)
#define S1D13524_PLLCFG0                0x340F
#define S1D13524_PLLCFG1                0x0300
#define S1D13524_PLLCFG2                0x1680
#define S1D13524_PLLCFG3                0x1880
#define S1D13524_I2C_CLOCK_DIV          7 /* 400 kHz */
#define S1D13524_I2C_DELAY              3
#define S1D13524_AUTO_RETRIEVE_ON       0x0000
#define S1D13524_AUTO_RETRIEVE_OFF      0x0001
#define S1D13524_LD_IMG_4BPP            (0 << 4)
#define S1D13524_LD_IMG_8BPP            (1 << 4)
#define S1D13524_LD_IMG_16BPP           (2 << 4)
#define S1D13541_WF_CHECKSUM_ERROR      0x1F00
#define S1D13524_CTLR_AUTO_WFID         0x0200
#define S1D13524_CTLR_NEW_AREA_PRIORITY 0x4000
#define S1D13524_CTLR_PROCESSED_SINGLE  0x0000
#define S1D13524_CTLR_PROCESSED_DOUBLE  0x0001
#define S1D13524_CTLR_PROCESSED_TRIPLE  0x0002

enum s1d13524_reg {
	S1D13524_REG_POWER_SAVE_MODE    = 0x0006,
	S1D13524_REG_FRAME_DATA_LENGTH  = 0x0300,
	S1D13524_REG_LINE_DATA_LENGTH   = 0x0306,
	S1D13524_REG_TEMP_AUTO_RETRIEVE = 0x0320,
	S1D13524_REG_TEMP               = 0x0322,
	S1D13541_REG_WF_ADDR_0          = 0x0390,
	S1D13541_REG_WF_ADDR_1          = 0x0392,
};

enum s1d13524_cmd {
	S1D13524_CMD_INIT_PLL           = 0x01,
	S1D13524_CMD_INIT_CTLR_MODE     = 0x0E,
	S1D13524_CMD_RD_WF_INFO         = 0x30,
};

static const struct pl_wfid epson_epdc_wf_table_s1d13524[] = {
	{ wf_refresh,      2 },
	{ wf_delta,        3 },
	{ wf_delta_mono,   4 },
	{ wf_refresh_mono, 1 },
	{ wf_init,         0 },
	{ NULL,           -1 }
};

/* -- private functions -- */

static int s1d13524_check_rev(struct s1d135xx *p);
static int s1d13524_init_clocks(struct s1d135xx *p);
static int s1d13524_init_ctlr_mode(struct s1d135xx *p);

/* -- pl_epdc interface -- */

static int s1d13524_clear_init(struct pl_epdc *epdc)
{
	static const uint16_t params[] = { 0x0500 };
	struct s1d135xx *p = epdc->data;

	s1d135xx_cmd(p, 0x32, params, ARRAY_SIZE(params));

	if (s1d135xx_wait_idle(p))
		return -1;

	if (s1d13524_init_ctlr_mode(p))
		return -1;

#if 1 /* ToDo: find out why the first image state goes away */
	if (s1d135xx_fill(p, S1D13524_LD_IMG_4BPP, 4, NULL, 0xFF))
		return -1;
#endif

	return 0;
}

static int s1d13524_load_wflib(struct pl_epdc *epdc)
{
	struct s1d135xx *p = epdc->data;
	uint16_t addr16[2];
	uint32_t addr32;
	uint16_t busy;

	addr16[0] = s1d135xx_read_reg(p, S1D13541_REG_WF_ADDR_0);
	addr16[1] = s1d135xx_read_reg(p, S1D13541_REG_WF_ADDR_1);
	addr32 = addr16[1];
	addr32 <<= 16;
	addr32 |= addr16[0];

	s1d135xx_write_reg(p, 0x0260, 0x8001);

	if (s1d135xx_load_wflib(p, &epdc->wflib, addr32))
		return -1;

	s1d135xx_cmd(p, S1D13524_CMD_RD_WF_INFO, addr16, ARRAY_SIZE(addr16));

	if (s1d135xx_wait_idle(p))
		return -1;

	busy = s1d135xx_read_reg(p, S1D135XX_REG_DISPLAY_BUSY);

	if (busy & S1D13541_WF_CHECKSUM_ERROR) {
		LOG("Waveform checksum error");
		return -1;
	}

	return 0;
}

static int s1d13524_set_temp_mode(struct pl_epdc *epdc,
				  enum pl_epdc_temp_mode mode)
{
	struct s1d135xx *p = epdc->data;
	int stat = 0;

	switch (mode) {
	case PL_EPDC_TEMP_MANUAL:
		s1d135xx_write_reg(p, S1D13524_REG_TEMP_AUTO_RETRIEVE,
				   S1D13524_AUTO_RETRIEVE_OFF);
		break;
	case PL_EPDC_TEMP_EXTERNAL:
		s1d135xx_write_reg(p, S1D13524_REG_TEMP_AUTO_RETRIEVE,
				   S1D13524_AUTO_RETRIEVE_ON);
		break;
	case PL_EPDC_TEMP_INTERNAL:
		LOG("Unsupported temperature mode");
		stat = -1;
		break;
	default:
		assert_fail("Invalid temperature mode");
	}

	epdc->temp_mode = mode;

	return stat;
}

static int s1d13524_update_temp(struct pl_epdc *epdc)
{
	struct s1d135xx *p = epdc->data;
	int stat = 0;
	int new_temp;

	switch (epdc->temp_mode) {
	case PL_EPDC_TEMP_MANUAL:
		s1d135xx_write_reg(p, S1D13524_REG_TEMP, epdc->manual_temp);
		new_temp = epdc->manual_temp;
		break;
	case PL_EPDC_TEMP_EXTERNAL:
		new_temp = s1d135xx_read_reg(p, S1D13524_REG_TEMP);
		break;
	case PL_EPDC_TEMP_INTERNAL:
		stat = -1;
		break;
	}

	if (stat)
		return -1;

#if VERBOSE_TEMPERATURE
	if (new_temp != p->measured_temp)
		LOG("Temperature: %d", new_temp);
#endif

	p->measured_temp = new_temp;

	return 0;
}

static int s1d13524_fill(struct pl_epdc *epdc, const struct pl_area *area,
			 uint8_t grey)
{
	struct s1d135xx *p = epdc->data;

	return s1d135xx_fill(p, S1D13524_LD_IMG_4BPP, 4, area, grey);
}

static int s1d13524_pattern_check(struct pl_epdc *epdc)
{
	struct s1d135xx *p = epdc->data;

	return s1d135xx_pattern_check(p, epdc->yres, epdc->xres, S1D13524_LD_IMG_8BPP);
}

static int s1d13524_load_image(struct pl_epdc *epdc, const char *path,
			       const struct pl_area *area, int left, int top)
{
	struct s1d135xx *p = epdc->data;

	return s1d135xx_load_image(p, path, S1D13524_LD_IMG_8BPP, 8, area,
				   left, top);
}

/* -- initialisation -- */

int epson_epdc_early_init_s1d13524(struct s1d135xx *p)
{
	p->hrdy_mask = S1D13524_STATUS_HRDY;
	p->hrdy_result = 0;
	p->measured_temp = -127;
	s1d135xx_hard_reset(p->gpio, p->data);

	if (s1d135xx_soft_reset(p))
		return -1;

	if (s1d13524_check_rev(p))
		return -1;

	s1d135xx_write_reg(p, S1D135XX_REG_I2C_STATUS, S1D13524_I2C_DELAY);

	if (s1d13524_init_clocks(p))
		return -1;

	return s1d135xx_set_power_state(p, PL_EPDC_RUN);
}

int epson_epdc_init_s1d13524(struct pl_epdc *epdc)
{
	struct s1d135xx *p = epdc->data;

	if (epson_epdc_early_init_s1d13524(p))
		return -1;

	if (s1d135xx_load_init_code(p)) {
		LOG("Failed to load init code");
		return -1;
	}

	/* Loading the init code turns the EPD power on as a side effect... */
	if (s1d135xx_set_epd_power(p, 0))
		return -1;

	if (s1d135xx_set_power_state(p, PL_EPDC_RUN))
		return -1;

	if (s1d135xx_init_gate_drv(p))
		return -1;

	if (s1d135xx_wait_dspe_trig(p))
		return -1;

	if (s1d13524_init_ctlr_mode(p))
		return -1;

	epdc->clear_init = s1d13524_clear_init;
	epdc->load_wflib = s1d13524_load_wflib;
	epdc->set_temp_mode = s1d13524_set_temp_mode;
	epdc->update_temp = s1d13524_update_temp;
	epdc->fill = s1d13524_fill;
	epdc->pattern_check = s1d13524_pattern_check;
	epdc->load_image = s1d13524_load_image;
	epdc->wf_table = epson_epdc_wf_table_s1d13524;
	epdc->xres = s1d135xx_read_reg(p, S1D13524_REG_FRAME_DATA_LENGTH);
	epdc->yres = s1d135xx_read_reg(p, S1D13524_REG_LINE_DATA_LENGTH);

	return epdc->set_temp_mode(epdc, PL_EPDC_TEMP_EXTERNAL);
}

/* ----------------------------------------------------------------------------
 * private functions
 */

static int s1d13524_check_rev(struct s1d135xx *p)
{
	uint16_t rev;
	uint16_t conf;

	if (s1d135xx_check_prod_code(p, S1D13524_PROD_CODE))
		return -1;

	rev = s1d135xx_read_reg(p, 0x0000);
	conf = s1d135xx_read_reg(p, 0x0004);

	LOG("Rev: %04X, conf: %04X", rev, conf);

	if ((rev != 0x0100) || (conf != 0x001F)) {
		LOG("Invalid rev/conf values");
		return -1;
	}

	return 0;
}

static int s1d13524_init_clocks(struct s1d135xx *p)
{
	static const uint16_t params[] = {
		S1D13524_PLLCFG0, S1D13524_PLLCFG1,
		S1D13524_PLLCFG2, S1D13524_PLLCFG3,
	};

	s1d135xx_cmd(p, S1D13524_CMD_INIT_PLL, params, ARRAY_SIZE(params));

	if (s1d135xx_wait_idle(p))
		return -1;

	s1d135xx_write_reg(p, S1D13524_REG_POWER_SAVE_MODE, 0x0);
	s1d135xx_write_reg(p, S1D135XX_REG_I2C_CLOCK, S1D13524_I2C_CLOCK_DIV);

	return s1d135xx_wait_idle(p);
}

static int s1d13524_init_ctlr_mode(struct s1d135xx *p)
{
	static const uint16_t par[] = {
		S1D13524_CTLR_AUTO_WFID,
		(S1D13524_CTLR_NEW_AREA_PRIORITY |
		 S1D13524_CTLR_PROCESSED_SINGLE),
	};

	s1d135xx_cmd(p, S1D13524_CMD_INIT_CTLR_MODE, par, ARRAY_SIZE(par));

	return s1d135xx_wait_idle(p);
}
