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
 * epson-utils.h -- Epson controller utility functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *          Andrew Cox <andrew.cox@plasticlogic.com>
 *
 */

#ifndef EPSON_UTILS_H_
#define EPSON_UTILS_H_

#include "FatFs/ff.h"
#include "types.h"
#include "S1D135xx.h"
#include <stdint.h>

/* Powerup the display power supplies using Epson power control pins. */
extern void epson_power_up(void);

/* Powerdown the display power supplies using Epson power control pins. */
extern void epson_power_down(void);

/* Reset the Epson controller over SPI.  */
extern void epson_softReset(void);

/* Place controller in Standby mode. Can still access registers and RAM.  */
extern int epson_mode_standby(struct s1d135xx *epson);

/* Place controller in Sleep mode. Can only access few registers and bring out
 * of sleep.  */
extern int epson_mode_sleep(struct s1d135xx *epson);

/* Place controller in Run state. Display updates possible.  */
extern int epson_mode_run(struct s1d135xx *epson);

/* Load the Epson configuration data. The file supplied by Epson is in the
 * correct format to be loaded directly so we do not need to worry about cpu
 * endianess.  */
extern int epson_loadEpsonCode(char *code_path);

/* Bulk transfer a file to the epson. The data is asssumed to be in cpu
 * endianess and may need swapping.  */
extern int epson_BulkTransferFile(char *path, uint32_t address);

/* Similar to epson_BulkTransferFile but deals with image data and area.  */
extern int epson_BulkTransferImage(FIL *file, int pack,
				   const struct area *area, int left, int top,
				   int img_width);

extern void epson_WaveformStreamInit(uint32_t address);
extern void epson_WaveformStreamTransfer(uint8_t *buffer, size_t len);
extern void epson_WaveformStreamClose(void);
extern int epson_loadEpsonWaveform(char *path, uint32_t address);
extern int epson_loadColorConfig(char *path, uint32_t address);
extern int epson_loadImageFile(FIL *image, uint16_t mode, int pack);
extern int epson_loadImageFileArea(FIL *image, uint16_t mode, int pack,
				   const struct area *area, int left, int top,
				   int img_width);
extern void epson_fill_buffer(uint16_t mode, uint8_t pack, uint16_t height,
			      uint16_t width, uint8_t fill);
extern void epson_fill_area(uint16_t mode, uint8_t pack,
			    const struct area *area, uint8_t fill);

#endif /* EPSON_UTILS_H_ */
