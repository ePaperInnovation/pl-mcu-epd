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
 * i2c_eeprom.h -- I2C EEPROM driver
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef I2C_EEPROM_H
#define I2C_EEPROM_H 1

#include <stdint.h>

struct pl_i2c;

enum i2c_eeprom_type {
	EEPROM_24LC014,
	EEPROM_24AA256,
};

struct i2c_eeprom {
	struct pl_i2c *i2c;
	uint8_t i2c_addr;
	enum i2c_eeprom_type type;
};

extern int eeprom_read(const struct i2c_eeprom *eeprom, uint16_t offset,
		       uint16_t count, uint8_t *data);
#if CONFIG_EEPROM_WRITE
extern int eeprom_write(const struct i2c_eeprom *eeprom, uint16_t offset,
			uint16_t count, const uint8_t *data);
#endif

#endif /* I2C_EEPROM_H */
