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
#include <string.h>
#include "assert.h"
#include "pnm-utils.h"
#include "crc16.h"
//#include "i2c-eeprom.h"
#include <spi-eeprom.h>
#include "config.h"

#define LOG_TAG "dispinfo"
#include "utils.h"

/* Set to 1 to enable verbose debug log messages */
#define VERBOSE 0

/* Root path on the SD card */
#define ROOT_SD_PATH "0:"

/* Ensure the last byte from an EEPROM string is a null character */
#define STR_TERM(_str) do { _str[PL_DISPINFO_STR_LEN] = '\0'; } while(0)


int pl_dispinfo_init_eeprom(struct pl_dispinfo *p,
                            const struct i2c_eeprom *eeprom)
{
    uint16_t dispInfoMagicWord = 0x504c;
    uint16_t crc;

    assert(p != NULL);
    assert(eeprom != NULL);

//	if (eeprom_read(eeprom, 0, sizeof *p, (uint8_t *)p)) {
//		LOG("Failed to read EEPROM");
//		return -1;
//	}

    if (nvm_MX25_spi_read(0, (uint8_t*) p, MX25U4033E_SIZE))
    {
        LOG("Failed to read EEPROM");
        return -1;
    }

    //crc = crc16_run(crc16_init, (const uint8_t *)&p->info, sizeof p->info);

    STR_TERM(p->info.panel_id);
    STR_TERM(p->info.panel_type);
    STR_TERM(p->info.waveform_id);
    STR_TERM(p->info.waveform_target);

//	uint16_t temp = p->vermagic.magic;
//	printf("Magic Word: %d\n", temp);

    if (p->vermagic.magic != PL_DISPINFO_MAGIC)
    {
        printf("%-16s " "Invalid magic number: 0x%08lX instead of 0x%08lX" "\n",
               "dispinfo", p->vermagic.magic, 0x504C);
        return -1;
    }

    if (p->vermagic.version != PL_DISPINFO_VERSION)
    {
        printf("%-16s " "Unsupported format version: %d, required: %d" "\n",
               "dispinfo", p->vermagic.version, 2);
        return -1;
    }

}
/////////////////////added by Mohamed ahmed: to read all the display info from here//////////////////////


int pl_dispinfo_init_UST(struct pl_dispinfo *p)
{
//    FIL vcom_file;
//    int stat;

    assert(p != NULL);

    LOG("Loading display data from FatFS");



    p->vermagic.magic = PL_DISPINFO_MAGIC;
    p->vermagic.version = PL_DISPINFO_VERSION;
    p->info.panel_id[0] = '\0';
    strncpy(p->info.panel_type, "S468_T1.1",
            16);

    p->info.vcom = 3000;
    memset(p->info.waveform_md5, 0xFF, sizeof p->info.waveform_md5);
    p->info.waveform_full_length = 0;
    p->info.waveform_lzss_length = 0;
    p->info.waveform_id[0] = '\0';
    p->info.waveform_target[0] = '\0';
    p->info_crc = 0xFFFF;

    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////


void pl_dispinfo_log(const struct pl_dispinfo *p)
{
#if VERBOSE
	const char *magic = (const char *)&p->vermagic.magic;

	LOG("Version: %d", p->vermagic.version);
	LOG("Magic: 0x%lX %c%c%c%c",
	    p->vermagic.magic, magic[0], magic[1], magic[2], magic[3]);
	LOG("Info CRC: 0x%04X", p->info_crc);
	LOG("Panel ID: %s", p->info.panel_id);
#endif
    printf("%-16s " "Panel Type: %s" "\n", "dispinfo", p->info.panel_type);
    printf("%-16s " "VCOM: %li" "\n", "dispinfo", p->info.vcom);
#if VERBOSE
	LOG("Waveform Length: %lu", p->info.waveform_full_length);
	LOG("Waveform Compressed Length: %lu",p->info.waveform_lzss_length);
	LOG("Waveform ID: %s", p->info.waveform_id);
#endif
    printf("%-16s " "Waveform Target: %s" "\n", "dispinfo",
           p->info.waveform_target);
#if VERBOSE
	printf("Waveform MD5: 0x");
	{
		int i;

		for (i = 0; i < sizeof(p->info.waveform_md5); i++)
			printf("%02x", p->info.waveform_md5[i]);
	}
	printf("\n");
#endif
}

