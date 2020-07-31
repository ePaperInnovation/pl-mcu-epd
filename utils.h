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

#include "FatFs/ff.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef LOG_TAG
#define LOG(msg, ...) \
	do { printf("%-16s "msg"\n", LOG_TAG, ##__VA_ARGS__); } while (0)
#else
#define LOG(msg, ...)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef min
#define min(x,y)	( (x) < (y) ? (x) : (y) )
#endif
#ifndef max
#define max(x,y)	( (x) > (y) ? (x) : (y) )
#endif

#define DIV_ROUND_CLOSEST(x, divisor)(				\
	{							\
		(((x) + ((divisor) / 2)) / (divisor));		\
	}							\
)

#define CPU_CLOCK_SPEED_IN_HZ	20000000L
#if CPU_CLOCK_SPEED_IN_HZ < 1000000L
#error CPU_CLOCK_SPEED_IN_HZ assumed to be more than 1MHz in delay timer calculations
#endif

typedef uint8_t bool;

enum bool_opts {
	false = 0,
	true = 1
};

/* -- Sleep & delay -- */

extern void udelay(uint16_t us);
extern void mdelay(uint16_t ms);
extern void msleep(uint16_t ms);

/** Check for the presence of a file in FatFs */
extern int is_file_present(const char *path);

/* FatFS only supports 8.3 filenames, and we work from the current directory so
   paths should be short... */
#define MAX_PATH_LEN 64

extern int join_path(char *path, size_t n, const char *dir, const char *file);
/* round up to 8 or 16 */
extern uint16_t align8(uint16_t value);
extern uint16_t align16(uint16_t value);

struct pnm_header;
extern int open_image(const char *dir, const char *file, FIL *f,
		      struct pnm_header *hrd);

/* -- Debug utilities */

/** Print the contents of a buffer with offsets on stdout */
extern void dump_hex(const void *data, uint16_t len);


/**
 * defines a structure representing a array scrambling configuration
 *
 * source_interlaced: defines whether the data will be interlaced or not
 *                    0 -> no interlacing:  S0, S1, S2, S3, S4, ...
 *                    1 -> interlacing:     S0, Sn, S1, Sn+1, S2, Sn+2, ...  (with n = slCount/2)
 *
 * source_direction: direction of source data
 * 					  0 -> upwards: 		S0, S1, S2, S3, ...
 * 					  1 -> downwards: 		Sn, Sn-1, Sn-2, Sn-3, ...        (with n = slCount)
 *
 * source_start: starting position for source outputs, either left (default) or right
 * 					  0 -> left:			S0 is output first
 * 					  1 -> right:			Sn is output first 				 (with n = slCount/2)
 *
 * gate_direction: direction of gate data
 * 					  0 -> upwards: 		G0, G1, G2, G3, ...
 * 					  1 -> downwards: 		Gn, Gn-1, Gn-2, Gn-3, ...		 (with n = glCount)
 *
 */

#define SCRAMBLING_SOURCE_START_BIT				0
#define SCRAMBLING_SOURCE_INTERLACED_BIT		1
#define SCRAMBLING_SOURCE_DIRECTION_BIT			2
#define SCRAMBLING_GATE_DIRECTION_BIT			3
#define SCRAMBLING_SOURCE_SCRAMBLE_BIT			4	// source line is connected to every 2nd pixel
#define SCRAMBLING_GATE_SCRAMBLE_BIT			5	// gate line is connected to every 2nd pixel
#define SCRAMBLING_SCRAMBLE_FIRST_ODD_LINE_BIT	6	// if is set the odd lines will be first
#define SCRAMBLING_SOURCE_MIRROR_RH_BIT 		7	// mirrors the first image half
#define SCRAMBLING_SOURCE_INTERLACED_FIRST_ODD_LINE_BIT 8
#define SCRAMBLING_SOURCE_MIRROR_LH_BIT			9	// mirrors the second image half

#define SCRAMBLING_SOURCE_START_MASK			(1 << SCRAMBLING_SOURCE_START_BIT)
#define SCRAMBLING_SOURCE_INTERLACED_MASK 		(1 << SCRAMBLING_SOURCE_INTERLACED_BIT)
#define SCRAMBLING_SOURCE_DIRECTION_MASK		(1 << SCRAMBLING_SOURCE_DIRECTION_BIT)
#define SCRAMBLING_GATE_DIRECTION_MASK			(1 << SCRAMBLING_GATE_DIRECTION_BIT)
#define SCRAMBLING_SOURCE_SCRAMBLE_MASK			(1 << SCRAMBLING_SOURCE_SCRAMBLE_BIT)
#define SCRAMBLING_GATE_SCRAMBLE_MASK			(1 << SCRAMBLING_GATE_SCRAMBLE_BIT)
#define SCRAMBLING_SCRAMBLE_FIRST_ODD_LINE_MASK	(1 << SCRAMBLING_SCRAMBLE_FIRST_ODD_LINE_BIT)
#define SCRAMBLING_SOURCE_MIRROR_RH_MASK 		(1 << SCRAMBLING_SOURCE_MIRROR_RH_BIT)
#define SCRAMBLING_SOURCE_INTERLACED_FIRST_ODD_LINE_MASK (1 << SCRAMBLING_SOURCE_INTERLACED_FIRST_ODD_LINE_BIT)
#define SCRAMBLING_SOURCE_MIRROR_LH_MASK 		(1 << SCRAMBLING_SOURCE_MIRROR_LH_BIT)


/** copies data from source to target array while applying a scrambling algorithm
 * S0, Sn, S1, Sn+1, S2, Sn+2; with n = slCount/2;
 * no scrambling in gate direction
 * Expects data in source array as sourceline fast addressed and starting with gate=0 and source=0.
 */
uint16_t scramble_array(uint8_t* source, uint8_t* target, uint16_t *glCount, uint16_t *slCount, uint16_t scramblingMode, uint8_t bitMode);

uint16_t calcScrambledIndex(uint16_t scramblingMode, uint16_t gl, uint16_t sl, uint16_t *glCount, uint16_t *slCount);


#endif /* INCLUDE_UTIL_H */
