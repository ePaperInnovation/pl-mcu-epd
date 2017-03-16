Preparing Your Own Images
=========================

Image Format
------------
For simplicity the code only supports image files in the PGM graphics file format.
For details see: `http://en.wikipedia.org/wiki/Netpbm_format <http://en.wikipedia.org/wiki/Netpbm_format>`_ .

.. note::

 There are two different PGM formats - ASCII (magic number: P2) and binary/raw (magic number: P5). The code only supports the binary/raw format.

This is a simple, uncompressed, file format that can be generated with `GIMP <http://www.gimp.org/>`_ or using the `Python Imaging Library (PIL) <http://www.pythonware.com/products/pil/>`_. Both GIMP and PIL are available for Windows and Linux.

When displaying images as a slideshow (i.e. without a ``slides.txt``), the image files are expected to match the full display size so that the contents can be transferred directly from storage to the display controller. In the case of the S049_T1.1 bracelet displays the pixel data must also be reordered (see `Image Conversion Tools`_ below).

When displaying images using a ``slides.txt`` sequence file the images can be any size up to the full display size, but must have a width that is exactly divisible by 2. 

The SD card content provided contains the original source PNG images which were used to create the PGM
files.


Image Conversion Tools
----------------------
The source code contains a python script, ``tools/prepare_pgm.py``, which uses the Python Imaging Library (PIL) to
support the translation of PNG files to PGM format. The script can also reorder the pixel data as required
for S049_T1.1 displays.

The script requires Python version 2.7.5 and a compatible version of the Python Imaging Library (PIL), and
works in both Linux and Windows.

Python 2.7.5 can be downloaded from the following URL:

 `https://www.python.org/downloads/ <https://www.python.org/downloads/>`_

For Windows, use the 32-bit or 64-bit 2.7.5 installer as appropriate for the host machine.
For Linux, use either of the source tarballs for 2.7.5. Installation instructions can be found within the tarball.

The Python Imaging Library can be found here:

 `http://pythonware.com/products/pil <http://pythonware.com/products/pil>`_

For Windows, use the most recent "Windows only" PIL for Python 2.7. When installing PIL, ensure the
destination directory is the same as the Python 2.7.5 installation directory.
For Linux, use the most recent PIL source kit. Again, installation instructions can be found within the tarball.

Execute the script from the command line in either operating system, passing the image to be converted as the first argument. If the target device is using a S049_T1.1 display, pass ``--interleave`` as the second argument in order to generate the correct pixel data ordering.

For example::

 python prepare_pgm.py image.png --interleave

For the above example, the output will be a file called ``image.pgm.``

The output files should be copied to the SD Card in the img folder of the appropriate display type folder, 
e.g.:

 ``0:/S049_T1.1/img/image.pgm``

Sequence File
-------------

An optional plain text file named ``slides.txt`` can be put in the image
directory ``0:/<Display-Type/img>`` to tell the software to run a specific
sequence instead of simply going through all the images found.  If the file
is found, the software will use it; otherwise the standard slideshow will be
run.

This sequence supports regional updates, image compositing by copying areas of
existing files and can fill rectangles with a uniform grey level.  Then the
waveform type and area coordinates are specified for each update, enabling more
advanced operation of the system.

File format
^^^^^^^^^^^^

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

The software will not keep more than one line in memory at a time, and it will
automatically jump back to the beginning of the file when it has processed the
last line to keep playing the sequence continuously.

It is worth noting that copying areas of image files into the EPD frame buffer
can take a significant amount of time compared to the duration of a display
update.  Drawing operations are separated from display updates, which take
little time to start, so it is still possible to achieve some basic animation
effects with appropriate sequencing of the drawing and display update commands.

Supported commands
^^^^^^^^^^^^^^^^^^^

``update, WAVEFORM, LEFT, TOP, WIDTH, HEIGHT, DELAY``
  Update the display with the given ``WAVEFORM`` (see `Waveform identifiers`_)
  in the area starting with the (``LEFT``, ``TOP``) pixel coordinates and the
  given ``WIDTH`` and ``HEIGHT``.  The software will wait until the update
  request has been processed by the controller, and then wait for ``DELAY``
  milliseconds. 

  .. note:: 

   ``WIDTH`` must be exactly divisible by 2 and the specified rectangle must
   not exceed the bounds of the display.

``power, ON_OFF``
  Turn the display power either on or off based on the value of ``ON_OFF``,
  which can be either ``on`` or ``off``.  When turning the power off, the
  software will wait for any on-going update to complete.

``fill, LEFT, TOP, WIDTH, HEIGHT, GREY_LEVEL``
  Fill a rectangle starting with the (``LEFT``, ``TOP``) pixel coordinates and
  the given ``WIDTH`` and ``HEIGHT`` with the given ``GREY_LEVEL`` which is a
  number between 0 and 15 - 0 being black and 15 white.

  .. note::

   ``WIDTH`` must be exactly divisible by 4 and the specified rectangle must
   not exceed the bounds of the display.

``image, FILE, LEFT_IN, TOP_IN, LEFT_OUT, TOP_OUT, WIDTH, HEIGHT``
  Copy an area from an image file ``FILE`` starting to read from (``LEFT_IN``,
  ``TOP_IN``) pixel coordinates into the EPD buffer at (``LEFT_OUT``,
  ``TOP_OUT``) pixel coordinates with the given ``WIDTH`` and ``HEIGHT``.

  .. note::

   ``WIDTH`` must be exactly divisible by 2 and the specified rectangles
   must not exceed the bounds of the image file or the display.

``sleep, DURATION``
  Sleep for the given ``DURATION`` in milliseconds.

Example sequence
^^^^^^^^^^^^^^^^

The following listing shows a sample sequence for S040_T1.1 400x240 displays::

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
|                  | levels |                                      | (ms) *   |
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

\* At typical room temperature. For full specification see the relevant display datasheet.

They all have a unique numerical identifier which can be different in each
waveform library.  To get the identifier of a waveform for a given path string,
use the function ``pl_epdc_get_wfid()`` (``pl/epdc.h``, ``pl/epdc.c``) in your application.

.. raw:: pdf

   PageBreak
