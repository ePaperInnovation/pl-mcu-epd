/*
 *  Plastic Logic EPD project on MSP430

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
 *
 * ite-epdc.h
 *
 *  Created on: 04.01.2021
 *      Author: oliver.lenz
 */

#ifndef ITE_ITE_EPDC_H_
#define ITE_ITE_EPDC_H_

struct pl_epdc;
struct pl_dispinfo;
struct it8951;

#define  MY_WORD_SWAP(x) ( ((x & 0xff00)>>8) | ((x & 0x00ff)<<8) )

extern int ite_epdc_init(struct pl_epdc *epdc,
               const struct pl_dispinfo *dispinfo,
               struct it8951 *it8951);

//extern int epson_epdc_init_s1d13541(struct pl_epdc *epdc);

#endif /* ITE_ITE_EPDC_H_ */
