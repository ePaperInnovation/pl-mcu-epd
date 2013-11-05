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
 * pmic-tps65185.c -- Driver for TI TPS65185 PMIC
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include "platform.h"
#include "types.h"
#include "assert.h"
#include "i2c.h"
#include "vcom.h"

#define	HVPMIC_DAC_MAX		((1 << 9)-1)
#define	HVPMIC_DAC_MIN		0
#define	HVPMIC_TEMP_DEFAULT	20

#if 0
#define MV_DIV	33		// Each DAC step is 33mV
#endif

enum tps65185_register {
	HVPMIC_REG_TMST_VALUE = 0x00,
	HVPMIC_REG_ENABLE     = 0x01,
	HVPMIC_REG_VADJ       = 0x02,
	HVPMIC_REG_VCOM1      = 0x03,
	HVPMIC_REG_VCOM2      = 0x04,
	HVPMIC_REG_INT_EN1    = 0x05,
	HVPMIC_REG_INT_EN2    = 0x06,
	HVPMIC_REG_INT1       = 0x07,
	HVPMIC_REG_INT2       = 0x08,
	HVPMIC_REG_UPSEQ0     = 0x09,
	HVPMIC_REG_UPSEQ1     = 0x0A,
	HVPMIC_REG_DWNSEQ0    = 0x0B,
	HVPMIC_REG_DWNSEQ1    = 0x0C,
	HVPMIC_REG_TMST1      = 0x0D,
	HVPMIC_REG_TMST2      = 0x0E,
	HVPMIC_REG_PG_STAT    = 0x0F,
	HVPMIC_REG_REV_ID     = 0x10,
	HVPMIC_REG_MAX
};

union tps65185_version {
	struct {
		char version:4;
		char minor:2;
		char major:2;
	};
	u8 byte;
};

struct tps65185_info {
	struct i2c_adapter *i2c;
	u8 i2c_addr;
	struct vcom_cal *cal;
};

static struct tps65185_info pmic_info;

struct pmic_data {
	u8 reg;
	u8 data;
};

static const struct pmic_data init_data[] = {
	{ HVPMIC_REG_ENABLE,     0x00 },
	{ HVPMIC_REG_VADJ,       0x03 },
	{ HVPMIC_REG_VCOM1,      0x00 },
	{ HVPMIC_REG_VCOM2,      0x00 },
	{ HVPMIC_REG_INT_EN1,    0x00 },
	{ HVPMIC_REG_INT_EN2,    0x00 },
	{ HVPMIC_REG_UPSEQ0,     0x78 },
	{ HVPMIC_REG_UPSEQ1,     0x00 },
	{ HVPMIC_REG_DWNSEQ0,    0x00 },
	{ HVPMIC_REG_DWNSEQ1,    0x00 },
	{ HVPMIC_REG_TMST1,      0x00 },
	{ HVPMIC_REG_TMST2,      0x78 }
};

/* Note. Reading some registers will modify the status of the device
 */
static void reg_dump(struct tps65185_info *pmic)
{
	u8 data;
	int reg;

	printk("TPS65185: registers\n");
	for (reg = HVPMIC_REG_TMST_VALUE; reg < HVPMIC_REG_MAX; reg++) {
		if (i2c_reg_read(pmic->i2c, pmic->i2c_addr, reg, &data) == 0)
			printk(" 0x%02X => 0x%02X\n", reg, data);
	}
}

int tps65185_init(struct i2c_adapter *i2c, u8 i2c_addr, struct tps65185_info **pmic)
{
	assert(i2c);
	assert(pmic);

	pmic_info.i2c_addr = i2c_addr;
	pmic_info.i2c = i2c;
	*pmic = &pmic_info;

	return 0;
}

int tps65185_configure(struct tps65185_info *pmic, struct vcom_cal *cal)
{
	int i;
	int ret = 0;
	union tps65185_version ver;

	assert(pmic);

	/* Cal may be NULL if not being used */
	pmic->cal = cal;

	i2c_reg_read(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_REV_ID, &ver.byte);
	printk("TPS65185: Version: %d.%d.%d\n", ver.major, ver.minor, ver.version);

	for (i = 0; (ret == 0) && (i < ARRAY_SIZE(init_data)); i++) {
		ret = i2c_reg_write(pmic->i2c, pmic->i2c_addr, init_data[i].reg, init_data[i].data);
	}

	return ret;
}

/* program the internal VCOM Dac to give us the required voltage */
int tps65185_set_vcom_register(struct tps65185_info *pmic, int value)
{
	int ret = 0;
	u8 vcom1 = value;
	u8 vcom2 = (value >> 8) & 0x01;

	assert(pmic);

	ret = i2c_reg_write(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_VCOM1, vcom1);
	if (ret == 0)
		ret = i2c_reg_write(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_VCOM2, vcom2);

	return ret;
}

/* program the internal VCOM Dac to give us the required voltage */
int tps65185_set_vcom_voltage(struct tps65185_info *pmic, int mv)
{
	int ret = 0;
	int dac_value;
	u8 vcom1;
	u8 vcom2;

	assert(pmic);

	dac_value = vcom_calculate(pmic->cal, mv);

	if (dac_value < HVPMIC_DAC_MIN)
		dac_value = HVPMIC_DAC_MIN;
	else if (dac_value > HVPMIC_DAC_MAX)
		dac_value = HVPMIC_DAC_MAX;

	vcom1 = dac_value & 0x00FF;
	vcom2 = ((dac_value >> 8) & 0x0001);

	ret = i2c_reg_write(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_VCOM1, vcom1);
	if (ret == 0)
		ret = i2c_reg_write(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_VCOM2, vcom2);

	return ret;
}

/* use i2c to determine when power up has completed */
int tps65185_wait_pok(struct tps65185_info *pmic)
{
	u8 pgstat;
	u8 int1, int2;
	volatile u8 dump = 0;

	assert(pmic);

	(void)i2c_reg_read(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_PG_STAT, &pgstat);
	(void)i2c_reg_read(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_INT1, &int1);
	(void)i2c_reg_read(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_INT2, &int2);

	if (int1 || int2)
		printk("TPS65185: PGSTAT: 0x%02X, INT1: 0x%02X, INT2: 0x%02X\n", pgstat, int1, int2);

	if (dump)
		reg_dump(pmic);

	return -EPERM;
}

/* use the i2c interface to power up the PMIC */
int tps65185_enable(struct tps65185_info *pmic)
{
	return -EPERM;
}

/* use the i2c interface to power down the PMIC */
int tps65185_disable(struct tps65185_info *pmic)
{
	return -EPERM;
}

int tps65185_temperature_measure(struct tps65185_info *pmic, short *measured)
{
	s8 temp;
	u8 progress;
	int stat;

	/* Trigger conversion */
	stat = i2c_reg_write(pmic->i2c, pmic->i2c_addr,
			HVPMIC_REG_TMST1, 0x80);

	/* wait for it to complete */
	do {
		stat = i2c_reg_read(pmic->i2c, pmic->i2c_addr,
				HVPMIC_REG_TMST1, &progress);
	} while (stat == 0 && (progress & 0x20));

	/* read the temperature */
	stat = i2c_reg_read(pmic->i2c, pmic->i2c_addr,
			HVPMIC_REG_TMST_VALUE, (u8*)&temp);

	if (stat) {
		temp = HVPMIC_TEMP_DEFAULT;
		stat = -EDEFAULT;
	}

	*measured = temp;

	printk("TPS65185: Temperature: %d\n", *measured);

	return stat;
}

