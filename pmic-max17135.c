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
 * max17135-pmic.c -- Driver for the MAXIM MAX17135 HVPMIC device.
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <stddef.h>
#include "types.h"
#include "assert.h"
#include "i2c.h"
#include "vcom.h"
#include "pmic-max17135.h"

/* problems with temperature sensor return magic temperature value */
#define	HVPMIC_TEMP_INVALID	0x7FC0
#define	HVPMIC_TEMP_DEFAULT	20

#define	HVPMIC_DAC_MAX		((1 << 8)-1)
#define	HVPMIC_DAC_MIN		0

#define	HVPMIC_TEMP_BUSY	(1 << 0)
#define	HVPMIC_TEMP_OPEN	(1 << 1)
#define	HVPMIC_TEMP_SHORT	(1 << 2)

#if 0
#define MV_DIV	41			// Each DAC step is 41mV ((620/150)*10)
#define MV_OFFSET	2066	// DAC value of 0 => 2066mV
#endif

/* HVPMIC MAX17135 timings for Type4 Display (Maxim Drivers) */
#define HVPMIC_NB_TIMINGS 8
#define HVPMIC_TIMING_SEQ0_UP_VGNEG 24
#define HVPMIC_TIMING_SEQ0_UP_VSNEG 7
#define HVPMIC_TIMING_SEQ0_UP_VSPOS 2
#define HVPMIC_TIMING_SEQ0_UP_VGPOS 12
#define HVPMIC_TIMING_SEQ0_DOWN_VGPOS 7
#define HVPMIC_TIMING_SEQ0_DOWN_VSPOS 14
#define HVPMIC_TIMING_SEQ0_DOWN_VSNEG 12
#define HVPMIC_TIMING_SEQ0_DOWN_VGNEG 2

/* HVPMIC MAX17135 timings for Type11 Display (ST Drivers) */
#define HVPMIC_TIMING_SEQ1_UP_VGNEG 12
#define HVPMIC_TIMING_SEQ1_UP_VSNEG 7
#define HVPMIC_TIMING_SEQ1_UP_VSPOS 2
#define HVPMIC_TIMING_SEQ1_UP_VGPOS 23
#define HVPMIC_TIMING_SEQ1_DOWN_VGPOS 2
#define HVPMIC_TIMING_SEQ1_DOWN_VSPOS 14
#define HVPMIC_TIMING_SEQ1_DOWN_VSNEG 12
#define HVPMIC_TIMING_SEQ1_DOWN_VGNEG 7

enum max17135_register {
	HVPMIC_REG_EXT_TEMP   = 0x00,
	HVPMIC_REG_CONF       = 0x01,
	HVPMIC_REG_INT_TEMP   = 0x04,
	HVPMIC_REG_TEMP_STAT  = 0x05,
	HVPMIC_REG_PROD_REV   = 0x06,
	HVPMIC_REG_PROD_ID    = 0x07,
	HVPMIC_REG_DVR        = 0x08,
	HVPMIC_REG_ENABLE     = 0x09,
	HVPMIC_REG_FAULT      = 0x0A,
	HVPMIC_REG_PROG       = 0x0C,
	HVPMIC_REG_TIMING_1   = 0x10,
	HVPMIC_REG_TIMING_2   = 0x11,
	HVPMIC_REG_TIMING_3   = 0x12,
	HVPMIC_REG_TIMING_4   = 0x13,
	HVPMIC_REG_TIMING_5   = 0x14,
	HVPMIC_REG_TIMING_6   = 0x15,
	HVPMIC_REG_TIMING_7   = 0x16,
	HVPMIC_REG_TIMING_8   = 0x17
};

union max17135_fault {
	struct {
		char fbpg:1;
		char hvinp:1;
		char hvinn:1;
		char fbng:1;
		char hvinpsc:1;
		char hvinnsc:1;
		char ot:1;
		char pok:1;
	};
	u8 byte;
};

struct max17135_hvpmic {
	u8 prod_id;
	u8 prod_rev;
	u8 timings[HVPMIC_NB_TIMINGS];
};

union max17135_temp_config {
	struct {
		char shutdown:1;
	};
	u8 byte;
};

union max17135_temp_status {
	struct {
		char isbusy:1;
		char isopen:1;
		char isshort:1;
	};
	u8 byte;
};

union max17135_temp_value {
	struct {
		int padding:7;
		int measured:9;
	};
	uint16_t word;
};

struct max17135_info {
	struct i2c_adapter *i2c;
	u8 i2c_addr;
	struct max17135_hvpmic hvpmic;
	struct vcom_cal *cal;
};

static struct max17135_info pmic_info;

int max17135_init(struct i2c_adapter *i2c, u8 i2c_addr, struct max17135_info **pmic)
{
	assert(i2c);
	assert(pmic);

	pmic_info.i2c_addr = i2c_addr;
	pmic_info.i2c = i2c;
	pmic_info.cal = NULL;
	*pmic = &pmic_info;
	return 0;
}

static int max17135_load_timings(struct max17135_info *pmic)
{
	u8 reg;
	int i;

	assert(pmic);

	for (i = 0, reg = HVPMIC_REG_TIMING_1;
	     i < HVPMIC_NB_TIMINGS;
	     ++i, ++reg) {
		if (i2c_reg_write_8(pmic->i2c, pmic->i2c_addr,
				    reg, pmic->hvpmic.timings[i]))
			return -1;
	}

	return 0;
}

int max17135_configure(struct max17135_info *pmic, struct vcom_cal *cal, int power_sequence)
{
	u8 timings[HVPMIC_NB_TIMINGS];

	assert(pmic);

	/* cal may be null if not being used */
	pmic->cal = cal;

	switch (power_sequence)
	{
	case MAX17135_SEQ_0:
		timings[0] = HVPMIC_TIMING_SEQ0_UP_VGNEG;
		timings[1] = HVPMIC_TIMING_SEQ0_UP_VSNEG;
		timings[2] = HVPMIC_TIMING_SEQ0_UP_VSPOS;
		timings[3] = HVPMIC_TIMING_SEQ0_UP_VGPOS;
		timings[4] = HVPMIC_TIMING_SEQ0_DOWN_VGPOS;
		timings[5] = HVPMIC_TIMING_SEQ0_DOWN_VSPOS;
		timings[6] = HVPMIC_TIMING_SEQ0_DOWN_VSNEG;
		timings[7] = HVPMIC_TIMING_SEQ0_DOWN_VGNEG;
		break;
	case MAX17135_SEQ_1:
		timings[0] = HVPMIC_TIMING_SEQ1_UP_VGNEG;
		timings[1] = HVPMIC_TIMING_SEQ1_UP_VSNEG;
		timings[2] = HVPMIC_TIMING_SEQ1_UP_VSPOS;
		timings[3] = HVPMIC_TIMING_SEQ1_UP_VGPOS;
		timings[4] = HVPMIC_TIMING_SEQ1_DOWN_VGPOS;
		timings[5] = HVPMIC_TIMING_SEQ1_DOWN_VSPOS;
		timings[6] = HVPMIC_TIMING_SEQ1_DOWN_VSNEG;
		timings[7] = HVPMIC_TIMING_SEQ1_DOWN_VGNEG;
		break;
	default:
		return -EINVAL;
	}

	memcpy(pmic->hvpmic.timings, timings, HVPMIC_NB_TIMINGS);
	printk("MAX17135 timings (seq:%d) on: %d, %d, %d, %d, "
	       "timings off: %d, %d, %d, %d\n",
	       power_sequence,
	       timings[0], timings[1], timings[2], timings[3],
	       timings[4], timings[5], timings[6], timings[7]);

	if (i2c_reg_read_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_PROD_REV,
			   &pmic->hvpmic.prod_rev))
		return -1;

	if (i2c_reg_read_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_PROD_ID,
			   &pmic->hvpmic.prod_id))
		return -1;

	printk("MAX17135: HVPMIC rev 0x%02X, id 0x%02X\n",
	       pmic->hvpmic.prod_rev, pmic->hvpmic.prod_id);

	if (max17135_temp_disable(pmic))
		return -1;

	return max17135_load_timings(pmic);
}

int max17135_set_vcom_register(struct max17135_info *pmic, int dac_value)
{
	assert(pmic);

	return i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_DVR,
			       (uint8_t)dac_value);
}

int max17135_set_vcom_voltage(struct max17135_info *pmic, int mv)
{
	int dac_value;

	assert(pmic);

	dac_value = vcom_calculate(pmic->cal, mv);

	if (dac_value < HVPMIC_DAC_MIN)
		dac_value = HVPMIC_DAC_MIN;
	else if (dac_value > HVPMIC_DAC_MAX)
		dac_value = HVPMIC_DAC_MAX;

	return i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_DVR,
			       (uint8_t)dac_value);
}

int max17135_wait_pok(struct max17135_info *pmic)
{
	static const unsigned POLL_DELAY_MS = 5;
	unsigned timeout = 100;
	int pok = 0;

	assert(pmic);

	while (!pok) {
		union max17135_fault fault;

		mdelay(POLL_DELAY_MS);

		if (i2c_reg_read_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_FAULT,
				   &fault.byte)) {
			printk("MAX17135: failed to read HVPMIC POK\n");
			return -1;
		}

		pok = fault.pok;

		if (timeout > POLL_DELAY_MS) {
			timeout -= POLL_DELAY_MS;
		} else {
			timeout = 0;

			if (!pok) {
				printk("MAX17135: POK timeout\n");
				return -1;
			}
		}
	}

	return 0;
}

/* use the i2c interface to power up the PMIC */
int max17135_enable(struct max17135_info *pmic)
{
	return -EPERM;
}

/* use the i2c interface to power down the PMIC */
int max17135_disable(struct max17135_info *pmic)
{
	return -EPERM;
}

/* enable temperature sensing */
int max17135_temp_enable(struct max17135_info *pmic)
{
	union max17135_temp_config config;

	config.byte = 0;
	config.shutdown = 0;

	return i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_CONF,
			       config.byte);
}

/* disable temperature sensing */
int max17135_temp_disable(struct max17135_info *pmic)
{
	union max17135_temp_config config;

	config.byte = 0;
	config.shutdown = 1;

	return i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_CONF,
			       config.byte);
}

/* read the temperature from the PMIC */
int max17135_temperature_measure(struct max17135_info *pmic, short *measured)
{
	union max17135_temp_status status;
	union max17135_temp_value temp;
	int stat;

	if (i2c_reg_read_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_TEMP_STAT,
			   &status.byte))
		goto error;

	if (i2c_reg_read_16be(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_EXT_TEMP,
			      &temp.word))
		goto error;

	if (status.byte & (HVPMIC_TEMP_OPEN | HVPMIC_TEMP_SHORT)) {
		printk("MAX17135: Temperature sensor error: 0x%02x\n", status.byte);
	}

	if (temp.word == HVPMIC_TEMP_INVALID) {
		*measured = HVPMIC_TEMP_DEFAULT;
		stat = -EDEFAULT;
	} else {
		*measured = (temp.measured >> 1);
		stat = 0;
	}
	printk("MAX17135: Temperature: %d\n", *measured);

error:
	return stat;
}
