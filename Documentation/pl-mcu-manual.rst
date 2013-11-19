Introduction
============

This document provides an introduction and overview to the code intended to be used by customers
wishing to drive Plastic Logic displays and associated controller hardware from a single chip microcontroller
platform. The MSP430 is the first reference target, based on customer interest.
As delivered the code runs on a hardware reference platform, available from Plastic Logic, comprising a
microcontroller processor board, a motherboard and display interface boards for a range of displays. The
code runs a slide show demonstration to show how the hardware components work together to drive
images on a display.
The code requires no underlying operating system to function. Some commonly available C runtime library
functions are used.
The project is in active development and feedback is welcomed on new features or issues found in the code
or documentation. Please send feedback via your sales/support representative.

Scope
=====

This document does not attempt to describe the detailed operation of any particular microcontroller or
Epson display controller as this information is readily available, or may require an NDA to disclose. Prior
experience with embedded programming is expected and discussion will focus on the specifics of this code
base.
The code is able to drive a slideshow of full size, pre-rendered, images in the PGM file format to a chosen
display. Due to restrictions on resources the code is not currently able to manipulate the images in any
way.
The code focusses on functionality and does not pretend to implement best practice for any specific
microcontroller. Power efficiency and data transfer speed improvements are planned for subsequent
releases.
The code attempts to strike a balance between minimising microcontroller resource usage while preserving
portability, good coding practices and the provision of good debug support (e.g. use of assertions).

Licensing
=========

The majority of the software in this codebase was written by Plastic Logic and is currently licensed under a
restrictive license for early adopters. For the avoidance of confusion:

This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Some key support functionality is provided by third parties who have their own licenses. The third party
components are:

FatFs – A FAT file-system driver. This is used to access configuration and image data stored on a micro SD
card on the reference microcontroller hardware. The license for FatFS can be found here:
`http://elm-chan.org/fsw/ff/en/appnote.html#license <http://elm-chan.org/fsw/ff/en/appnote.html#license>`_, it is not restrictive.

Sample code - This is sample source code made freely available by the microcontroller vendor. The
copyright notices vary from source file to source file but are not restrictive other than limiting the use of
such processor specific sample code to a given range of processor devices. Please see Appendix A for
license text.

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
--------
The Ruddock2 board is a motherboard that sits between a processor module, currently either BeagleBone
or a microcontroller (MSP430) and the display interface board. It provides signal routing from the processor
to the interface connectors together with some LED’s and switches that can be used to configure the
software or create a user interface. The board allows the Epson serial, parallel and TFT interfaces to be
used depending on the interface board and controller selected. The processor board can remove all power
from the Ruddock2 under software control allowing hardware components, e.g. display interface boards, to
be safely exchanged. The board has a 128B EEPROM which can be used as non-volatile storage if required.

HB Z6/Z7
--------
The Z6 and Z7 are very similar boards differing in the display connector used and the provision on the Z7 to
turn off 3V3 power to the display controller. Both boards are intended to drive an S1D13541 small display
controller which is bonded to the display itself. The board has a TI PMIC and a 128B EEPROM for storing
power supply calibration data. The VCOM DAC in the PMIC is used to set the VCOM value for the display.
The Z7 board is used to drive the Type-19 “Bracelet display” and the Z6 is used to drive all other Plastic
Logic small displays.

HB Z1.3
-------
The Z1.3 board is intended to drive an S1D13541 small display controller which is bonded to the display
itself. The board has a MAXIM PMIC and a separate Maxim 5820 DAC for setting the VCOM voltage. There
is no storage for power supply calibration data on this board. Driving this board requires some physical
modifications to the microcontroller board to resolve an SPI wiring issue. It is no longer recommended to
use the MAXIM PMIC for small displays but this board remains useful of an example of how to if required.

Raven
-----
The Raven board is designed to drive large 10.7” Type-11 displays. The board has an Epson S1D13524
controller and associated memory, a Maxim PMIC, a 128B EEPROM for storing power supply calibration
data and an LM75 temperature sensor. The VCOM DAC in the PMIC is used to set the VCOM value for the
display.

The board has input connectors that allow it to be controlled via the Serial host interface (SPI) or Parallel
host interface. Additionally the signals to support data transfer using the TFT interface are available. The
board has 5 test pads which bring out the 5 Epson GPIO pins found on the S1D13524.

Cuckoo
------
The Cuckoo board is designed to drive large 10.7” Type-4 displays. The board has an Epson S1D13524
controller and associated memory, and a Maxim PMIC. A separate Maxim 5820 DAC is used to set the
VCOM value for the display. There is no storage for power supply calibration data on this board.

.. raw:: pdf

    PageBreak

Getting Started
===============
This section covers setting up the hardware and software so that a given display type can be driven. Please
follow the steps outlined in order to setup and build the software.

Obtaining the Code
------------------
The code is delivered as a zip archive containing the source code, SD card contents (initialisation data,
waveforms and images), and documentation. Unzip this archive in some suitable location that the
development tools will be able to access.

Hardware Setup
--------------
The software requires a processor board, Ruddock2, a display interface board and a display to match the
interface board.

On the Ruddock 2 ensure that:

1. The “I2C isolate” 2 pin header, has no link fitted
2. The P4 2 pin header, has a link fitted
3. The switch SW7 is set to ON
4. 5V power supply, 200mA for small displays, 2A for large displays
	
The processor board plugs into the Ruddock2 using the two parallel headers, note the processor board
outline in the silk screen on the Ruddock2 for correct orientation.

The display interface board connects to the Ruddock2 serial interface connector (the smaller of the two FFC
connectors) using a flexi-cable and finally the display itself connects to the display interface board either
directly in the case of the small displays or via a Mercury board using a 50way flexi cable.

SD Card Setup
-------------
The micro SD card for the processor board must be formatted as a FAT/FAT16 file-system (not FAT32).
Unzip the contents of the archive ”Support/sd-card-content/sd-card-gold.zip” and place the resulting files
on the SD card so that the root directory of the file-system contains the folders Type-4, Type-11 etc.

The supplied content provides a safe set of configuration data for each type of display. In order to obtain
the best image quality the waveform.bin and vcom.txt files must be replaced with data specific to the
display you are using. These files are located at:

0:/<Display-Type>/display/waveform.bin
0:/<Display-Type>/display/vcom.txt

Place the micro SD card in the micro SD card socket on the processor board.

Building and Configuring the Code
---------------------------------
Please refer to the microcontroller specific section for details on how to configure and build the code for
your selected microcontroller platform.

Please continue reading here once the code is running.

You should now be able to see a slide show of stock images from the “0:/<Display-Type>/img” folder being
shown on the display until execution is halted. The slideshow will skip any files that do not have the
extension “.pgm”


Preparing Your Own Images
=========================

Image Format
------------
For simplicity the code only supports image files in the PGM graphics file format.
For details see: `http://en.wikipedia.org/wiki/Netpbm_format <http://en.wikipedia.org/wiki/Netpbm_format>`_ .

This is a simple, uncompressed, file format that can be generated with GIMP (Linux or Windows) or using
the Python Imaging Library.

Image files are expected to match the full display size, and in the case of the Type-19 “bracelet” displays
the pixel data must be reordered, so that the contents can be transferred directly from storage to the
display controller.

The SD card content provided contains the original source PNG images which were used to create the PGM
files should they be required.

Image Conversion Tools
----------------------
The code archive contains a python script, img_convert.py, which uses the Python Imaging Library (PIL) to
support the translation of PNG files to PGM format. The script can also reorder the pixel data as required
for Type-19 displays.

The script requires Python version 2.7.5 and a compatible version of the Python Imaging Library (PIL), and
works in both Linux and Windows.

Python 2.7.5 can be downloaded from the following URL:

`http://www.python.org/getit <http://www.python.org/getit>`_

For Windows, use the 32-bit or 64-bit 2.7.5 installer as appropriate for the host machine.
For Linux, use either of the source tarballs for 2.7.5. Installation instructions can be found within the tarball.

The Python Imaging Library can be found here:

`http://pythonware.com/products/pil <http://pythonware.com/products/pil>`_

For Windows, use the most recent "Windows only" PIL for Python 2.7. When installing PIL, ensure the
destination directory is the same as the Python 2.7.5 installation directory.
For Linux, use the most recent PIL source kit. Again, installation instructions can be found within the tarball.

Execute the script from the command line in either operating system, passing the image to be converted as
the first argument. If the target device is using a Type-19 display, pass "scramble" as the second argument
in order to generate the correct pixel data ordering.

E.g. (Linux example):
$: python img_convert.png image.png scramble

For the above example, the output will be a file called image_s.pgm. For a non-scrambled image, the
output will be image_n.pgm.

The output files should be copied to the SD Card in the img folder of the appropriate display type folder
e.g.:

0:/Type-16/img/image_n.pgm


Code Structure
==============

Overview
--------
The diagram below shows an overview of the code base.

.. image:: images/codeblock.jpg

Things to note are:

1. The application sits right on top of the common components. There is no layer that abstracts a complete display system that can be manipulated by calling methods on it. 
2. The Host abstraction layer allows for porting to different CPU’s, either members of the same family or different architectures. Interrupts and Timers are not mandatory for the sample code to work.
3. There is an “Access Abstraction Layer”. This exists because the Epson controllers contain a number of resources, e.g. I2C master, SPI master, and on chip GPIO’s that the Application layer may want to use. This abstraction layer allows the application to access either a host CPU resource or one contained in the Epson controller without needing to know its location once initialised. Currently only support for I2C is implemented.


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

+---------------------------------------+-------------------------------------------------------+
|SD Card path                           | Contents                                              |
+=======================================+=======================================================+
|0:/<display-type>                      | Root of the subtree for the selected display type     |
+---------------------------------------+-------------------------------------------------------+
|0:/<display-type>/bin/ecode.bin        | Epson controller initialisation file for display type |
+---------------------------------------+-------------------------------------------------------+
|0:/<display-type>/img/*.pgm            | Image files to be displayed                           |
+---------------------------------------+-------------------------------------------------------+
|0:/<display-type>/display/vcom.txt     | VCOM voltage for display                              |
+---------------------------------------+-------------------------------------------------------+
|0:/<display-type>/display/waveform.bin | Waveform for the display                              |
+---------------------------------------+-------------------------------------------------------+

Note a default waveform and VCOM is provided for each display type. These should be replaced with
module specific data in order to get the best display quality.

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

1. A controller initialisation file which customises the controllers behaviour to the type of display it is going to drive, e.g. resolution, driver configuration, clock timings.
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
3. External – The display controller will communicate directly with an LM75 compatible I2C temperature sensor to obtain the temperature.

To trigger the acquisition or processing of temperature data the controllers measure _temperature()
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
when it is initialised. The display interface boards either store this data in an EEPROM on the board or it is
measured once and stored in the code.

The VCOM calibration procedure is described in the document “Electronics for small displays” available
from Plastic Logic.

Hardware Components
-------------------
This section lists the hardware components commonly found on boards intended to drive Plastic Logic
displays that require software drivers.

Maxim 5820 DAC
--------------
The 5820 DAC is a general purpose I2C 8bit DAC used to set the VCOM voltage on some boards. It can be
turned off to save power. The need for an external DAC has largely been removed from new designs by the
ability to use the VCOM DAC provided in the PMIC instead.

Microchip EEPROMs
-----------------
The code supports I2C EEPROMs up to 64KB in size. The code currently supports two I2C EEPROM types:

1. 24LC014 – this is a small 128B EEPROM fitted to later display interface boards and is used to store power supply calibration data. This permits accurate VCOM voltages to be achieved when the display interface board is swapped.
2. 24AA256 – this is a 32KB EEPROM found on some display types. It is intended to store waveform information so that the necessary information to drive a display travels with the display. This allows the system to ensure the correct waveform information is used for the display. Since waveforms are likely to exceed 32KB in size some sort of compression will be required. Support of this feature will be in Version 2.0 of this software.
3. EEPROM types can be added by extending the table that defines the device characteristics.

Maxim LM75 Temperature Sensor
-----------------------------
The LM75 temperature sensor is a configurable I2C temperature sensor that can measure temperature
autonomously at programmable intervals. It can be used when the temperature measuring facilities of the
PMIC’s cannot be used for some reason.
The measured temperature register can be read automatically by the Epson controllers.

Maxim 17135 HV PMIC
-------------------
The Maxim PMIC is used on boards primarily intended to drive the large 10.7” displays. Its key features are:

1. I2C interface for configuration of power sequence timings
2. Hardware signals for PowerUp/Down, PowerGood and PowerFault
3. I2C commands for PowerUp/Down and power supply monitoring
4. Inbuilt 8bit VCOM DAC
5. In built LM75 compatible temperature sensor with automatic temperature sensing

TI 65185 HV PMIC
----------------
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


Host Abstraction Layer
======================
The host abstraction layer isolates the platform neutral code from the underlying platform hardware. The
abstraction layers are kept as self-contained and thin as practical. While interrupts and timers are listed
their availability is not required to create a working system.

Host GPIO Interface
-------------------
The GPIO interface provides a way to reserve and define a GPIO pin on the host processor at run time. On
small microcontrollers pins are typically either GPIO’s or connected to a single special purpose hardware
unit e.g. an I2C unit. Some, or all, of the GPIO’s supported may be able to generate interrupts.

The GPIO interface records which GPIO’s are already defined but not the mode in which they are
configured. This allows the code to trap errors where a pin is defined multiple times, or used before being
defined. GPIO pins are typically used to control the power sequence hardware and manipulate signals in
the serial and parallel interface to the Epson controller.

Host I2C Interface
------------------
The host I2C interface provides access to an I2C interface physically attached to the host processor. Only a
single I2C interface is supported by the code. A host I2C interface may not be required if the system is
configured to use the Epson SPI-I2C bridge feature instead.

Examples of devices connected to I2C include the HVPMIC, temperature sensors, and EEPROMs.

Host SPI Interface – Epson
--------------------------
The host SPI-Epson interface provides access to an SPI interface that is connected to the Epson controller
when it is operating in serial interface mode. On short cables this interface has been operated at 20MHz
successfully. In general the Epson controller should be placed on its own SPI bus due to the need to keep
the chip selected for the entire duration of the image data transfer operation which may be up to 1MB.

Host SPI Interface – SD Card
----------------------------
The host SPI-SDCard interface provides access to an SPI interface that is connected to the SD Card. The SD
Card is operated at 20MHz. If additional hardware is available in the host processor the SD Card could be
operated in 4 bit parallel mode for improved data transfer speed.

Host Interrupt Interface
------------------------
The interrupt interface supports the processing of interrupts. The code currently does not use interrupts
but the first usage will be for notifying the code that the Epson is ready to accept a new command by the
assertion of the HRDY line.
The abstraction is still to be defined

Host Timer Interface
--------------------
The timer interface provides platform specific timer implementations. Currently delays are coded as busy
loops. A more power efficient mechanism will follow in a future release.


Texas Instruments – MSP430
==========================

This section provides information specific to deploying the code on the reference MSP430 platform
hardware available from Plastic Logic. This provides a supported, stable environment in which to
investigate the behaviour of the code before moving on to a customised platform.

MSP430 - Getting Started
------------------------

Importing the project
---------------------
The project code needs to be imported into the Code Composer Studio (CCS) IDE in order to build it using
the following steps:

1. Project | Import Existing CCS Eclipse Project
2. “Select Search-directory:” and use the Browse button to navigate to the folder created when the distribution archive was unzipped.
3. In the “Discovered projects” window select the uc-epson project
4. Click Finish and the project will be imported into the IDE
5. Click on uc-epson [Active - Debug] in the Project Explorer Window
6. Project | Build All
   This will rebuild the code. Note that there is one error and one warning. This is expected. The
   warning is in the Fat file-system code and can be ignored. The error indicates that the code has not
   yet been correctly configured.
	
Configuring the code
--------------------
The code is configured at compile time to define the type of display interface board and display-type that it
will be driving.

At the top of main.c (shown in the Project Explorer window with a red cross) are several macros.

1. Select a display interface board type: define one of PLAT_CUCKOO, PLAT_RAVEN, PLAT_Z13, PLAT_Z6 or PLAT_Z7 to be 1
2. Select a display type to be driven by uncommenting the appropriate CONFIG_DISPLAY_TYPE macro. The available options are shown directly below the corresponding PLAT_* macros. The Cuckoo and Raven boards can only drive a single display type so no display needs to be selected.
3. If the I2C interface is to be provided by the Epson controller define CONFIG_I2C_ON_EPSON to be 1. The Cuckoo and Raven boards have this predefined by the board design so this macro will be ignored. 0 is the safe default.

Rebuild the code (Project | Build All (or Control-B)). The error in main.c will go away and the code is now
configured to drive the selected board and display type.

Download and Run
----------------
Plug the USB MSP430-FET430UIF programmer into the JTAG programming port on the MSP430 board.
Turn on the 5V supply to the Ruddock2 and select “Run | Debug” in the IDE.

The first time the IDE downloads the code the “Ultra-Low-Power Advisor” may appear warning about
possible power optimisations. Select “Do not show this again” and proceed.

The code will be downloaded into the MSP430 and the IDE will stop at the entry to main().

To run the code press F8 (or the green go arrow in the debugger tool bar). The code will start and the
display should initialise and start to cycle through the images appropriate for the type of display selected.

Debug messages will be displayed in the Debugger Console in the IDE.

Once the MSP430 has been programmed the slideshow will run automatically when power is applied to the
board (unless the debugger is attached holding it in reset).

Target Processor
----------------
The code is targeted to the MSP430F5438A controller. This part was chosen because it gave plenty of
resources to work with during early development and was compatible with the MSP-EXP430F5438
development board from TI.

Further development has refined the resources required and the MSP430F5310 would have sufficient
resources to drive a display with similar performance while using only 48 pins.

Parrot - MSP430 Processor Board
-------------------------------
The Parrot board docks with the Ruddock2 motherboard to provide access to the display interfaces. It has
the same form factor and connector pin out as a BeagleBone allowing the processors to be easily swapped
for evaluation or development work.

The board has the following features:

1. MSP430F5438A, clocked at 20MHz
2. A 32KHz oscillator for low power operation
3. micro SD card socket
4. On-board reset switch
5. JTAG programming header (an adapter may be required to mate with the MSP-FET430UIF programmer)
6. All 100 processor pins available on debug headers
7. On-board power regulation and power socket
8. The board has 1 LED for power good and another connected to a pin on the processor for status indication.
9. Provision for an SPI daisy-chain of MSP430 boards using 2 SPI channels (upstream and downstream)

If required the board can be used as a standalone development platform be powered from the JTAG-FET
programmer.

Toolchains
----------

Code Composer Studio
--------------------
This has been used extensively during development of the code in conjunction with the MSP-FET430UIF
USB/JTAG programmer. Both have proved to be extremely reliable in use. There is a free version of the
tools which restrict the size of code they will generate to 16KB. The full version can be evaluated free for 90
days.

The current configuration of the code is too large to fit within the 16K limit, however by removing some
features, e.g. Fat file system support then the free version may be sufficient.

A very useful feature of the IDE is the ability to use standard printf type functions and have the output
displayed in a console window within the IDE. In order for this to work the amount of memory set aside for
the stack and heap must be increased and the “cio” functionality must be enabled in the project build
configuration.

A small amount of source code in the platform common layer was taken from Plastic Logic’s equivalent
Linux drivers. The code uses anonymous unions extensively and in order to get the code to compile it was
necessary to add a compiler flag to tell it to behave more like gcc.

The shipped project configuration file has these settings changes made.

msp430-gcc
----------
There is an open source msp430 tool chain available for Linux – msp430-gcc. Some work has been done to
support this tool chain but the work is not yet complete. Much of the code compiles cleanly however there
are some issues related to pragmas used to declare interrupt handlers. Full support for this tool chain will
depend on customer demand.

MSP430 Specific Host Interfaces
===============================
GPIO Interface
--------------
This is the reference implementation for the GPIO host interface and can be found in msp430-gpio.c. It
supports the configuration of all features on all pins that can be configured. It is only possible to configure
one pin at a time in a port. It is not possible to define the configuration of multiple pins in a port with one
call – e.g. when defining an 8 bit bus as output or input. The code attempts to verify the request as much as
it can. Much of the error checking code can be disabled once the porting process to a new platform has
been completed and the platform configuration is stable.

I2C Interface
-------------
A single i2c interface is supported. I2C is only supported in UCSB modules and the chosen UCSB module is
defined in the msp430-i2c.c source file by setting the macros “USCI_UNIT” and “USCI_CHAN” as required.
The code will then reconfigure itself to reference the correct I2C unit. In addition to specifying which UCSI
module to use the I2C SDA and SCL pins need to be connected to the USCI unit by defining the appropriate
pins as PIN_SPECIAL in the gpio_request() call.

SPI Interface – Epson
---------------------
SPI is supported in both USCI_A and USCI_B modules and the chosen USCI module is defined in the
msp430-spi.c source file by setting the macros “USCI_UNIT” and “USCI_CHAN” as required. The code will
then reconfigure itself to reference the correct SPI unit. In addition to specifying which USCI module to use
the SPI_CLK, SPI_MOSI and SPI_MISO pins need to be connected to the USCI unit by defining the
appropriate pins as PIN_SPECIAL in the gpio_request() call. Note that it is possible to use both the USCI_A
and USCI_B units. i.e. USCI_A0 and USCI_B0 are physically different hardware units.

A single SPI interface is supported for Epson controller communications. Multiple controllers can be
connected to this bus and are selected using their chip select lines as required. This interface runs at
20Mbps reliably. Due to the need to keep the Epson chip selected for the duration of the image data
transfer the Epson controller must be placed on a separate bus to the SD card so that multiple blocks can
be read from the SD card.

SPI Interface – SD Card
-----------------------
SPI is supported in both USCI_A and USCI_B modules and the chosen USCI module is defined in the
msp430-sdcard.c source file by setting the macros “USCI_UNIT” and “USCI_CHAN” as required. The code
will then reconfigure itself to reference the correct SPI unit. In addition to specifying which USCI module to
use the SPI_CLK, SPI_MOSI and SPI_MISO pins need to be connected to the USCI unit by defining the
appropriate pins as PIN_SPECIAL in the gpio_request() call. Note that it is possible to use both the USCI_A
and USCI_B units. i.e. USCI_A0 and USCI_B0 are physically different hardware units.

A single SPI interface is supported for transferring data from the micro SD card slot. This interface runs at
20Mbps reliably.

UART Interface
--------------
A serial interface is supported using a pin header on the MSP430 board into which can be plugged an FTDI
active serial –to-USB cable. The code can be configured to route all standard output to the serial port rather
than back to the debugger. This allows debug output still be seen when no debugger is attached.

Porting the Existing Code to a New MSP430 Processor
---------------------------------------------------
Porting the existing code to a design which requires a different pin out is relatively straightforward. The
necessary configuration information is not centrally located and is kept close to the code it affects.

Define a new file that replaces the board setup file plat-ruddock2.c. This should define any required setup
for your platform but not the pins required by the reference code.

To reconfigure the reference code follow the sequence below:

1. Determine which USCI units will be used in the new configuration. Ensure the unit is suitable for its intended purpose.
2. Determine which pins are associated with the chosen USCI units.
3. Determine which pins will be used for the Epson SPI signals HRDY, HDC, and RESET
4. Determine which pin(s) will be used for the Epson SPI chip select
5. Determine which pins may be necessary to control the power supplies
6. In each of the msp430-spi.c, msp430-sdcard.c, msp430-i2c.c and msp430-uart.c

    a. Define USCI_UNIT and USCI_CHAN as required
    b. Modify the definitions for the pins so they match the chosen UCSI unit.
    c. E.g.:

.. code-block:: usci

    #define USCI_UNIT B
    #define USCI_CHAN 0
    // Pins from MSP430 connected to the SD Card
    #define SD_CS GPIO(5,5)
    #define SD_SIMO GPIO(3,1)
    #define SD_SOMI GPIO(3,2)
    #define SD_CLK GPIO(3,3)

7. In epson-if.c define the EPSON SPI interface signals E.g.:

.. code-block:: spi

    // Remaining Epson interface pins
    #define EPSON_HDC GPIO(1,3)
    #define EPSON_HRDY GPIO(2,7)
    #define EPSON_RESET GPIO(5,0)

8. In the platform implementation file, e.g. plat-hbz6.c, define the power control and Epson chip select pins. E.g.:

.. code-block:: plat

    #define B_HWSW_CTRL GPIO(1,2)
    #define B_POK GPIO(1,0)
    #define B_PMIC_EN GPIO(1,1)
    #define EPSON_CS_0 GPIO(3,6)
	
Recompile the code and it has now been retargeted to the new pin assignments.

Microchip - PIC
===============

The Microchip PIC is probably the most numerous of microcontrollers. It is available in a wide range of
configurations with numerous peripherals. The Harvard architecture nature of many of the devices is visible
in C source code (data location in ROM or RAM matters) and will require some effort to support.
No port to this platform has been completed yet.

Many of the 8bit PIC microcontrollers have the necessary resources to support the code base.

.. raw:: pdf

    PageBreak
	
ARM
===

There are numerous ARM based microcontrollers available in the market. The wide availability of ARM
expertise may make this a suitable target platform.

.. raw:: pdf

    PageBreak
	
Relevant Data Sheets and Additional Resources
=============================================

TI 65185 PMIC
Maxim 17135 PMIC
Maxim LM75 Temperature sensor
Maxim 5820 DAC
Microchip 24LC014
Microchip 24AA256
Epson S1D13524
Epson S1D13421
MSP430 User Guide

Detailed schematics and design documents are available for all of the boards supported by this software.

An overview of the electronics required to drive a small displays is available in the document “Electronics
for small displays” available from Plastic Logic.
