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
#define S1D13524_I2C_CLOCK_DIV          7 /* 100 kHz */
#define S1D13524_PWR_CTRL_UP            0x8001
#define S1D13524_PWR_CTRL_DOWN          0x8002
#define S1D13524_PWR_CTRL_BUSY          0x0080
#define S1D13524_PWR_CTRL_CHECK_ON      0x2200
#define S1D13524_AUTO_RETRIEVE_ON       0x0000
#define S1D13524_AUTO_RETRIEVE_OFF      0x0001
#define S1D13524_LD_IMG_8BPP            0x0010
#define S1D13541_WF_CHECKSUM_ERROR      0x1F00

enum s1d13524_reg {
	S1D13524_REG_FRAME_DATA_LENGTH  = 0x0300,
	S1D13524_REG_LINE_DATA_LENGTH   = 0x0306,
	S1D13524_REG_TEMP_AUTO_RETRIEVE = 0x0320,
	S1D13524_REG_TEMP               = 0x0322,
	S1D13541_REG_WF_ADDR_0          = 0x0390,
	S1D13541_REG_WF_ADDR_1          = 0x0392,
};

enum s1d13524_cmd {
	S1D13524_CMD_INIT_PLL           = 0x01,
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

static int s1d13524_init_clocks(struct s1d135xx *p);
static int s1d13524_load_init_code(struct s1d135xx *p);
static int s1d13524_set_pwr_ctrl(struct s1d135xx *p, int state);

/* -- pl_epdc interface -- */

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
	int stat;

	if (mode == epdc->temp_mode)
		return 0;

	stat = 0;

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
		abort_msg("Invalid temperature mode");
	}

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
		/* ToDo */
		/*p->measured_temp = s1d135xx_read_reg(...);*/
		new_temp = p->measured_temp;
		break;
	case PL_EPDC_TEMP_INTERNAL:
		stat = -1;
		break;
	}

	if (stat)
		return -1;

#if VERBOSE_TEMPERATURE
	if (new_temp != p->measured_temp)
		LOG("Temperature: %d", regval);
#endif

	p->measured_temp = new_temp;

	return 0;
}

static int s1d13524_fill(struct pl_epdc *epdc, const struct pl_area *area,
			 uint8_t grey)
{
	struct s1d135xx *p = epdc->data;

	return s1d135xx_fill(p, S1D13524_LD_IMG_8BPP, 8, area, grey);
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
	abort_msg("not implemented");

	return -1;
}

int epson_epdc_init_s1d13524(struct pl_epdc *epdc)
{
	struct s1d135xx *p = epdc->data;

	p->hrdy_mask = S1D13524_STATUS_HRDY;
	p->hrdy_result = 0;

	s1d135xx_hard_reset(p->gpio, p->data);

	if (s1d135xx_soft_reset(p))
		return -1;

	if (s1d13524_init_clocks(p))
		return -1;

	if (s1d135xx_check_prod_code(p, S1D13524_PROD_CODE))
		return -1;

	if (s1d13524_load_init_code(p)) {
		LOG("Failed to load init code");
		return -1;
	}

	if (s1d135xx_init_gate_drv(p))
		return -1;

	if (s1d135xx_wait_dspe_trig(p))
		return -1;

	epdc->wf_table = epson_epdc_wf_table_s1d13524;
	epdc->xres = s1d135xx_read_reg(p, S1D13524_REG_FRAME_DATA_LENGTH);
	epdc->yres = s1d135xx_read_reg(p, S1D13524_REG_LINE_DATA_LENGTH);
	epdc->load_wflib = s1d13524_load_wflib;
	epdc->set_temp_mode = s1d13524_set_temp_mode;
	epdc->update_temp = s1d13524_update_temp;
	epdc->fill = s1d13524_fill;
	epdc->load_image = s1d13524_load_image;

	return epdc->set_temp_mode(epdc, PL_EPDC_TEMP_MANUAL);
}

/* ----------------------------------------------------------------------------
 * private functions
 */

static int s1d13524_init_clocks(struct s1d135xx *p)
{
	static const uint16_t params[] = {
		S1D13524_PLLCFG0, S1D13524_PLLCFG1,
		S1D13524_PLLCFG2, S1D13524_PLLCFG3,
	};

	s1d135xx_cmd(p, S1D13524_CMD_INIT_PLL, params, ARRAY_SIZE(params));

	if (s1d135xx_wait_idle(p))
		return -1;

	s1d135xx_write_reg(p, S1D135XX_REG_I2C_CLOCK, S1D13524_I2C_CLOCK_DIV);

	return s1d135xx_wait_idle(p);
}

static int s1d13524_load_init_code(struct s1d135xx *p)
{
	if (s1d135xx_load_init_code(p))
		return -1;

	/* A side effect of the CMD_INIT_STBY is that the power up sequence
	   runs. For now run power down sequence here. */
	return s1d13524_set_pwr_ctrl(p, 0);
}

static int s1d13524_set_pwr_ctrl(struct s1d135xx *p, int state)
{
	uint16_t tmp;
	uint16_t arg = state ? S1D13524_PWR_CTRL_UP : S1D13524_PWR_CTRL_DOWN;

	s1d135xx_write_reg(p, S1D135XX_REG_PWR_CTRL, arg);

	do {
		tmp = s1d135xx_read_reg(p, S1D135XX_REG_PWR_CTRL);
	} while (tmp & S1D13524_PWR_CTRL_BUSY);

	if (state && ((tmp & S1D13524_PWR_CTRL_CHECK_ON) !=
		      S1D13524_PWR_CTRL_CHECK_ON)) {
		LOG("Failed to turn the EPDC power on");
		return -1;
	}

	return 0;
}
