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
 * plat-raven.h -- Platform file for the Raven 10.7" display electronics
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef PLAT_RAVEN_H_
#define PLAT_RAVEN_H_

int plat_raven_init(void);
int plat_s1d13524_init_display(struct s1d135xx *epson);
void plat_s1d13524_slideshow(struct s1d135xx *epson);

#endif /* PLAT_RAVEN_H_ */
