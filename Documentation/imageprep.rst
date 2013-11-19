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

.. raw:: pdf 
   
   PageBreak
