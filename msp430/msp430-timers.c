/*
 * Copyright (C) 2013 Plastic Logic Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
/*
 * msp430-timers.c -- MSP430 timer functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <msp430.h>
#include "types.h"
#include "msp430-gpio.h"
#include "main.h"

#define INIT_COUNT_L 0xC3
#define INIT_COUNT_H 0x50

static int delay;

#define CPU_CYCLES_PER_USECOND (CPU_CLOCK_SPEED_IN_HZ/1000000L)
#define CPU_CYCLES_PER_MSECOND (CPU_CLOCK_SPEED_IN_HZ/1000L)

/* Functions for delay/sleep. Needs to be calibrated */
void udelay(u16 us)
{
	while (us--)
	{
		__delay_cycles(CPU_CYCLES_PER_USECOND);
	}
}

void mdelay(u16 ms)
{
    while (ms--)
    {
        __delay_cycles(CPU_CYCLES_PER_MSECOND);
    }
}

void msleep(u16 ms)
{
	mdelay(ms);
}


void init_rtc()
{
	  // Setup RTC Timer
	  RTCCTL01 |= RTCTEVIE;		// interrupt enable
	  RTCCTL01 &= ~RTCMODE; 	// counter mode
	  RTCCTL01 |= RTCSSEL_0;
	  RTCCTL01 |= RTCTEV_1;

	  // load the counter for a 10s timeout
	  RTCNT1 = 0xFF - INIT_COUNT_H;
	  RTCNT2 = 0xFF - INIT_COUNT_L;

	  RTCCTL01 &= ~RTCHOLD; 	// start counter
}

#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
{
	unsigned int gie = __get_SR_register();

	LPM3_EXIT;

    __disable_interrupt();

    switch(__even_in_range(RTCIV,16))
    {
		case 0: break;            		// No interrupts
		case 2: break;            		// RTCRDYIFG
		case 4: break;             		// RTCEVIFG => timer interrupt
		case 6: break;                  // RTCAIFG
		case 8: break;                  // RT0PSIFG
		case 10: break;                 // RT1PSIFG
		case 12: break;                 // Reserved
		case 14: break;                 // Reserved
		case 16: break;                 // Reserved
		default: break;
    }

    __bis_SR_register(gie);
}

void timer_start(void)
{
	delay = 1;
	TA0CTL = TASSEL_1 | ID_3 | MC_2 | TACLR | TAIE;         // ACK ( = 10kHz), contmode, clear interrupt enable
	TA0R = 0xFFFF - 0x927C;									// Configure the timer
}

void timer_stop(void)
{
	TA0CTL = MC_0;
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void TIMER0_ISR(void)
{
	unsigned int gie = __get_SR_register();

	__disable_interrupt();

	switch(__even_in_range(TA0IV,16))
	{
	case 0x0E:
		delay--;
		if(delay < 0)
		{
			timer_stop();
			timer_start();
		}

		// reload the timer for 1 minute
		TA0R = 0xFFFF - 0x927C;

		break;
	default:break;

	}

    __bis_SR_register(gie);
}
