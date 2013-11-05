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
 * plat-ruddock2.h -- initialisation code/drivers for Ruddock2 mainboard
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef PLAT_RUDDOCK2_H_
#define PLAT_RUDDOCK2_H_

#define RUDDOCK2_SEL1	0x01
#define RUDDOCK2_SEL2	0x02
#define RUDDOCK2_SEL3	0x04
#define	RUDDOCK2_SEL4	0x08

#define RUDDOCK2_LED1	0x01
#define	RUDDOCK2_LED2	0x02
#define	RUDDOCK2_LED3	0x04
#define	RUDDOCK2_LED4	0x08

int ruddock2_init(void);
int ruddock2_selswitch_read(void);
void ruddock2_led_set(int led, int state);

#endif /* PLAT_RUDDOCK2_H_ */
