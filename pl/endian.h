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
 * pl/endian.h -- byte order utilities
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef PL_ENDIAN_H
#define PL_ENDIAN_H 1

#include <stdint.h>
#include "config.h"

/** Swap the 4 bytes of a uint32_t in place */
#define swap32(_x) do {							\
	uint8_t *_b = (uint8_t *)&(_x);					\
	uint8_t _c;							\
	_c = _b[0];							\
	_b[0] = _b[3];							\
	_b[3] = _c;							\
	_c = _b[1];							\
	_b[1] = _b[2];							\
	_b[2] = _c;							\
 } while (0)

#if CONFIG_LITTLE_ENDIAN
#define htobe16(_x) _swap_bytes(_x)
#define htole16(_x) (_x)
#define be16toh(_x) _swap_bytes(_x)
#define le16toh(_x) (_x)
#else
# error "BIG ENDIAN NOT SUPPORTED, only tested on MSP430 little-endian 16-bit"
#endif

#endif /* PL_ENDIAN_H */
