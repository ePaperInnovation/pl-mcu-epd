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
#include "psu-data.h"

#define LOG_TAG "psu-data"
#include "utils.h"

int psu_data_init(struct psu_data *data, struct i2c_eeprom *eeprom)
{
	uint16_t crc;
	struct vcom_info *vcom_info;
	struct hw_info *hw_info;

	assert(data != NULL);
	assert(eeprom != NULL);

	if (eeprom_read(eeprom, 0, sizeof(*data), data) < 0) {
		LOG("Failed to read PSU eeprom");
		return -1;
	}

	if (data->version != PSU_DATA_VERSION) {
		LOG("Unsupported version number: %d, required version is %d",
		    data->version, PSU_DATA_VERSION);
		return -1;
	}

	crc = crc16_run(crc16_init, (const uint8_t *)data,
			sizeof(data->version) + sizeof(data->vcom_info) + sizeof(data->hw_info));
	data->crc = be16toh(data->crc);

	if (crc != data->crc) {
		LOG("CRC mismatch: %04X instead of %04X", crc, data->crc);
		return -1;
	}

	vcom_info = &data->vcom_info;
	hw_info = &data->hw_info;
	vcom_info->dac_x1 = be16toh(vcom_info->dac_x1);
	vcom_info->dac_y1 = be16toh(vcom_info->dac_y1);
	vcom_info->dac_x2 = be16toh(vcom_info->dac_x2);
	vcom_info->dac_y2 = be16toh(vcom_info->dac_y2);
	vcom_info->vgpos_mv = be32toh(vcom_info->vgpos_mv);
	vcom_info->vgneg_mv = be32toh(vcom_info->vgneg_mv);

	LOG("PSU EEPROM VCOM info: dac[0x%x]=%d, dac[0x%x]=%d, vgpos=%ld,"
		" vgneg=%ld", vcom_info->dac_x1, vcom_info->dac_y1, vcom_info->dac_x2,
		vcom_info->dac_y2, vcom_info->vgpos_mv, vcom_info->vgneg_mv);

	hw_info->swing_ideal = be32toh(hw_info->swing_ideal);
	hw_info->board_type[8] = '\0';
	hw_info->adc_scale_1 = be16toh(hw_info->adc_scale_1);
	hw_info->adc_scale_2 = be16toh(hw_info->adc_scale_2);

	LOG("PSU EEPROM HW info: ideal swing=%ld, board_type=%s, "
		"board version=%d.%d, vcom mode=%d, hv pmic=%d, vcom dac=%d, "
		"vcom_adc=%d, io config=%d, i2c_mode=%d, temp sensor=%d, "
		"frame_buffer=%d, epdc_ref=%d, adc scale 1=%d, adc scale 2 = %d",
		hw_info->swing_ideal, hw_info->board_type, hw_info->board_ver_maj,
		hw_info->board_ver_min,	hw_info->vcom_mode, hw_info->hv_pmic,
		hw_info->vcom_dac, hw_info->vcom_adc, hw_info->io_config,
		hw_info->i2c_mode, hw_info->temp_sensor, hw_info->frame_buffer,
		hw_info->epdc_ref, hw_info->adc_scale_1, hw_info->adc_scale_2);

	return 0;
}
