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

#include <pl/endian.h>
#include <pl/i2c.h>
#include <pl/wflib.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "crc16.h"
#include "types.h"
#include "assert.h"
#include "i2c-eeprom.h"
#include "vcom.h"
#include "plwf.h"
#include "lzss.h"
#include "epson/epson-utils.h"

#define LOG_TAG "plwf"
#include "utils.h"

/* Set to 1 to enable verbose debug log messages */
#define PLWF_VERBOSE_LOG 0

/* LZSS parameters */
#define PLWF_LZSS_EI 7
#define PLWF_LZSS_EJ 4

/* Ensure the last byte from an EEPROM string is a null character */
#define PLWF_STR_TERM(_str) do { _str[PL_DATA_STR_LEN] = '\0'; } while(0)

/* Offset where to start reading the waveform data from */
#define PLWF_WF_OFFS (sizeof(struct pl_disp_data))

/* Context to read from the EEPROM */
struct lzss_rd_ctx {
	uint8_t buffer[64];        /* buffer to read a block of data */
	size_t buflen;             /* length of payload in buffer */
	size_t datalen;            /* length of the data left to read */
	size_t index;              /* index of current byte in buffer */
	size_t offset;             /* offset where to read next in EEPROM */
	uint16_t crc;              /* accumulated CRC for all data read */
	const struct i2c_eeprom *eeprom; /* EEPROM instance */
};

/* Context to write to the EPDC */
struct lzss_wr_ctx {
	uint8_t buffer[128];       /* buffer to read a block of data */
	size_t buflen;             /* length of payload in buffer */
	size_t index;              /* index of current byte in buffer */
	pl_wflib_wr_t wflib_wr;    /* function to write to the output */
	void *wflib_ctx;           /* write context for wflib */
};

int plwf_data_init(struct pl_disp_data *data, const struct i2c_eeprom *eeprom)
{
	uint16_t crc;

	assert(data != NULL);
	assert(eeprom != NULL);

	if (eeprom_read(eeprom, 0, sizeof *data, (uint8_t *)data)) {
		LOG("Failed to read EEPROM");
		return -1;
	}

	crc = crc16_run(crc16_init, (const uint8_t *)&data->info,
					sizeof data->info);
	data->vermagic.version = be16toh(data->vermagic.version);
	PLWF_STR_TERM(data->info.panel_id);
	PLWF_STR_TERM(data->info.panel_type);
	swap32(data->info.vcom);
	swap32(data->info.waveform_full_length);
	swap32(data->info.waveform_lzss_length);
	PLWF_STR_TERM(data->info.waveform_id);
	PLWF_STR_TERM(data->info.waveform_target);
	data->info_crc = be16toh(data->info_crc);

	if (data->vermagic.magic != PL_DATA_MAGIC) {
		LOG("Invalid magic number: %lX instead of %lX",
		    data->vermagic.magic, PL_DATA_MAGIC);
		return -1;
	}

	if (data->vermagic.version != PL_DATA_VERSION) {
		LOG("Unsupported format version: %d, requires %d",
		    data->vermagic.version, PL_DATA_VERSION);
		return -1;
	}

	if (data->info_crc != crc) {
		LOG("Info CRC mismatch: %04X instead of %04X",
		    data->info_crc, crc);
		return -1;
	}

	return  0;
}

void plwf_log(const struct pl_disp_data *data)
{
#if PLWF_VERBOSE_LOG
	const char *magic = (const char *)&data->vermagic.magic;
#endif

	LOG("Version: %d", data->vermagic.version);

#if PLWF_VERBOSE_LOG
	LOG("Magic: 0x%lX %c%c%c%c",
	    data->vermagic.magic, magic[0], magic[1], magic[2], magic[3]);
#endif

#if PLWF_VERBOSE_LOG
	LOG("Info CRC: 0x%04X", data->info_crc);
#endif

	LOG("Panel ID: %s", data->info.panel_id);
	LOG("Panel Type: %s", data->info.panel_type);
	LOG("VCOM: %li", data->info.vcom);
#if PLWF_VERBOSE_LOG
	LOG("Waveform Length: %lu", data->info.waveform_full_length);
	LOG("Waveform Compressed Length: %lu",data->info.waveform_lzss_length);
#endif
	LOG("Waveform ID: %s", data->info.waveform_id);
	LOG("Waveform Target: %s", data->info.waveform_target);
#if PLWF_VERBOSE_LOG
	printf("Waveform MD5: 0x");
	{
		int i;

		for (i = 0; i < sizeof(data->info.waveform_md5); i++)
			printf("%02x", data->info.waveform_md5[i]);
	}
	printf("\n");
#endif
}

static int pl_wflib_lzss_rd(struct lzss_rd_ctx *ctx)
{
	if (ctx->index == ctx->buflen) {
		if (!ctx->datalen)
			return EOF;

		ctx->index = 0;
		ctx->buflen = min(sizeof(ctx->buffer), ctx->datalen);

		if (eeprom_read(ctx->eeprom, ctx->offset, ctx->buflen,
				ctx->buffer)) {
			LOG("Failed to read LZSS data from EEPROM");
			return EOF;
		}

		ctx->crc = crc16_run(ctx->crc, ctx->buffer, ctx->buflen);
		ctx->offset += ctx->buflen;
		ctx->datalen -= ctx->buflen;
	}

	return ctx->buffer[ctx->index++];
}

static int pl_wflib_lzss_wr(int c, struct lzss_wr_ctx *ctx)
{
	ctx->buffer[ctx->index++] = c;

	if (ctx->index == ctx->buflen) {
		if (ctx->wflib_wr(ctx->wflib_ctx, ctx->buffer, ctx->buflen)) {
			LOG("Failed to write waveform data");
			return EOF;
		}

		ctx->index = 0;
	}

	return c;
}

static int pl_wflib_eeprom_xfer(struct pl_wflib *wflib, pl_wflib_wr_t wr,
				void *ctx)
{
	struct pl_wflib_eeprom *p = wflib->priv;
	struct lzss lzss;
	struct lzss_io io;
	struct lzss_rd_ctx rd_ctx;
	struct lzss_wr_ctx wr_ctx;
	char lzss_buffer[LZSS_BUFFER_SIZE(PLWF_LZSS_EI)];
	uint16_t crc;

	if (lzss_init(&lzss, PLWF_LZSS_EI, PLWF_LZSS_EJ)) {
		LOG("Failed to initialise LZSS");
		return -1;
	}

	lzss.buffer = lzss_buffer;

	rd_ctx.buflen = 0;
	rd_ctx.datalen = p->disp_data->info.waveform_lzss_length;
	rd_ctx.index = 0;
	rd_ctx.offset = sizeof(struct pl_disp_data);
	rd_ctx.crc = crc16_init;
	rd_ctx.eeprom = p->eeprom;

	io.rd = (lzss_rd_t)pl_wflib_lzss_rd;
	io.i = &rd_ctx;

	wr_ctx.buflen = sizeof(wr_ctx.buffer);
	wr_ctx.index = 0;
	wr_ctx.wflib_wr = wr;
	wr_ctx.wflib_ctx = ctx;

	io.wr = (lzss_wr_t)pl_wflib_lzss_wr;
	io.o = &wr_ctx;

	if (lzss_decode(&lzss, &io)) {
		LOG("Failed to decode LZSS data");
		return -1;
	}

	if (wr(ctx, wr_ctx.buffer, wr_ctx.index)) {
		LOG("Failed to flush output data");
		return -1;
	}

	if (eeprom_read(p->eeprom, rd_ctx.offset + rd_ctx.datalen,
			sizeof crc, (uint8_t *)&crc)) {
		LOG("Failed to read CRC");
		return -1;
	}

	crc = be16toh(crc);

	if (crc != rd_ctx.crc) {
		LOG("CRC mismatch: %04X instead of %04X", rd_ctx.crc, crc);
		return -1;
	}

	return 0;
}

int pl_wflib_init_eeprom(struct pl_wflib *wflib, struct pl_wflib_eeprom *p,
			 const struct i2c_eeprom *eeprom,
			 const struct pl_disp_data *disp_data)
{
	p->eeprom = eeprom;
	p->disp_data = disp_data;
	p->offset = 0;

	wflib->xfer = pl_wflib_eeprom_xfer;
	wflib->size = disp_data->info.waveform_full_length;
	wflib->priv = p;

	LOG("EEPROM + LZSS %s", disp_data->info.waveform_id);

	return 0;
}
