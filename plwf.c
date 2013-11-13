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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "crc16.h"
#include "types.h"
#include "assert.h"
#include "i2c.h"
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
#define PLWF_STR_TERM(_str) do { _str[PLWF_STR_LEN] = '\0'; } while(0)

/* Offset where to start reading the waveform data from */
#define PLWF_WF_OFFS (sizeof(struct plwf_data))

struct buffer_context {
	uint8_t buffer[64];
	size_t buflen;
	size_t wvf_length;
	size_t offset;
	int index;
	struct i2c_eeprom *eeprom;
	struct s1d135xx *controller;
	uint16_t crc;
};

/* Static functions */

static int plwf_wf_rd(struct buffer_context *ctx)
{
	if (ctx->index == ctx->buflen) {
		const size_t wvf_read = ctx->offset - PLWF_WF_OFFS;

		if (wvf_read == ctx->wvf_length)
			return EOF;

		ctx->index = 0;
		ctx->buflen = min(sizeof(ctx->buffer),
				  ctx->wvf_length - wvf_read);
		eeprom_read(ctx->eeprom, ctx->offset, ctx->buflen, ctx->buffer);
		ctx->crc = crc16_run(ctx->crc, ctx->buffer, ctx->buflen);
		ctx->offset += ctx->buflen;
	}

	return ctx->buffer[ctx->index++];
}

static int plwf_wf_wr(int c, struct buffer_context *ctx)
{
	ctx->buffer[ctx->index++] = c;

	if (ctx->index == ctx->buflen) {
		epson_WaveformStreamTransfer(ctx->buffer, ctx->buflen);
		ctx->index = 0;
	}

	return c;
}

int plwf_data_init(struct plwf_data *data, struct i2c_eeprom *eeprom)
{
	uint16_t crc;

	assert(data != NULL);
	assert(eeprom != NULL);

	if (eeprom_read(eeprom, 0, sizeof *data, data)) {
		LOG("Failed to read EEPROM");
		return -1;
	}

	crc = crc16_run(crc16_init, (const uint8_t *)&data->info,
					sizeof data->info);
	data->vermagic.version = be16toh(data->vermagic.version);
	PLWF_STR_TERM(data->info.panel_id);
	PLWF_STR_TERM(data->info.panel_type);
	data->info.vcom = be32toh(data->info.vcom);
	data->info.waveform_full_length =
		be32toh(data->info.waveform_full_length);
	data->info.waveform_lzss_length =
		be32toh(data->info.waveform_lzss_length);
	PLWF_STR_TERM(data->info.waveform_id);
	PLWF_STR_TERM(data->info.waveform_target);
	data->info_crc = be16toh(data->info_crc);

#if PLWF_VERBOSE_LOG
	LOG("Vermagic: %lx", data->vermagic.magic);
#endif

	if (data->vermagic.magic != PLWF_MAGIC) {
		LOG("Invalid magic number: %lX instead of %lX",
		    data->vermagic.magic, PLWF_MAGIC);
		return -1;
	}

	LOG("Version: %x", data->vermagic.version);

	if (data->vermagic.version != PLWF_VERSION) {
		LOG("Unsupported format version: %d, requires %d",
		    data->vermagic.version, PLWF_VERSION);
		return -1;
	}

#if PLWF_VERBOSE_LOG
	LOG("Info CRC: %04X", crc);
#endif

	if (data->info_crc != crc) {
		LOG("Info CRC mismatch: %04X instead of %04X",
		    data->info_crc, crc);
		return -1;
	}

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
	printf("Waveform MD5:");
	{
		int i;

		for (i = 0; i < sizeof(data->info.waveform_md5); i++)
			printf("%02x", data->info.waveform_md5[i]);
	}
	printf("\n");
#endif

	return  0;
}

int plwf_load_wf(struct plwf_data *data, struct i2c_eeprom *eeprom,
		 struct s1d135xx *epson, uint32_t addr)
{
	struct lzss lzss;
	struct lzss_io io;
	struct buffer_context wr_ctx;
	struct buffer_context rd_ctx;
	char lzss_buffer[LZSS_BUFFER_SIZE(PLWF_LZSS_EI)];
	uint16_t crc;

#if PLWF_VERBOSE_LOG
	LOG("LZSS buffer size: %d", sizeof(lzss_buffer));
#endif

	if (lzss_init(&lzss, PLWF_LZSS_EI, PLWF_LZSS_EJ)) {
		LOG("Failed to initialise LZSS");
		return -1;
	}

	lzss.buffer = lzss_buffer;

	memset(rd_ctx.buffer, 0, sizeof(rd_ctx.buffer));
	rd_ctx.wvf_length = data->info.waveform_lzss_length;
	rd_ctx.offset = PLWF_WF_OFFS;
	rd_ctx.eeprom = eeprom;
	rd_ctx.controller = NULL;
	rd_ctx.buflen = 0;
	rd_ctx.index = rd_ctx.buflen;
	rd_ctx.crc = crc16_init;

	memset(wr_ctx.buffer, 0, sizeof(wr_ctx.buffer));
#if 1 /* ToDo: use separate buffer types for reading and writing */
	wr_ctx.wvf_length = 0;
#endif
	wr_ctx.buflen = sizeof(wr_ctx.buffer);
	wr_ctx.offset = 0;
	wr_ctx.eeprom = NULL;
	wr_ctx.controller = epson;
	wr_ctx.index = 0;

	io.rd = (lzss_rd_t)plwf_wf_rd;
	io.i = &rd_ctx;
	io.wr = (lzss_wr_t)plwf_wf_wr;
	io.o = &wr_ctx;

	epson_WaveformStreamInit(addr);

	if (lzss_decode(&lzss, &io)) {
		LOG("Failed to decode LZSS-encoded waveform data");
		return -1;
	}

	epson_WaveformStreamTransfer(wr_ctx.buffer, wr_ctx.index);
	epson_WaveformStreamClose();

#if PLWF_VERBOSE_LOG
	LOG("Waveform CRC: %04X", rd_ctx.crc);
#endif

	eeprom_read(eeprom, PLWF_WF_OFFS + data->info.waveform_lzss_length,
		    sizeof crc, &crc);
	crc = be16toh(crc);

	if (crc != rd_ctx.crc) {
		LOG("Waveform CRC mismatch: %04X instead of %04X",
		    rd_ctx.crc, crc);
		return -1;
	}

	LOG("Waveform loaded OK");

	return 0;
}
