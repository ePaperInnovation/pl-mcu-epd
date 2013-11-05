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
 * i2c_eeprom.c -- Microchip i2c EEPROM driver
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include "platform.h"
#include <stdlib.h>
#include "types.h"
#include "assert.h"
#include "i2c.h"

#define	EEPROM_16BIT_ADDRESS	0x01

struct eeprom_data {
	u16	size;			// size in bytes (-1)
	u8	page_size;		// size of a page
	u8  flags;			// various flags
};

static struct eeprom_data device_data[] = {
		{ 0x007F, 16, 0 },						// 24lc014 (power board EEPROM)
		{ 0x7FFF, 64, EEPROM_16BIT_ADDRESS }	// 24aa256 (Display EEPROM)
};

struct i2c_eeprom {
	struct i2c_adapter *i2c;
	u8 i2c_addr;
	u8 type;
};

int eeprom_init(struct i2c_adapter *i2c, u8 i2c_addr, u8 type, struct i2c_eeprom **eeprom)
{
	struct i2c_eeprom *p = (struct  i2c_eeprom*)malloc(sizeof(struct i2c_eeprom));

	assert(p);
	assert(eeprom);
	assert(type < ARRAY_SIZE(device_data));

	p->i2c = i2c;
	p->i2c_addr = i2c_addr;
	p->type = type;

	*eeprom = p;

	return 0;
}

int eeprom_read(struct i2c_eeprom *eeprom, u16 address, u16 count, void *data)
{
	int ret;
	struct eeprom_data *device;
	u8 addr[2];

	assert(eeprom);
	assert(data);

	device = &device_data[eeprom->type];

	if (address + count >= device->size)
		return -EINVAL;

	printk("eeprom_read(0x%02x): addr:0x%04X, Len:0x%04X\n", eeprom->i2c_addr, address, count);

	addr[0] = ((address >> 8) & 0x00FF);
	addr[1] = (address & 0x00FF);

	if (device->flags & EEPROM_16BIT_ADDRESS)
	{
		ret = i2c_write_bytes(eeprom->i2c, eeprom->i2c_addr, &addr[0], 2, I2C_NO_STOP);
	}
	else
	{
		ret = i2c_write_bytes(eeprom->i2c, eeprom->i2c_addr, &addr[1], 1, I2C_NO_STOP);
	}
	if (ret >= 0)
		ret = i2c_read_bytes(eeprom->i2c, eeprom->i2c_addr, (u8*)data, count, 0);

	return ret;
}

int eeprom_write(struct i2c_eeprom *eeprom, u16 address, u16 count, void *data)
{
	int ret = 0;
	struct eeprom_data *device;
	u8 addr[2];

	assert(eeprom);
	assert(data);

	device = &device_data[eeprom->type];

	if (address + count >= device->size)
		return -EINVAL;

	while (count && (ret == 0))
	{
		int write_count = min(count, (device->page_size - (address % device->page_size)));

		printk("eeprom_write(0x%02x): addr:0x%04X, Len:0x%02X\n", eeprom->i2c_addr, address, write_count);

		addr[0] = ((address >> 8) & 0x00FF);
		addr[1] = (address & 0x00FF);

		if (device->flags & EEPROM_16BIT_ADDRESS)
		{
			ret = i2c_write_bytes(eeprom->i2c, eeprom->i2c_addr, &addr[0], 2, I2C_NO_STOP);
		}
		else
		{
			ret = i2c_write_bytes(eeprom->i2c, eeprom->i2c_addr, &addr[1], 1, I2C_NO_STOP);
		}

		if (ret >= 0)
			ret = i2c_write_bytes(eeprom->i2c, eeprom->i2c_addr, (u8*)data, write_count, I2C_NO_START);

		count -= write_count;
		address += write_count;
		data = ((u8*)data) + write_count;
	}

	return ret;
}
