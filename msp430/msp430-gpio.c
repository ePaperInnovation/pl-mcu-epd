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
 * msp430-gpio.c -- MSP430 GPIO pin management functions
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com?
 *
 * The purpose of this module is to allow some dynamic control of GPIO
 * configuration. It is not intended to validate all requests completely
 * but where possible it will given the data it has available.
 *
 * It is only intended to handle 8 bit ports at the moment and can only handle
 * 1 bit at a time. It is intended to help avoid multiple use of resources
 * and to take care of all the register settings required to configure the pin.
 *
 */

#include <pl/gpio.h>
#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include "assert.h"
#include "msp430-gpio.h"
#include "plat-gpio.h"
#include "msp430f5438a.h"

#define LOG_TAG "msp430-gpio"
#include "utils.h"

#define GPIO_PORT(_gpio) (((_gpio) >> 8) & 0xFF)
#define GPIO_PIN(_gpio) ((_gpio) & 0xFF)

/* Set to 1 to enable checks (takes a bit more code space) */
#define GPIO_CHECK_PARAMETERS 0

/* Set to 1 to enable all get-set checks (slows down I/O) */
#define GPIO_CHECK_GET_SET 0

#define PxIN(_x_)	P ##_x_ ##IN
#define PxOUT(_x_)	P ##_x_ ##OUT
#define PxDIR(_x_)	P ##_x_ ##DIR
#define PxREN(_x_)	P ##_x_ ##REN
#define PxDS(_x_)	P ##_x_ ##DS
#define PxSEL(_x_)	P ##_x_ ##SEL
#define PxIES(_x_)	P ##_x_ ##IES
#define PxIE(_x_)	P ##_x_ ##IE
#define PxIFG(_x_)	P ##_x_ ##IFG

unsigned char rdata[256];
unsigned char sdata[16];

/* Private functions */

#if PL_GPIO_DEBUG
static int msp430_gpio_pin_number(uint16_t pinmask);
#endif
static void msp430_gpio_check_port(uint16_t port);
static const struct io_config *msp430_gpio_get_port(unsigned gpio);


/* Could maybe not store offsets if we can compute them?
 * This is a big table but it's in flash. */
static const struct io_config {
	volatile uint8_t *in; /* if NULL then this port not present */
	volatile uint8_t *out;
	volatile uint8_t *dir;
	volatile uint8_t *resenable;
	volatile uint8_t *strength;
	volatile uint8_t *fnselect;
	volatile uint8_t *edge;         /* only in ports 1 & 2 */
	volatile uint8_t *intenable;    /* only in ports 1 & 2 */
	volatile uint8_t *intflag;      /* only in ports 1 & 2 */
} msp430_gpio_defs[] = {
#ifdef __MSP430_HAS_PORT1_R__
		{ &PxIN(1), &PxOUT(1), &PxDIR(1), &PxREN(1), &PxDS(1),
		  &PxSEL(1), &PxIES(1), &PxIE(1), &PxIFG(1) },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT2_R__
		{ &PxIN(2), &PxOUT(2), &PxDIR(2), &PxREN(2), &PxDS(2),
		  &PxSEL(2), &PxIES(2), &PxIE(2), &PxIFG(2) },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT3_R__
		{ &PxIN(3), &PxOUT(3), &PxDIR(3), &PxREN(3), &PxDS(3),
		  &PxSEL(3), NULL, NULL, NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT4_R__
		{ &PxIN(4), &PxOUT(4), &PxDIR(4), &PxREN(4), &PxDS(4),
		  &PxSEL(4), NULL, NULL, NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT5_R__
		{ &PxIN(5), &PxOUT(5), &PxDIR(5), &PxREN(5), &PxDS(5),
		  &PxSEL(5), NULL, NULL, NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT6_R__
		{ &PxIN(6), &PxOUT(6), &PxDIR(6), &PxREN(6), &PxDS(6),
		  &PxSEL(6), NULL, NULL, NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT7_R__
		{ &PxIN(7), &PxOUT(7), &PxDIR(7), &PxREN(7), &PxDS(7),
		  &PxSEL(7), NULL, NULL, NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT8_R__
		{ &PxIN(8), &PxOUT(8), &PxDIR(8), &PxREN(8), &PxDS(8),
		  &PxSEL(8), NULL, NULL, NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT9_R__
		{ &PxIN(9), &PxOUT(9), &PxDIR(9), &PxREN(9), &PxDS(9),
		  &PxSEL(9), NULL, NULL, NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT10_R__
		{ &PxIN(10),&PxOUT(10), &PxDIR(10), &PxREN(10), &PxDS(10),
		  &PxSEL(10), NULL, NULL, NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT11_R__
		{ &PxIN(11),&PxOUT(11), &PxDIR(11), &PxREN(11), &PxDS(11),
		  &PxSEL(11), NULL, NULL, NULL },
#else
		{ NULL },
#endif
};

static int msp430_gpio_config(unsigned gpio, uint16_t flags)
{
	const struct io_config *io;
	const uint16_t port = GPIO_PORT(gpio);
	const uint16_t pinmask = GPIO_PIN(gpio);

#if PL_GPIO_DEBUG
	LOG("config gpio=0x%04X (%d.%d)", gpio, (port + 1),
	    msp430_gpio_pin_number(pinmask));
	pl_gpio_log_flags(flags);
#endif

	msp430_gpio_check_port(port);
	io = &msp430_gpio_defs[port];

#if GPIO_CHECK_PARAMETERS
	if (pl_gpio_check_flags(flags))
		return -1;
#endif

	if (flags & PL_GPIO_SPECIAL)
		*io->fnselect |= pinmask;
	else
		*io->fnselect &= ~pinmask;

	if (flags & PL_GPIO_OUTPUT) {
		*io->dir |= pinmask;

		if (flags & PL_GPIO_INIT_H)
			*io->out |= pinmask;
		else
			*io->out &= ~pinmask;

		if (flags & PL_GPIO_DRIVE_REDUCED)
			*io->strength &= ~pinmask;
		else
			*io->strength |= pinmask;
	} else if (flags & PL_GPIO_INPUT) {
		*io->dir &= ~pinmask;
		*io->resenable &= ~pinmask;

		if (flags & (PL_GPIO_PU | PL_GPIO_PD)) {
			*io->resenable |= pinmask;

			if (flags & PL_GPIO_PU)
				*io->out |= pinmask;
			else
				*io->out &= ~pinmask;
		}

		if (flags & PL_GPIO_INTERRUPT) {
			*io->intenable |= pinmask;

			if (flags & PL_GPIO_INT_RISE)
				*io->edge &= ~pinmask;
			else
				*io->edge |= pinmask;
		} else {
			*io->intenable &= ~pinmask;
		}
	}

	return 0;
}

int msp430_gpio_get(unsigned gpio)
{
	const struct io_config *port = msp430_gpio_get_port(gpio);

	return *port->in & GPIO_PIN(gpio);
}

void msp430_gpio_set(unsigned gpio, int value)
{
	const struct io_config *port = msp430_gpio_get_port(gpio);
	const uint16_t pinmask = GPIO_PIN(gpio);

	if (value)
		*port->out |= pinmask;
	else
		*port->out &= ~pinmask;
}

int msp430_gpio_init(struct pl_gpio *gpio)
{
	gpio->config = msp430_gpio_config;
	gpio->get = msp430_gpio_get;
	gpio->set = msp430_gpio_set;

	// RCL GPIO Connections for SPI flash - all set as in or out, no special functions
	// winbond drivers use bitbang, not uart functions
	P8DIR |= BIT0;  //Set cs Chip select as output
	P8DIR |= BIT1;  //Set wp Write Protect as output
	P8DIR |= BIT2;  //Set hold as Output
	P5DIR |= BIT6;  //Set si Slave In as output (this chip is master)
	P3DIR |= BIT6;  //Set clk Clock as output
    P5DIR &= ~BIT7; // data out from spi flash

	P3OUT &= ~BIT6; //Set clk low
	P8OUT |= BIT1;  //wp high
	P8OUT |= BIT0;  //Set cs high
	P8OUT |= BIT2;  // set hold high
/*
#define    cs     MSP430_GPIO(8,0)-
#define    sio1   GPIOA_Pin_2
#define    so     MSP430_GPIO(5,7)
#define    sio2   GPIOA_Pin_3
#define    wp     MSP430_GPIO(8,1)-
#define    sio0   GPIOA_Pin_5
#define    si     MSP430_GPIO(5,6)-
#define    clk    MSP430_GPIO(3,6)-
#define    sio3   GPIOA_Pin_7
#define    hold   MSP430_GPIO(8,2)-
	*/

	return 0;
}

// WRITE to device on rising edge of clk
// READ from device on falling edge of clk
// For Mode 0, the CLK signal is normally low on the falling and
// rising edges of /CS

void Bump_cs(void)
{
    P3OUT &= ~BIT6;         // clk low
    P8OUT &= ~BIT0;         // cs low
    __delay_cycles(100);
    P8OUT |= BIT0;          // cs high
}


unsigned int SectorErase(unsigned int addr)
{//0x20 Sector erase
    if (CheckBusy()==1) return 1;        //Flash Busy
    //if (addr> FlashSize) return 1;       //out of range

    P8OUT &= ~BIT0;             // cs low
    __delay_cycles(50);
    bitbang(0x06);              //Write enable 0x06
    __delay_cycles(50);
    P8OUT |= BIT0;              // cs high
    __delay_cycles(50);
    P8OUT &= ~BIT0;             // cs low
    __delay_cycles(50);
    bitbang(0x20);              //0x20 Sector erase
    __delay_cycles(50);
    bitbang(0x00);              // 24bits used first bit first - add 4 trailing zeroes to real address
    bitbang(0x00);              // RCL REMOVE -   hard coded address
    bitbang(0x00);
    __delay_cycles(50);
    P8OUT |= BIT0;              // cs high
    return 0;
}

// we
// wd


void Setup_Flash_Write(void)
{
    unsigned int i;
    while(CheckBusy()==1)
        {
        __delay_cycles(505);
        }
/*
    P8OUT &= ~BIT0;         // cs low
    __delay_cycles(50);    // Reset SPI Flash
    bitbang(0x66);
    __delay_cycles(50);
    P8OUT |= BIT0;          // cs high
    __delay_cycles(50);
    P8OUT &= ~BIT0;         // cs low
    bitbang(0x99);
    __delay_cycles(50);    // Reset SPI Flash
    P8OUT |= BIT0;          // cs high
    P3OUT &= ~BIT6;         //clk=0;
    __delay_cycles(50);
    */
    P8OUT &= ~BIT0;         // cs low
    __delay_cycles(50);
    bitbang(0x06);          //Write enable 0x06
    __delay_cycles(50);
    P8OUT |= BIT0;          // cs high
    __delay_cycles(50);
    P8OUT &= ~BIT0;         // cs low
    __delay_cycles(50);
    bitbang(0x02);          // Page program instruction
    __delay_cycles(50);
    bitbang(0x00);         // 24bits used first bit first - add 4 trailing zeroes to real address
    bitbang(0x00);
    bitbang(0x00);
    __delay_cycles(50);
    bitbang(0xAA);          // first byte of data after Page Program - ref data sheet
    for(i=0; i<255; i++)
      {
        bitbang(0x21);
      }
    bitbang(0x00);
    P8OUT |= BIT0;       // cs high
  //  while(status_register); // poll status register
    __delay_cycles(620000);
}

void Setup_Flash_Read(void)
{
    unsigned int i;
    __delay_cycles(5000);
    P3OUT &= ~BIT6;         //clk=0;
    __delay_cycles(50);
    P8OUT &= ~BIT0;         // cs low
    __delay_cycles(50);
    bitbang(0x03);          //
    __delay_cycles(50);
    bitbang(0x00);         // 24bits used first bit first - add 4 trailing zeroes to real address
    bitbang(0x00);
    bitbangL(0x00);
    __delay_cycles(125);
    for(i=0; i<256; i++)
    {
        __delay_cycles(150);
        rdata[i] = CpuIn();
    }

    P8OUT |= BIT0;
}

unsigned char CheckBusy(void)
{                         //Check ReadyBusy Bit
    unsigned char temp;
    temp = ReadStatusRegister1();
    if((temp& 0x01)==0x01)
        return 1;   //busy
    else
        return 0;   //ready
}

unsigned char ReadStatusRegister1(void)
{
    //0x05 Read Status Register 1
    unsigned char StatusRegister1data;

    P8OUT &= ~BIT0;         // cs low

    bitbangL(0x05);
    StatusRegister1data = CpuIn();

    P8OUT |= BIT0;          // cs high
    return StatusRegister1data;
}

// WRITE to device on rising edge of clk
// READ from device on falling edge of clk
// For Mode 0, the CLK signal is normally low on the falling and
// rising edges of /CS
void Read(unsigned char addr)
{ //0x03 Read data
    unsigned int i;

   // if(CheckBusy()==1) return 1;
   // if(addr > FlashSize) return 1;
   // if(count<1) return 1;

    for(i=0; i<256; i++)
    {
        rdata[i] = CpuIn();
    }

    P8OUT |= BIT0;

}

void Read_Status_Registers(void)
{
    unsigned int i;

    P8OUT &= ~BIT0;         // cs low
    P3OUT |= BIT6;      // clk=1
    bitbang(0x05);
    __delay_cycles(100);

        for(i=0; i<16; i++)
        {
            sdata[i] = CpuIn();
        }
        P8OUT |= BIT0;          // cs high
}

void Read_Jedec(void)
{
    unsigned int i;

    P8OUT &= ~BIT0;         // cs low
    __delay_cycles(50);
    bitbang(0x9F);
    __delay_cycles(50);

    for(i=0; i<3; i++) // RCL CHANGE data shifted out 1st falling edge
    {
        sdata[i] = CpuIn();
    }
    P8OUT |= BIT0;          // cs high

}

void ChipErase(void)
{//0xC7/60 Chip erase
    P8OUT &= ~BIT0;             // cs low
    bitbang(0x06);
    P8OUT |= BIT0;              // cs high
    __delay_cycles(100);
    P8OUT &= ~BIT0;             // cs low
    bitbang(0xC7);
    P8OUT |= BIT0;              // cs high
    __delay_cycles(650320);

}

void bitbang(unsigned char instr)
{
    unsigned int i;
    P3OUT &= ~BIT6;     //clk=0;
    for(i=0; i<8; i++)
    {

        if((instr &0x80)==0x80) // set instruction on MSP output pin - rising clock edge writes to flash
        {
            P5OUT |= BIT6;
        }
        else
        {
            P5OUT &= ~BIT6;
        }
        instr = instr <<1;
        __delay_cycles(50);
        P3OUT |= BIT6;      // clk=1
        __delay_cycles(50);
        P3OUT &= ~BIT6;     //clk=0;
        P5OUT &= ~BIT6;     // data to zero-low
        __delay_cycles(50);
    }
}


void bitbangL(unsigned char instr)
{
    unsigned int i;
    P3OUT &= ~BIT6;     //clk=0;
    for(i=0; i<8; i++)
    {

        if((instr &0x80)==0x80) // set instruction on MSP output pin - rising clock edge writes to flash
        {
            P5OUT |= BIT6;
        }
        else
        {
            P5OUT &= ~BIT6;
        }
        instr = instr <<1;
        __delay_cycles(50);
        P3OUT |= BIT6;      // clk=1
        __delay_cycles(50);
        if(i<7){P3OUT &= ~BIT6;}     //clk=0;
        P5OUT &= ~BIT6;     // data to zero-low
        __delay_cycles(50);
    }
}


unsigned char CpuIn(void)
{
    unsigned int i;
    unsigned char buf;
 //   unsigned char tst,tst1;

    buf=0;
    char* p5point = (char*)0x000240;


    __delay_cycles(50);
    for(i=0; i<8; i++)
    {
        P3OUT |= BIT6;      // clk=1
        __delay_cycles(50);
        P3OUT &= ~BIT6;      // clk=0
        __delay_cycles(50);
        d = p5point[0];

        if(d == 0xBF)
        {
            buf=(buf|(0x80>>i));
        }
        // P3OUT |= BIT6;      // clk=1

    }
    return buf;
}

//char value = *pointer;
//char fist_byte = pointer[0];
//char second_byte = pointer[1];






/* ----------------------------------------------------------------------------
 * private functions
 */

#if PL_GPIO_DEBUG
static int msp430_gpio_pin_number(uint16_t pinmask)
{
	int bit;

	for (bit = 0; pinmask; pinmask >>= 1, bit++);

	return bit;
}
#endif

static void msp430_gpio_check_port(uint16_t port)
{
	if (port >= ARRAY_SIZE(msp430_gpio_defs))
		abort_msg("Invalid port number", ABORT_MSP430_GPIO_INIT);

	if (msp430_gpio_defs[port].in == NULL)
		abort_msg("Port not available", ABORT_MSP430_GPIO_INIT);
}

static const struct io_config *msp430_gpio_get_port(unsigned gpio)
{
	const uint16_t port = GPIO_PORT(gpio);

#if GPIO_CHECK_GET_SET
	msp430_gpio_check_port(port);
#endif

	return &msp430_gpio_defs[port];
}
