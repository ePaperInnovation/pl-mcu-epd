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
struct rd_ctx {
	uint8_t buffer[64];        /* buffer to read a block of data */
	size_t buflen;             /* length of payload in buffer */
	size_t datalen;            /* length of the data left to read */
	size_t index;              /* index of current byte in buffer */
	size_t offset;             /* offset where to read next in EEPROM */
	uint16_t crc;              /* accumulated CRC for all data read */
	const struct i2c_eeprom *eeprom; /* EEPROM instance */
};

/* Context to write to the Epson controller */
struct wr_ctx {
	uint8_t buffer[128];       /* buffer to read a block of data */
	size_t buflen;             /* length of payload in buffer */
	size_t index;              /* index of current byte in buffer */
};

/* Static functions */

static int plwf_wf_rd(struct rd_ctx *ctx)
{
	if (ctx->index == ctx->buflen) {
		if (!ctx->datalen)
			return EOF;

		ctx->index = 0;
		ctx->buflen = min(sizeof(ctx->buffer), ctx->datalen);
		eeprom_read(ctx->eeprom, ctx->offset, ctx->buflen,ctx->buffer);
		ctx->crc = crc16_run(ctx->crc, ctx->buffer, ctx->buflen);
		ctx->offset += ctx->buflen;
		ctx->datalen -= ctx->buflen;
	}

	return ctx->buffer[ctx->index++];
}

static int plwf_wf_wr(int c, struct wr_ctx *ctx)
{
	ctx->buffer[ctx->index++] = c;

	if (ctx->index == ctx->buflen) {
		epson_WaveformStreamTransfer(ctx->buffer, ctx->buflen);
		ctx->index = 0;
	}

	return c;
}

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
	const char *magic = (const char *)&data->vermagic.magic;

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

int plwf_load_wf(struct pl_disp_data *data, const struct i2c_eeprom *eeprom,
		 struct s1d135xx *epson, uint32_t addr)
{
	struct lzss lzss;
	struct lzss_io io;
	struct rd_ctx rd_ctx;
	struct wr_ctx wr_ctx;
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

	rd_ctx.buflen = 0;
	rd_ctx.datalen = data->info.waveform_lzss_length;
	rd_ctx.index = 0;
	rd_ctx.offset = PLWF_WF_OFFS;
	rd_ctx.crc = crc16_init;
	rd_ctx.eeprom = eeprom;

	io.rd = (lzss_rd_t)plwf_wf_rd;
	io.i = &rd_ctx;

	wr_ctx.buflen = sizeof(wr_ctx.buffer);
	wr_ctx.index = 0;

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
		    sizeof crc, (uint8_t *)&crc);
	crc = be16toh(crc);

	if (crc != rd_ctx.crc) {
		LOG("Waveform CRC mismatch: %04X instead of %04X",
		    rd_ctx.crc, crc);
		return -1;
	}

	LOG("Waveform loaded OK");

	return 0;
}
