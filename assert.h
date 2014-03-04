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

#ifndef ASSERT_H_
#define ASSERT_H_

/* Set to 1 to enable all assert statements.  Alternatively, define
 * LOCAL_ASSERT in each individual .c file before including this file.  */
#define GLOBAL_ASSERT 0

#define _STR(x)  __STR(x)
#define __STR(x) #x

enum abort_error {
	ABORT_ERROR = 0,
	ABORT_ASSERT,
	ABORT_CHECK,
};

extern void do_abort_msg(const char *file, unsigned line,
			 enum abort_error error, const char *message);

#define abort_msg(_msg) do { \
	do_abort_msg(__FILE__, __LINE__, ABORT_ERROR, _msg); \
} while (0)

#if (defined(NDEBUG) || !GLOBAL_ASSERT) && !defined(LOCAL_ASSERT)
#define assert(_e)
#define assert_fail(_msg)
#else
#define assert(_e) do { \
	if (!(_e)) do_abort_msg(__FILE__, __LINE__, ABORT_ASSERT, _STR(_e)); \
} while (0)
#define assert_fail(_msg) do { \
	do_abort_msg(__FILE__, __LINE__, ABORT_ASSERT, _msg); \
} while (0)
#endif

#endif /* ASSERT_H_ */
