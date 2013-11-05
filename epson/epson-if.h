/*
 * Copyright (C) 2013 Plastic Logic Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
/*
 * epson-if.h -- Epson interface driver
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef EPSON_IF_H_
#define EPSON_IF_H_

void epsonif_init(int spi_channel, u16 divisor);

int epsonif_claim(int spi_channel, screen_t screen_id, screen_t *previous);
int epsonif_release(int spi_channel, screen_t previous);

void epsonif_close(void);
int  epsonif_select_epson(void);
int  epsonif_deselect_epson(void);
void epsonif_set_command(void);
void epsonif_set_data(void);
void epsonif_assert_reset(void);
void epsonif_negate_reset(void);

void epsonif_init_hdc(void);
void epsonif_init_hrdy(void);
void epsonif_init_reset(void);
int  epsonif_read_hrdy(void);



#endif /* EPSON_IF_H_ */
