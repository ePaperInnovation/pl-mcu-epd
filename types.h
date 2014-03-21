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
 * types.h -- General type and constant definitions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef TYPES_H_
#define TYPES_H_

#define	CONFIG_NO_PRINTK	0

/* borrow some type definitions from the Linux kernel
 * Note: C99 compilers support uint8_t etc so these might be more portable
 * also defined in stdint.h
 */
typedef unsigned char u8; 	/* unsigned byte (8 bits) */
typedef unsigned short u16; /* unsigned word (16 bits) */
typedef unsigned long u32; 	/* unsigned 32-bit value */
// u64; /* unsigned 64-bit value */
typedef signed char s8; 	/* signed byte (8 bits) */
typedef signed short s16; 	/* signed word (16 bits) */
typedef signed long s32; 	/* signed 32-bit value */
// s64; /* signed 64-bit value */

typedef u8 bool;
enum bool_opts {
	false = 0,
	true = 1
};

typedef  s16 screen_t;		/* screen selector - a handle */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef min
#define min(x,y)	( (x) < (y) ? (x) : (y) )
#endif
#ifndef max
#define max(x,y)	( (x) > (y) ? (x) : (y) )
#endif

#define DIV_ROUND_CLOSEST(x, divisor)(			\
{												\
	(((x) + ((divisor) / 2)) / (divisor));		\
}												\
)

#define CPU_CLOCK_SPEED_IN_HZ	20000000L
#if CPU_CLOCK_SPEED_IN_HZ < 1000000L
#error CPU_CLOCK_SPEED_IN_HZ assumed to be more than 1MHz in delay timer calculations
#endif

/* -- Sleep & delay -- */

extern void udelay(u16 us);
extern void mdelay(u16 ms);
extern void msleep(u16 ms);

#endif /* TYPES_H_ */
