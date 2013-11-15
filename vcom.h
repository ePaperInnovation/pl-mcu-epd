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
 * vcom.h -- VCOM Calculation support
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef VCOM_H_
#define VCOM_H_

#include <stdint.h>

/* structure defining the minimum information we need to have to calculate vcom
 * This is likely to be stored in the EEPROM on the power board
 */
struct  __attribute__((__packed__)) vcom_info {
	int16_t dac_x1;     /* first DAC register value (25% of full scale) */
	int16_t dac_y1;     /* corresponding first voltage in mV */
	int16_t dac_x2;     /* second DAC register value (75% of full scale) */
	int16_t dac_y2;     /* corresponding second voltage in mV */
	int32_t vgpos_mv;   /* VGPOS in mV */
	int32_t vgneg_mv;   /* VGNEG in mV */
};

struct vcom_cal {
	int32_t swing;
	int32_t swing_ideal;
	int32_t dac_offset;
	int32_t dac_dx;
	int32_t dac_dy;
	int32_t dac_step_mv;
};

/** Initialise a vcom_cal structure */
extern void vcom_init(struct vcom_cal *v, const struct vcom_info *c,
		      int32_t vgswing_ideal);

/** Get the DAC register value for a given VCOM input voltage */
extern int vcom_calculate(const struct vcom_cal *v, int input_mv);

#endif /* VCOM_H_ */
