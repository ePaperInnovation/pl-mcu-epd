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
 * plat-ruddock2.c -- initialisation code/drivers for Ruddock2 mainboard
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <pl/platform.h>
#include <pl/gpio.h>
#include <stdint.h>
#include "types.h" /* ARRAY_SIZE */
#include "msp430-gpio.h"
#include "plat-ruddock2.h"

/* Ruddock2 board EEPROM address */
#define	I2C_ID_EEPROM 0x52

#if 0 /* dead code */
/* General bits */
#define	RESET       MSP430_GPIO(5,0)
#define	SHUTDOWN    MSP430_GPIO(5,1)

/* HV-PMIC things */
#define	PMIC_FLT    MSP430_GPIO(2,5)
#endif

static uint8_t led_state = 0;

#if 0 /* dead code */
static void ruddock2_leds_update(struct pl_gpio *gpio, uint8_t leds)
{
	gpio->set(LED1, (leds & RUDDOCK2_LED1));
	gpio->set(LED2, (leds & RUDDOCK2_LED2));
	gpio->set(LED3, (leds & RUDDOCK2_LED3));
	gpio->set(LED4, (leds & RUDDOCK2_LED4));
}
#endif

int ruddock2_init(struct pl_gpio *gpio)
{
#if 0 /* dead code */
	{ HIRQ,     PL_GPIO_INPUT | PL_GPIO_PU      },

	/* Input pins that will move to new owner */
	{ PMIC_FLT, PL_GPIO_INPUT | PL_GPIO_PU      },
	{ SHUTDOWN, PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
#endif


	/* parallel interface signals, define as low outputs for now */

#if 0
	ruddock2_leds_update(gpio, led_state);
#endif

	return 0;
}

#if 0
static void test(void)
{
	/* Test program. Tests LEDS, navigation and selection switches */
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

#if 0 /* not used anywhere */
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
#endif
