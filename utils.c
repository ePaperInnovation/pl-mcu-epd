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
 * utils.c -- random homeless functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */
#include "platform.h"
#include "types.h"
#include "FatFs/ff.h"
#include "msp430-gpio.h"
#include "pnm-utils.h"

u16 __bswap_16(u16 x)
{
	return _swap_bytes(x);
}

u32 __bswap_32(u32 x)
{
	param32 tmp;

	tmp.data32 = x;
	u16 tmp_msw = __bswap_16(tmp.msw);
	tmp.msw = __bswap_16(tmp.lsw);
	tmp.lsw = tmp_msw;
	return tmp.data32;
}

void util_id_pins(short *gpio, int pin_count)
{
	int pin;
	int clk;

	for (pin = 0; pin < pin_count; pin++)
	{
		gpio_request(gpio[pin], PIN_GPIO | PIN_OUTPUT | PIN_INIT_LOW);
	}

	while (1) {
		for (pin = 0; pin < pin_count; pin++)
		{
			for (clk = 0; clk <= pin; clk++) {
				gpio_set_value_hi(gpio[pin]);
				gpio_set_value_lo(gpio[pin]);
			}
		}
	}
}


int util_read_vcom(void)
{
	FIL vcom_file;
	int ret;

	if (f_open(&vcom_file, "display/VCOM.txt", FA_READ) != FR_OK)
		return -ENOENT;

	ret = pnm_read_int(&vcom_file);

	f_close(&vcom_file);

	return ret;
}
