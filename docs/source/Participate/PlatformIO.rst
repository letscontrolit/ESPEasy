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
* reStructuredText - for .rst language support (by LeXtudio Inc.)
* reStructuredText Syntax highlighting (by Trond Snekvik)
* Markdown All in One (by Yu Zhang)


Uncrustify
----------

The extension Uncrustify is mainly to format the code using a standard code format definition.
This code format standard is defined in the file uncrustify.cfg in the main directory of this repository.
For new code contributions, it is highly appreciated if the code is formatted using this tool.

To do so:

* Right click mouse in editor
* "Format Document"

The first time (after installing uncrustify) it must be confirmed to use Uncrustify as formatter and using the default suggested config file.

After setting it as the default formatter, the hotkey Ctrl-Shift-F (Cmd-Shift-F on MacOS) can be used to format the entire document.


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
* Clean
* ... many more.

Some of these options only are available when you have registered with PlatformIO and some are only for paid subscriptions.
At least the basic ones used for almost any user are available with the free account.

The environment definitions all have at least the used micro controller in the name and the amount of flash memory used.

For example:

* ..._ESP8266_4Mnn -> ESP8266 has external flash, which can vary in size from 512 kB to 16 MB, with nn configured as filesystem.
* ..._ESP8285_1M -> ESP8285 has the flash internal, so is always 1 MB.
* ..._ESP32_4M316k -> ESP32 with 4 MB flash and a 1.8 MB partition for the sketch. (316k SPIFFS)
* ..._ESP32_16M2M -> ESP32 with 16 MB flash and a 4 MB partition for the sketch. (2MB LittleFS)

Make a custom build using PlatformIO
------------------------------------

The easiest is to go for the environment "custom_ESP8266_4M1M" and unfold that one.
Then select "Build" to see if it will start building.

If that's working, you can open the file "pre_custom_esp8266.py" and add or remove the plugins and controllers you need.
That Python file is used in the "env:custom_ESP8266_4M1M" (or any "custom" build environment) to define what should be embedded and what not.

For example to have only the controller "C014", you can remove "CONTROLLER_SET_ALL", and just add "USES_C014", 
The same for the plugins you need.

The file is built in the ".pio/build/...." directory right under the main repository directory (the one with the platformio.ini in it)

Instead of modifying "pre_custom_esp8266.py" (or "pre_custom_esp32.py" for that matter), one can also copy "src/Custom-sample.h" to "src/Custom.h" and make the desired changed in this file. This file is excluded from Github, so can be adjusted to your own requirements. When the Custom.h file is there (mind the uppercase C!), it will be used by the build scripts instead of the defaults set by "pre_custom_esp8266.py" (or "pre_custom_esp32.py").

All builds will be made in a directory with the same name as the environment used.

Once the build is successful, the .bin file(s) and .bin.gz file (where applicable) are copied to the ``build_output/bin`` folder.



Upload to ESP
=============



Linux
-----

For Linux, you may need to install 99-platformio-udev.rules to make PlatformIO upload tools work in vscode.


Starter guide for setting up local development of ESPEasy
=========================================================

For those with less development experience, or less experience in using Github, this chapter is intended as a ``How To`` guide to get started with development on ESPEasy.

Github account
--------------

First requirement is to have a Github account. You can either use an existing account or create one (it's free), by opening a browser on https://github.com, and following the steps after clicking the Sign up (for Github) button.

Fork the ESPEasy repository
---------------------------

As an 'external' developer, no (direct) write-access is granted to the ESPEasy repository. To still be able to do development work, a 'fork' (git terminology for a copy) has to be made to your own account. So, log into your Github account from a webbrowser, browse to https://github.com/letscontrolit/ESPEasy and click the Fork button to create that copy:

.. image:: Github_fork_button.png
    :alt: Github fork button

After this completes, you can view the fork in your Github dashboard at https://github.com/[your_github_handle]

(You have to replace [your_github_handle] with the name you selected during the Github sign-up procedure)

Install VSCode and PlatformIO
------------------------------

Earlier on this page, a complete description has been given on how to install **PlatformIO with VSCode** with the required and advised optional extensions and git command-line tools.

NB: PlatformIO is often shortened to PIO.

Clone your forked repository to your computer
---------------------------------------------

To get the ESPEasy sources on your computer for compilation and making modifications, a 'clone' (git terminology) has to be made, using the ``git clone`` command

Open a Command prompt (Windows) or Terminal session (MacOS or Linux), and ``cd`` to a folder where the ESPEasy project can/should be a subfolder of.

Then type this command to create the clone::

    git clone https://github.com/[your_github_handle]/ESPEasy.git

This will create a new folder called ``ESPEasy``, and download all files that make up the project into that folder.

Working on it:

.. image:: Github_clone_working.png
    :alt: Github clone working

Completed:

.. image:: Github_clone_completed.png
    :alt: Github clone completed

To be able to bring your changes as a 'pull request' (git terminology, often referred to as a 'PR') to the ESPEasy repository, a connection has to be made from your local clone to the 'upstream' (git terminology) repository, being the original ESPEasy repo. This command needs to be issued **only once** after cloning the repository into a folder on your computer, and should be executed from the ``ESPEasy`` folder that was just created::

    git remote add upstream https://github.com/letscontrolit/ESPEasy

Now this Command prompt / termninal can be closed.

Open the folder with ESPEasy project
------------------------------------

Start VSCode, and open the ESPEasy folder that was just created. First thing, VSCode will ask you if you trust the authors of the files. The easiest option is to respond by clicking the ' Yes, I trust the authors' button, as that is the only way to get unrestricted access to the sources. After that confirmation, VSCode will take a little time to initialize all plugins.

Depending on your usual workflow, the current VSCode environment can be saved as a 'Workspace' (VSCode terminology), so it can be easily re-opened. This is especially useful if you also use VSCode for other projects/editing work.

Compile an ESPEasy PIO environment
----------------------------------

ESPEasy supports several differnt configurations of ESP units, ESP8266, ESP8285 and ESP32, and also different predefined hardware configurations and sets of plugins & controllers. This has been turned into several different PlatformIO environments, to make managing the different builds as easy as possible.

To compile such 'environment' (PIO terminology), select the PIO button (it looks like an alien) in VSCode:

.. image:: VSCode_PIO_Environments.png
    :alt: VSCode Platform IO environments

Expand an environment from the list, so the PIO options become visible (this will take some time for PIO to scan the configuration of that environment).

.. image:: VSCode_PIO_custom_ESP8266_4M1M.png
    :alt: VSCode Platform IO custom ESP8266 4M 1M expanded

Now, the ``Build`` option is visible, and clicking that will build the project for the selected environment (configuration).

The first build will take some extra time, as PIO needs to first install some of its tooling and other required components and libraries, but as you haven't changed any files yet, the build should be successful:

.. image:: VSCode_build_success.png
    :alt: VSCode build success

(NB: For this build all tools and libraries where already installed, and the computer isn't that slow, so total execution didn't take too much time.)

Regular maintenance of your fork
--------------------------------

If you have forked ESPEasy before (or some time ago), and want to start (new) work on a plugin, it is required to update your fork with the latest state of affairs of ESPEasy, to avoid surprises after submitting a PR.

This expects the currently selected 'branch' (git terminology) to be ``mega``, as is visible in the VSCode statusbar:

.. image:: VSCode_statusbar_mega.png
    :alt: VSCode statusbar current branch mega

The desired branch can be selected by clicking the currently selecte branch name as shown in the status bar, or by typing this command from a VSCode terminal window::

    git checkout mega

The update is 'pulled' (git terminology) by getting the latest from the ``upstream`` source (we defined that source after the initial clone), by opening a terminal window in VSCode and issuing this command::

    git pull upstream mega

(NB: The current development branch of ESPEasy is called ``mega`` where other Github repos often use ``master``. ESPEasy *does* have a ``master`` branch, but it isn't actively maintained.)

Depending on the time passed since the last update, some files will be updated from the git pull command.

To update your fork on Github, these changes should be 'pushed' (git terminology) by using the command::

    git push

If this is the first time you try to push any changes to your repository, VSCode, or actually the GitLens plugin, will ask for your Github credentials, and will switch back and forth a few times between your webbrowser and VSCode to complete the authentication process. This is as intended.

Updating your fork this way should be done at least every time you start new work, and can be done more often if desired. If kept up to date you will avoid starting with an out-dated state of the repository.

Create a new branch
-------------------

An often used git workflow starts by creating a new branch to do the development work in. This will record all changes to the sourcecode you make, and can be put in as a pull request for ESPEasy.

A new branch is created either by clicking on the 'mega' branch name and selecting the option Create new branch... from the list presented. Then a new braanch name should be typed. Naming does have some conventions. New features should get named like 'feature/purpose-of-the-feature', and bugfixes are usually named like 'bugfix/what-is-to-be-fixed'. For the addition of this documentation, I've created a branch named 'feature/how-to-guide-for-new-developers':

.. image:: VSCode_create_branch.png
    :alt: VSCode create branch

.. image:: VSCode_type_branch_name.png
    :alt: VSCode type the branch name

.. image:: VSCode_statusbar_new_branch.png
    :alt: VSCode statusbar with new branch name

As an alternative, a new branch can also be created using command-line commands::

    git branch feature/how-to-guide-for-new-developers
    git checkout feature/how-to-guide-for-new-developers

The nett result of these commands is the same as from using the UI flow shown above.

Change an existing plugin
-------------------------

To improve or extend an existing plugin, after creating a new branch for it, just open the plugin source from the src folder, or its accompanying code in the ``src/src/PluginStructs`` folder, and modify code the as needed. Then compile and see if it all is according to the requirements of the compiler.

Testing is done by uploading the result to an ESPEasy unit and enabling the plugin, testing the changed functionality to ensure no errors or undesired behavior remain in the code.

This uploading can be done in 2 ways:

* *Use the Upload feature of PIO*: If the ESP unit is connected to the computer via USB and the serial chip of the unit is recognized by the OS, the Upload option can be selected to compile the sources (only what was changed since the last compilation) and start the upload procedure. After uploading the ESP will restart
* *Use the Update Firmware option of ESPEasy*: On the Tools tab of ESPEasy, there is a button Update Firmware available (on units that have enough free Flash space) so a new .bin file can be uploaded. The latest successful compiled file can be found in the ``build_output/bin`` subfolder of your ``ESPEasy`` folder.

Updating, or adding if it does not yet exist, the documentation is a useful activity that should be part of changing or adding to the ESPEasy code. Some of the optional VSCode extensions are specifically aimed at that task.

The sources for the documentation are in the repository in the ``docs`` folder and its subfolders.

Add a plugin to ESPEasy
-----------------------

Instead of just changing an existing plugin or some other feature of ESPEasy, also, new plugins can be added. Plugins can be taken from the ESPEasyPlayground repository at https://github.com/letscontrolit/ESPEasyPluginPlayground, or from other sources.

It requires sufficient testing, and analysis of the runtime behavior, of that piece of code, before it shopuld be submitted for a pull request.

Commit and create a pull request
--------------------------------

After changing and testing sourcecode, the time has come to submite the code to ESPEasy to be included in the regular build.

The way to make changes available for others they have to be 'staged' and 'committed' (git terminology) before it can be pushed to the repository. This stage and commit is a 2 step process, best be done from the VSCode UI. First select the GitLens plugin, and select the files that need to be staged and committed:

.. image:: VSCode_stage_changes.png
    :alt: VSCode stage changed files

Clicking one of the + buttons form the selected files, will put the files in the staging area, so they can be committed. Every commit will need a useful commit message, that describes what the commit is all about:

.. image:: VSCode_staged_files.png
    :alt: VSCode list of staged files

Clicking the marked check button will commit the staged files, using the commit message just typed. As an alternative, Ctrl-Enter can also be used to complete the commit.

After the commit is completed, more commits can be added, if desired.

To prepare the commit(s) to be presented as a pull request, the easiest way to accomplish that is to use the Publish Changes button:

.. image:: VSCode_Publish_changes.png
    :alt: VSCode publish change button

After clicking that button, you have to select the source the changes should be published to. As we don't have (direct) write access to the upstream ESPEasy repository, we can only publish to the 'origin' (git terminology), our own fork of the repository, so that option should be selected by clicking it, or pressing the Enter key:

.. image:: VSCode_select_publish_source.png
    :alt: VSCode select publish source

Now that the Publish Changes is done, the pull request can be created. We have to switch to the ESPEasy repository to complete that task. The Github website will show the options for that, assuming you are still logged in to your Github account from that browser:



