Getting Started
===============
This section covers setting up the hardware and software so that a given display type can be driven. Please follow the steps outlined in order to setup and build the software.


Hardware Setup
--------------
The software requires a processor board, Ruddock2 (optional if using the Parrot processor board), a display interface board and a display to match the interface board.

If using the Parrot board and the optional Ruddock2 board, ensure that:

1. The P5 â€œI2C isolateâ€� 2-pin header on the Ruddock2, has no link fitted
2. The P4 2-pin header on the Ruddock2, has a link fitted
3. The switch SW7 on the Ruddock2 is set to ON
4. 5V power supply, 200mA for small displays, 2A for large displays, is used
   and the P16 3-pin header on the Parrot has a link fitted on position "JACK"
   (1-2)
	

The processor board plugs into the Ruddock2 using the two parallel headers, note the processor board
outline in the silk screen on the Ruddock2 for correct orientation.

The display interface board connects to the Ruddock2 serial interface connector (the smaller of the two FFC
connectors) using a flexi-cable. If using a Parrot board, the display interface board can be connected directly to the Parrot using a flexi-cable. Finally the display itself connects to the display interface board either directly in the case of the small displays or via a Mercury board using a 50-way flexi-cable.


.. _Code Composer Studio Setup:

Code Composer Studio Setup
--------------------------

The Plastic Logic reference code project uses Texas Instruments' Code Composer Studio IDE v5.5. This section details the steps necessary to set up Code Composer Studio to build and debug the code.

1. Get the source code, either by downloading an archive or cloning the
   repository::

    git clone https://github.com/plasticlogic/pl-mcu-epd.git

   .. important::

    This project uses **DOS line endings**.  The ``.gitattributes`` file tells
    Git to automatically convert text file line endings to DOS at commit time.
    To avoid complications on Unix-like operating systems (Linux, MacOSX...)
    please configure your text editor to use DOS line endings.


2. Install `Code Composer Studio <http://processors.wiki.ti.com/index.php/Download_CCS>`_ from TI on MS Windows or GNU/Linux
3. Create new CCS project

   - Open the menu **File -> New -> CCS Project**
   - Project name: **pl-mcu-epd** (for example)
   - Project location: Ensure the location is different to that of the cloned repository
   - Output: **Executable**
   - Device family: **MSP430**
   - Variant: **MSP430F5438A** (for PL Parrot v1.x boards)
   - Connection: **TI MSP430 USB1** (uses an MSP-FET430UIF USB-JTAG interface)
   - Project template: **Empty Project**

4. Import the source code

   - Open the menu **File -> Import -> General**
   - To import a clone repository, choose **File System**
   - To import a source archive, choose **Archive File**

     .. note::
      When using a source archive, the source will need to be moved out
      of the extra sub-directory that is created.

   - Browse to select your pl-mcu-epd cloned repository or archive file
   - Select all the files (checkbox near pl-mcu-epd)
   - Click on **Finish**

5. Compiler configuration

   - Open the menu **Project -> Properties**
   - Click on the **Show Advanced Settings** link in the bottom left of the properties window
   - The following project settings need to be modified:

     - **CCS Build -> MSP430 Compiler -> Debug Options**
        **Debugging Model:** Should be set to **'Full Symbolic Debug'**

     - **CCS Build -> MSP430 Compiler -> Advanced Options -> Language Options:** 
        Check the box for **Enable Support for GCC Extensions**

     - **CCS Build -> MSP430 Compiler -> Advanced Options -> Library Function Assumptions:** 
        Set the level of printf support to **nofloat**

     - **CCS Build -> MSP430 Compiler -> ULP Advisor:**
        Disable the following items by unchecking the associated box:

         **2: Software (SW) delay**

         **5: Processing/power intensive operations**

     - **CCS Build -> MSP430 Compiler -> Advanced Options -> Diagnostic Options:** 
        Check the box for **Emit Diagnostic Identifier Numbers**

     - **CCS Build -> MSP430 Linker -> Basic Options**

        **Set C system stack size: 160**

        **Heap size for C/C++ dynamic memory allocation: 1000**

     - **C/C++ General -> Paths and Symbols**
        **Includes:** Add the following paths to the includes list (check all three boxes in the creation dialog window)

         /${ProjName}/msp430 

         /${ProjName}

        **Includes:** Re-order the includes list so that the order is as below:

         /${ProjName}/msp430 

         ${CCS_BASE_ROOT}/msp430/include 

         /${ProjName} 

         ${CG_TOOL_ROOT}/include 


6. Setup ``config.txt``

   Finally, a ``config.txt`` file must be placed on the SD-Card. A sample config file is supplied for each
   supported display type. 

   More information on the various code configuration options can be found in the section `Configuring the Code`_.



Configuring the Code
--------------------

The code includes a number of features and demonstrations that can be configured at run time via the use of settings in the ``config.txt`` file.

Configuration of the display interface board type and display type
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following example defines a Raven board with D107_T2.1 display:

.. code-block::

	# Set one of the following to 1 to manually select the platform.
	# This will be used if no platform can be discovered at runtime.
	#  CONFIG_PLAT_RAVEN             < Raven board
	#  CONFIG_PLAT_Z6                < Hummingbird Z6.x board
	#  CONFIG_PLAT_Z7                < Hummingbird Z7.x board
	board CONFIG_PLAT_RAVEN
	# Set this to manually specify the display type when it could not be detected
	# at run-time.  This is especially useful for displays without an EEPROM such
	# as S049_T1.1.  */
	display_type            D107_T2.1

Configuration of how display-specific data is used
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

All Plastic Logic displays require display-specific information such as waveform data and VCOM
voltage. Some displays contain an EEPROM that can be used to store this information; alternatively
the information can be provided on the SD card. The following settings define where the information
will be read from:

.. code-block::

	# Each display has a type and some associated data such as a VCOM voltage and
	# waveform library.  This can either be stored in the display EEPROM or on the
	# SD card.  The display type may also be manually specified with
	# CONFIG_DISPLAY_TYPE.
	#
	# Set data_source to one of the following values in order to choose where the data
	# should be read from:
	#	CONFIG_DISP_DATA_EEPROM_ONLY,  < Only use display EEPROM
	#	CONFIG_DISP_DATA_SD_ONLY,      < Only use SD card
	#	CONFIG_DISP_DATA_EEPROM_SD,    < Try EEPROM first, then SD card
	#	CONFIG_DISP_DATA_SD_EEPROM     < Try SD card first, then EEPROM
	
	data_source CONFIG_DISP_DATA_EEPROM_SD


Configuration of I2C master
^^^^^^^^^^^^^^^^^^^^^^^^^^^

A number of components are configured and accessed via I2C. The following setting defines the
device used as the I2C master:

.. code-block::

	# Default I2C master mode used with CONFIG_HWINFO_DEFAULT
	#	I2C_MODE_NONE,      /* invalid mode */
	#	I2C_MODE_HOST,      /* use the host */
	#	I2C_MODE_DISP,      /* use SPI-I2C bridge on the display (S1D13541) */
	#	I2C_MODE_S1D13524,  /* use SPI-I2C bridge on the S1D13524 */
	#	I2C_MODE_SC18IS6XX, /* not currently supported */

	i2c_mode I2C_MODE_HOST

The code also includes a number of features and demonstrations that can be configured at compile time via the use of preprocessor directives in the ``config.h`` file.


Configuration of how hardware information is used
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Plastic Logic display interface boards (Raven, Hummingbird Z6/Z7) contain an EEPROM that can
be used to store board-specific calibration data and other relevant information. The following
settings define whether or not the code will use this information and whether or not to use a
default if the information is not available:

.. code-block:: c

    /** Set to 1 to use the VCOM and hardware info stored in board EEPROM */
    #define CONFIG_HWINFO_EEPROM          1

    /** Set to 1 to use default VCOM calibration settings if HW info EEPROM data
     * cannot be used (either not programmed, or hardware fault, or
     * CONFIG_HWINFO_EEPROM is not defined).  If set to 0, the system will not be
     * able to work without valid EEPROM data.  */
    #define CONFIG_HWINFO_DEFAULT         1



Configuration of serial interface
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A serial interface is supported via the USB port (the Parrot board is
fitted with a TUSB3410 USB to serial port controller). Alternatively a
FTDI active serial-to-USB cable can be plugged into a pin header on the
Parrot board. The code can be configured to route all standard output to
the serial port rather than back to the debugger. This allows debug output
still to be seen when no debugger is attached. The following setting defines
whether ``stdout`` and ``stderr`` are sent to the serial port or the debugger:

.. code-block:: c

    /** Set to 1 to have stdout, stderr sent to serial port */
    #define CONFIG_UART_PRINTF         1

Power mode demonstration
^^^^^^^^^^^^^^^^^^^^^^^^

The following setting can be used to configure a demonstration of power state transitions:

.. code-block:: c

    /** Set to 1 to use the power state transition demo rather than the slideshow */
    #define CONFIG_DEMO_POWERMODES        1

Pattern demonstration
^^^^^^^^^^^^^^^^^^^^^

The following settings can be used to display a checker-board pattern of the specified size:

.. code-block:: c

    /** Set to 1 to use the pattern demo rather than the slideshow */
    #define CONFIG_DEMO_PATTERN           1  /** Not intended for S049_T1.1 displays  */
    #define CONFIG_DEMO_PATTERN_SIZE      32 /** Size of checker-board */


SD Card Setup
-------------
The micro SD card for the processor board must be formatted as a FAT/FAT16 file-system (not FAT32).
The SD card contents (initialisation data and images) can be retrieved from the Plastic Logic GitHub repository (https://github.com/plasticlogic/pl-mcu-sd-card.git). Unzip this archive and place the resulting files on the SD card so that the root directory of the file-system contains the folders ``D107_T2.1``, ``S040_T1.1``, etc.

The supplied content provides a safe set of configuration data for each type of display. In order to obtain the best image quality the waveform binary file, ``waveform.bin`` (for S1D13541) or ``waveform.wbf`` (for S1D13524), and the text file ``vcom``, containing the VCOM voltage in mV, must be replaced with data specific to the display used. These files are located at:

 ``0:/<Display-Type>/display/waveform.bin``

 ``0:/<Display-Type>/display/waveform.wbf``

 ``0:/<Display-Type>/display/vcom``

Place the micro SD card in the micro SD card socket on the processor board.


Running the Code
---------------------

Once the code has been configured and built in Code Composer Studio, the resulting binary can be transferred to the Parrot board using the MSP-FET430UIF USB-JTAG programmer. Depending on the configuration, you should now be able to see one of the following:

- A slideshow of stock images from the ``0:/<Display-Type>/img`` folder being shown on the display until execution is halted (with or without power sequencing). The slideshow will skip any files that do not have the extension ".pgm"
- A sequence of images defined by the ``0:/<Display-Type>/img/slides.txt`` file
- A checkerboard image


Error codes
^^^^^^^^^^^

If a fatal error occurs while running the code, the type of error is indicated via the status
LED. Specifically the status LED will be flashed on/off a number of times, followed by a delay,
after which the pattern will repeat. The error types are as follows (see also ``assert.h``):


+----------+----------------------------------------------+
| Flashes  | Description                                  |
+==========+==============================================+
| 1        | General error initialising GPIO              |
+----------+----------------------------------------------+
| 2        | Error initialising MSP430 comms              |
+----------+----------------------------------------------+
| 3        | Error reading HWINFO EEPROM.                 |
|          | Could be a comms error or a content error    |
+----------+----------------------------------------------+
| 4        | Error initialising I2C (Epson)               |
+----------+----------------------------------------------+
| 5        | Error reading display information. Could     |
|          | be many errors (comms error, content error,  |
|          | missing or invalid file, etc).               |
|          | Also depends on preprocessor settings        |
+----------+----------------------------------------------+
| 6        | Error initialising HVPSU. Most likely to be  |
|          | a comms error, but could indicate a failed   |
|          | PMIC                                         |
+----------+----------------------------------------------+
| 7        | Error initialising EPDC. Could be many       |
|          | errors (comms error, EPDC failure, failed    |
|          | to load init code, failed on one of several  |
|          | commands needed to initialise the EPDC,      |
|          | failed to load waveform, etc)                |
+----------+----------------------------------------------+
| 8        | Failed while running application. Multiple   |
|          | causes for this, depending on application    |
|          | that is running. Most likely failures are    |
|          | due to missing/invalid files or hardware     |
|          | problems such as POK or comms failure        |
+----------+----------------------------------------------+
| 9        | Failed assert statement (debug use only)     |
+----------+----------------------------------------------+
| 10       | Failed to read the config file               |
+----------+----------------------------------------------+
| 0 (off)  | Undefined error                              |
+----------+----------------------------------------------+
| 0 (on)   | No error                                     |
+----------+----------------------------------------------+

Additional information relating to the error can be obtained by inspecting ``stderr`` via the
debugger or the serial port (depending on how ``CONFIG_UART_PRINTF`` has been defined).


Toolchains
----------

Code Composer Studio
^^^^^^^^^^^^^^^^^^^^
Code Composer Studio has been used extensively during development of the code in conjunction with the MSP-FET430UIF
USB/JTAG programmer. Both have proved to be extremely reliable in use. There is a free version of the
tools which restrict the size of code they will generate to 16KB. The full version can be evaluated free for 90
days.

The current configuration of the code is too large to fit within the 16K limit, however by removing some
features, e.g. Fat file system support then the free version may be sufficient.

A very useful feature of the IDE is the ability to use standard printf type functions and have the output
displayed in a console window within the IDE. In order for this to work the amount of memory set aside for
the stack and heap must be increased and the â€œcioâ€� functionality must be enabled in the project build
configuration.

A small amount of source code in the platform common layer was taken from Plastic Logicâ€™s equivalent
Linux drivers. The code uses anonymous unions extensively and in order to get the code to compile it was
necessary to add a compiler flag (``--gcc``) to tell it to behave more like gcc.


msp430-gcc
^^^^^^^^^^
There is an open source msp430 tool chain available â€“ msp430-gcc. Some work has been done to support this tool 
chain but the work is not yet complete. Much of the code compiles cleanly however there are some issues related 
to pragmas used to declare interrupt handlers. Full support for this tool chain will depend on customer demand.


.. raw:: pdf

   PageBreak
