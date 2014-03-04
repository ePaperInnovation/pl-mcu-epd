/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013. 2014 Plastic Logic Limited

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
 * pl/dispinfo.c -- Plastic Logic display information
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *          Andrew Cox <andrew.cox@plasticlogic.com>
 *          Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <pl/dispinfo.h>
#include <pl/endian.h>
#include <FatFs/ff.h>
#include <string.h>
#include "pnm-utils.h"
#include "crc16.h"
#include "i2c-eeprom.h"
#include "config.h"

#define LOG_TAG "dispinfo"
#include "utils.h"

/* Set to 1 to enable verbose debug log messages */
#define VERBOSE_LOG 0

/* Root path on the SD card */
#define ROOT_SD_PATH "0:"

/* Ensure the last byte from an EEPROM string is a null character */
#define STR_TERM(_str) do { _str[PL_DISPINFO_STR_LEN] = '\0'; } while(0)

static int change_panel_dir(const char *panel_type);

int pl_dispinfo_init_eeprom(struct pl_dispinfo *p,
			    const struct i2c_eeprom *eeprom)
{
	uint16_t crc;

	if (eeprom_read(eeprom, 0, sizeof *p, (uint8_t *)p)) {
		LOG("Failed to read EEPROM");
		return -1;
	}

	crc = crc16_run(crc16_init, (const uint8_t *)&p->info, sizeof p->info);
	p->vermagic.version = be16toh(p->vermagic.version);
	STR_TERM(p->info.panel_id);
	STR_TERM(p->info.panel_type);
	swap32(p->info.vcom);
	swap32(p->info.waveform_full_length);
	swap32(p->info.waveform_lzss_length);
	STR_TERM(p->info.waveform_id);
	STR_TERM(p->info.waveform_target);
	p->info_crc = be16toh(p->info_crc);

	if (p->vermagic.magic != PL_DISPINFO_MAGIC) {
		LOG("Invalid magic number: 0x%08lX instead of 0x%08lX",
		    p->vermagic.magic, PL_DISPINFO_MAGIC);
		return -1;
	}

	if (p->vermagic.version != PL_DISPINFO_VERSION) {
		LOG("Unsupported format version: %d, required: %d",
		    p->vermagic.version, PL_DISPINFO_VERSION);
		return -1;
	}

	if (p->info_crc != crc) {
		LOG("Info CRC mismatch: %04X instead of %04X",
		    p->info_crc, crc);
		return -1;
	}

	return change_panel_dir(p->info.panel_type);
}

int pl_dispinfo_init_fatfs(struct pl_dispinfo *p)
{
	FIL vcom_file;
	int stat;

	LOG("Loading display data from FatFS");

	if (change_panel_dir(CONFIG_DISPLAY_TYPE))
		return -1;

	p->vermagic.magic = PL_DISPINFO_MAGIC;
	p->vermagic.version = PL_DISPINFO_VERSION;
	p->info.panel_id[0] = '\0';

	/* ToDo: read panel type from display/type */
	strncpy(p->info.panel_type, CONFIG_DISPLAY_TYPE,
		sizeof p->info.panel_type);

	if (f_open(&vcom_file, "display/vcom", FA_READ) != FR_OK) {
		LOG("VCOM file not found");
		return -1;
	}

	stat = pnm_read_int32(&vcom_file, &p->info.vcom);
	f_close(&vcom_file);

	if (stat) {
		LOG("Failed to read VCOM");
		return -1;
	}

	memset(p->info.waveform_md5, 0xFF, sizeof p->info.waveform_md5);
	p->info.waveform_full_length = 0;
	p->info.waveform_lzss_length = 0;
	p->info.waveform_id[0] = '\0';
	p->info.waveform_target[0] = '\0';
	p->info_crc = 0xFFFF;

	return 0;
}

void pl_dispinfo_log(const struct pl_dispinfo *p)
{
#if VERBOSE_LOG
	const char *magic = (const char *)&p->vermagic.magic;
#endif

	LOG("Version: %d", p->vermagic.version);

#if VERBOSE_LOG
	LOG("Magic: 0x%lX %c%c%c%c",
	    p->vermagic.magic, magic[0], magic[1], magic[2], magic[3]);
#endif

#if VERBOSE_LOG
	LOG("Info CRC: 0x%04X", p->info_crc);
#endif

	LOG("Panel ID: %s", p->info.panel_id);
	LOG("Panel Type: %s", p->info.panel_type);
	LOG("VCOM: %li", p->info.vcom);
#if VERBOSE_LOG
	LOG("Waveform Length: %lu", p->info.waveform_full_length);
	LOG("Waveform Compressed Length: %lu",p->info.waveform_lzss_length);
#endif
	LOG("Waveform ID: %s", p->info.waveform_id);
	LOG("Waveform Target: %s", p->info.waveform_target);
#if VERBOSE_LOG
	printf("Waveform MD5: 0x");
	{
		int i;

		for (i = 0; i < sizeof(p->info.waveform_md5); i++)
			printf("%02x", p->info.waveform_md5[i]);
	}
	printf("\n");
#endif
}

/* ----------------------------------------------------------------------------
 * static functions
 */

static int change_panel_dir(const char *panel_type)
{
	char panel_path[MAX_PATH_LEN];

	if (join_path(panel_path, MAX_PATH_LEN, ROOT_SD_PATH, panel_type) ||
	    (f_chdir(panel_path) != FR_OK))
		return -1;

	return 0;
}
