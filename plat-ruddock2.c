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
 * plat-ruddock2.c -- initialisation code/drivers for Ruddock2 mainboard
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include "platform.h"
#include "types.h"
#include "msp430-gpio.h"
#include "plat-ruddock2.h"

/* Ruddock2 board EEPROM address */
#define	I2C_ID_EEPROM	0x52

/* Navigation buttons */
#define	SW1			GPIO(2,0)
#define	SW2			GPIO(2,1)
#define	SW3			GPIO(2,2)
#define	SW4			GPIO(2,3)
#define	SW5			GPIO(2,4)

/* User leds */
#define	LED1		GPIO(8,0)
#define	LED2		GPIO(8,1)
#define	LED3		GPIO(8,2)
#define	LED4		GPIO(8,3)

/* User selection switches */
#define	SEL1		GPIO(8,4)
#define	SEL2		GPIO(8,5)
#define	SEL3		GPIO(8,6)
#define	SEL4		GPIO(8,7)

/* Parallel interface */
#define	HDB0		GPIO(4,0)
#define	HDB1		GPIO(4,1)
#define	HDB2		GPIO(4,2)
#define	HDB3		GPIO(4,3)
#define	HDB4		GPIO(4,4)
#define	HDB5		GPIO(4,5)
#define	HDB6		GPIO(4,6)
#define	HDB7		GPIO(4,7)

#define	HDB8		GPIO(6,0)
#define	HDB9		GPIO(6,1)
#define	HDB10		GPIO(6,2)
#define	HDB11		GPIO(6,3)
#define	HDB12		GPIO(6,4)
#define	HDB13		GPIO(6,5)
#define	HDB14		GPIO(6,6)
#define	HDB15		GPIO(6,7)

/* TFT interface extensions */
#define	TFT_HSYNC	GPIO(7,2)
#define	TFT_VSYNC	GPIO(7,3)
#define	TFT_DE		GPIO(7,4)
#define	TFT_CLK		GPIO(7,5)

/* General bits */
#define	RESET		GPIO(5,0)
#define	SHUTDOWN 	GPIO(5,1)

/* HBZ7 uses RESERVE1 to control 3V3 on the board */
#define	RESERVE1	GPIO(1,6)
#define	RESERVE2	GPIO(1,7)

#define	PMIC_FLT	GPIO(2,5)
#define	HIRQ		GPIO(2,6)

static int led_state = 0;

static int parallel_interface[] = {
	HDB0, HDB1, HDB2, HDB3,	HDB4, HDB5,	HDB6, HDB7,
	HDB8, HDB9,	HDB10,HDB11,HDB12,HDB13,HDB14,HDB15,
	TFT_HSYNC, TFT_VSYNC, TFT_DE, TFT_CLK
};

static void ruddock2_leds_update(int leds)
{
	gpio_set_value(LED1, (leds & RUDDOCK2_LED1));
	gpio_set_value(LED2, (leds & RUDDOCK2_LED2));
	gpio_set_value(LED3, (leds & RUDDOCK2_LED3));
	gpio_set_value(LED4, (leds & RUDDOCK2_LED4));
}

int ruddock2_init(void)
{
	int gpio;

	/* Navigation buttons */
	gpio_request(SW1, PIN_GPIO | PIN_INPUT | PIN_INTERRUPT | PIN_FALLING_EDGE);
	gpio_request(SW2, PIN_GPIO | PIN_INPUT | PIN_INTERRUPT | PIN_FALLING_EDGE);
	gpio_request(SW3, PIN_GPIO | PIN_INPUT | PIN_INTERRUPT | PIN_FALLING_EDGE);
	gpio_request(SW4, PIN_GPIO | PIN_INPUT | PIN_INTERRUPT | PIN_FALLING_EDGE);
	gpio_request(SW5, PIN_GPIO | PIN_INPUT | PIN_INTERRUPT | PIN_FALLING_EDGE);

	/* User leds */
	gpio_request(LED1, PIN_GPIO | PIN_OUTPUT | PIN_INIT_HIGH);
	gpio_request(LED2, PIN_GPIO | PIN_OUTPUT | PIN_INIT_HIGH);
	gpio_request(LED3, PIN_GPIO | PIN_OUTPUT | PIN_INIT_HIGH);
	gpio_request(LED4, PIN_GPIO | PIN_OUTPUT | PIN_INIT_HIGH);

	/* User selection switches */
	gpio_request(SEL1, PIN_GPIO | PIN_INPUT | PIN_PULL_UP);
	gpio_request(SEL2, PIN_GPIO | PIN_INPUT | PIN_PULL_UP);
	gpio_request(SEL3, PIN_GPIO | PIN_INPUT | PIN_PULL_UP);
	gpio_request(SEL4, PIN_GPIO | PIN_INPUT | PIN_PULL_UP);

	/* Input pins that will move to new owner */
	gpio_request(PMIC_FLT, PIN_GPIO | PIN_INPUT | PIN_PULL_UP);
	gpio_request(HIRQ,     PIN_GPIO | PIN_INPUT | PIN_PULL_UP);

	gpio_request(SHUTDOWN, 	PIN_GPIO | PIN_OUTPUT | PIN_INIT_HIGH);

	/* Reserve is more specific */
	gpio_request(RESERVE1, 	PIN_GPIO | PIN_OUTPUT | PIN_INIT_HIGH);
	gpio_request(RESERVE2, 	PIN_GPIO | PIN_OUTPUT | PIN_INIT_HIGH);

	/* parallel interface signals, define as low outputs for now */
	for (gpio = 0; gpio < ARRAY_SIZE(parallel_interface); gpio++)
	{
		gpio_request(parallel_interface[gpio], PIN_GPIO | PIN_OUTPUT | PIN_INIT_LOW);
	}

	ruddock2_leds_update(led_state);

#if 0
	/* Test program. Tests LEDS, navigation and selection switches */
	{
		extern u8 port2_int_summary;
		u8 bit;
		u8 last_sel = ~ruddock2_selswitch_read();
		u8 last_int_summary = ~port2_int_summary;
		int test1 = 0;
		int test2 = 0;
		while (1)
		{
			u8 sel = ruddock2_selswitch_read();
			if (sel != last_sel)
			{
				printk("Sel: 0x%02X\n", sel);
				last_sel = sel;
				if (sel == 0x0F) {
					test1 = 1;
				}
			}
			for (bit = 0x08; bit; bit >>= 1)
			{
				if (port2_int_summary != last_int_summary)
				{
					printk("Int: 0x%02X\n", port2_int_summary);
					last_int_summary = port2_int_summary;
					if (port2_int_summary == 0x1F) {
						test2 = 1;
					}
				}
				ruddock2_leds_update(bit);
				mdelay(500);
			}
			if (test1 && test2) {
				printk("IO tests done\n");
				break;
			}
		}
	}
#endif

	return 0;
}

int ruddock2_selswitch_read(void)
{
	int settings = 0;

	settings |= (gpio_get_value(SEL1) ? 0 : RUDDOCK2_SEL1);
	settings |= (gpio_get_value(SEL2) ? 0 : RUDDOCK2_SEL2);
	settings |= (gpio_get_value(SEL3) ? 0 : RUDDOCK2_SEL3);
	settings |= (gpio_get_value(SEL4) ? 0 : RUDDOCK2_SEL4);

	return settings;
}

void ruddock2_led_set(int led, int state)
{
	if (state)
		led_state |= led;
	else
		led_state &= led;

	ruddock2_leds_update(led_state);
}
