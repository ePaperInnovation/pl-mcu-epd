.. _Code Composer Studio Setup:

Code Composer Studio Setup
==========================

The Plastic Logic reference code project uses Texas Instrument's Code Composer Studio IDE. This section details the steps necessary to set up Code Composer Studio to build and debug the code.

1. Get the source code, either by downloading an archive or cloning the repository:

.. code-block:: gitclone

	git clone https://github.com/plasticlogic/pl-mcu-epd.git


2. Install `Code Composer Studio <http://processors.wiki.ti.com/index.php/Download_CCS>`_ from TI on MS Windows or GNU/Linux
3. Create new CCS project:

   - Open this menu **File -> New -> CCS Project**
   - Project name: **pl-mcu-epd** (for example)
   - Project location: Ensure the location is different to that of the cloned repository
   - Output: **Executable**
   - Device family: **MSP430**
   - Variant: **MSP430F5438A** (for PL Parrot v1.x boards)
   - Connection: **TI MSP430 USB1** (uses an MSP-FET430UIF USB-JTA interface)
   - Project template: **Empty Project**

4. Import the source code

   - Open this menu: **File -> Import -> General**
   - To import a clone repository, choose **File System**
   - To import a source archive, choose **Archive File**

     (Note: When using a source archive, the source will need to be moved out
     of the extra sub-directory that is created.)

   - Browse to select your pl-mcu-epd cloned repository or archive file
   - Select all the files (checkbox near pl-mcu-epd)
   - Click on **Finish**

5. Compiler configuration

   Firstly, click on the "Show Advanced Settings" link in the bottom left of the settings window.

   The following project settings need to be modified:

   * **CCS Build -> MSP430 Compiler -> Advanced Debugging Options**
        **Debugging Model:** Should be set to **'Full Symbolic Debug'**

   * **CCS Build -> MSP430 Compiler -> Advanced Options -> Language Options:** 
        Check the box for **Enable Support for GCC Extensions**

   * **CCS Build -> MSP430 Compiler -> Advanced Options -> Library Function Assumptions:** 
        Set the level of printf support to **nofloat**

   * **CCS Build -> MSP430 Compiler -> ULP Advisor:** 
       Disable numbers **2** and **5** in the list


   * **CCS Build -> MSP430 Linker -> Advanced Options -> Diagnostics:** 
      Check the box for **Emit Diagnostic Identifier Numbers**

   * **CCS Build -> MSP430 Linker -> Basic Options**
      **Stack Size:** Set to **160**
      **Heap Size:** Set to **1000**

   * **C/C++ General -> Paths and Symbols**
      **Includes:** Add the following paths to the includes list (check all three boxes in the creation dialog window)
         /${ProjName}/msp430 

         /${ProjName} 

      **Includes:** Re-order the includes list so that the order is as below:
         /${ProjName}/msp430 

         ${CCS_BASE_ROOT}/msp430/include 

         /${ProjName} 

         ${CG_TOOL_ROOT}/include 




   Finally, a ``config.h`` file must be created in the source location. Go to the cloned repository directory 
   and copy one of the sample config files (eg. ``config-Type18.h``) to ``config.h``. Go back to CCS, right click 
   on the project in the Project Explorer and select **Add Files**. Select the newly created config.h file. 
   When prompted, select the **copy file** radio button. The config file should now appear in the Project Explorer.

.. important::

   This project uses **DOS line endings**.  The ``.gitattributes`` file tells
   Git to automatically convert text file line endings to DOS at commit time.
   To avoid complications on Unix-like operating systems (Linux, MacOSX...)
   please configure your text editor to use DOS line endings.

