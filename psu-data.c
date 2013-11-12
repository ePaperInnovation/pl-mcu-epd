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
 * psu-data.c -- Read the PSU configuration eeprom
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <stdint.h>
#include "types.h"
#include "assert.h"
#include "i2c.h"
#include "i2c-eeprom.h"
#include "crc16.h"
#include "vcom.h"
#include "utils.h"
#include "psu-data.h"

#define LOG_TAG "psu-data"

int psu_data_init(struct psu_data *data, struct i2c_eeprom *eeprom)
{
	uint16_t crc;
	struct vcom_info *info;

	assert(data != NULL);
	assert(eeprom != NULL);

	if (eeprom_read(eeprom, 0, sizeof(*data), data) < 0)
		return -1;

	if (data->version != PSU_DATA_VERSION) {
		LOG("Unsupported version number: %d, required version is %d",
		    data->version, PSU_DATA_VERSION);
		return -1;
	}

	crc = crc16_run(crc16_init, (const uint8_t *)data,
			sizeof(data->version) + sizeof(data->info));
	data->crc = be16toh(data->crc);

	if (crc != data->crc) {
		LOG("CRC mismatch: %04X instead of %04X", crc, data->crc);
		return -1;
	}

	info = &data->info;
	info->dac_x1 = be16toh(info->dac_x1);
	info->dac_y1 = be16toh(info->dac_y1);
	info->dac_x2 = be16toh(info->dac_x2);
	info->dac_y2 = be16toh(info->dac_y2);
	info->vgpos_mv = be32toh(info->vgpos_mv);
	info->vgneg_mv = be32toh(info->vgneg_mv);

	LOG("dac[0x%x]=%d, dac[0x%x]=%d, vgpos=%d, vgneg=%d, crc=0x%04X",
	    info->dac_x1, info->dac_y1, info->dac_x2, info->dac_y2,
	    info->vgpos_mv, info->vgneg_mv, data->crc);

	return 0;
}
