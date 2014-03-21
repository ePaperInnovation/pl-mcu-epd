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

#ifndef INCLUDE_UTIL_H
#define INCLUDE_UTIL_H 1

#include "types.h"
#include "FatFs/ff.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef LOG_TAG
#define LOG(msg, ...) \
	do { printf("%-16s: "msg"\n", LOG_TAG, ##__VA_ARGS__); } while (0)
#else
#define LOG(msg, ...)
#endif

/** Check for the presence of a file in FatFs */
#define is_file_present(_path) ({ FILINFO i; f_stat(_path, &i) == FR_OK; })

/* FatFS only supports 8.3 filenames, and we work from the current directory so
   paths should be short... */
#define MAX_PATH_LEN 64

extern int join_path(char *path, size_t n, const char *dir, const char *file);

struct pnm_header;
extern int open_image(const char *dir, const char *file, FIL *f,
		      struct pnm_header *hrd);

/* -- Debug utilities */

/** Print the contents of a buffer with offsets on stdout */
extern void dump_hex(const void *data, uint16_t len);

#endif /* INCLUDE_UTIL_H */
