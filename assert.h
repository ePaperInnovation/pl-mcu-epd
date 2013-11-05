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

void assert_test(int expr, const char *abort_msg);

#define _STR(x)  __STR(x)
#define __STR(x) #x


#if defined(NDEBUG)

#define assert(_ignore) ((void)0)
#define	check(_expr)	assert_test((_expr) != 0, NULL)

#else
#define assert(_expr)   assert_test((_expr) != 0, \
						"Assert, (" _STR(_expr) "), " _STR(__FILE__) \
						":" _STR(__LINE__) "\n")

#define check(_expr)    assert_test((_expr) != 0, \
						"Check, (" _STR(_expr) "), " _STR(__FILE__) \
						":" _STR(__LINE__) "\n")

#endif /* NDEBUG */


#endif /* ASSERT_H_ */
