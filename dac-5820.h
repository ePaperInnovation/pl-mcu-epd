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
 * dac5820.h -- Driver for MAXIM 5820 DAC
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef DAC5820_H_
#define DAC5820_H_

struct dac5820_info;

int dac5820_init(struct i2c_adapter *i2c, u8 i2c_addr, struct dac5820_info **dac);
int dac5820_configure(struct dac5820_info *dac, struct vcom_cal *cal);
int dac5820_set_power(struct dac5820_info *dac, bool on);
void dac5820_set_voltage(struct dac5820_info *dac, int vcom_mv);
int dac5820_write(struct dac5820_info *dac);

#endif /* DAC5820_H_ */
