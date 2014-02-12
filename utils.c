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

#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include "FatFs/ff.h"
#include "msp430-gpio.h"
#include "pnm-utils.h"
#include "assert.h"
#include "utils.h"

/* ----------------------------------------------------------------------------
 * Lightweight string parser
 */

int parser_find_str(const char *str, const char *sep, int skip)
{
	int i;

	for (i = 0; str[i]; ++i) {
		int found = 0;
		int j;

		for (j = 0; sep[j]; ++j) {
			if (str[i] == sep[j]) {
				found = 1;
				break;
			}
		}

		if (found ^ skip)
			return i;
	}

	return -1;
}

int parser_read_str(const char *str, const char *sep, char *out, int out_len)
{
	int len;

	len = parser_find_str(str, sep, 0);

	if (!len)
		return 0;

	if (len == -1)
		while (str[++len]);

	if (len >= out_len)
		return -1;

	memcpy(out, str, len);
	out[len] = '\0';
	len += parser_find_str((str + len), sep, 1);

	return len;
}

int parser_read_int(const char *str, const char *sep, int *out)
{
	char value[16];
	const int len = parser_read_str(str, sep, value, sizeof(value));

	if (len >= 0)
		*out = atoi(value);

	return len;
}

int parser_read_int_list(const char *str, const char *sep, int **list)
{
	const char *opt = str;

	while (*list != NULL) {
		const int len = parser_read_int(opt, sep, *list++);

		if (len <= 0)
			return len;

		opt += len;
	}

	return (opt - str);
}

int parser_read_area(const char *str, const char *sep, struct area *a)
{
	int *coords[] = { &a->left, &a->top, &a->width, &a->height, NULL };

	return parser_read_int_list(str, sep, coords);
}

int parser_read_file_line(FIL *f, char *buffer, int max_length)
{
	size_t count;
	char *out;
	int i;

	for (i = 0, out = buffer; i < max_length; ++i, ++out) {
		if (f_read(f, out, 1, &count) != FR_OK)
			return -1;

		if ((*out == '\n') || !count)
			break;

		if (*out == '\r')
			--out;
	}

	if (i == max_length)
		return -1;

	*out = '\0';

	return !!count;
}

/* ----------------------------------------------------------------------------
 * Debug utilies
 */

/* Defined in main.c */
extern void abort_now(const char *abort_msg);

void do_abort_msg(const char *file, unsigned line,
		  enum abort_error error, const char *message)
{
	static const char *error_str[] = {
		"Fatal error\n", "Assertion failed\n", "Check failed\n",
	};

	fprintf(stderr, "%s, line %u: %s\n", file, line, message);
	abort_now(error_str[error]);
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
