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
 * i2c.h -- i2c interface abstraction layer
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef I2C_H_
#define I2C_H_

#include "types.h"

enum i2c_flags {
	I2C_NO_STOP  = (1 << 0),
	I2C_NO_START = (1 << 1)
};

struct i2c_adapter {
	int (*write_bytes)(struct i2c_adapter *i2c, u8 i2c_addr, u8 *data, u8 count, u8 flags);
	int (*read_bytes)(struct i2c_adapter *i2c, u8 i2c_addr, u8 *data, u8 count, u8 flags);
};

int i2c_write_bytes(struct i2c_adapter *i2c, u8 i2c_addr, u8 *data, u8 count, u8 flags);
int i2c_read_bytes(struct i2c_adapter *i2c, u8 i2c_addr, u8 *data, u8 count, u8 flags);

int i2c_reg_read(struct i2c_adapter *i2c, u8 i2c_addr, u8 reg, u8 *data);
int i2c_reg_write(struct i2c_adapter *i2c, u8 i2c_addr, u8 reg, u8 data);

int i2c_reg_read16be(struct i2c_adapter *i2c, u8 i2c_addr, u8 reg, u16 *data);
int i2c_reg_write16be(struct i2c_adapter *i2c, u8 i2c_addr, u8 reg, u16 data);

#endif /* I2C_H_ */
