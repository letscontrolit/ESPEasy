Project Structure
*****************

The files and directories in the ESPEasy file repository are organized in folders.
Some of the choices for folder structure may not look very logical at first sight and may need some explanation to understand.


ESPEasy Project Directories
===========================

Below a list of the most important directories and files used in this project.

* ``.pio/`` Working dir for PlatformIO (Ignored by Git)
* ``dist/`` Files also includd in the nightly builds (e.g. blank flash files and ESPEasy flasher)
* ``docs/`` Documentation tree using Sphinx to build the documents.
* ``hooks/`` Used for the continous integration process, used by Travis CI.
* ``include/`` PlatformIO's suggested folder for .h files (not yet used)
* ``lib/`` Libraries used in this project. Some have patches which make them different from the maintainers version.
* ``misc/`` Additional files needed for some use cases, like specialized a firmware for some boards or decoder files for the TTN project.
* ``patches/`` Patches we apply during build for core libraries.
* ``src/`` The source code of ESPeasy
* ``static/`` Static files we use in ESPEasy, like CSS/JS/ICO files.
* ``test/`` Test scripts to be used in the continous integration process.
* ``tools/`` Tools to help build ESPeasy.
* ``venv/`` Directory to store the used Python Virtual Environment. (Ignored by Git)
* ``platformio.ini``  Configuration file for PlatformIO to define various build setups.
* ``uncrustify.cfg``  Configuration file for Uncrustify, to format source code using some uniform formatting rules.
* ``pre_custom_esp32.py`` Python helper script for building custom ESP32 binary
* ``pre_extra_script.py`` Python helper script for building custom ESP8266 binary
* ``requirements.txt``  List of used Python libraries and their version (result of ``pip freeze`` with Virtual env active)
* ``esp32_partition_app1810k_spiffs316k.csv`` Used partition layout in ESP32 builds.
* ``crc2.py``  Python script used during nightly build to include the environment name in the build and compute checksums.


ESPEasy src dir
===============

In order to compile the project using PlatformIO, you need to make sure the ``platformio.ini`` file is in the 
root of the project dir opened in the editor.

The ``src/`` directory has the source files of the project.

In this directory, there is another ``src/`` dir, which is needed by ArduinoIDE in order to find files in sub-directories.
Arduino IDE also demands include directives to be relative compared to the file they are used in. 
For example: ``#include "../DataStructs/Settings.h"``

We are currently working on converting the core ESPeasy code from .ino files into .h/.cpp files. (see PR #2617 and issue #2621)

The header and source files are organized in a few directories in ``src/src/``.

* ``Commands`` Functions to process commands.
* ``ControllerQueue`` Mainly macro definitions for the queue system used by controllers.
* ``DataStructs`` Data structures used in ESPEasy. Note that some of them are stored in settings files, so take care when changing them.
* ``Globals`` Declaration of global variables. Declare them using ``extern`` in .h files and construct them in .cpp files or else you may end up with several instances of the same object with the same name.
* ``Static`` C++ encoded version of objects which have to be included in the binary. N.B. JS and CSS files are minified.

The Arduino based .ino files are still in the ``src/`` directory. (and some .h files, which are not yet moved)

In the end, there should be only .ino files left for the plugins and controllers and ``ESPEasy.ino`` 
All other .ino files used in the ESPEasy core should be converted to .h/.cpp.

This conversion is needed to have full control over ``#define`` statements used to determine what is included in a binary and what not.
This also allows for more flexible control over user defined builds.


