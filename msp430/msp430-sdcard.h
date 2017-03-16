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
 * msp430-sdcard.h -- MSP430 SD card SPI interface driver
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef MSP430_SDCARD_H
#define MSP430_SDCARD_H

#include <stdint.h>

struct pl_platform;
extern struct pl_platform *SDCard_plat;

extern void SDCard_init(void);
extern void SDCard_fastMode(void);
extern void SDCard_readFrame(uint8_t *pBuffer, uint16_t size);
extern void SDCard_sendFrame(uint8_t *pBuffer, uint16_t size);
extern void SDCard_setCSHigh(void);
extern void SDCard_setCSLow(void);
extern void SDCard_uDelay(uint16_t usecs);

#endif  /* MSP430_SDCARD_H */
