# ESPEasy
Easy MultiSensor device based on ESP8266

This is where development takes place. Beware that latest versions may be unstable.

Releases are here: https://github.com/letscontrolit/ESPEasy-platformio/releases [![Build Status](https://travis-ci.org/letscontrolit/ESPEasy-platformio.svg?branch=master)](https://travis-ci.org/letscontrolit/ESPEasy-platformio)

Wiki: http://www.esp8266.nu
Forum: http://www.esp8266.nu/forum


## New release management

Hi, i'm trying to continue maintenance on ESPeasy.

I've already tried to make some improvements to speed up developement: https://github.com/letscontrolit/ESPEasy-platformio

 * I've created a simple wrapper repository to allow building via platformio.
 * The git-tag and version are stored in the firmware.bin file and viewable via the webgui.
 * There are now multiple plugin sets to speed up testing and releasing of new plugins:
   * Minimal: Only contains base plugins until this point, to keep firmware size below 512k for smaller ESP8266's.
   * Normal: All stable plugins
   * Testing: New plugins that need testing before they will be marked stable. (marked with [TESTING] in the webgui)
   * Development: Plugins that are not finished yet or are broken. (marked with [DEVELOPMENT] in the webgui)

   Every release will have 4 firmware files, so everyone can choose which plugin set they want without a need to compile them yourself. This way its possible to use a stable release of ESPeasy and still have some testing plugins that you absolutely need. 
   
 * All commits will be compiled automaticly via Travis
 * When a version in git is tagged, Travis will automaticly create a new release of the binaries on github.
 * Ideally we can now gradually move most play-ground plugins over to the main tree and add them to the approriate plugin set.
 * I would love to use different versioning numbering from now on, perhaps sematic versioning (http://semver.org), starting with 1.x.x since our internal plugin API is already stable.
 
Currently i'm merging the open pull requests and try to give some feedback on them if neccesary.
 
Edwin (psy0rz)
