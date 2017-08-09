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

/* Defines how error will be signalled on status LED */
enum abort_error {
	ABORT_UNDEFINED = 0,     /* Undefined error */
	ABORT_MSP430_GPIO_INIT,	 /* General error initialising GPIO */
	ABORT_MSP430_COMMS_INIT, /* Error initialising MSP430 comms */
	ABORT_HWINFO,            /* Error reading HWINFO EEPROM. Could be
	                          * comms error or content error */
	ABORT_I2C_INIT,          /* Error initialising I2C (Epson) */
	ABORT_DISP_INFO,         /* Error reading display information. Could
	                          * be many errors (comms error, content error
	                          * missing or invalid file, etc).
	                          * Also depends on preprocessor settings */
	ABORT_HVPSU_INIT,        /* Error initialising HVPSU. Most likely to
	                          * be a comms error, but could indicate a
	                          * failed PMIC */
	ABORT_EPDC_INIT,         /* Error initialising EPDC. Could be many
	                          * errors (comms error, EPDC failure, failed
	                          * to load init code, failed on one of several
	                          * commands needed to initialise the EPDC,
	                          * failed to load waveform, etc) */
	ABORT_APPLICATION,       /* Failed while running application. Multiple
	                          * causes for this, depending on application
	                          * that is running. Most likely failures are
	                          * due to missing/invalid files or hardware
	                          * problems such as POK or comms failure */
	ABORT_ASSERT,            /* Failed assert statement (debug use only) */
	ABORT_CONFIG,			 /* Failed to read config file */
};

/* This is always enabled */
#define abort_msg(_msg, _error_code) do { \
	do_abort_msg_error(__FILE__, __LINE__, _msg, _error_code); \
} while (0)

/* These are typically not called directly but via assert or abort_msg */
extern void do_abort_msg_assert(const char *f, unsigned line, const char *msg);
extern void do_abort_msg_error(const char *f, unsigned line, const char *msg, enum abort_error error_code);

#if (defined(NDEBUG) || !GLOBAL_ASSERT) && !defined(LOCAL_ASSERT)
#define assert(_e)
#define assert_fail(_msg)
#else
#define assert(_e) do { \
	if (!(_e)) do_abort_msg_assert(__FILE__, __LINE__, _STR(_e)); \
} while (0)
#define assert_fail(_msg) do { \
	do_abort_msg_assert(__FILE__, __LINE__, _msg); \
} while (0)
#endif

#endif /* ASSERT_H_ */
