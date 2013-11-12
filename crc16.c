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

#include <crc16.h>

#define WIDTH 16
#define TOPBIT 0x8000
#define POLYNOMIAL 0x1021

const uint16_t crc16_init = 0xFFFF;

uint16_t crc16_run(uint16_t crc, const uint8_t *data, size_t length)
{
	size_t i;

	/* Perform modulo-2 division, a byte at a time. */

	for (i = 0; i < length; ++i) {
		int bit;

		/* Next byte into the remainder (crc) */
		crc ^= ((uint16_t)data[i]) << (WIDTH - 8);

		/* Perform modulo-2 division, a bit at a time. */
		for (bit = 8; bit; --bit) {
			if (crc & TOPBIT)
				crc = (crc << 1) ^ POLYNOMIAL;
			else
				crc = crc << 1;
		}
	}

	return crc;
}




