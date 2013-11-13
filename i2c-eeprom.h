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
 * i2c_eeprom.h -- Microchip i2c EEPROM driver
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef I2C_EEPROM_H_
#define I2C_EEPROM_H_

#include "types.h"

struct i2c_eeprom;
struct i2c_adapter;

#define	EEPROM_24LC014	0
#define	EEPROM_24AA256	1

int eeprom_init(struct i2c_adapter *i2c, u8 i2c_addr, u8 type,
		struct i2c_eeprom **eeprom);
int eeprom_read(struct i2c_eeprom *eeprom, u16 address, u16 count, void *data);
int eeprom_write(struct i2c_eeprom *eeprom, u16 address, u16 count, void *data);

#endif /* I2C_EEPROM_H_ */
