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
 * epson-if.h -- Epson interface driver
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef EPSON_IF_H_
#define EPSON_IF_H_

#include "types.h"

struct pl_gpio;
struct epson_config;

extern int epsonif_init(struct pl_gpio *gpio,
			const struct s1d135xx_data *config);

/* stubs - to be removed */
extern int epsonif_claim(int spi_channel, screen_t screen_id,
			 screen_t *previous);
extern int epsonif_release(int spi_channel, screen_t previous);

extern void epsonif_select_epson(void);
extern void epsonif_deselect_epson(void);
extern void epsonif_set_command(void);
extern void epsonif_set_data(void);
extern void epsonif_assert_reset(void);
extern void epsonif_negate_reset(void);

extern int epsonif_init_reset(void);
extern int epsonif_read_hrdy(void);

#endif /* EPSON_IF_H_ */
