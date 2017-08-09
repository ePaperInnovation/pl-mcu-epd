/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013, 2014 Plastic Logic Limited

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
 * app/parser.h -- Lightweight string parser
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <app/parser.h>
#include <pl/types.h>
#include <stdlib.h>

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

int parser_read_word(const char *str, const char *sep, unsigned int *out)
{
	char value[16];
	const int len = parser_read_str(str, sep, value, sizeof(value));

	if (len >= 0)
		*out = strtoul(value, NULL, 0);

	return len;
}


int parser_read_area(const char *str, const char *sep, struct pl_area *a)
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
