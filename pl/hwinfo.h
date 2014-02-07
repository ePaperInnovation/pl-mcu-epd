/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013, 2014 Plastic Logic Limited

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
 * pl/hwinfo.h -- Read and parse the hardware info EEPROM
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *   Andrew Cox <andrew.cox@plasticlogic.com>
 *
 */

#ifndef PL_HWINFO_H_
#define PL_HWINFO_H_

#include <stdint.h>

/** Version of the VCOM PSU EEPROM format */
#define PL_HW_INFO_VERSION 1

/** Calibration information to set the VCOM voltage accurately */
struct  __attribute__((__packed__)) pl_hw_vcom_info {
	int16_t dac_x1;     /* first DAC register value (25% of full scale) */
	int16_t dac_y1;     /* corresponding first voltage in mV */
	int16_t dac_x2;     /* second DAC register value (75% of full scale) */
	int16_t dac_y2;     /* corresponding second voltage in mV */
	int32_t vgpos_mv;   /* VGPOS in mV */
	int32_t vgneg_mv;   /* VGNEG in mV */
	int32_t swing_ideal;/* Ideal VG swing in mV for this design */
};

/** Board information */
struct __attribute__((__packed__)) pl_hw_board_info {
	char board_type[9];
	uint8_t board_ver_maj;
	uint8_t board_ver_min;
	uint8_t vcom_mode;
	uint8_t hv_pmic;
	uint8_t vcom_dac;
	uint8_t vcom_adc;
	uint8_t io_config;
	uint8_t i2c_mode;
	uint8_t temp_sensor;
	uint8_t frame_buffer;
	uint8_t epdc_ref;
	int16_t adc_scale_1;
	int16_t adc_scale_2;
};

/** Data used in VCOM PSU EEPROM format v1 */
struct __attribute__((__packed__)) pl_hw_info {
	uint8_t version;
	struct pl_hw_vcom_info vcom;
	struct pl_hw_board_info board;
	uint16_t crc;
};

struct i2c_eeprom;

extern int pl_hw_info_init(struct pl_hw_info *info, struct i2c_eeprom *eeprom);

#endif /* PL_HWINFO_H_ */
