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
 *          Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef PSU_DATA_H_
#define PSU_DATA_H_

#include "vcom.h"
#include <stdint.h>

/** Version of the VCOM PSU EEPROM format */
#define PSU_DATA_VERSION 0

/** Data used in VCOM PSU EEPROM format v0 */
struct __attribute__((__packed__)) psu_data {
	uint8_t version;
	struct vcom_info info;
	uint16_t crc;
};

extern int psu_data_init(struct psu_data *data, struct i2c_eeprom *eeprom);

#endif /* PSU_DATA_H_ */
