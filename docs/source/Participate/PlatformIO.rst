PlatformIO
**********

ESP easy can be built using the Arduino IDE or PlatformIO (PIO).
Arduino IDE is not being used during development, so it may take some more effort to get it setup for building ESPeasy.

We advice to use PlatformIO as build environment.

PlatformIO is just the build, test and upload environment for many micro controllers like the ESP8266 and ESP32 we use.

On top of that you need to use an editor, or so called IDE in which PlatformIO will be used.

The two main choices are:

* Atom
* Microsoft Visual Studio Code. (MS VS-Code)

Both are free to use and are available for Windows, MacOS and Linux.

Apart from these two, there are more available, like Eclipse and probably more.


PlatformIO Prerequisites
========================

PlatformIO does need at least the following:

* Python
* Git command line tools (`download <https://git-scm.com/downloads>`_)

For most operating systems, Python is already present, but for Windows you may need to install it.
Starting October 2019, Python 3.x is supported in all build tools we use for ESPEasy.

Please follow `these steps <https://docs.platformio.org/en/latest/faq.html#faq-install-python>`_ to 
install Pyton in Windows for PlatformIO.

**Do not forget to check "Add Python xxx to PATH".**

Windows ExecutionPolicy
-----------------------

For PlatformIO 4.1.x and newer in Windows, you may need to change the Windows ExecutionPolicy 
to be able to start a powershell script.
PlatformIO does use a PowerShell script to activate the Python virtual environment.

Default Windows security settings prevent execution of a PowerShell script.

Enter in the PowerShell terminal window in VScode::

    Set-ExecutionPolicy -ExecutionPolicy Unrestricted -Scope CurrentUser

Please note this does lower your security, so make sure you know its implications.
See `Microsoft - About Execution Policies <https:/go.microsoft.com/fwlink/?LinkID=135170>`_ for more details.


PlatformIO with Atom
====================



PlatformIO with VS-Code
=======================

Install
-------

For development of ESPeasy, a number of extensions has to be installed in VS-Code:

* PlatformIO IDE (by PlatformIO)
* C/C++ IntelliSense (by Microsoft)
* Arduino for Visual Studio Code (by Microsoft)
* Uncrustify (by Zachary Flower, originally by Laurent Tréguier)

Optional:

* Bookmarks (by Alessandro Fragnani)
* Bracket Pair Colorizer 2 (by CoenraadS)
* GitLens - Git supercharged (by Eric Amodio)
* Todo Tree (by Gruntfuggly)
* All Autocomplete (by Atishay Jain)
* Excel Viewer (by GrapeCity)
* esbonio - An extension for editing sphinx projects (by Swyddfa)


Uncrustify
----------

The extension Uncrustify is mainly to format the code using a standard code format definition.
This code format standard is defined in the file uncrustify.cfg in the main directory of this repository.
For new code contributions, it is highly appreciated if the code is formatted using this tool.

To do so:

* Right click mouse in editor
* "Format Document"

The first time (after installing uncrustify) it must be confirmed to use Uncrustify as formatter and using the default suggested config file.


Load a project using PlatformIO
-------------------------------

If you have PIO installed and the source tree cloned to your hard drive, then you can open the main dir of the repository.
The main directory of the repository is the level with platformio.ini in it.

Then in a few moments after opening the directory, on the left there will appear an alien logo, the logo of PlatformIO.
If you click that one, you will get a tree with lots and lots of project tasks and environments.

It is important  to note that PlatformIO does everything based on environments, which are defined in the platformio.ini file.
In the PlatformIO menu (on the left) everything is grouped per environment.

An environment entry has several tasks, like:

* Build
* Upload
* Monitor
* Upload and Monitor
* ... many more.

Some of these options only are available when you have registered with PlatformIO and some are only for paid subscriptions.
At least the basic ones used for almost any user are available with the free account.

The environment definitions all have at least the used micro controller in the name and the amount of flash memory used.

For example:

* ..._ESP8266_4M -> ESP8266 has external flash, which can vary in size from 512 kB to 16 MB.
* ..._ESP8285_1M -> ESP8285 has the flash internal, so is always 1 MB.
* ..._ESP32_4M316k -> ESP32 with 4 MB flash and a 1.8 MB partition for the sketch. (316k SPIFFS)

Make a custom build using PlatformIO
------------------------------------

The easiest is to go for the environment "env:custom_ESP8266_4M" and unfold that one.
Then select "Build" to see if it will start building.

If that's working, you can open the file "pre_extra_script.py" and add or remove the plugins and controllers you need.
That Python file is used in the "env:custom_ESP8266_4M" to define what should be embedded and what not.

For example to have only the controller "C014", you can remove "CONTROLLER_SET_ALL", and just add "USES_C014", 
The same for the plugins you need.

The file is built in the ".pio/build/...." directory right under the main repository directory (the one with the platformio.ini in it)

All builds will be made in a directory with the same name as the environment used.



Upload to ESP
=============



Linux
-----

For Linux, you may need to install 99-platformio-udev.rules to make PlatformIO upload tools work in vscode.