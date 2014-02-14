/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2014 Plastic Logic Limited

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
 * epson-s1d13524.c -- Epson EPDC S1D13524
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include "epson-s1d135xx.h"
#include <pl/epdc.h>
#include <stdlib.h>
#include "assert.h"

#define LOG_TAG "s1d13524"
#include "utils.h"

#define S1D13524_STATUS_HRDY    (1 << 5)

static const struct pl_wfid epson_epdc_wf_table_s1d13524[] = {
	{ wf_refresh,      2 },
	{ wf_delta,        3 },
	{ wf_delta_mono,   4 },
	{ wf_refresh_mono, 1 },
	{ wf_init,         0 },
	{ NULL,           -1 }
};

int epson_epdc_init_s1d13524(struct pl_epdc *epdc)
{
	struct s1d135xx *s1d135xx = epdc->data;

	epdc->wf_table = epson_epdc_wf_table_s1d13524;
	s1d135xx->hrdy_mask = S1D13524_STATUS_HRDY;
	s1d135xx->hrdy_result = 0;

	abort_msg("S1D13524 not tested yet");

	return 0;
}
