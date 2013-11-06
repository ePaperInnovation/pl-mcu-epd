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
 * i2c_eeprom.h -- Microchip i2c EEPROM driver
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef PLWF_H_
#define PLWF_H_

#include <stdint.h>

#define PLWF_MAGIC 0x46574C50UL
#define PLWF_VERSION 1

#define PLWF_STR_LEN 63
#define PLWF_STR_SIZE (PLWF_STR_LEN + 1)
#define PLWF_STR_TERM(_str) do { _str[PLWF_STR_LEN] = '\0'; } while(0)

#define PLWF_VERMAGIC_OFFS 0x00
#define PLWF_VERSION_OFFS 0x04
#define PLWF_ID_OFFS 0x06
#define PLWF_TYPE_OFFS 0x46
#define PLWF_VCOM_OFFS 0x86
#define PLWF_WF_MD5_OFFS 0x8a
#define PLWF_WF_FULL_LEN_OFFS 0x9a
#define PLWF_WF_LEN_OFFS 0x9e
#define PLWF_WF_ID 0xa2
#define PLWF_TARGET 0xe2
#define PLWF_INFO_CRC_OFFS 0x122
#define PLWF_WF_OFFS 0x124

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

struct buffer_context {
	uint8_t buffer[64];
	size_t buflen;
	size_t wvf_length;
	size_t offset;
	int index;
	struct i2c_eeprom *eeprom;
	struct s1d135xx *controller;
	uint16_t crc;
};

int plwf_data_init(struct plwf_data **data);
void plwf_data_free(struct plwf_data **data);
int plwf_load_waveform(struct s1d135xx *epson, struct i2c_eeprom *plwf_eeprom, struct plwf_data *data, uint32_t address);

#endif /* PLWF_H_ */
