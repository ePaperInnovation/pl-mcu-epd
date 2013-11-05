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
 * S1D135xx.h -- Controller common functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef S1D135XX_H_
#define S1D135XX_H_

enum {
	PWR_STATE_UNDEFINED = 0,
	PWR_STATE_SLEEP,
	PWR_STATE_STANDBY,
	PWR_STATE_RUN
};

enum {
	TEMP_MODE_UNDEFINED = 0,
	TEMP_MODE_MANUAL,
	TEMP_MODE_INTERNAL,
	TEMP_MODE_EXTERNAL
};

struct s1d135xx {
	short id;
	screen_t screen;
	int power_mode;
	short xres;
	short yres;
	u16 keycode1;
	u16 keycode2;

	u8 temp_mode;		/* temperature sensing mode */
	s8 temp_measured;	/* last temp measured */
	s8 temp_set;		/* last temp specificed */

};

/*
 * All known commands. Not all controllers support all commands
 */
#define INIT_CMD_SET 			0x00 // Initialize instruction code if external serial flash is not used
#define INIT_PLL 				0x01 // Initialize PLL. this command should only be used when auto load from flash is disabled (CNF4=1)
#define RUN_SYS 				0x02 // Go to RUN mode
#define STBY 					0x04 // Go to Stand By mode
#define SLP 					0x05 // Go to Sleep Mode
#define INIT_SYS_RUN 			0x06 // Initialize system and go into Run State (524)
#define INIT_SYS_STBY			0x06 // Initialise system and go into standby (541)
#define INIT_TFT_CFG 			0x08 // Initialize TFT config
#define INIT_DSPE_CFG			0x09 // Initialize Display engine
#define INIT_DSPE_TMG			0x0A // Initialize Driver timing
#define INIT_HOST_ROT_MODE		0x0B // Initialize rotation mode timing
#define INIT_WAVE_DEV			0x0C // Setup Waveform device select
#define INIT_DSPE_TMG_ADV		0x0D // Initialize the display engine advance timing
#define INIT_CTLR_MODE 			0x0E // Initialize controller for color or grayscale mode

// register and memory access commands
#define RD_REG		 			0x10 // Read register
#define WR_REG		 			0x11 // Write register
//
#define	RD_TEMP					0x12 // Read Temperature (541)

#define WAIT_DUC_NOT_BUSY		0x13 // Wait for DUC to be IDLE
#define WAIT_DUC_DSPE_IDLE		0x14 // Wait for DUC's DSPE trigger is IDLE
#define WAIT_DUC_CLRPRC_IDLE	0x15 // Wait for DUC's Color Proc trigger is IDLE

// Host Raw Burst Access Commands
#define BST_RD_SDR		 		0x1C // Start burst read SDRAM memory
#define BST_WR_SDR		 		0x1D // Start Burst write SDRAM memory
#define BST_END_SDR				0x1E // Burst End

// Host Image Loading Commands
#define LD_IMG_HOST		 		0x20 // Load image Full
#define LD_IMG_HOST_AREA		0x22 // Load image Area with parameters
#define LD_IMG_HOST_END			0x23 // Load image End

// common Image Loading Commands
#define LD_IMG_SET_RIMGADR		0x24 // Set Raw image address (16bpp for color only)
#define LD_IMG_SET_PIMGADR		0x25 // Set processed image address (4bpp)

// display engine polling commands
#define WAIT_DSPE_TRG		 	0x28 // wait for display engine trigger done
#define WAIT_DSPE_FREND			0x29 // wait for display engine frame end
#define WAIT_DSPE_LUTFREE		0x2a // wait for display engine at least 1 LUT is free

//TFT Loading Commands
#define LD_IMG_TFT		 		0x2C // Start full TFT Capture
#define LD_IMG_TFT_AREA			0x2D // Start Area Sweep TFT capture
#define LD_IMG_TFT_END			0x2E // TFT Load Stop

// Waveform Update commands
#define RD_WFM_INFO 			0x30 // Read Wevform information
#define UPD_INIT 				0x32 // Update Buffer Initialize
#define UPD_FULL 				0x33 // update Buffer Full
#define UPD_FULL_AREA 			0x34 // Update Buffer Full Area
#define UPD_PART 				0x35 // update buffer partial
#define UPD_PART_AREA 			0x36 // update buffer partial area
#define UPD_GDRV_CLR 			0x37 // gate Driver Clear Command

// color engine commands
#define CLR_PROC 				0x39 // trigger color processing
#define CLR_PROC_AREA 			0x3A // trigger color processing area target
#define WAIT_CLR_PRC_END 		0x3B // wait for color processing end
#define LD_CLR_CFG 				0x3C // start color config binary file loading
#define LD_CLR_CFG_END 			0x3D // terminates color config bin file loading
#define SET_CLR_CFG 			0x3E // setup color engine function

// ======================= Registers
//
//system configuration registers
#define REV_CODE_REG 						0x0000 // revision code register
#define PROD_CODE_REG 						0x0002 // product code register
#define CONF_PIN_RD_VAL_REG					0x0004 // config pin read value register
#define PWR_SAVE_MODE_REG 					0x0006 // power save mode register
#define SOFT_RST_REG 						0x0008 // Software reset register
#define SYS_STAT_REG 						0x000A // system status register

#define	PERIPHERAL_CONFIG_REG				0x0020	// Peripheral device configuration

// Host interface memory Access Configuration
#define HOST_MEM_CONF_REG 					0x0140 // host memory access configuration and status register
#define HOST_MEM_PORT_REG 					0x0154 // host memory access port register

// power pin control configuration register
#define PWR_CTRL_REG						0x0230 // Power pin control register

// interrupt status registers
#define INT_RAW_STATUS_REG					0x0240	// Summary of interrupt sources

// command sequencer controller register
#define CMD_SEQ_AUTOBOOT_CMD_REG			0x02A8 // Command sequencer auto boot command register

// Display Engine: Status register
#define DISPLAY_BUSY_REG					0x0338 // Display engine busy register

// Display Engine: Interrupt register
#define DISPLAY_INT_RAW_STAT_REG			0x033A // Display engine interrupt raw status register
#define DISPLAY_INT_TEMP_OUT_OF_RANGE		(1<<10)

#define SYS_STAT_BUSY_BIT					BIT5 // 5	// BUSY status

#define WAVEFORM_MODE(x)		((x << 8) & 0x0f00)
#define UPDATE_LUT(x)			((x << 4) & 0x00f0)

#define WAVEFORM_WHITEOUT		0
#define WAVEFORM_DIRECT_MONO	1
#define WAVEFORM_HIGH_QUALITY	2
#define WAVEFORM_HIGH_SPEED		3


int s1d135xx_select(struct s1d135xx *epson, screen_t *previous);
int s1d135xx_deselect(struct s1d135xx *epson, screen_t previous);

#endif /* S1D135XX_H_ */
