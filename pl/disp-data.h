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
 * pl_disp_data.h -- Plastic Logic display information
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *          Andrew Cox <andrew.cox@plasticlogic.com>
 *          Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_PL_DISP_DATA_H
#define INCLUDE_PL_DISP_DATA_H 1

#include <stdint.h>

#define PL_DATA_MAGIC 0x46574C50UL
#define PL_DATA_VERSION 1

#define PL_DATA_STR_LEN 63
#define PL_DATA_STR_SIZE (PL_DATA_STR_LEN + 1)

struct __attribute__((__packed__)) pl_disp_data_vermagic {
	uint32_t magic;
	uint16_t version;
};

struct __attribute__((__packed__)) pl_disp_data_info {
	char panel_id[PL_DATA_STR_SIZE];
	char panel_type[PL_DATA_STR_SIZE];
	int32_t vcom;
	uint8_t waveform_md5[16];
	uint32_t waveform_full_length;
	uint32_t waveform_lzss_length;
	char waveform_id[PL_DATA_STR_SIZE];
	char waveform_target[PL_DATA_STR_SIZE];
};

struct __attribute__((__packed__)) pl_disp_data {
	struct pl_disp_data_vermagic vermagic;
	struct pl_disp_data_info info;
	uint16_t info_crc;
};

#endif /* INCLUDE_PL_DISP_DATA_H */
