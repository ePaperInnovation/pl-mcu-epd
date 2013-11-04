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

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <crc16.h>
#include "types.h"
#include "assert.h"
#include "i2c.h"
#include "i2c-eeprom.h"
#include "vcom.h"
#include "plwf.h"
#include "lzss.h"
#include "epson/epson-utils.h"

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

static int get_waveform (uint32_t wf_length, struct i2c_eeprom *eeprom,
		struct s1d135xx *epson, uint32_t address, uint16_t *crc)
{
	static const unsigned int ei = 7, ej = 4;
	struct lzss lzss;
	struct lzss_io io;
	struct buffer_context wr_ctx;
	struct buffer_context rd_ctx;
	int stat = 0;

	printk("Load Waveform From Eeprom\n");

	lzss_init(&lzss, ei, ej);
	lzss_alloc_buffer(&lzss);

	memset(rd_ctx.buffer, 0, sizeof(rd_ctx.buffer));
	rd_ctx.wvf_length = wf_length;
	rd_ctx.offset = PLWF_WF_OFFS;
	rd_ctx.eeprom = eeprom;
	rd_ctx.controller = NULL;
	rd_ctx.buflen = 0;
	rd_ctx.index = rd_ctx.buflen;
	rd_ctx.crc = crc16_init;

	memset(wr_ctx.buffer, 0, sizeof(wr_ctx.buffer));
#if 1
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

	epson_WaveformStreamInit(address);
	stat = lzss_decode(&lzss, &io);

#if MCU_DEBUG
	{
		int i;
		printf("last buffer:");
		for (i = 0; i < wr_ctx.index; ++i)
			printf(" %02X", wr_ctx.buffer[i]);
		printf("\n");
	}
#endif

	epson_WaveformStreamTransfer(wr_ctx.buffer, wr_ctx.index);
	epson_WaveformStreamClose();
	*crc = be16toh(rd_ctx.crc);

	if (stat) {
		printk("Failed to decode waveform data\n");
	}

	*crc = rd_ctx.crc;
	lzss_free_buffer(&lzss);
	return stat;
}

int plwf_data_init(struct plwf_data **data)
{
	struct plwf_data *p;

	assert(data);

	p = (struct plwf_data*)malloc(sizeof(struct plwf_data));
	if (NULL == p)
		return -ENOMEM;

	*data = p;

	return 0;
}

void plwf_data_free(struct plwf_data **data)
{
	assert(*data);
	free(*data);
	*data = NULL;
}

int plwf_load_waveform(struct s1d135xx *epson, struct i2c_eeprom *eeprom, struct plwf_data *data, uint32_t address)
{
	int ret = 0;
	int i;
	uint16_t crc;
	uint16_t crc_wf;
	uint16_t crc_wf_eeprom;

	assert(epson);
	assert(eeprom);
	assert(data);

	if (eeprom_read(eeprom, 0, sizeof *data, data) != 0) {
		printk("Failed to read EEPROM!\n");
		return -1;
	}
	crc = crc16_run(crc16_init, (const uint8_t *)&data->info,
					sizeof data->info);
	data->vermagic.version = be16toh(data->vermagic.version);
	data->info.vcom = be32toh(data->info.vcom);
	data->info.waveform_full_length =
		be32toh(data->info.waveform_full_length);
	data->info.waveform_lzss_length =
		be32toh(data->info.waveform_lzss_length);
	data->info_crc = be16toh(data->info_crc);

	printk("PLWF: Vermagic: %lx\n", data->vermagic.magic);

	if (data->vermagic.magic != PLWF_MAGIC) {
		printk("Invalid magic number: %lX instead of %lX\n",
				data->vermagic.magic, PLWF_MAGIC);
		return -1;
	}

	printk("PLWF: Version: %x\n", data->vermagic.version);

	if (data->vermagic.version != PLWF_VERSION) {
		printk("Unsupported format version: %d, requires %d\n",
				data->vermagic.version, PLWF_VERSION);
		return -1;
	}

	printk("Info CRC: %04X\n", crc);

	if (data->info_crc != crc) {
		printk("Info CRC mismatch: %04X instead of %04X\n",
				data->info_crc, crc);
		return -1;
	}

	printk("PLWF: Panel ID: %s\n", data->info.panel_id);
	printk("PLWF: Panel Type: %s\n", data->info.panel_type);
	printk("PLWF: Vcom: %li\n", data->info.vcom);
	printk("PLWF: Waveform Length = %lu\n",
	       data->info.waveform_full_length);
	printk("PLWF: Waveform Compressed Length = %lu\n",
	       data->info.waveform_lzss_length);
	printk("PLWF: Waveform ID = %s\n", data->info.waveform_id);
	printk("PLWF: Waveform Target = %s\n", data->info.waveform_target);
	printk("PLWF: Waveform MD5 = ");
	for (i=0; i<16; i++)
	{
		printk("%x", data->info.waveform_md5[i]);
	}
	printk("\n");

	ret = get_waveform(data->info.waveform_lzss_length, eeprom, epson,
			   address, &crc_wf);

	if (ret != 0) {
		printk("Error getting waveform from EEPROM\n");
	}

	eeprom_read(eeprom, PLWF_WF_OFFS + data->info.waveform_lzss_length,
		    sizeof crc_wf_eeprom, &crc_wf_eeprom);
	crc_wf_eeprom = be16toh(crc_wf_eeprom);

	printf("PLWF: Waveform CRC: %04X\n", crc_wf);

	if (crc_wf_eeprom != crc_wf) {
		printk("Waveform CRC mismatch: %04X instead of %04X\n",
				crc_wf, crc_wf_eeprom);
	}

	return  ret;
}
