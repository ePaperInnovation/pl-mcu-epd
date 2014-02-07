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

#define _swap_4bytes(_x) ({						   \
	const uint16_t *_y = (uint16_t *)&(_x);				   \
	const uint16_t _z[2] = { _swap_bytes(_y[1]), _swap_bytes(_y[0]) }; \
	*((const uint32_t*)_z); })

#if CONFIG_LITTLE_ENDIAN
#define htobe16(_x) _swap_bytes(_x)
#define htole16(_x) (_x)
#define be16toh(_x) _swap_bytes(_x)
#define le16toh(_x) (_x)
#define htobe32(_x) _swap_4bytes(_x)
#define htole32(_x) (_x)
#define be32toh(_x) _swap_4bytes(_x)
#define le32toh(_x) (_x)
#else
# error "BIG ENDIAN NOT SUPPORTED, only tested on MSP430 little-endian 16-bit"
#endif

#endif /* PL_ENDIAN_H */
