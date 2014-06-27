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


.. raw:: pdf

   PageBreak


Scope
=====

This document does not attempt to describe the detailed operation of any particular microcontroller or
Epson display controller as this information is readily available, or may require an NDA to disclose. Prior
experience with embedded programming is expected and discussion will focus on the specifics of this code
base.
The code is able to drive a slideshow of pre-rendered images in the PGM file format to a chosen
display.  Due to restrictions on resources the code is not currently able to manipulate the images in any
way.
The code focusses on functionality and does not pretend to implement best practice for any specific
microcontroller. Data transfer speed improvements are planned for subsequent
releases.
The code attempts to strike a balance between minimising microcontroller resource usage while preserving
portability, good coding practices and the provision of good debug support (e.g. use of assertions).


.. raw:: pdf

   PageBreak


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

 
.. raw:: pdf

   PageBreak
