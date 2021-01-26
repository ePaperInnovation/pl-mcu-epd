/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013. 2014 Plastic Logic Limited

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
 * pl/dispinfo.h -- Plastic Logic display information
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *          Andrew Cox <andrew.cox@plasticlogic.com>
 *          Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_PL_DISPINFO_H
#define INCLUDE_PL_DISPINFO_H 1

#include <stdint.h>

struct i2c_eeprom;

#define PL_DISPINFO_MAGIC 0x504C
#define PL_DISPINFO_VERSION 2



#define PL_DISPINFO_STR_LEN 63
#define PL_DISPINFO_STR_SIZE (0x00010)

struct __attribute__((__packed__)) pl_dispinfo_vermagic {
	uint32_t magic;
	uint16_t version;
};

struct __attribute__((__packed__)) pl_dispinfo_info {
	char panel_id[PL_DISPINFO_STR_SIZE];
	char panel_type[PL_DISPINFO_STR_SIZE];
	uint32_t vcom;
	uint8_t waveform_md5[16];
	uint32_t waveform_full_length;
	uint32_t waveform_lzss_length;
	char waveform_id[PL_DISPINFO_STR_SIZE];
	char waveform_target[PL_DISPINFO_STR_SIZE];
};

struct __attribute__((__packed__)) pl_dispinfo {
	struct pl_dispinfo_vermagic vermagic;
	struct pl_dispinfo_info info;
	uint16_t info_crc;
};

/** Initialise the pl_dispinfo structure from a display EEPROM */
extern int pl_dispinfo_init_eeprom(struct pl_dispinfo *p,
				   const struct i2c_eeprom *eeprom);

/** Initialise the pl_dispinfo structure from the SD card */
extern int pl_dispinfo_init_fatfs(struct pl_dispinfo *p);

/** Log the display data */
extern void pl_dispinfo_log(const struct pl_dispinfo *p);

#endif /* INCLUDE_PL_DISPINFO_H */
