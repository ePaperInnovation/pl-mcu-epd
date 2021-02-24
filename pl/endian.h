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
#include "intrinsics.h"

/** Swap the 4 bytes of a uint32_t in place, unaligned */
extern void swap32(void *x);
/** Apply swap32 to an arrary of 32-bit words */
extern void swap32_array(int32_t **x, uint16_t n);
/** Swap the 2 bytes of a uint16_t in place, unaligned */
extern void swap16(void *x);
/** Apply swap16 to an array of 16-bit words */
extern void swap16_array(int16_t **x, uint16_t n);

#define htobe16(_x) _swap_bytes(_x)
#define htole16(_x) (_x)
#define be16toh(_x) _swap_bytes(_x)
#define le16toh(_x) (_x)

#endif /* PL_ENDIAN_H */
