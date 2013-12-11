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

e.g. (Linux example):
$: python img_convert.png image.png scramble

For the above example, the output will be a file called image_s.pgm. For a non-scrambled image, the
output will be image_n.pgm.

The output files should be copied to the SD Card in the img folder of the appropriate display type folder
e.g.:

0:/Type-16/img/image_n.pgm

Sequence file
-------------

An optional plain text file can be put in the image directory to tell the
software to run a specific sequence instead of simply going through all the
images found.  If the file is found, the software will use it; otherwise the
standard slideshow will be run.

This sequencer supports regional updates, image compositing by copying areas of
existing files and can fill rectangles with a uniform grey level.  Then the
waveform type and area coordinates are specified for each update, enabling more
advanced operation of the system.

The format of the file can be summarised with the following characteristics:

* each line corresponds to a single command
* each line is processed sequentially
* commands can not be nested and there is no flow control
* each line has a maximum length of 80 characters (fixed by buffer size in
  software)
* only ASCII characters can be used in the file
* line endings can be either DOS or Unix like
* each line is composed of values separated by commas and any number of spaces
* not all commands take the same number of arguments, so lines can have varying
  numbers of values
* a line starting with a hash sign ``#`` is a comment and is ignored by the
  software
* blank lines are allowed
* there is no limit to the length of the file other than what the file system
  infrastructure permits
* the file needs to be named ``slides.txt`` and located in the same directory
  as the image files

The software will not keep more than one line in memory at a time, and it will
automatically jump back to the beginning of the file when it has processed the
last line to keep playing the sequence continuously.

It is worth noting that copying areas of image files into the EPD frame buffer
can take a significant amount of time compared to the duration of a display
update.  Drawing operations are separated from display updates, which take
little time to start, so it is still possible to achieve some basic animation
effects with appropriate sequencing of the drawing and display update commands.

*Supported commands*:

``update, WAVEFORM, LEFT, TOP, WIDTH, HEIGHT, DELAY``
  Update the display with the given ``WAVEFORM`` (see `Waveform identifiers`_)
  in the area starting with the (``LEFT``, ``TOP``) pixel coordinates and the
  given ``WIDTH`` and ``HEIGHT``.  The software will wait until the update
  request has been processed by the controller, and then wait for ``DELAY``
  milliseconds.
``power, ON_OFF``
  Turn the display power either on or off based on the value of ``ON_OFF``,
  which can be either ``on`` or ``off``.  When turning the power off, the
  software will wait for any on-going update to complete.
``fill, LEFT, TOP, WIDTH, HEIGHT, GREY_LEVEL``
  Fill a rectangle starting with the (``LEFT``, ``TOP``) pixel coordinates and
  the given ``WIDTH`` and ``HEIGHT`` with the givel ``GREY_LEVEL`` which is a
  number between 0 and 15 - 0 being black and 15 white.
``image, FILE, LEFT_IN, TOP_IN, LEFT_OUT, TOP_OUT, WIDTH, HEIGHT``
  Copy an area from an image file ``FILE`` starting to read from (``LEFT_IN``,
  ``TOP_IN``) pixel coordinates into the EPD buffer at (``LEFT_OUT``,
  ``TOP_OUT``) pixel coordinates with the given ``WIDTH`` and ``HEIGHT``.
``sleep, DURATION``
  Sleep for the given ``DURATION`` in milliseconds.

*Example with Type18 400x240 display*::

  # Fill the screen with white and trigger a refresh update
  #
  #                              x,    y,    w,    h, gl
  fill,                          0,    0,  400,  240, 15
  power,  on
  update, refresh,               0,    0,  400,  240, 50
  power,  off

  # Load some image data in 4 different areas
  #
  #      file,     i_x,  i_y,  o_x,  o_y,  wid,  hgt
  image, 01_N.PGM, 290,   65,  290,   20,  100,  120
  image, 06_N.PGM, 150,   50,   10,   10,  140,  180
  image, 11_N.PGM, 150,    0,  155,    0,  130,   90
  image, 13_N.PGM,  20,   20,  150,  150,  240,   80

  # Update the same 4 areas with a small delay in between each
  #
  #       waveform,           left,  top,  wid,  hgt, delay
  power,  on
  update, refresh,             290,   20,  100,  120, 50
  update, refresh,              10,   10,  140,  180, 50
  update, refresh,             155,    0,  130,   90, 50
  update, refresh,             150,  150,  240,   80, 50
  power,  off

.. _Waveform identifiers:

Waveform identifiers
^^^^^^^^^^^^^^^^^^^^^

The following waveforms are always available in Plastic Logic's waveform
libraries:

+------------------+--------+--------------------------------------+----------+
| Path             | Grey \ | Description                          | Length \ |
|                  | levels |                                      | (ms)*    |
+==================+========+======================================+==========+
| ``refresh``      | 16     | All pixels are updated.              | 670      |
+------------------+--------+--------------------------------------+----------+
| ``delta``        | 16     | Only changing pixels are updated.    | 670      |
+------------------+--------+--------------------------------------+----------+
| ``refresh/mono`` | 2      | All b&w pixels are updated.          | 250      |
+------------------+--------+--------------------------------------+----------+
| ``delta/mono``   | 2      | Only changing b&w pixels are         | 250      |
|                  |        | updated.                             |          |
+------------------+--------+--------------------------------------+----------+
| ``init``         | 2      | Use only to wipe the screen when     | 1300     |
|                  |        | the image content is lost.           |          |
+------------------+--------+--------------------------------------+----------+

* At typical room temperature. For full specification see the relevent display datasheet.

They all have a unique numerical identifier which can be different in each
waveform library.  To get the identifier of a waveform for a given path string,
use the ``s1d135xx_get_wfid`` function in your application.

.. raw:: pdf

   PageBreak
