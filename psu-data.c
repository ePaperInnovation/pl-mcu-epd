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
 * psu-data.c -- Read/Write to the PSU configuration eeprom
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "types.h"
#include "assert.h"
#include "i2c.h"
#include "i2c-eeprom.h"
#include "vcom.h"

#define	CONFIG_EEPROM_WRITE	1	// 1 to enable psu eeprom data write

#define PSU_EEPROM_HEADER_UNDEFINED	0xFF

struct board_info {
	uint8_t version;
};

struct __attribute__((__packed__)) eeprom_data_v0 {
	struct board_info info;
	struct vcom_info vcom;
};

struct __attribute__((__packed__)) eeprom_data {
	uint8_t hdr_version;
	union {
		struct eeprom_data_v0 v0;
		// struct eeprom_data_vn vn;
		u8 payload[1];
	};
	uint16_t crc; // for all the data from offset 0 to here
};

#define WIDTH               16
#define TOPBIT              0x8000
#define POLYNOMIAL			0x1021
#define INITIAL_REMAINDER	0xFFFF
#define CHECK_VALUE			0x29B1

static u16 crc_compute(void *message, u16 count)
{
	register u16 byte;
	register u8  bit;
    register u16 remainder = INITIAL_REMAINDER;
    register u8 *data = (u8*)message;

    /* Perform modulo-2 division, a byte at a time. */
    for (byte = 0; byte < count; byte++)
    {
        /* Next byte into the remainder */
        remainder ^= (((u16)data[byte]) << (WIDTH - 8));

        /* Perform modulo-2 division, a bit at a time. */
        for (bit = 8; bit > 0; --bit)
        {
            if (remainder & TOPBIT)
            {
                remainder = (remainder << 1) ^ POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    /* The final remainder is the CRC result. */
    return (remainder);
}

int psu_data_init(struct eeprom_data **data)
{
	struct eeprom_data *p;

	assert(data);

	p = (struct eeprom_data*)malloc(sizeof(struct eeprom_data));
	if (NULL == p)
		return -ENOMEM;

	p->hdr_version = PSU_EEPROM_HEADER_UNDEFINED;
	*data = p;

	return 0;
}

void psu_data_free(struct eeprom_data **data)
{
	assert(*data);
	free(*data);
	*data = NULL;
}

int psu_data_get_vcom_data(struct eeprom_data *data, struct vcom_info *vcom)
{
	assert(data);
	assert(vcom);

	switch (data->hdr_version) {
	default:
		return -1;
	case 0:
		memcpy(vcom, &data->v0.vcom, sizeof(*vcom));
		break;
	}

	return 0;
}

int psu_data_get_board_info(struct eeprom_data *data, u8 *board)
{
	assert(data);
	assert(board);

	switch (data->hdr_version) {
	default:
		return -1;
	case 0:
		*board = data->v0.info.version;
		break;
	}

	return 0;
}


int psu_data_read(struct i2c_eeprom *eeprom, struct eeprom_data *data)
{
	u16 crc;
	u16 eeprom_crc;
	u16 payload_size;

	assert(eeprom);
	assert(data);

	// read the version byte
	if (eeprom_read(eeprom, 0, sizeof(data->hdr_version), &data->hdr_version) < 0)
		goto read_error;

	switch (data->hdr_version) {
		case 0:
			payload_size = sizeof(struct eeprom_data_v0);
			break;
		default:
			goto version_error;
	}

	// read the payload data
	if (eeprom_read(eeprom, sizeof(data->hdr_version), payload_size, data->payload) < 0)
		goto read_error;

	// read the checksum that follows the payload
	if (eeprom_read(eeprom, sizeof(data->hdr_version) + payload_size, sizeof(eeprom_crc), &eeprom_crc) < 0)
		goto read_error;

	eeprom_crc = be16toh(eeprom_crc);

	// check the checksum (of the big endian data)
	crc = crc_compute(data, sizeof(data->hdr_version) + payload_size);

	if (crc != eeprom_crc)
	{
		printk("psu_data: CRC failure\n");
		goto format_error;
	}

	// convert configuration data to host endianess
	switch (data->hdr_version) {
		default:
			goto version_error;
		case 0:
			data->v0.vcom.dac_x1   = be16toh(data->v0.vcom.dac_x1);
			data->v0.vcom.dac_y1   = be16toh(data->v0.vcom.dac_y1);
			data->v0.vcom.dac_x2   = be16toh(data->v0.vcom.dac_x2);
			data->v0.vcom.dac_y2   = be16toh(data->v0.vcom.dac_y2);
			data->v0.vcom.vgpos_mv = be32toh(data->v0.vcom.vgpos_mv);
			data->v0.vcom.vgneg_mv = be32toh(data->v0.vcom.vgneg_mv);
			printk("psu_data(read): Version: 0x%02X, Board: 0x%02X\n",
			       data->hdr_version, data->v0.info.version);
			break;
	}

	return 0;

format_error:
read_error:
version_error:
	// any error leaves the data undefined.
	data->hdr_version = PSU_EEPROM_HEADER_UNDEFINED;
	return -1;
}

#if CONFIG_EEPROM_WRITE

void psu_data_set_header_version(struct eeprom_data *data, u8 version)
{
	data->hdr_version = version;
}

int psu_data_set_vcom_data(struct eeprom_data *data, struct vcom_info *vcom)
{
	assert(data);
	assert(vcom);

	switch (data->hdr_version) {
		default:
			return -1;
		case 0:
			memcpy(&data->v0.vcom, vcom, sizeof(*vcom));
			break;
	}

	return 0;
}

int psu_data_set_board_info(struct eeprom_data *data, u8 board)
{
	assert(data);
	assert(board);

	switch (data->hdr_version) {
		default:
			return -1;
		case 0:
			data->v0.info.version = board;
			break;
	}

	return 0;
}

/* Note that this function modifies the data it is handed as it converts
 * the endianess of the data in place.
 */
int psu_data_write(struct i2c_eeprom *eeprom, struct eeprom_data *data)
{
	u16 payload_size;
	u16 crc;

	assert(eeprom);
	assert(data);

	switch(data->hdr_version) {
	default:
		goto format_error;
	case 0:
		// convert configuration data to eeprom endianess
		printk("psu_data(write): Version: 0x%02X, Board: 0x%02X\n",
		       data->hdr_version, data->v0.info.version);
		data->v0.vcom.dac_x1   = htobe16(data->v0.vcom.dac_x1);
		data->v0.vcom.dac_y1   = htobe16(data->v0.vcom.dac_y1);
		data->v0.vcom.dac_x2   = htobe16(data->v0.vcom.dac_x2);
		data->v0.vcom.dac_y2   = htobe16(data->v0.vcom.dac_y2);
		data->v0.vcom.vgpos_mv = htobe32(data->v0.vcom.vgpos_mv);
		data->v0.vcom.vgneg_mv = htobe32(data->v0.vcom.vgneg_mv);
		payload_size = sizeof(data->hdr_version) + sizeof(struct eeprom_data_v0);
		break;
	}

	// compute the CRC
	crc = crc_compute(data, payload_size);
	crc = htobe16(crc);

	// write the eeprom data
	if (eeprom_write(eeprom, 0, payload_size, data) < 0)
		goto write_error;

	// write the crc
	if (eeprom_write(eeprom, payload_size, sizeof(crc), &crc) < 0)
		goto write_error;

	return 0;

format_error:
write_error:
	return -1;
}

#endif
