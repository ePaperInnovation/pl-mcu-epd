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

#ifndef PLWF_H_
#define PLWF_H_

#include <pl/disp-data.h>
#include <stdint.h>

struct i2c_eeprom;
struct s1d135xx;

enum plwf_mode {
	PLWF_MODE_EEPROM,
	PLWF_MODE_SD_CARD,
};

struct plwf {
	enum plwf_mode mode;
	struct i2c_eeprom *eeprom; /* use with PLWF_MODE_EEPROM */
	struct pl_disp_data data;  /* initialised by plwf_init */
};

/** Initialise the pl_disp_data structure from a display EEPROM */
extern int plwf_data_init(struct pl_disp_data *data,
			  const struct i2c_eeprom *eeprom);

/** Log the display data */
extern void plwf_log(const struct pl_disp_data *data);

/** Read the waveform from the EEPROM and send it to the EPSON */
extern int plwf_load_wf(struct pl_disp_data *data,
			const struct i2c_eeprom *eeprom,
			struct s1d135xx *epson, uint32_t addr);

#endif /* PLWF_H_ */
