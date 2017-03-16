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
 * app/app.h -- Application
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_APP_H
#define INCLUDE_APP_H 1

struct pl_platform;

extern int app_stop;

extern int app_demo(struct pl_platform *plat);
extern int app_clear(struct pl_platform *plat);
extern int app_power(struct pl_platform *plat, const char *path);
extern int app_slideshow(struct pl_platform *plat, const char *path);
extern int app_sequencer(struct pl_platform *plat, const char *path);
extern int app_pattern(struct pl_platform *plat);

#endif /* INCLUDE_APP_H */
