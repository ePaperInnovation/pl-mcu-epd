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
 * pmic-tps65185.h -- Driver for TI TPS65185 PMIC
 *
 * Authors:
 *  Nick Terry <nick.terry@plasticlogic.com>
 *  Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_PMIC_TPS65185_H
#define INCLUDE_PMIC_TPS65185_H 1

#include <stdint.h>

struct pl_i2c;
struct vcom_cal;

struct tps65185_info {
	struct pl_i2c *i2c;
	uint8_t i2c_addr;
	const struct vcom_cal *cal;
};

extern int tps65185_init(struct tps65185_info *pmic, struct pl_i2c *i2c,
			 uint8_t i2c_addr, const struct vcom_cal *cal);
extern int tps65185_set_vcom_voltage(struct tps65185_info *pmic, int mv);
extern int tps65185_set_vcom_register(struct tps65185_info *pmic, int value);

extern int tps65185_wait_pok(struct tps65185_info *pmic);
extern int tps65185_enable(struct tps65185_info *pmic);
extern int tps65185_disable(struct tps65185_info *pmic);

extern int tps65185_temperature_measure(struct tps65185_info *pmic,
					int16_t *measured);

#endif /* INCLUDE_PMIC_TPS65185_H */
