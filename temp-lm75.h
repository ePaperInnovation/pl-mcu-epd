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
 * temp-lm75.h -- Driver for MAXIM LM75 temperature sensor
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

struct lm75_info;
struct i2c_temp_sensor_info;

int lm75_init(struct i2c_adapter *i2c, u8 i2c_addr, struct lm75_info **lm75);
int lm75_temperature_measure(struct lm75_info *lm75, short *measured);

int lm75_export_temp_sensor(struct lm75_info *lm75, struct i2c_temp_sensor_info **sensor);

#ifndef TEMP_LM75_H_
#define TEMP_LM75_H_



#endif /* TEMP_LM75_H_ */
