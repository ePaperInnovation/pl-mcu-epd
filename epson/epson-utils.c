/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013 Plastic Logic Limited

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
#include "utils.h"

#define LOG_TAG "epson"

/* Size of the buffer used for plain file transfers */
#define FILE_BUFFER_SIZE 256

/* Size of the buffer used for image file transfers, must be able to hold a
 * line worth of pixels */
#define IMAGE_BUFFER_LENGTH 720

void epson_power_up(void)
{
	uint16_t temp;

	epson_wait_for_idle();
	epson_reg_write(PWR_CTRL_REG, 0x8001);

	/* wait for powerup sequence to complete */
	do {
		epson_reg_read(PWR_CTRL_REG, &temp);
	} while (temp & 0x0080);

	/* check PWRSTAT is good, PWR0 asserted */
	if ((temp & 0x2200) != 0x2200) {
		LOG("e_p_o: 0x%04x", temp);
		abort_msg("Failed to turn the display power on");
	}
}

void epson_power_down(void)
{
	uint16_t temp;

	epson_wait_for_idle();
	epson_reg_write(PWR_CTRL_REG, 0x8002);

	/* wait for power down sequence to complete */
	do {
		epson_reg_read(PWR_CTRL_REG, &temp);
	} while (temp & 0x0080);
}

void epson_softReset(void)
{
	epson_reg_write(SOFT_RST_REG, 0xff);
}

static int set_power_mode(struct s1d135xx *e, enum s1d135xx_pwr_state mode)
{
	static const u8 pwr_cmds[3] = { SLP, STBY, RUN_SYS };
	int result;

	if (e->power_mode == mode)
		return 0;

	assert(mode < ARRAY_SIZE(pwr_cmds));

	epson_wait_for_idle();
	result = epson_cmd_p0(pwr_cmds[mode]);
	epson_wait_for_idle();
	e->power_mode = mode;

	return result;
}

int epson_mode_standby(struct s1d135xx *epson)
{
	return set_power_mode(epson, PWR_STATE_STANDBY);
}

int epson_mode_sleep(struct s1d135xx *epson)
{
	return set_power_mode(epson, PWR_STATE_SLEEP);
}

int epson_mode_run(struct s1d135xx *epson)
{
	return set_power_mode(epson, PWR_STATE_RUN);
}

/* Copy a byte stream to the controller as words.
 * Need to sort out swap requirements.
 */
static void pack_4bpp(endianess *in, endianess *out, int in_count)
{
	while (in_count > 0) {
		out->bytes[0] = (in[0].bytes[1] & 0xf0) | (in[0].bytes[0] >> 4);
		out->bytes[1] = (in[1].bytes[1] & 0xf0) | (in[1].bytes[0] >> 4);
		in += 2;
		out++;
		in_count -= 2;
	}
}

static int read_file_data(FIL *file, endianess *data, size_t length,
			  size_t *count, int swap, int pack)
{
	if (f_read(file, data, length, count) != FR_OK)
		return -1;

	/* read count of 0 with no error is end of file */
	if (!(*count))
		return 0;

	/* if we read an odd number of bytes make it even to flush it out */
	if ((*count) & 1)
		*count++;

	if (pack) {
		*count /= 2;
		pack_4bpp(data, data, *count);
	}

	if (swap) {
		const size_t n = *count / 2;
		size_t i;

		for (i = 0; i < n; ++i)
			data[i].data = swap_bytes(data[i].data);
	}

	return 0;
}

static void transfer_data(const endianess *data, size_t n)
{
	size_t i;

	for (i = 0; i < n; ++i)
		epson_bulk_transfer_word(data[i].data);
}

static int transfer_file(FIL *f, int swap, int pack)
{
	endianess data[FILE_BUFFER_SIZE];
	size_t count;

	do {
		if (read_file_data(f, data, sizeof(data), &count, swap, pack))
			return -1;

		transfer_data(data, count / 2);
	} while (count);

	return 0;
}

static int transfer_image(FIL *f, int swap, int pack,
			  const struct area *area, int left, int top,
			  int img_width)
{
	const size_t offset = left / 2;
	const size_t length = area->width / 2;
	endianess data[IMAGE_BUFFER_LENGTH / 2]; /* ToDo: make it dynamic */
	size_t count;
	int line;

	if (img_width > IMAGE_BUFFER_LENGTH) {
		LOG("Image width is bigger than buffer size: %d, max=%d",
		    img_width, IMAGE_BUFFER_LENGTH);
		return -1;
	}

	f_lseek(f, f->fptr + (top * img_width));

	for (line = area->height; line; --line) {
		if (read_file_data(f, data, img_width, &count, swap, pack))
			return -1;

		transfer_data(&data[offset], length);
	}

	return 0;
}

int epson_loadEpsonCode(char *code_path)
{
	FIL ControllerCode;
	int result;

	if (f_open(&ControllerCode, code_path, FA_READ) != FR_OK)
		return -1;

	epson_wait_for_idle();
	epson_begin_bulk_code_transfer(INIT_CMD_SET);
	result = transfer_file(&ControllerCode, false, false);
	epson_end_bulk_transfer();
	f_close(&ControllerCode);
	epson_wait_for_idle();

	return result;
}

int epson_BulkTransferFile(FIL *file, int pack)
{
	int result;

	epson_wait_for_idle();
	epson_begin_bulk_transfer(HOST_MEM_PORT_REG);
	result = transfer_file(file, false, pack);
	epson_end_bulk_transfer();
	epson_wait_for_idle();

	return result;
}

int epson_BulkTransferImage(FIL *file, int pack, const struct area *area,
			    int left, int top, int width)
{
	int result;

	epson_wait_for_idle();
	epson_begin_bulk_transfer(HOST_MEM_PORT_REG);
	result = transfer_image(file, false, pack, area, left, top, width);
	epson_end_bulk_transfer();
	epson_wait_for_idle();

	return result;
}

int epson_loadEpsonWaveform(char *path, uint32_t address)
{
	FIL File;
	uint32_t file_size;
	int result;

	LOG("Load Waveform From SD Card\n");

	if (f_open(&File, path, FA_READ) != FR_OK)
		return -1;

	/* Accomodate waveforms that had odd byte counts. */
	file_size = f_size(&File);

	if (file_size & 0x01)
		file_size++;

	/* Begin the burst transfer operation */
	epson_wait_for_idle();
	epson_cmd_p4(BST_WR_SDR,
		     (address & 0x0ffff), ((address >> 16) & 0x0ffff),
		     (file_size / 2) & 0x0ffff,
		     ((file_size / 2) >> 16) & 0x0ffff);

	result = epson_BulkTransferFile(&File, false);

	/* Terminate burst operation */
	epson_cmd_p0(BST_END_SDR);
	epson_wait_for_idle();

	f_close(&File);

	return result;
}

void epson_WaveformStreamInit(uint32_t address)
{
	epson_wait_for_idle();
	epson_cmd_p4(BST_WR_SDR, (address & 0x0ffff),
		     ((address >> 16) & 0x0ffff), 64000, 64000);
	epson_wait_for_idle();
	epson_begin_bulk_transfer(HOST_MEM_PORT_REG);
}

void epson_WaveformStreamTransfer(uint8_t *buffer, size_t len)
{
	const size_t n = len / 2;
	size_t i;
	int swap = 1;
	uint16_t word;
	uint16_t *buffer16 = (uint16_t*)buffer;

	for (i = 0; i < n; i++) {
		word = buffer16[i];

		if (swap)
			buffer16[i] = swap_bytes(buffer16[i]);

		epson_bulk_transfer_word(word);
	}
}

void epson_WaveformStreamClose(void)
{
	epson_end_bulk_transfer();
	epson_wait_for_idle();
	epson_cmd_p0(BST_END_SDR);
	epson_wait_for_idle();
}

int epson_loadColorConfig(char *path, uint32_t address)
{
	FIL File;
	int result;

	if (f_open(&File, path, FA_READ) != FR_OK)
		return -1;

	result = epson_BulkTransferFile(&File, false);

	f_close(&File);

	return result;
}

int epson_loadImageFile(FIL *image, uint16_t mode, int pack)
{
	/* load complete image, typically 8bpp and no transparency */
	epson_cmd_p1(LD_IMG_HOST, mode);

	if (epson_BulkTransferFile(image, pack))
		return -1;

	epson_cmd_p0(LD_IMG_HOST_END);
	epson_wait_for_idle();

	return 0;
}

int epson_loadImageFileArea(FIL *image, uint16_t mode, int pack,
			    const struct area *area, int x, int y, int width)
{
	epson_cmd_p5(LD_IMG_HOST_AREA, mode,
		     (area->left & S1D135XX_XMASK),
		     (area->top & S1D135XX_YMASK),
		     (area->width & S1D135XX_XMASK),
		     (area->height & S1D135XX_YMASK));

	if (epson_BulkTransferImage(image, pack, area, x, y, width))
		return -1;

	epson_cmd_p0(LD_IMG_HOST_END);
	epson_wait_for_idle();

	return 0;
}

static void do_fill(const struct area *area, uint8_t fill, int pack)
{
	uint16_t wfill;
	int width;
	int n_x;
	int y;

	if (pack) {
		wfill = (fill << 12) | (fill << 8) | (fill << 4) | fill;
		width = area->width / 2;
	} else {
		wfill = (fill << 8) | fill;
		width = area->width;
	}

	n_x = width / 2;

	epson_wait_for_idle();
	epson_begin_bulk_transfer(HOST_MEM_PORT_REG);

	for (y = area->height; y; --y) {
		int x;

		for (x = n_x; x; --x)
			epson_bulk_transfer_word(wfill);
	}

	epson_end_bulk_transfer();
	epson_wait_for_idle();

	epson_cmd_p0(LD_IMG_HOST_END);
	epson_wait_for_idle();
}

void epson_fill_area(uint16_t mode, uint8_t pack,
		     const struct area *area, uint8_t fill)
{
	epson_cmd_p5(LD_IMG_HOST_AREA, mode,
		     (area->left & S1D135XX_XMASK),
		     (area->top & S1D135XX_YMASK),
		     (area->width & S1D135XX_XMASK),
		     (area->height & S1D135XX_YMASK));
	do_fill(area, fill, pack);
}

void epson_fill_buffer(uint16_t mode, uint8_t pack, uint16_t height,
		      uint16_t width, uint8_t fill)
{
	const struct area area = { 0, 0, width, height };

	epson_cmd_p1(LD_IMG_HOST, mode);
	do_fill(&area, fill, pack);
}
