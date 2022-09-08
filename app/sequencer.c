/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013, 2014 Plastic Logic Limited

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
 * app/sequencer.c -- Sequencer app
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <app/app.h>
#include <app/parser.h>
#include <pl/platform.h>
#include <pl/epdc.h>
#include <pl/types.h>
#include <stdlib.h>
#include <string.h>
#include "assert.h"

#define LOG_TAG "sequencer"
#include "utils.h"

/* Set to 1 to enable verbose log messages */
#define VERBOSE 1

/** Sequencer item with regions, waveform and timing information */
struct sequencer_item {
	char file[32];          /**< path to the image file to open */
	struct pl_area area;    /**< area coordinates on the display */
	int left_in;            /**< left coordinate to start reading from */
	int top_in;             /**< top coordinate to start reading from */
};

static const char SEP[] = ", ";

/* -- private functions -- */

static int load_image(struct pl_epdc *epdc, const struct sequencer_item *item,
		      const char *dir);
static int parse_item(const char *line, struct sequencer_item *item);
static int cmd_sleep(struct pl_platform *plat, const char *line);
static int cmd_image(struct pl_platform *plat, const char *line);
static int cmd_fill(struct pl_platform *plat, const char *line);
static int cmd_power(struct pl_platform *plat, const char *line);
static int cmd_update(struct pl_platform *plat, const char *line);

/* -- public entry point -- */
/*
int app_sequencer(struct pl_platform *plat, const char *path)
{
	//FIL slides;
	int stat, i;
	unsigned long lno;

	LOG("Running sequence from %s", path);

	if (f_open(&slides, path, FA_READ) != FR_OK) {
		LOG("Failed to open slideshow text file [%s]", path);
		return -1;
	}

	stat = 0;
	lno = 0;

	while (!stat) {
		struct cmd {
			const char *name;
			int (*func)(struct pl_platform *plat, const char *str);
		};
		static const struct cmd cmd_table[] = {
			{ "update", cmd_update },
			{ "power", cmd_power },
			{ "fill", cmd_fill },
			{ "image", cmd_image },
			{ "sleep", cmd_sleep },
			{ NULL, NULL }
		};
		const struct cmd *cmd;
		char line[81];
		char cmd_name[16];
		int len;

		++lno;
		stat = parser_read_file_line( line, 81);

		if (stat < 0) {
			LOG("Failed to read line");
			break;
		}
		for(i=0;i<81;i++){
			if(line[i] < 0x19) break;
			LOG("%c", line[i]);
		}
		LOG("-----------------------");
//		if (!stat) {
	//		f_lseek(&slides, 0);
		//	lno = 0;
			//continue;
		}

		stat = 0;

		if ((line[0] == '\0') || (line[0] == '#'))
			continue;

		len = parser_read_str(line, SEP, cmd_name, sizeof(cmd_name));

		if (len < 0) {
			LOG("Failed to read command");
			stat = -1;
			break;
		}

		for (cmd = cmd_table; cmd->name != NULL; ++cmd) {
			if (!strcmp(cmd->name, cmd_name)) {
				stat = cmd->func(plat, (line + len));
				break;
			}
		}

		if (cmd->name == NULL) {
			LOG("Invalid command");
			stat = -1;
			break;
		}
	}

//	f_close(&slides);

	return stat;
}
*/
/* ----------------------------------------------------------------------------
 * private functions
 */
/*
static int load_image(struct pl_epdc *epdc, const struct sequencer_item *item,
		      const char *dir)
{
	char path[MAX_PATH_LEN];

	if (join_path(path, sizeof(path), dir, item->file))
		return -1;

#if VERBOSE
	LOG("area: (%d, %d) ->  (%d, %d) %dx%d",
	    item->left_in, item->top_in, item->area.left,
	    item->area.top, item->area.width, item->area.height);
#endif

	return epdc->load_image(epdc, path, (struct pl_area*) &item->area, item->left_in,
				item->top_in);
}

static int parse_item(const char *line, struct sequencer_item *item)
{
	int *coords[] = {
		&item->left_in, &item->top_in, &item->area.left,
		&item->area.top, &item->area.width, &item->area.height,
		NULL
	};
	static const char sep[] = ", ";
	const char *opt;
	int len;

	assert(line != NULL);
	assert(item != NULL);

	opt = line;
	len = parser_read_str(opt, sep, item->file, sizeof(item->file));

	if (len <= 0)
		goto exit_now;

	opt += len;
	len = parser_read_int_list(opt, sep, coords);

	if (len <= 0)
		goto exit_now;

#if VERBOSE
	LOG("%s (%d, %d) -> (%d, %d) %dx%d",
	    item->file, item->left_in, item->top_in,
	    item->area.left, item->area.top,
	    item->area.width, item->area.height);
#endif

	return 0;

exit_now:
	if (!len)
		LOG("Not enough arguments");

	return -1;
}*/
/*
static int cmd_update(struct pl_platform *plat, const char *line)
{
	// update structure: update, wfid, update_mode, area->left, area->top, area->width, area->height,delay_ms
	struct pl_epdc *epdc = &plat->epdc;
	//char waveform[16];
	//char update_mode[16];
	enum pl_update_mode update_mode;
	struct pl_area area;
	int delay_ms;
	const char *opt;
	int len;
	int stat = 0;
	int wfid;

	opt = line;
	len = parser_read_int(opt, SEP, &wfid);

	if (len < 0)
		return -1;

	opt += len;
	//len = parser_read_str(opt, SEP, update_mode, sizeof(update_mode));
	len = parser_read_int(opt, SEP, (int*) &update_mode);

	if (len <= 0)
		return -1;

	opt += len;
	len = parser_read_area(opt, SEP, &area);

	if (len <= 0)
		return -1;

	opt += len;
	len = parser_read_int(opt, SEP, &delay_ms);

	if (len < 0)
		return -1;

	if (wfid < 0) {
		LOG("Invalid waveform id name: %i", wfid);
		return -1;
	}

	if (epdc->update(epdc,
			pl_epdc_get_wfid(epdc, wfid),
			update_mode,
			&area))
		return -1;

	mdelay(delay_ms);

	return stat;
}

static int cmd_power(struct pl_platform *plat, const char *line)
{
	struct pl_epdc *epdc = &plat->epdc;
	struct pl_epdpsu *psu = &plat->psu;
	char on_off[4];

	if (parser_read_str(line, SEP, on_off, sizeof(on_off)) < 0)
		return -1;

	if (!strcmp(on_off, "on")) {
		if (epdc->update_temp(epdc))
			return -1;

		if (psu->on(psu))
			return -1;
	} else if (!strcmp(on_off, "off")) {
		if (epdc->wait_update_end(epdc))
			return -1;

		if (psu->off(psu))
			return -1;
	} else {
		LOG("Invalid on/off value: %s", on_off);
		return -1;
	}

	return 0;
}

static int cmd_fill(struct pl_platform *plat, const char *line)
{
	struct pl_epdc *epdc = &plat->epdc;
	struct pl_area area;
	const char *opt;
	int len;
	int gl;

	opt = line;
	len = parser_read_area(opt, SEP, &area);

	if (len <= 0)
		return -1;

	opt += len;
	len = parser_read_int(opt, SEP, &gl);

	if (len < 0)
		return -1;

	if ((gl > 15) || (gl < 0)) {
		LOG("Invalid grey level value: %d", gl);
		return -1;
	}

	return epdc->fill(epdc, &area, PL_GL16(gl));
}

static int cmd_image(struct pl_platform *plat, const char *line)
{
	struct sequencer_item item;

	if (parse_item(line, &item))
		return -1;

	if (load_image(&plat->epdc, &item, "img"))
		return -1;

	return 0;
}

static int cmd_sleep(struct pl_platform *plat, const char *line)
{
	int sleep_ms;
	int len;

	len = parser_read_int(line, SEP, &sleep_ms);

	if (len < 0)
		return -1;

	if (sleep_ms < 0) {
		LOG("Invalid sleep duration: %d", sleep_ms);
		return -1;
	}

	msleep(sleep_ms);

	return 0;
}
*/
