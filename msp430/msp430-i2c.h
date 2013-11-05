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
 * msp430-i2c.h -- MSP430 i2c interface driver
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef MSP430_I2C_H_
#define MSP430_I2C_H_

int msp430_i2c_init(u8 channel, struct i2c_adapter **i2c);

#endif /* MSP430_I2C_H_ */
