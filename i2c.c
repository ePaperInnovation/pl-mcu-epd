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
 * i2c.c -- i2c interface abstraction layer
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include "i2c.h"

int i2c_write_bytes(struct i2c_adapter *i2c, u8 i2c_addr, u8 *data, u8 count, u8 flags)
{
	return i2c->write_bytes(i2c, i2c_addr, data, count, flags);
}

int i2c_read_bytes(struct i2c_adapter *i2c, u8 i2c_addr, u8 *data, u8 count, u8 flags)
{
	return i2c->read_bytes(i2c, i2c_addr, data, count, flags);
}

int i2c_reg_read(struct i2c_adapter *i2c, u8 i2c_addr, u8 reg, u8 *data)
{
	int ret;

	ret = i2c->write_bytes(i2c, i2c_addr, &reg, 1, I2C_NO_STOP);
	if (ret >= 0)
		ret = i2c->read_bytes(i2c, i2c_addr, data, 1, 0);

	return ret;
}

int i2c_reg_write(struct i2c_adapter *i2c, u8 i2c_addr, u8 reg, u8 data)
{
	u8 w_data[2] = { reg, data };
	return i2c->write_bytes(i2c, i2c_addr, w_data, sizeof(w_data), 0);
}

int i2c_reg_read16be(struct i2c_adapter *i2c, u8 i2c_addr, u8 reg, u16 *data)
{
	int ret;
	endianess x;

	ret = i2c->write_bytes(i2c, i2c_addr, &reg, 1, I2C_NO_STOP);
	if (ret >= 0) {
		ret = i2c->read_bytes(i2c, i2c_addr, x.bytes, 2, 0);
		*data = be16toh(x.data);
	}

	return ret;
}

int i2c_reg_write16be(struct i2c_adapter *i2c, u8 i2c_addr, u8 reg, u16 data)
{
	u8 w_data[3] = { reg, ((data >> 8) & 0x00ff), (data & 0x00ff) };
	return i2c->write_bytes(i2c, i2c_addr, w_data, sizeof(w_data), 0);
}
