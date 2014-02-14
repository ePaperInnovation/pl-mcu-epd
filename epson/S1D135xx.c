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
 * S1D135xx.c -- Controller common functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <pl/hwinfo.h>
#include "types.h"
#include "assert.h"
#include "S1D135xx.h"
#include "epson-if.h"
#include <string.h>

#if 1
int s1d135xx_select(struct _s1d135xx *epson, screen_t *previous)
{
#if 1
	return 0;
#else
	assert(epson != NULL);
	assert(previous != NULL);

	return epsonif_claim(0, epson->screen, previous);
#endif
}

int s1d135xx_deselect(struct _s1d135xx *epson, screen_t previous)
{
#if 1
	return 0;
#else
	assert(epson);

	return epsonif_release(0, previous);
#endif
}
#endif /* 0 */
