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
 * i2c_eeprom.c -- I2C EEPROM driver
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <pl/i2c.h>
#include <stdlib.h>
#include "i2c-eeprom.h"
#include "platform.h"
#include "assert.h"
#include "types.h" /* min() */

enum i2c_eeprom_flags {
	EEPROM_16BIT_ADDRESS = 0x01
};

struct eeprom_data {
	uint16_t size;               /* size in bytes (-1) */
	uint8_t page_size;           /* size of a page in bytes */
	uint8_t flags;               /* flags from i2c_eeprom_flags */
};

static const struct eeprom_data device_data[] = {
	{ 0x007F, 16, 0 },                   /* 24lc014 (128 Bytes) */
	{ 0x7FFF, 64, EEPROM_16BIT_ADDRESS } /* 24aa256 (32 KBytes) */
};

int eeprom_read(struct i2c_eeprom *eeprom, uint16_t offset, uint16_t count,
		uint8_t *data)
{
	const struct eeprom_data *device;
	struct pl_i2c *i2c;
	uint8_t addr[2];
	int ret;

	assert(eeprom != NULL);
	assert(data != NULL);

	i2c = eeprom->i2c;

#if MCU_DEBUG
	LOG("eeprom_read (i2c_addr=0x%02x, offset=0x%04X, count=0x%04X)",
	    eeprom->i2c_addr, offset, count);
#endif

	device = &device_data[eeprom->type];

	if (offset + count >= device->size)
		return -1;

	addr[0] = (offset >> 8) & 0x00FF;
	addr[1] = offset & 0x00FF;

	if (device->flags & EEPROM_16BIT_ADDRESS) {
		ret = i2c->write(i2c, eeprom->i2c_addr, addr, 2,
				 PL_I2C_NO_STOP);
	} else {
		ret = i2c->write(i2c, eeprom->i2c_addr, &addr[1], 1,
				 PL_I2C_NO_STOP);
	}

	if (ret)
		return -1;

	while (count) {
		const uint8_t n = min(count, 255);

		if (i2c->read(i2c, eeprom->i2c_addr, data, n, 0))
			return -1;

		count -= n;
		data += n;
	}

	return 0;
}

#if CONFIG_EEPROM_WRITE
int eeprom_write(struct i2c_eeprom *eeprom, uint16_t offset, uint16_t count,
		 const uint8_t *data)
{
	struct eeprom_data *device;
	struct pl_i2c *i2c;
	uint8_t addr[2];
	int ret;

	assert(eeprom != NULL);
	assert(data != NULL);

	i2c = eeprom->i2c;

#if MCU_DEBUG
	LOG("eeprom_write (i2c_addr=0x%02x, offset=0x%04X, count=0x%04X)",
	    eeprom->i2c_addr, offset, count);
#endif

	device = &device_data[eeprom->type];

	if (offset + count >= device->size)
		return -1;

	while (count) {
		const int n = min(count, (device->page_size -
					  (offset % device->page_size)));

		addr[0] = ((offset >> 8) & 0x00FF);
		addr[1] = (offset & 0x00FF);

		if (device->flags & EEPROM_16BIT_ADDRESS) {
			ret = i2c->write(i2c, eeprom->i2c_addr, addr, 2,
					 PL_I2C_NO_STOP);
		} else {
			ret = i2c->write(i2c, eeprom->i2c_addr, &addr[1], 1,
					 PL_I2C_NO_STOP);
		}

		if (ret)
			return -1;

		if (i2c->write(i2c, eeprom->i2c_addr, data, n,
			       PL_I2C_NO_START))
			return -1;

		count -= n;
		offset += n;
		data += n;
	}

	return 0;
}
#endif
