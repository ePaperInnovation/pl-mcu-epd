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
 * probe.c -- Probing the hardware
 *
 * Authors:
 *    Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <epson/epson-epdc.h>
#include <epson/epson-i2c.h>
#include <pl/platform.h>
#include <pl/hwinfo.h>
#include <pl/dispinfo.h>
#include <pl/wflib.h>
#include <string.h>
#include <stdio.h>
#include "probe.h"
#include "assert.h"
#include "config.h"
#include "i2c-eeprom.h"
#include "pnm-utils.h"
#include "vcom.h"
#include "pmic-tps65185.h"

#define LOG_TAG "probe"
#include "utils.h"

#define I2C_PMIC_ADDR        0x68

#include "epson/epson-s1d135xx.h"

#if S1D135XX_INTERIM
#include "epson/S1D135xx.h"
struct _s1d135xx g_epson;
#endif

int probe_i2c(struct platform *plat, struct s1d135xx *s1d135xx,
	      struct pl_i2c *host_i2c, struct pl_i2c *disp_i2c)
{
	int stat;

	switch (plat->hwinfo->board.i2c_mode) {
	case I2C_MODE_HOST: /* MSP430, I2C already initialised */
		LOG("I2C: Host");
		stat = 0;
		plat->i2c = host_i2c;
		break;
	case I2C_MODE_DISP: /* This must be the Epson S1D13541... */
		LOG("I2C: S1D13541");
		stat = epson_i2c_init(s1d135xx, disp_i2c, EPSON_EPDC_S1D13541);
		plat->i2c = disp_i2c;
		break;
	case I2C_MODE_S1D13524:
		LOG("I2C: S1D13524");
		stat = epson_i2c_init(s1d135xx, disp_i2c, EPSON_EPDC_S1D13524);
		plat->i2c = disp_i2c;
		break;
	case I2C_MODE_SC18IS6XX:
		LOG("I2C: not supported");
		stat = -1;
		break;
	case I2C_MODE_NONE:
	default:
		abort_msg("Invalid I2C mode");
	}

	return stat;
}

int probe_dispinfo(struct pl_dispinfo *dispinfo, struct pl_wflib *wflib,
		   FIL *fatfs_file, const char *fatfs_path,
		   const struct i2c_eeprom *e,
		   struct pl_wflib_eeprom_ctx *e_ctx)
{
#if CONFIG_DISP_DATA_EEPROM_ONLY
	return (pl_dispinfo_init_eeprom(dispinfo, e) ||
		pl_wflib_init_eeprom(wflib, e_ctx, e, dispinfo));
#elif CONFIG_DISP_DATA_SD_ONLY
	return (pl_dispinfo_init_fatfs(dispinfo) ||
		pl_wflib_init_fatfs(wflib, fatfs_file, fatfs_path));
#elif CONFIG_DISP_DATA_EEPROM_SD
	if (pl_dispinfo_init_eeprom(dispinfo, e))
		return (pl_dispinfo_init_fatfs(dispinfo) ||
			pl_wflib_init_fatfs(wflib, fatfs_file, fatfs_path));
	else
		return pl_wflib_init_eeprom(wflib, e_ctx, e, dispinfo);
#elif CONFIG_DISP_DATA_SD_EEPROM
	if (pl_dispinfo_init_fatfs(dispinfo))
		return (pl_dispinfo_init_eeprom(dispinfo, e) ||
			pl_wflib_init_eeprom(wflib, e_ctx, e, dispinfo));
	else
		return pl_wflib_init_fatfs(wflib, fatfs_file, fatfs_path);
#endif
}

int probe(struct platform *plat, struct s1d135xx *s1d135xx)
{
	const struct pl_hwinfo *hwinfo = plat->hwinfo;
	struct pl_epdc *epdc = &plat->epdc;

	/* ToDo: This should be either in platform or main */
	struct vcom_cal vcomcal;
	static struct tps65185_info *pmic_info;

	int stat;

	/* -- Initialise the VCOM and HV-PMIC -- */

	vcom_init(&vcomcal, &hwinfo->vcom);

	switch (hwinfo->board.hv_pmic) {
	case HV_PMIC_NONE:
		LOG("HV-PMIC: None");
		stat = 0;
		break;
	case HV_PMIC_MAX17135:
		LOG("HV-PMIC: MAX17135");
		abort_msg("Not verified yet");
		stat = -1;
		break;
	case HV_PMIC_TPS65185:
		LOG("HV-PMIC: TPS65185");
		stat = tps65185_init(plat->i2c, I2C_PMIC_ADDR, &pmic_info,
				     &vcomcal);
		if (!stat) /* ToDo: generalise set_vcom with HV-PMIC API */
			tps65185_set_vcom_voltage(pmic_info,
						  plat->dispinfo->info.vcom);
		break;
	default:
		abort_msg("Invalid HV-PMIC id");
	}

	if (stat) {
		LOG("Failed to initialise HV-PMIC");
		return -1;
	}

#if S1D135XX_INTERIM
	s1d135xx->epson = &g_epson;
	epsonif_hack(&plat->gpio, s1d135xx->data);
#endif

	/* -- Initialise the EPDC controller -- */

	switch (hwinfo->board.epdc_ref) {
	case EPDC_S1D13524:
		LOG("EPDC: S1D13524");
		stat = epson_epdc_init(epdc, plat->dispinfo,
				       EPSON_EPDC_S1D13524, s1d135xx);
		break;
	case EPDC_S1D13541:
		LOG("EPDC: S1D13541");
		stat = epson_epdc_init(epdc, plat->dispinfo,
				       EPSON_EPDC_S1D13541, s1d135xx);
		break;
	case EPDC_NONE:
#if PL_EPDC_STUB
		LOG("EPDC: Stub");
		stat = pl_epdc_stub_init(epdc);
		break;
#endif /* fall through otherwise */
	default:
		abort_msg("Invalid EPDC identifier");
	}

	if (stat) {
		LOG("Failed to initialise EPDC");
		return -1;
	}

#if 0 /* enable during development of new EPDC implementations */
	assert(epdc->init != NULL);
	assert(epdc->load_wflib != NULL);
	assert(epdc->update != NULL);
	assert(epdc->wait_update_end != NULL);
	assert(epdc->set_power != NULL);
	assert(epdc->set_temp_mode != NULL);
	assert(epdc->update_temp != NULL);
	assert(epdc->fill != NULL);
	assert(epdc->load_image != NULL);
#endif

	return 0;
}
