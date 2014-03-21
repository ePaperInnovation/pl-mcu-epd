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
 * dac5820.c -- Driver for MAXIM 5820 DAC
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <pl/i2c.h>
#include <stddef.h>
#include "assert.h"
#include "vcom.h"

#define LOG_TAG "dac-max5820"
#include "utils.h"

/* VCOM DAC */
#define	DAC5820_DAC_MAX		((1 << 8)-1)
#define	DAC5820_DAC_MIN		0

/* ToDo: make platform-dependent */
#define VCOM_MAX 13000
#define VCOM_MIN 1000
#define VCOM_DEFAULT ((VCOM_MAX + VCOM_MIN) / 2)
#define VCOM_OFFSET (-19532)
#define VCOM_COEF_INT 20L
#define VCOM_COEF_DEC 779L
#define DAC5820_CMD_LOAD_IN_DAC_A__UP_DAC_B__OUT_AB 0x0
#define DAC5820_CMD_EXT__DATA_0 0xF

/* VCOM DAC definitions */

enum dac5820_pd {
	DAC5820_PD_ON         = 0x0,
	DAC5820_PD_OFF_FLOAT  = 0x1,
	DAC5820_PD_OFF_1K     = 0x2,
	DAC5820_PD_OFF_100K   = 0x3
};

struct dac5820_byte_write_cmd {
	char data_high:4;
	char cmd:4;
};

struct dac5820_byte_cmd_ext {
	char pd:2;
	char a:1;
	char b:1;
	char reserved:4;
};

struct dac5820_byte_write_data {
	char reserved:4;
	char data_low:4;
};

union dac5820_write_payload {
	struct {
		struct dac5820_byte_write_cmd cmd_byte;
		struct dac5820_byte_write_data data_byte;
	};
	u8 bytes[2];
};

union dac5820_ext_payload {
	struct {
		struct dac5820_byte_write_cmd cmd_byte;
		struct dac5820_byte_cmd_ext ext_byte;
	};
	u8 bytes[2];
};

struct dac5820_info {
	struct pl_i2c *i2c;
	u8 i2c_addr;
	struct vcom_cal *cal;
	u8 dac_value;
	int vcom_mv;
};

struct dac5820_info vcom_dac;

int dac5820_init(struct pl_i2c *i2c, u8 i2c_addr, struct dac5820_info **dac)
{
	vcom_dac.i2c_addr = i2c_addr;
	vcom_dac.i2c = i2c;
	vcom_dac.cal = NULL;
	*dac = &vcom_dac;
	return 0;
}

int dac5820_configure(struct dac5820_info *dac, struct vcom_cal *cal)
{
	assert(dac);
	assert(cal);

	dac->cal = cal;

	return 0;
}

int dac5820_set_power(struct dac5820_info *dac, bool on)
{
	union dac5820_ext_payload payload;

	payload.cmd_byte.cmd = DAC5820_CMD_EXT__DATA_0;
	payload.cmd_byte.data_high = 0;
	payload.ext_byte.reserved = 0;
	payload.ext_byte.a = 1;
	payload.ext_byte.b = 0;
	payload.ext_byte.pd = on ? DAC5820_PD_ON : DAC5820_PD_OFF_100K;

	return dac->i2c->write(dac->i2c, dac->i2c_addr, payload.bytes,
			       sizeof payload, 0);
}

void dac5820_set_voltage(struct dac5820_info *dac, int vcom_mv)
{
	long dac_value;
	int round;

	if (!dac->cal) {
		dac_value = (vcom_mv * VCOM_COEF_INT);
		dac_value += (vcom_mv * VCOM_COEF_DEC / 1000);
		dac_value += VCOM_OFFSET;
		round = ((dac_value % 1000) > 500) ? 1 : 0;
		dac_value /= 1000;
		dac_value += round;
	}
	else {
		dac_value = vcom_calculate(dac->cal, vcom_mv);
	}

	if (dac_value < DAC5820_DAC_MIN)
		dac_value = DAC5820_DAC_MIN;
	else if (dac_value > DAC5820_DAC_MAX)
		dac_value = DAC5820_DAC_MAX;

	dac->dac_value = (u8)dac_value;
	dac->vcom_mv = vcom_mv;

	LOG("VCOM DAC %dmV -> %d", dac->vcom_mv, dac->dac_value);
}

int dac5820_write(struct dac5820_info *dac)
{
	union dac5820_write_payload payload;

	payload.cmd_byte.cmd = DAC5820_CMD_LOAD_IN_DAC_A__UP_DAC_B__OUT_AB;
	payload.cmd_byte.data_high = (dac->dac_value >> 4) & 0xF;
	payload.data_byte.data_low = dac->dac_value & 0xF;
	payload.data_byte.reserved = 0;

	return dac->i2c->write(dac->i2c, dac->i2c_addr, payload.bytes,
			       sizeof payload, 0);
}

