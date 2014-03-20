Supported Hardware
==================

Epson Controllers
-----------------
Epson have a range of controllers designed to support the output of images onto electrophoretic (EPD)
displays. The controllers differ in the size of display they can support, whether they have external or
internal frame buffer memory, on-board or external power supplies and support for color displays.

The controllers can be accessed via SPI or a 16 bit parallel data bus.

In addition to the main EPD functionality the controllers contain a varying collection of useful hardware
units that may be required in a system fitted with an electrophoretic display. For example, an I2C master,
SPI master, GPIO ports, and internal temperature sensor.

Which options are available will ultimately depend on the controller selected and how it is connected to
the display and other system components.

The code supports the Epson S1D13524 and S1D13541 controllers in various configurations. The ‘524
controller supports large and color displays and is fitted to a circuit board with its external SDRAM. The ‘541
controller supports smaller displays and is physically bonded to the display module.


Power State Management
^^^^^^^^^^^^^^^^^^^^^^
The Epson S1D13541 controller can be configured to one of several power states; helping to minimise power use 
when appropriate. 

These power states are:

- Power Off
   - Clock chip disabled
   - 3V3 power to S1D13541 disabled
- Standby
   - Can be set from SLEEP or RUN mode
   - Clock chip enabled
   - Power save status bit set to 0
   - Source/gate driver powered off  
- Run 
   - Can be set from SLEEP or STANDBY mode
   - Clock chip enabled
   - Power save status bit set to 1
   - Source/gate driver powered on
- Sleep 
   - Can be set from RUN or STANDBY mode
   - Clock chip disabled
   - Source/gate driver powered off 
   - Power save status bit set to 0



Fig 4-1-1, below, shows the possible power state transitions.

.. image:: pwr_transitions.jpeg
   :width: 75%

*Fig 4-1-1: Power State Transition Diagram*


Below is a breakdown of the actions that must be taken for each of the power state transitions:


Run -> Standby:
***************

- STBY command (CMD(0x04), no parameters) issued to epson controller
- Wait for HRDY = 1
- Standby Mode entered

Sleep -> Standby
****************

- Set CLK_EN GPIO true to re-enable clock
- Set REG[0x0006] bit 8 to 1 for normal power supply
- STBY command (CMD(0x04), no parameters) issued to epson controller
- Wait for HRDY = 1
- Standby Mode entered

Run/Standby -> Sleep:
*********************

- SLP command (CMD(0x05), no parameters) issued to epson controller
- Wait for HRDY = 1
- Set REG[0x0006] bit 8 to 0 for minimum power supply
- Set CLK_EN GPIO to false to disable clock
- Sleep Mode entered

Standby -> Run:
***************

- RUN command (CMD(0x02), no parameters) issued to epson controller
- Wait for HRDY = 1
- Run Mode entered

Sleep -> Run:
*************

- Set CLK_EN GPIO to true to re-enable clock
- Set REG[0x0006] bit 8 to 1 for normal power supply
- RUN command (CMD(0x02), no parameters) issued to epson controller
- Wait for HRDY
- Run Mode entered

Run/Standby/Sleep -> Power Off
******************************

Note: Any data in the image buffer will be lost when going into off mode. If the current displayed image
is to be retained when powering back up, the contents of the image buffer should be copied to a suitable
location (eg. an SD card) before continuing with the power off. This image can then be loaded back into 
the image buffer when coming out of power off mode.

- SLP command (CMD(0x05), no parameters) issued to epson controller
- Set CLK_EN GPIO to false to disable clock
- Set 3V3_EN GPIO to false to disable 3V3 power supply

Power Off -> Standby Mode:
**************************

Note: after each of the following commands, the host should wait for HRDY to be 1 before continuing

- Set 3V3_EN GPIO to true to enable 3V3 power supply
- Set CLK_EN GPIO to true to enable clock
- INIT_CMD_SET command (CMD(0x00 + Epson Instruction Code Binaries)) issued to epson controller
- INIT_SYS_STBY command (CMD(0x06, no parameters) issued to epson controller
- Set Protect Key Code to REG[0x042C] and REG[0x042E]
- BST_WR_MEM command (CMD(0x1D) + Waveform Storage Address) to start loading waveform data
- WR_REG command (CMD(0x11), 0x154 + Waveform) to load waveform data
- BST_END_MEM command (CMD(0x1E), no parameters) to end loading waveform data
- RUN command (CMD(0x02), no parameters) issued to epson controller
- UPD_GDRV_CLR command (CMD(0x37), no parameters)
- WAIT_DSPE_TRG command (CMD(0x28), no parameters)
- S1D13541 is initialised into known state

The EPD Panel and Image Buffer should now be initialised to a known state; either the standard
white initialisation waveform, or image data copied to a safe medium before power off was called.

Power State Demo
****************

A power state demo can be launched using the Plastic Logic reference code by including the following in config.h:

.. code-block:: c

   #define CONFIG_DEMO_POWERMODES 1

This demo will transition through the power states with the following behaviour:

- Go into RUN mode
- Load an image into the image buffer
- Update the display
- Go into SLEEP mode for 2 seconds
- Go into STANDBY mode for 2 seconds
- Go into RUN mode
- Update the display (with image data retained from the previous update)
- Go into POWER OFF mode (CLKI and 3V3 disabled) for 2 seconds
- Go through power on initialize



Plastic Logic Evaluation Hardware
---------------------------------
Display Types
^^^^^^^^^^^^^
The code supports the following Plastic Logic display types. Additional displays will be supported as
required.

+--------------+------------+------------------------------------------------------+
| Display Type | Resolution | Notes                                                |
+==============+============+======================================================+
| Type4        | 1280x960   | External Controller                                  |
|              |            | Requires wiring harness - not supported long term    |
|              |            | This display is no longer available for new designs  |
+--------------+------------+------------------------------------------------------+
| Type11       | 1280x960   | External Controller                                  |
|              |            | Use the Mercury display connector board              |
+--------------+------------+------------------------------------------------------+
| Type16       | 320x240    | Bonded Controller                                    |
|              |            | 4.7" @85ppi, 2.7" @150ppi                            |
+--------------+------------+------------------------------------------------------+
| Type18       | 400x240    | Bonded Controller                                    |
|              |            | 4.0" @115ppi                                         |
+--------------+------------+------------------------------------------------------+
| Type19       | 720x120    | Bonded Controller                                    |
|              |            | 4.9" @150ppi                                         |
|              |            | Requires pixel data to be reordered                  |
+--------------+------------+------------------------------------------------------+


Ruddock2
^^^^^^^^
The Ruddock2 board is a motherboard that sits between a processor module, currently either BeagleBone
or a microcontroller (MSP430) and the display interface board. It provides signal routing from the processor
to the interface connectors together with some LED’s and switches that can be used to configure the
software or create a user interface. The board allows the Epson serial, parallel and TFT interfaces to be
used depending on the interface board and controller selected. The processor board can remove all power
from the Ruddock2 under software control allowing hardware components, e.g. display interface boards, to
be safely exchanged. The board has a 128B EEPROM which can be used as non-volatile storage if required.


HB Z6/Z7
^^^^^^^^
The Z6 and Z7 are very similar boards differing in the display connector used. All versions of the  Z7 board 
have the provision to turn off 3V3 power to the display controller; this feature is absent on version 6.1 of 
the Z6, but has been introduced as of version 6.3, along with the ability to control the clock enable and 
PMIC wake signals. Both boards are intended to drive an S1D13541 small display controller which is bonded to 
the display itself. The board has a TI PMIC and a 128B EEPROM for storing power supply calibration data. The 
VCOM DAC in the PMIC is used to set the VCOM value for the display. The Z7 board is used to drive the 
Type19 “Bracelet display” and the Z6 is used to drive all other Plastic Logic small displays.


HB Z1.3
^^^^^^^
The Z1.3 board is intended to drive an S1D13541 small display controller which is bonded to the display
itself. The board has a MAXIM PMIC and a separate Maxim 5820 DAC for setting the VCOM voltage. There
is no storage for power supply calibration data on this board. Driving this board requires some physical
modifications to the microcontroller board to resolve an SPI wiring issue. It is no longer recommended to
use the MAXIM PMIC for small displays but this board remains useful of an example of how to if required.


Raven
^^^^^
The Raven board is designed to drive large 10.7” Type11 displays. The board has an Epson S1D13524
controller and associated memory, a Maxim PMIC, a 128B EEPROM for storing power supply calibration
data and an LM75 temperature sensor. The VCOM DAC in the PMIC is used to set the VCOM value for the
display.

The board has input connectors that allow it to be controlled via the Serial host interface (SPI) or Parallel
host interface. Additionally the signals to support data transfer using the TFT interface are available. The
board has 5 test pads which bring out the 5 Epson GPIO pins found on the S1D13524.


.. raw:: pdf

    PageBreak

