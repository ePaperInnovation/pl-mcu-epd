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
 * temp-lm75.c -- Driver for MAXIM LM75 temperature sensor
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include "types.h"
#include "assert.h"
#include "i2c.h"

#define	LM75_TEMP_DEFAULT	20

/* used to describe external i2c sensor to epson controller.
 * include i2c adapter so epson can check its the same one */
struct i2c_temp_sensor_info {
	struct i2c_adapter *i2c;
	u8 i2c_addr;
	u8 reg;
};

/* Lm75 i2c temperature sensor */
struct lm75_info {
	struct i2c_adapter *i2c;
	u8 i2c_addr;
	struct i2c_temp_sensor_info sensor;
} lm75_data;

union lm75_temp_value {
	struct {
		int padding:7;
		int measured:9;
	};
	uint16_t word;
};

int lm75_init(struct i2c_adapter *i2c, u8 i2c_addr, struct lm75_info **lm75)
{
	assert(i2c);
	assert(lm75);

	lm75_data.i2c = i2c;
	lm75_data.i2c_addr = i2c_addr;
	lm75_data.sensor.i2c_addr = i2c_addr;
	lm75_data.sensor.reg = 0;

	*lm75 = &lm75_data;

	return 0;
}

int lm75_export_temp_sensor(struct lm75_info *lm75, struct i2c_temp_sensor_info **sensor)
{
	assert(lm75);
	assert(sensor);

	*sensor = &lm75->sensor;

	return 0;
}

int lm75_temperature_measure(struct lm75_info *lm75, short *measured)
{
	union lm75_temp_value temp;
	int ret;

	ret = i2c_reg_read_16be(lm75->i2c, lm75->i2c_addr, lm75->sensor.reg,
				&temp.word);

	if (ret) {
		*measured = LM75_TEMP_DEFAULT;
		ret = -EDEFAULT;
	}
	else
		*measured = (temp.measured >> 1);

	printk("LM75: Temperature: %d\n", *measured);

	return ret;
}
