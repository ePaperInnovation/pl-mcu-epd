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
 * probe.h -- Probing the hardware
 *
 * Authors:
 *    Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_PROBE_H
#define INCLUDE_PROBE_H 1

#include <FatFs/ff.h>

struct platform;
struct s1d135xx;
struct pl_i2c;
struct pl_dispinfo;
struct i2c_eeprom;
struct pl_wflib_eeprom_ctx;
struct vcom_cal;
struct pl_epdpsu_gpio;
struct tps65185_info;

extern int probe_i2c(struct platform *plat, struct s1d135xx *s1d135xx,
		     struct pl_i2c *host_i2c, struct pl_i2c *disp_i2c);
extern int probe_dispinfo(struct pl_dispinfo *dispinfo, struct pl_wflib *wflib,
			  FIL *fatfs_file, const char *fatfs_path,
			  const struct i2c_eeprom *e,
			  struct pl_wflib_eeprom_ctx *e_ctx);
extern int probe_hvpmic(struct platform *plat, struct vcom_cal *vcom_cal,
			struct pl_epdpsu_gpio *epdpsu_gpio,
			struct tps65185_info *pmic_info);
extern int probe_epdc(struct platform *plat, struct s1d135xx *s1d135xx);

#endif /* INCLUDE_PROBE_H */
