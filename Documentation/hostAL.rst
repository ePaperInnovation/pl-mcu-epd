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

