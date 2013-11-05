/*
 * Copyright (C) 2013 Plastic Logic Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
/*
 * epson-utils.c -- Epson controller utility functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 * Note: The Epson binary code and waveform files are stored as words
 * in little endian format but must be transferred in big endian format.
 * This can lead to much swapping of bytes.
 */

#include <msp430.h>
#include "types.h"
#include "assert.h"
#include "epson-cmd.h"
#include "FatFs/ff.h"
#include "S1D135xx.h"

/* Reset the Epson controller over SPI
 */
void epson_softReset()
{
	epson_reg_write(SOFT_RST_REG, 0xff);
}

/* Place controller in Standby mode
 * Can still access registers and RAM
 */
int epson_mode_standby(struct s1d135xx *epson)
{
	int ret = 0;

	if (epson->power_mode != PWR_STATE_STANDBY) {
		epson_wait_for_idle();
		ret = epson_cmd_p0(STBY);
		epson_wait_for_idle();
		epson->power_mode = PWR_STATE_STANDBY;
	}
	return ret;
}

/*
 * Place controller in Sleep mode.
 * Can only access few registers and bring out of sleep
 */
int epson_mode_sleep(struct s1d135xx *epson)
{
	int ret = 0;

	if (epson->power_mode != PWR_STATE_SLEEP) {
		epson_wait_for_idle();
		ret = epson_cmd_p0(SLP);
		epson_wait_for_idle();
		epson->power_mode = PWR_STATE_SLEEP;
	}
	return ret;
}

/* Place controller in Run state. Display updates possible.
 */
int epson_mode_run(struct s1d135xx *epson)
{
	int ret = 0;

	if (epson->power_mode != PWR_STATE_RUN) {
		epson_wait_for_idle();
		ret = epson_cmd_p0(RUN_SYS);
		epson_wait_for_idle();
		epson->power_mode = PWR_STATE_RUN;
	}
	return ret;
}

/***********************************************************************
 * Powerup the display power supplies using Epson power control pins
 */
int epson_power_up()
{
	short temp;
	epson_wait_for_idle();
	epson_reg_write(PWR_CTRL_REG, 0x8001);

	// wait for powerup sequence to complete
	do {
		epson_reg_read(PWR_CTRL_REG, &temp);
	} while (temp & 0x0080);
	// check PWRSTAT is good, PWR0 asserted
	if ((temp & 0x2200) != 0x2200) {
		printk("e_p_o: 0x%04x\n", temp);
	}
	assert((temp & 0x2200) == 0x2200);
	return 0;
}

/***********************************************************************
 * Powerdown the display power supplies using Epson power control pins
 */
int epson_power_down()
{
	short temp;
	epson_wait_for_idle();
	epson_reg_write(PWR_CTRL_REG, 0x8002);

	// wait for power down sequence to complete
	do {
		epson_reg_read(PWR_CTRL_REG, &temp);
	} while (temp & 0x0080);

	return 0;
}

/* Copy a byte stream to the controller as words.
 * Need to sort out swap requirements.
 */
int pack_4bpp(endianess *in, endianess *out, int in_count)
{
	while (in_count > 0) {
		out->bytes[0] = (in[0].bytes[1] & 0xf0) | (in[0].bytes[0] >> 4);
		out->bytes[1] = (in[1].bytes[1] & 0xf0) | (in[1].bytes[0] >> 4);
		in += 2;
		out++;
		in_count -= 2;
	}
	return 0;
}

#define BUFF_SIZE		(256 & ~3)
static endianess data[BUFF_SIZE];
static int transfer_file(FIL *file, int swap, int pack)
{
	int result;
	UINT count;
	UINT i;

	result = -EIO;
	while (1) {
		if (f_read(file,data[0].bytes,sizeof(data),&count) != FR_OK)
			goto read_error;

		// read count of 0 with no error is end of file.
		if (count == 0)
			break;

		// if we read an odd number of bytes make it even to flush it out.
		if (count & 1)
			count++;

		if (pack) {
			pack_4bpp(data, data, (count / 2));
			// amount of data to be output has halved
			count /= 2;
		}

		for (i = 0; i < (count/2); i++)
		{
			if (swap) {
				data[i].data = swap_bytes(data[i].data);
			}
			epson_bulk_transfer_word(data[i].data);
		}
	}

	result = 0;

read_error:

	return result;
}


/***************************************************************************************
 * Load the Epson configuration data. The file supplied by Epson is in the correct
 * format to be loaded directly so we do not need to worry about cpu endianess.
 ***************************************************************************************/
int epson_loadEpsonCode(char *code_path)
{
	FIL ControllerCode;
	int result;

	printk("Load controller code\n");

	if (f_open(&ControllerCode, code_path, FA_READ) != FR_OK)
		return (-ENOENT);

	epson_wait_for_idle();

	epson_begin_bulk_code_transfer(INIT_CMD_SET);
	result = transfer_file(&ControllerCode, false, false);
	epson_end_bulk_transfer();

	f_close(&ControllerCode);

	epson_wait_for_idle();

	return result;
}

/* Bulk transfer a file to the epson. The data is asssumed to be in cpu
 * endianess and may need swapping.
 */
int epson_BulkTransferFile(FIL *file, int pack)
{
	int result;
	result = -ENOENT;

	epson_wait_for_idle();
	epson_begin_bulk_transfer(HOST_MEM_PORT_REG);
	result = transfer_file(file, false, pack);
	epson_end_bulk_transfer();
	epson_wait_for_idle();

	return result;
}

int epson_loadEpsonWaveform(char *path, u32 address)
{
	FIL File;
	u32 file_size;

	printk("Load Waveform\n");

	if (f_open(&File, path, FA_READ) != FR_OK)
		return (-ENOENT);

	// Accomodate waveforms that had odd byte counts.
	file_size = f_size(&File);
	if (file_size & 0x01)
		file_size++;

	// begin the burst transfer operation
	epson_wait_for_idle();
	epson_cmd_p4(BST_WR_SDR,
		(address & 0x0ffff), ((address >> 16) & 0x0ffff),
		(file_size / 2) & 0x0ffff, ((file_size / 2) >> 16) & 0x0ffff);

	epson_BulkTransferFile(&File, false);

	// terminate burst operation
	epson_cmd_p0(BST_END_SDR);
	epson_wait_for_idle();

	f_close(&File);

	return 0;
}

int epson_loadColorConfig(char *path, u32 address)
{
	FIL File;
	if (f_open(&File, path, FA_READ) != FR_OK)
		return (-ENOENT);

	epson_BulkTransferFile(&File, false);

	f_close(&File);
	return 0;
}

int epson_loadImageFile(FIL *image, short mode, int pack)
{
	// load complete image, typically (1Bpp, no transparency)
	epson_cmd_p1(LD_IMG_HOST, mode);
	epson_BulkTransferFile(image, pack);
	epson_cmd_p0(LD_IMG_HOST_END);
	epson_wait_for_idle();

	return 0;
}

int epson_fill_buffer(short mode, u8 pack, u16 height, u16 width, u8 fill)
{
	u16 x,y;
	short wfill = (fill << 8) | fill;

	if (pack) {
		wfill = (fill << 12) | (fill << 8) | (fill << 4) | fill;
		width /= 2;
	}

	epson_cmd_p1(LD_IMG_HOST, mode);
	epson_wait_for_idle();

	epson_begin_bulk_transfer(HOST_MEM_PORT_REG);

	for (y = 0;  y < height; y++) {
		for (x = 0; x < width/2; x++)
			epson_bulk_transfer_word(wfill);
	}

	epson_end_bulk_transfer();
	epson_wait_for_idle();

	epson_cmd_p0(LD_IMG_HOST_END);
	epson_wait_for_idle();

	return 0;
}
