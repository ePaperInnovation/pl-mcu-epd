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
 * plwf.h -- Plastic Logic waveform handling using display EEPROM
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *          Andrew Cox <andrew.cox@plasticlogic.com>
 *          Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_PLWF_H
#define INCLUDE_PLWF_H 1

#include <pl/disp-data.h>
#include <stdint.h>

struct i2c_eeprom;
struct pl_wflib;

/** Initialise the pl_disp_data structure from a display EEPROM */
extern int plwf_data_init(struct pl_disp_data *data,
			  const struct i2c_eeprom *eeprom);

/** Log the display data */
extern void plwf_log(const struct pl_disp_data *data);

struct pl_wflib_eeprom {
	const struct i2c_eeprom *eeprom;
	const struct pl_disp_data *disp_data;
	uint16_t offset;
};

/** Initialise a wflib interface for an I2C EEPROM + LZSS */
extern int pl_wflib_init_eeprom(struct pl_wflib *wflib,
				struct pl_wflib_eeprom *p,
				const struct i2c_eeprom *eeprom,
				const struct pl_disp_data *disp_data);

#endif /* INCLUDE_PLWF_H */
