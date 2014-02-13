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
 * epdc.h -- EPDC interface abstraction layer
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_PL_EPDC_H
#define INCLUDE_PL_EPDC_H 1

/* Set to 1 to enable stub EPDC implementation */
#define PL_EPDC_STUB 1

struct pl_area;

struct pl_wfid {
	const char *path;
	int id;
};

enum pl_epdc_power_state {
	PL_EPDC_RUN,
	PL_EPDC_STANDBY,
	PL_EPDC_SLEEP,
	PL_EPDC_OFF,
};

struct pl_epdc{
	int (*update)(struct pl_epdc *p, int wfid);
	int (*update_area)(struct pl_epdc *p, int wfid,
			   const struct pl_area *area);
	int (*wait_idle)(struct pl_epdc *p);
	int (*set_power)(struct pl_epdc *p, enum pl_epdc_power_state state);

	/* ToDo:
	   send waveform library
	   manage temperature
	 */

	const struct pl_wfid *wf_table;
	unsigned xres;
	unsigned yres;
	void *data;
};

/* --- Waveform management --- */

/** Waveform string elements */
#define WF_INIT "init"
#define WF_REFRESH "refresh"
#define WF_DELTA "delta"
#define WF_MONO "mono"

/** Optimised look-up path strings - use for improved performance */
extern const char wf_init[];          /**< init */
extern const char wf_refresh[];       /**< refresh */
extern const char wf_delta[];         /**< delta */
extern const char wf_refresh_mono[];  /**< refresh/mono */
extern const char wf_delta_mono[];    /**< delta/mono */

/** Get a waveform identifier or -1 if not found */
extern int pl_epdc_get_wfid(struct pl_epdc *p, const char *wf_path);

#if PL_EPDC_STUB
/** Initialise a stub implementation for debugging purposes */
extern int pl_epdc_stub_init(struct pl_epdc *p);
#endif

#endif /* INCLUDE_PL_EPDC_H */
