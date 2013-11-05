Plastic Logic EPD on MSP430
===========================

About
-----

This repository contains the source code to run on a TI MSP430 micro-controller
as a reference to use Plastic Logic e-paper displays and related hardware based
on Epson EPD controllers S1D13541 and S1D13524.

Development environment
-----------------------

#. Get the source code, either by downloading an archive or cloning the
   repository::

     git clone https://github.com/plasticlogic/pl-mcu-epd.git

#. Install `Code Composer Studio
   <http://processors.wiki.ti.com/index.php/Download_CCS>`_ from TI on MS
   Windows or GNU/Linux

#. Create new CCS project:

   * Open this menu: **Project -> New CCS Project**
   * Project name: pl-mcu-epd (for example)
   * Output type: Executable
   * Family: **MSP430**
   * Variant: **MSP430F5438A** (for PL Parrot v1.x boards)
   * Connection: TI MSP430 USB1 (uses a MSP-FET430UIF USB-JTAG interface)
   * Project template: **Empty Project**
   * Click on Finish

#. Import source code

   * Open this menu: **File -> Import... -> General**
   * To import a cloned repository, choose **File System**
   * To import a source archive, choose **Archive File**
   * Browse to select your ``pl-mcu-epd`` cloned repository or archive file
   * Select all the files (checkbox near pl-mcu-epd)
   * Click on Finish

#. Compiler configuration

   Some compiler options are stored by CCS in the ``.cproject`` file, which is
   not kept under source control as it is automatically managed by CCS.  To
   start with the correct set of compiler options, first close the project in
   CCS then copy the ``dot-cproject`` file contained in this repository on top
   of ``.cproject``.  Open the project again and run ``Clean`` to avoid any
   false compiler errors.  This initial file was created with CCS v5.1.0; it
   will be automatically updated by more recent versions of CCS.

Going further
-------------

You have now configured the project and can compile, run and debug the code on
a Plastic Logic Parrot v1.x platform.  Please contact Plastic Logic for the
complete manual, Parrot boards and e-paper displays.

.. important::

   This project uses **DOS line endings**.  The ``.gitattributes`` file tells
   Git to automatically convert text file line endings to DOS at commit time.
   To avoid complications on Unix-like operating systems (Linux, MacOSX...)
   please configure your text editor to use DOS line endings.
