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
 * epson-cmd.c -- Epson controller high level commands
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include "assert.h"
#include "types.h"
#include "S1D135xx.h"
#include "epson-cmd.h"
#include "epson-if.h"

/* Enable prototype parallel interface support */
#define CONFIG_IF_PARALLEL		0

/* Enable Epson command tracing */
#define CONFIG_TRACE_COMMANDS	0

/* Note: Tracing SPI commands when communicating with the Maxim PMIC
 * results in i2c failures. No other device is affected by this.
 * Debug output format:
 * [ 		- Epson chip selected
 * ] 		- Epson chip deselected
 * {xxxx}	- Command word
 * (xxxx)	- Data word
 * Bulk transfers display the first and last N words transferred and
 * the total of words transferred
 * Register read commands display the value read back from the controller
 * after the => symbol
 *
 */
#define	DEBUG_SPI_CMDS		CONFIG_TRACE_COMMANDS	// trace commands to Epson
#if	DEBUG_SPI_CMDS
#define DEBUG_FIRST_N		4		// first N words of data displayed
#define DEBUG_LAST_N		4		// last N words of data displayed (must be power of 2)
static u32 total;
static u16 rate_limit;
static u8  print_first_n;			// false when first N printed
static uint16_t last_n[DEBUG_LAST_N];
#endif

// idle = ((status & mask) == result)
static uint16_t idle_mask = 0xffff;	// default value will result in busy loop
static uint16_t idle_result = 0x0000;	//

#if 0
// later if we add wait decoration to command codes.
int epson_select_screen(int wait)
{
	spi_select_epson();
	if (wait)
	{
		epson_wait_for_idle();
	}
	return 0;
}

int epson_deselect_screen(int wait)
{
	if (wait)
	{
		epson_wait_for_idle();
	}
	spi_deselect_epson();
	return 0;
}
#endif

static int sendCmd(uint16_t command)
{
	endianess x;

	x.command = htobe16(command);

	epsonif_set_command();
	spi_write_bytes(x.bytes,2);
	epsonif_set_data();

	return 0;
}

static int sendParam(uint16_t param)
{
	endianess x;

	x.data = htobe16(param);
	spi_write_bytes(x.bytes,2);

	return 0;
}

int epson_cmd_p0(uint16_t command)
{
#if	DEBUG_SPI_CMDS
	printk("[{0x%04x}]\n", command);
#endif
	epsonif_select_epson();
	sendCmd(command);
	epsonif_deselect_epson();

	return 0;
}

int epson_cmd_p1(uint16_t command, uint16_t p1)
{
#if	DEBUG_SPI_CMDS
	printk("[{0x%04x}, (0x%04x)]\n", command, p1);
#endif
	epsonif_select_epson();
	sendCmd(command);
	sendParam(p1);
	epsonif_deselect_epson();
	return 0;
}

int epson_cmd_p2(uint16_t command, uint16_t p1, uint16_t p2)
{
#if	DEBUG_SPI_CMDS
	printk("[{0x%04x}, (0x%04x), (0x%04x)]\n", command, p1, p2);
#endif
	epsonif_select_epson();
	sendCmd(command);
	sendParam(p1);
	sendParam(p2);
	epsonif_deselect_epson();
	return 0;
}

int epson_cmd_p3(uint16_t command, uint16_t p1, uint16_t p2, uint16_t p3)
{
#if	DEBUG_SPI_CMDS
	printk("[{0x%04x}, (0x%04x), (0x%04x), (0x%04x)]\n",
	       command, p1, p2, p3);
#endif
	epsonif_select_epson();
	sendCmd(command);
	sendParam(p1);
	sendParam(p2);
	sendParam(p3);
	epsonif_deselect_epson();
	return 0;
}

int epson_cmd_p4(uint16_t command, uint16_t p1, uint16_t p2, uint16_t p3,
		 uint16_t p4)
{
#if	DEBUG_SPI_CMDS
	printk("[{0x%04x}, (0x%04x), (0x%04x), (0x%04x), (0x%04x)]\n",
	       command, p1, p2, p3, p4);
#endif
	epsonif_select_epson();
	sendCmd(command);
	sendParam(p1);
	sendParam(p2);
	sendParam(p3);
	sendParam(p4);
	epsonif_deselect_epson();
	return 0;
}

int epson_cmd_p5(uint16_t command, uint16_t p1, uint16_t p2, uint16_t p3,
		 uint16_t p4, uint16_t p5)
{
#if	DEBUG_SPI_CMDS
	printk("[{0x%04x}, (0x%04x), (0x%04x), (0x%04x), (0x%04x), (0x%04x)]\n",
	       command, p1, p2, p3, p4, p5);
#endif
	epsonif_select_epson();
	sendCmd(command);
	sendParam(p1);
	sendParam(p2);
	sendParam(p3);
	sendParam(p4);
	sendParam(p5);
	epsonif_deselect_epson();
	return 0;
}

int epson_begin_bulk_code_transfer(uint16_t command)
{
#if	DEBUG_SPI_CMDS
	total = 0;
	rate_limit = 0;
	print_first_n = 1;
	printk("[{0x%04x},...\n", command);
#endif
	epsonif_select_epson();
	return sendCmd(command);
}

int epson_begin_bulk_transfer(uint16_t reg)
{
#if	DEBUG_SPI_CMDS
	total = 0;
	rate_limit = 0;
	print_first_n = 1;
	printk("[{0x%04x}, (0x%04x),...\n", WR_REG, reg);
#endif
	epsonif_select_epson();
	sendCmd(WR_REG);
	return sendParam(reg);
}


/* write word to the SPI interface. Data is assumed to be in cpu endianess
 * format. BeginBulkTransfer() must have been called first
 */
int epson_bulk_transfer_word(uint16_t data)
{
#if	DEBUG_SPI_CMDS
	total++;
	last_n[rate_limit++ & (DEBUG_LAST_N-1)] = data;
	if (print_first_n && rate_limit > DEBUG_FIRST_N)
		print_first_n = 0;
	if (print_first_n)
		printk(" (0x%04x)\n", data);
#endif
	return sendParam(data);
}

/* write raw data to the SPI interface. Data is assumed to be in correct
 * format for direct transfer to Epson controller (16 bits words, Msb first)
 * BeginBulkTransfer() must have been called first
 */
int epson_bulk_transfer_raw_data(u8 *buffer, size_t length)
{
	spi_write_bytes(buffer,length);
	return 0;
}

int epson_end_bulk_transfer()
{
	epsonif_deselect_epson();
#if	DEBUG_SPI_CMDS
	unsigned int index;
	int i;
	printk("...\n");
	index = (rate_limit - DEBUG_LAST_N);
	for (i = 0; i < DEBUG_LAST_N; i++)
	{
		printk(" (0x%04x)\n", last_n[index++ & (DEBUG_LAST_N-1)]);
	}
	printk("...] - (total sent = 0x%lx)\n", total);
#endif
	return 0;
}

int epson_reg_read(uint16_t reg, uint16_t* value)
{
	int ret;
	endianess x;

	epsonif_select_epson();

	ret = sendCmd(RD_REG);
	ret = sendParam(reg);
#if !CONFIG_IF_PARALLEL
	/* read 2 dummy bytes sent by Epson */
	spi_read_bytes(x.bytes,2);	// read the two dummy bytes
#endif
	spi_read_bytes(x.bytes,2); 	// read the register contents
	*value = be16toh(x.data);

	epsonif_deselect_epson();

#if	DEBUG_SPI_CMDS
	printk("[{0x%04x}, (0x%04x) =>0x%04x]\n", RD_REG, reg, *value);
#endif
	return ret;
}

int epson_reg_write(uint16_t reg, uint16_t value)
{
	return epson_cmd_p2(WR_REG, reg, value);
}

int epson_is_busy(void)
{
	uint16_t status = 0;

#if SPI_HRDY_USED
	// Check the status of the HRDY pin if we are using it
#if SPI_HRDY_SELECT
	epsonif_select_epson();
#endif
	if (spi_read_hrdy())
		status |= idle_mask;
#if SPI_HRDY_SELECT
	epsonif_deselect_epson();
#endif
#else
	// Read the system status register to determine if busy
	(void)epson_reg_read(SYS_STAT_REG, &status);
#endif

	return ((status & idle_mask) != idle_result);
}

void epson_wait_for_idle_mask(uint16_t mask, uint16_t result)
{
	idle_mask = mask;
	idle_result = result;
}

void epson_wait_for_idle_timeout(unsigned timeout_ms)
{
	while (epson_is_busy()) {
		if (!timeout_ms--)
			abort_msg("S1D13541 HRDY timeout");

		mdelay(1);
	}
}
