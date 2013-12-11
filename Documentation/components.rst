Platform Neutral Components
===========================
File System
-----------
The micro SD card uses a FAT/FAT16 file system for data storage (not FAT-32). In order to minimise code
and data size the FatFs driver is configured to support Read-Only operations, to reuse memory aggressively
and not to support long filenames. This has some small impact on access time and transfer speed for the
data within files.

Long filenames can be used when writing files to the SD card from a PC however the FatFs code can only
use the 8.3 compatible filenames. These names can be displayed under Windows by entering “DIR /X” e.g.:

21/05/2011 07:01 8,863,336 NVWGF2~1.DLL nvwgf2umx.dll


+----------------------------------------+-------------------------------------------------------+
| SD Card path                           | Contents                                              |
+========================================+=======================================================+
| 0:/<display-type>                      | Root of the subtree for the selected display type     |
+----------------------------------------+-------------------------------------------------------+
| 0:/<display-type>/bin/ecode.bin        | Epson controller initialisation file for display type |
+----------------------------------------+-------------------------------------------------------+
| 0:/<display-type>/img/\*.pgm           | Image files to be displayed                           |
+----------------------------------------+-------------------------------------------------------+
| 0:/<display-type>/display/vcom.txt     | VCOM voltage for display                              |
+----------------------------------------+-------------------------------------------------------+
| 0:/<display-type>/display/waveform.bin | Waveform for the display                              |
+----------------------------------------+-------------------------------------------------------+

Note: The VCOM and waveform data for each display should be stored on the display's EEPROM where applicable
(Type 19 displays have no EEPROM). The Plastic Logic reference code uses the data stored on the EEPROM by
default. The data on the SD card can be used instead by modifying the value of ``CONFIG_WF_ON_SD_CARD`` in
the config header file (``config.h``). For the best results, it is advisable to use the EEPROM-based data
as this is tuned for each display.


Epson Controller Interface
--------------------------
The Epson controller interface provides a layer to send high level commands and data to the controllers.
This encapsulates the SPI interface and the management of other interface signals.

All commands must pass through this layer on the way to the Epson controller. In order to assist in
debugging communication issues a detailed trace of interaction with the controller can be enabled by
setting the CONFIG_TRACE_COMMANDS macro in Epson/Epson-cmd.c to 1:

.. code-block:: config

    /* Enable Epson command tracing */
    #define CONFIG_TRACE_COMMANDS 0

Sample output:

.. code-block:: sample-output

    [{0x0011}, (0x0008), (0x00ff)] // write register {0x0011}, reg:0x0008, value:0x00ff
    [{0x0010}, (0x000a) =>0x2000] // read register {0x0010}, reg:0x000a, read:0x2000
	
The output can be used with a scope to verify correct operation of the interface signals.
[ - Epson controller selected
{Command Word} – HDC => Low
(Data Word) – HDC => High
] – Epson controller deselected

Utility functions provide higher level functions on top of command transfer layer. These functions support
initialisation code and waveform loading, frame buffer RAM fill, image data transfer and power state
transition control.

It is worth noting that Epson name the SPI data signals with respect to the controller. Hence DI (DataIn) =>
MOSI, and DO(DataOut) => MISO.

To prepare the controller for operation it is necessary to send two files to it:

1. A controller initialisation file which customises the controller's behaviour to the type of display it is going to drive, e.g. resolution, driver configuration, clock timings.
2. A waveform data file which provides display specific timing information required to maximise the performance and image quality of a display.


Epson S1D135xx I2C Interface
----------------------------
The Epson controllers provide an SPI to I2C bridge that can be used to communicate with I2C peripherals
instead of using an I2C interface on the host processor. The I2C interface abstraction defined in i2c.h allows
higher level software to communicate using either method once an interface has been initialised.

The bridge results in a slower overall I2C data rate than a host I2C interface would achieve due to the
overhead of communicating over SPI to manage the transfer. However, in normal use the amount of I2C
traffic is limited to one-time device configuration.

Note that some peripherals, the MAXIM 17135 PMIC specifically, have inbuilt timeouts which can be
triggered when Epson command tracing is taking place and the Epson I2C bridge is in use.


Temperature Measurement
-----------------------
The accurate measurement of temperature is important to obtaining the best image quality from the
display. The temperature is used to select the correct waveform used to drive the display. It is common for
display updates to take longer at lower temperatures due to the physical attributes of the display media.
The ‘524 and ‘541 have differing methods of handling temperature measurement. These are exposed in the
code as “modes”:

1. Manual – The application software will obtain the temperature from some other component, e.g. the PMIC and pass it to the controller.
2. Internal – The display controller will use its own internal temperature sensor, if it has one, to measure the temperature.
   - The S1D13541 controller contains a temperature sensor, which requires an NTC thermistor to be fitted.
3. External – The display controller will communicate directly with an LM75 compatible I2C temperature sensor to obtain the temperature.

To trigger the acquisition or processing of temperature data the controller's measure _temperature()
function is called. On completion a new temperature will be in effect. On the ‘541 controller an indication
that the waveform data must be reloaded is given if the temperature measured has moved outside the
range of the currently cached waveform data.

Currently only the Manual and Internal modes are implemented.


VCOM Calibration
----------------
The accurate setting of the VCOM voltage is essential to obtaining the best image quality from the display.
Each display has associated with it a VCOM voltage that must be used – specified in millivolts. In order to
translate from mV to the required VCOM DAC value a software component takes the requested VCOM
value and the power supply calibration information and returns a value to be written to the DAC register.
The calibration data is determined by measuring a sample of power supplies using a defined calibration
procedure. The output of the calibration procedure must be made available to the VCOM software module
when it is initialised (See ``vcom_init()`` in the ``vcom.c`` source file). The display interface boards either store this data in an EEPROM on the board or it is
measured once and stored in the code.

The VCOM calibration procedure is described in the document “Electronics for small displays” available
from Plastic Logic.


Hardware Components
-------------------
This section lists the hardware components commonly found on boards intended to drive Plastic Logic
displays that require software drivers.


Maxim 5820 DAC
^^^^^^^^^^^^^^
The 5820 DAC is a general purpose I2C 8bit DAC used to set the VCOM voltage on some boards. It can be
turned off to save power. The need for an external DAC has largely been removed from new designs by the
ability to use the VCOM DAC provided in the PMIC instead.


Microchip EEPROMs
^^^^^^^^^^^^^^^^^
The code supports I2C EEPROMs up to 64KB in size. The code currently supports two I2C EEPROM types:

1. 24LC014 – this is a small 128B EEPROM fitted to later display interface boards and is used to store power supply calibration data. This permits accurate VCOM voltages to be achieved when the display interface board is swapped.
2. 24AA256 – this is a 32KB EEPROM found on some display types. It is intended to store waveform information so that the necessary information to drive a display travels with the display. This allows the system to ensure the correct waveform information is used for the display. Since waveforms can exceed 32KB in size, the data stored on this EEPROM is compressed using the LZSS compression alorithm.
3. EEPROM types can be added by extending the table that defines the device characteristics.


Maxim LM75 Temperature Sensor
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The LM75 temperature sensor is a configurable I2C temperature sensor that can measure temperature
autonomously at programmable intervals. It can be used when the temperature measuring facilities of the
PMIC’s cannot be used for some reason.
The measured temperature register can be read automatically by the Epson controllers.


Maxim 17135 HV PMIC
^^^^^^^^^^^^^^^^^^^
The Maxim PMIC is used on boards primarily intended to drive the large 10.7” displays. Its key features are:

1. I2C interface for configuration of power sequence timings
2. Hardware signals for PowerUp/Down, PowerGood and PowerFault
3. I2C commands for PowerUp/Down and power supply monitoring
4. Inbuilt 8bit VCOM DAC
5. In built LM75 compatible temperature sensor with automatic temperature sensing


TI 65185 HV PMIC
^^^^^^^^^^^^^^^^
The TI PMIC is used on boards intended to drive the small displays. Its key features are:

1. I2C interface for configuration of power sequence timings
2. Hardware signals PowerUp/Down, PowerGood and PowerFault
3. I2C commands for PowerUp/Down and power supply monitoring
4. Inbuilt 9bit VCOM DAC
5. In built LM75 compatible temperature sensor with on demand temperature sensing.


Putting it all Together
-----------------------
The source code contains examples of how to drive a number of different display interface boards.

The files plat-cuckoo.c, plat-hbz13.c, plat-hbz6.c and plat-raven.c collect together the necessary hardware
component support in one place and show how they should be initialised and managed to produce a
working system.

plat-hbz6.c and plat-raven.c are the primary reference platforms with the others being legacy platforms
which are still supported as they provide useful references.

Reviewing these files will make it much clearer how the software components are put together to create a
working system.

.. raw:: pdf

   PageBreak
