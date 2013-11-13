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

#include <stdint.h>

#define PLWF_MAGIC 0x46574C50UL
#define PLWF_VERSION 1

#define PLWF_STR_LEN 63
#define PLWF_STR_SIZE (PLWF_STR_LEN + 1)

struct __attribute__((__packed__)) plwf_vermagic {
	uint32_t magic;
	uint16_t version;
};

struct __attribute__((__packed__)) plwf_info {
	char panel_id[PLWF_STR_SIZE];
	char panel_type[PLWF_STR_SIZE];
	int32_t vcom;
	uint8_t waveform_md5[16];
	uint32_t waveform_full_length;
	uint32_t waveform_lzss_length;
	char waveform_id[PLWF_STR_SIZE];
	char waveform_target[PLWF_STR_SIZE];
};

struct __attribute__((__packed__)) plwf_data {
	struct plwf_vermagic vermagic;
	struct plwf_info info;
	uint16_t info_crc;
};

struct i2c_eeprom;
struct s1d135xx;

/** Initialise the plwf_data structure */
extern int plwf_data_init(struct plwf_data *data, struct i2c_eeprom *eeprom);

/** Read the waveform from the EEPROM and send it to the EPSON */
extern int plwf_load_wf(struct plwf_data *data, struct i2c_eeprom *eeprom,
			struct s1d135xx *epson, uint32_t addr);

#endif /* PLWF_H_ */
