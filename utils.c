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
 * utils.c -- random homeless functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *          Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <pl/types.h>
#include <pl/endian.h>
#include <stdlib.h>
#include <stdio.h>
#include "FatFs/ff.h"
#include "msp430-gpio.h"
#include "pnm-utils.h"
#include "assert.h"
#include "utils.h"

void swap32(void *x)
{
	uint8_t *b = x;
	uint8_t tmp;

	tmp = b[0];
	b[0] = b[3];
	b[3] = tmp;
	tmp = b[1];
	b[1] = b[2];
	b[2] = tmp;
}

void swap32_array(int32_t **x, uint16_t n)
{
	while (n--)
		swap32(*x++);
}

void swap16(void *x)
{
	uint8_t *b = x;
	uint8_t tmp;

	tmp = b[0];
	b[0] = b[1];
	b[1] = tmp;
}

void swap16_array(int16_t **x, uint16_t n)
{
	while (n--)
		swap16(*x++);
}

int join_path(char *path, size_t n, const char *dir, const char *file)
{
	return (snprintf(path, n, "%s/%s", dir, file) >= n) ? -1 : 0;
}

int open_image(const char *dir, const char *file, FIL *f,
	       struct pnm_header *hdr)
{
	char path[MAX_PATH_LEN];

	if (snprintf(path, MAX_PATH_LEN, "%s/%s", dir, file) >= MAX_PATH_LEN) {
		LOG("File path is too long, max=%d", MAX_PATH_LEN);
		return -1;
	}

	if (f_open(f, path, FA_READ) != FR_OK) {
		LOG("Failed to open image file");
		return -1;
	}

	if (pnm_read_header(f, hdr) < 0) {
		LOG("Failed to parse PGM header");
		return -1;
	}

	return 0;
}

/* ----------------------------------------------------------------------------
 * Debug utilies
 */

/* Defined in main.c */
extern void abort_now(const char *abort_msg);

static void do_abort_msg(const char *file, unsigned line,
			 const char *error_str, const char *message)
{
	fprintf(stderr, "%s, line %u: %s\n", file, line, message);
	abort_now(error_str);
}

void do_abort_msg_assert(const char *file, unsigned line, const char *message)
{
	do_abort_msg(file, line, "Assertion failed\n", message);
}

void do_abort_msg_error(const char *file, unsigned line, const char *message)
{
	do_abort_msg(file, line, "Fatal error\n", message);
}

void dump_hex(const void *data, uint16_t len)
{
	static const char hex[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};
	char s[] = "[XXXX] XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX";
	char *cur;
	uint16_t i;

	if (!len)
		return;

	for (i = 0, cur = s; i < len; ++i) {
		const uint8_t byte = ((const uint8_t *)data)[i];

		if (!(i & 0xF)) {
			uint16_t addr = i;
			uint16_t j;

			if (i)
				puts(s);

			cur = s + 4;

			for (j = 4; j; --j) {
				*cur-- = hex[addr & 0xF];
				addr >>= 4;
			}

			cur = s + 7;
		}

		*cur++ = hex[byte >> 4];
		*cur++ = hex[byte & 0xF];
		++cur;
	}

	i %= 16;

	if (i) {
		cur = s + 6 + (i * 3);
		*cur++ = '\n';
		*cur++ = '\0';
	}

	puts(s);
}
