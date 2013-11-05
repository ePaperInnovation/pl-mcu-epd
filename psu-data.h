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
 * psu-data.h -- Read/Write to the PSU configuration eeprom
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef PSU_DATA_H_
#define PSU_DATA_H_

enum PSU_BOARD_ID {
		PSU_HB_Z6 = 0x01,
		PSU_HB_Z7 = 0x02,
		PSU_RAVEN = 0x03
};

struct eeprom_data;

int psu_data_init(struct eeprom_data **data);
void psu_data_free(struct eeprom_data **data);
int psu_data_get_vcom_data(struct eeprom_data *data, struct vcom_info *vcom);
int psu_data_get_board_info(struct eeprom_data *data, u8 *board);
int psu_data_read(struct i2c_eeprom *eeprom, struct eeprom_data *data);

void psu_data_set_header_version(struct eeprom_data *data, u8 version);
int psu_data_set_vcom_data(struct eeprom_data *data, struct vcom_info *vcom);
int psu_data_set_board_info(struct eeprom_data *data, u8 board);
int psu_data_write(struct i2c_eeprom *eeprom, struct eeprom_data *data);

#endif /* PSU_DATA_H_ */
