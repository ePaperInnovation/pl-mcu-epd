Code Structure
==============

Overview
--------
The diagram below shows an overview of the code base.


.. image:: overview.jpg
   :width: 100%

Things to note are:

1. The application sits right on top of the common components. There is no layer that abstracts a complete display system that can be manipulated by calling methods on it. 
2. The Host abstraction layer allows for porting to different CPU’s, either members of the same family or different architectures. Interrupts and Timers are not mandatory for the sample code to work.
3. There is an “Access Abstraction Layer”. This exists because the Epson controllers contain a number of resources, e.g. I2C master, SPI master, and on chip GPIO’s that the Application layer may want to use. This abstraction layer allows the application to access either a host CPU resource or one contained in the Epson controller without needing to know its location once initialised. Currently only support for I2C is implemented.

.. raw:: pdf
 
   PageBreak

