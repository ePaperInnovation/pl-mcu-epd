/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2014 Plastic Logic Limited

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
 * epson-epdc.h -- Epson EPDC implementations
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_EPSON_EPDC_H
#define INCLUDE_EPSON_EPDC_H 1

struct pl_epdc;
struct s1d135xx;

enum epson_epdc_ref {
	EPSON_EPDC_S1D13524,
	EPSON_EPDC_S1D13541,
};

extern int epson_epdc_init(struct pl_epdc *epdc,
			   const struct pl_disp_data *disp_data,
			   enum epson_epdc_ref ref,
			   struct s1d135xx *s1d135xx);

#if 0
extern int epson_epdc_early_init_s1d13541(void);
#endif
extern int epson_epdc_init_s1d13541(struct pl_epdc *epdc);
extern int epson_epdc_init_s1d13524(struct pl_epdc *epdc);

#endif /* INCLUDE_EPSON_EPDC_H */
