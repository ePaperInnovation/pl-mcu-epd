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


Plastic Logic Evaluation Hardware
---------------------------------
Display Types
^^^^^^^^^^^^^
The code supports the following Plastic Logic display types. Additional displays will be supported as
required.

+--------------+------------+------------------------------------------------------+
| Display Type | Resolution | Notes                                                |
+==============+============+======================================================+
| Type-4       | 1280x960   | External Controller                                  |
|              |            | Requires wiring harness - not supported long term    |
|              |            | This display is no longer available for new designs  |
+--------------+------------+------------------------------------------------------+
| Type-11      | 1280x960   | External Controller                                  |
|              |            | Use the Mercury display connector board              |
+--------------+------------+------------------------------------------------------+
| Type-16      | 320x240    | Bonded Controller                                    |
|              |            | 4.7" @85ppi, 2.7" @150ppi                            |
+--------------+------------+------------------------------------------------------+
| Type-18      | 400x240    | Bonded Controller                                    |
|              |            | 4.0" @115ppi                                         |
+--------------+------------+------------------------------------------------------+
| Type-19      | 720x120    | Bonded Controller                                    |
|              |            | 4.9" @115ppi                                         |
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
The Z6 and Z7 are very similar boards differing in the display connector used and the provision on the Z7 to
turn off 3V3 power to the display controller. Both boards are intended to drive an S1D13541 small display
controller which is bonded to the display itself. The board has a TI PMIC and a 128B EEPROM for storing
power supply calibration data. The VCOM DAC in the PMIC is used to set the VCOM value for the display.
The Z7 board is used to drive the Type-19 “Bracelet display” and the Z6 is used to drive all other Plastic
Logic small displays.


HB Z1.3
^^^^^^^
The Z1.3 board is intended to drive an S1D13541 small display controller which is bonded to the display
itself. The board has a MAXIM PMIC and a separate Maxim 5820 DAC for setting the VCOM voltage. There
is no storage for power supply calibration data on this board. Driving this board requires some physical
modifications to the microcontroller board to resolve an SPI wiring issue. It is no longer recommended to
use the MAXIM PMIC for small displays but this board remains useful of an example of how to if required.


Raven
^^^^^
The Raven board is designed to drive large 10.7” Type-11 displays. The board has an Epson S1D13524
controller and associated memory, a Maxim PMIC, a 128B EEPROM for storing power supply calibration
data and an LM75 temperature sensor. The VCOM DAC in the PMIC is used to set the VCOM value for the
display.

The board has input connectors that allow it to be controlled via the Serial host interface (SPI) or Parallel
host interface. Additionally the signals to support data transfer using the TFT interface are available. The
board has 5 test pads which bring out the 5 Epson GPIO pins found on the S1D13524.


Cuckoo
^^^^^^
The Cuckoo board is designed to drive large 10.7” Type-4 displays. The board has an Epson S1D13524
controller and associated memory, and a Maxim PMIC. A separate Maxim 5820 DAC is used to set the
VCOM value for the display. There is no storage for power supply calibration data on this board.

.. raw:: pdf

    PageBreak

