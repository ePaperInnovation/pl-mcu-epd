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
 * plat-hbz6.h -- Plastic Logic Hummingbird Z6 adapter
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef PLAT_HBZ6_H_
#define PLAT_HBZ6_H_

struct platform;
struct s1d135xx;

#if 0
extern int plat_hbZn_init(struct platform *plat, const char *platform_path,
			  int i2c_on_epson);
#endif
extern int plat_s1d13541_clear(struct platform *plat);
extern int plat_s1d13541_app(struct platform *plat);

#endif /* PLAT_HBZ6_H_ */
