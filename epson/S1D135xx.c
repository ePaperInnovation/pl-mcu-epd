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

#include "platform.h"
#include "types.h"
#include "assert.h"
#include "S1D135xx.h"
#include "epson-if.h"
#include <string.h>

int s1d135xx_get_wfid(const char *wf_name)
{
	static const char *wfid_table[_WVF_N_] = {
		"init", "refresh", "delta", "delta/mono"
	};
	int wfid;

	assert(wf_name != NULL);

	for (wfid = 0; wfid < _WVF_N_; ++wfid)
		if (!strcmp(wfid_table[wfid], wf_name))
			break;

	if (wfid == _WVF_N_)
		return -1;

	return wfid;
}

int s1d135xx_select(struct s1d135xx *epson, screen_t *previous)
{
	assert(epson != NULL);
	assert(previous != NULL);

	return epsonif_claim(0, epson->screen, previous);
}

int s1d135xx_deselect(struct s1d135xx *epson, screen_t previous)
{
	assert(epson);

	return epsonif_release(0, previous);
}
