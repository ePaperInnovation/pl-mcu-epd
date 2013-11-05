/*
 * Copyright (C) 2013 Plastic Logic Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
/*
 * pmic-tps65185.h -- Driver for TI TPS65185 PMIC
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef PMIC_TPS65185_H_
#define PMIC_TPS65185_H_

struct tps65185_info;

int tps65185_init(struct i2c_adapter *i2c, u8 i2c_addr, struct tps65185_info **pmic);
int tps65185_configure(struct tps65185_info *pmic, struct vcom_cal *cal);
int tps65185_set_vcom_voltage(struct tps65185_info *pmic, int mv);
int tps65185_set_vcom_register(struct tps65185_info *pmic, int value);

int tps65185_wait_pok(struct tps65185_info *pmic);
int tps65185_enable(struct tps65185_info *pmic);
int tps65185_disable(struct tps65185_info *pmic);

int tps65185_temperature_measure(struct tps65185_info *pmic, short *measured);

#endif /* PMIC_TPS65185_H_ */
