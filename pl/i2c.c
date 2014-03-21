/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013, 2014 Plastic Logic Limited

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
 * i2c.c -- i2c interface abstraction layer
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <pl/i2c.h>
#include <pl/endian.h>

int pl_i2c_reg_read_8(struct pl_i2c *i2c, uint8_t i2c_addr, uint8_t reg,
		      uint8_t *data)
{
	if (i2c->write(i2c, i2c_addr, &reg, 1, PL_I2C_NO_STOP))
		return -1;

	return i2c->read(i2c, i2c_addr, data, 1, 0);
}

int pl_i2c_reg_write_8(struct pl_i2c *i2c, uint8_t i2c_addr, uint8_t reg,
		       uint8_t data)
{
	const uint8_t w_data[2] = { reg, data };

	return i2c->write(i2c, i2c_addr, w_data, sizeof(w_data), 0);
}

int pl_i2c_reg_read_16be(struct pl_i2c *i2c, uint8_t i2c_addr, uint8_t reg,
			 uint16_t *data)
{
	if (i2c->write(i2c, i2c_addr, &reg, 1, PL_I2C_NO_STOP))
		return -1;

	if (i2c->read(i2c, i2c_addr, (uint8_t *)data, 2, 0))
		return -1;

#if CONFIG_LITTLE_ENDIAN
	swap16(data);
#endif

	return 0;
}

int pl_i2c_reg_write_16be(struct pl_i2c *i2c, uint8_t i2c_addr, uint8_t reg,
			  uint16_t data)
{
	const uint8_t w_data[3] = {
		reg, ((data >> 8) & 0xff), (data & 0xff) };

	return i2c->write(i2c, i2c_addr, w_data, sizeof(w_data), 0);
}
