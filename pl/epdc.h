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

#include <stdint.h>
#include <pl/wflib.h>

/* Set to 1 to enable stub EPDC implementation */
#define PL_EPDC_STUB 0

/* Use this macro to convert a 16-greyscale value to 8 bits */
#define PL_GL16(_g) ({			\
	uint8_t g16 = (_g) & 0xF;	\
	g16 | g16 << 4;			\
})

#define PL_WHITE PL_GL16(15)
#define PL_BLACK PL_GL16(0)

enum pl_epdc_power_state {
	PL_EPDC_RUN = 0,
	PL_EPDC_STANDBY,
	PL_EPDC_SLEEP,
	PL_EPDC_OFF,
};

enum pl_epdc_temp_mode {
	PL_EPDC_TEMP_MANUAL,
	PL_EPDC_TEMP_EXTERNAL,
	PL_EPDC_TEMP_INTERNAL,
};

struct pl_area;
struct pl_dispinfo;
struct pl_epdpsu;

struct pl_wfid {
	const char *path;
	int id;
};

struct pl_epdc{
	int (*clear_init)(struct pl_epdc *p);
	int (*load_wflib)(struct pl_epdc *p);
	int (*update)(struct pl_epdc *p, int wfid, const struct pl_area *area);
	int (*wait_update_end)(struct pl_epdc *p);
	int (*set_power)(struct pl_epdc *p, enum pl_epdc_power_state state);
	int (*set_temp_mode)(struct pl_epdc *p, enum pl_epdc_temp_mode mode);
	int (*update_temp)(struct pl_epdc *p);
	int (*fill)(struct pl_epdc *p, const struct pl_area *area, uint8_t g);
	int (*pattern_check)(struct pl_epdc *p);
	int (*load_image)(struct pl_epdc *p, const char *path,
			  const struct pl_area *area, int left, int top);
	int (*set_epd_power)(struct pl_epdc *p, int on);

	const struct pl_wfid *wf_table;
	const struct pl_dispinfo *dispinfo;
	struct pl_wflib wflib;
	enum pl_epdc_power_state power_state;
	enum pl_epdc_temp_mode temp_mode;
	int manual_temp;
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

/** Perform a typical single image update:
 * # Update temperature
 * # Turn the EPD PSU on
 * # Generate an update with the given waveform and area
 * # Wait for the update to end
 * # Turn the EPD PSU off
 */
extern int pl_epdc_single_update(struct pl_epdc *epdc, struct pl_epdpsu *psu,
				 int wfid, const struct pl_area *area);

#if PL_EPDC_STUB
/** Initialise a stub implementation for debugging purposes */
extern int pl_epdc_stub_init(struct pl_epdc *p);
#endif

#endif /* INCLUDE_PL_EPDC_H */
