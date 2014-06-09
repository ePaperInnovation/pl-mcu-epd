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

#ifndef INCLUDE_PL_HWINFO_H
#define INCLUDE_PL_HWINFO_H 1

#include <stdint.h>

/** Version of the VCOM PSU EEPROM format */
#define PL_HWINFO_VERSION 1

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

enum hv_pmic_id {
	HV_PMIC_NONE = 0,
	HV_PMIC_MAX17135,
	HV_PMIC_TPS65185,
};

enum i2c_mode_id {
	I2C_MODE_NONE = 0,  /* invalid mode */
	I2C_MODE_HOST,      /* use the host */
	I2C_MODE_DISP,      /* use SPI-I2C bridge on the display (S1D13541) */
	I2C_MODE_S1D13524,  /* use SPI-I2C bridge on the S1D13524 */
	I2C_MODE_SC18IS6XX, /* not currently supported */ 
};

enum temp_sensor_id {
	TEMP_SENSOR_NONE = 0,
	TEMP_SENSOR_LM75,
};

enum epdc_ref {
	EPDC_NONE = 0,
	EPDC_S1D13524,
	EPDC_S1D13541,
};

/** Board information */
struct __attribute__((__packed__)) pl_hw_board_info {
	char board_type[9];
	uint8_t board_ver_maj;
	uint8_t board_ver_min;
	uint8_t vcom_mode;
	uint8_t hv_pmic;          /* enum hv_pmic_id */
	uint8_t vcom_dac;
	uint8_t vcom_adc;
	uint8_t io_config;
	uint8_t i2c_mode;         /* enum i2c_mode_id */
	uint8_t temp_sensor;      /* enum temp_sensor_id */
	uint8_t frame_buffer;
	uint8_t epdc_ref;         /* enum epdc_ref */
	int16_t adc_scale_1;
	int16_t adc_scale_2;
};

/** Data used in VCOM PSU EEPROM format v1 */
struct __attribute__((__packed__)) pl_hwinfo {
	uint8_t version;
	struct pl_hw_vcom_info vcom;
	struct pl_hw_board_info board;
	uint16_t crc;
};

#if CONFIG_HWINFO_EEPROM
struct i2c_eeprom;
extern int pl_hwinfo_init(struct pl_hwinfo *info,
			   const struct i2c_eeprom *eeprom);
#endif

extern void pl_hwinfo_log(const struct pl_hwinfo *info);

#endif /* INCLUDE_PL_HWINFO_H */
