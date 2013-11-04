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

void epson_softReset(void);
int epson_mode_standby(struct s1d135xx *epson);
int epson_mode_sleep(struct s1d135xx *epson);
int epson_mode_run(struct s1d135xx *epson);

int epson_power_up(void);
int epson_power_down(void);
int epson_loadEpsonCode(char *code_path);
int epson_BulkTransferFile(char *path, u32 address);
void epson_WaveformStreamInit(u32 address);
void epson_WaveformStreamTransfer(u8 *buffer, size_t len);
void epson_WaveformStreamClose(void);
int epson_loadEpsonWaveform(char *path, u32 address);
int epson_loadColorConfig(char *path, u32 address);
int epson_loadImageFile(FIL *image, short mode, int pack);
int epson_fill_buffer(short mode, u8 pack, u16 height, u16 width, u8 fill);

#endif /* EPSON_UTILS_H_ */
