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

/* structure defining the minimum information we need to have to calculate vcom
 * This is likely to be stored in the EEPROM on the power board
 */
struct  vcom_info {
	s16 dac_x1; 		/* first DAC register value (25% of full scale) */
	s16 dac_y1; 		/* corresponding first voltage in mV */
	s16 dac_x2; 		/* second DAC register value (75% of full scale) */
	s16 dac_y2; 		/* corresponding second voltage in mV */
	s32 vgpos_mv; 		/* VGPOS in mV */
	s32 vgneg_mv; 		/* VGNEG in mV */
};
#if 0
struct vcom_info {
	int16_t dac_x1;
	int16_t dac_y1;
	int16_t dac_x2;
	int16_t dac_y2;
	int32_t vgpos_mv;
	int32_t vgneg_mv;
};
#endif

struct vcom_cal;

int vcom_init(struct vcom_info *c, s32 vgswing_ideal, struct vcom_cal **p);
int vcom_calculate(struct vcom_cal *p, int mv);

#endif /* VCOM_H_ */
