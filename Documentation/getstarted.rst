Getting Started
===============
This section covers setting up the hardware and software so that a given display type can be driven. Please
follow the steps outlined in order to setup and build the software.


Obtaining the Code
------------------
The code can be retrieved from the Plastic Logic GitHub repository (https://github.com/plasticlogic/pl-mcu-epd.git).
The archive contains the source code, SD card contents (initialisation data and images), and documentation. 
Unzip this archive in some suitable location that the development tools will be able to access.


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
on the SD card so that the root directory of the file-system contains the folders Type4, Type11 etc.

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

.. raw:: pdf

   PageBreak
