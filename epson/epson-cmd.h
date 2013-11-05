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
 * epson-cmd.h -- Epson controller high level commands
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef EPSON_CMD_H_
#define EPSON_CMD_H_

#include <msp430.h>
#include "epson-if.h"
#include "msp430-spi.h"

/* we could decorate the commands with bits to indicate that a wait should
 * be performed pre/post command issue. Would save a lot of calls to
 * epson_wait_for_idle()
 */
#define	PRE_WAIT_READY	0x8000
#define	POST_WAIT_READY	0x4000

#define PRE_WAIT(_x_)	((_x_) | PRE_WAIT_READY)
#define POST_WAIT(_x_)	((_x_) | POST_WAIT_READY)

// function prototypes
void epson_init_comm(void);
void epson_close_comm(void);
int epson_cmd_p0(short command);
int epson_cmd_p1(short command, short p1);
int epson_cmd_p2(short command, short p1, short p2);
int epson_cmd_p3(short command, short p1, short p2, short p3);
int epson_cmd_p4(short command, short p1, short p2, short p3, short p4);
int epson_cmd_p5(short command, short p1, short p2, short p3, short p4, short p5);

int epson_begin_bulk_code_transfer(short command);
int epson_begin_bulk_transfer(short command);
int epson_bulk_transfer_word(short data);
int epson_bulk_transfer_raw_data(u8 *buffer, size_t length);
int epson_end_bulk_transfer(void);

int epson_is_busy(void);
void epson_wait_for_idle(void);
void epson_wait_for_idle_mask(short mask, short result);
int epson_reg_read(short reg, short* value);
int epson_reg_write(short reg, short value);

#endif /* EPSON_CMD_H_ */
