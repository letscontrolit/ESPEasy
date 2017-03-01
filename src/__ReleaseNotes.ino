// Rxxx 02-12-2016 EasyMega build (experimental)
// Native SPIFFS support, old custom flash routines removed
// Support OTA up to 1.5MB sketch size (change to 1M SPIFFS model) on 4M modules
// Fallback Wifi AP support
// Supports multiple active controllers
// Supports notification plugins (basic email _N001.ino provided)
// Native SD card support and system logging to SD card
// Taskvalues are preserved during reboot (not after power loss!)
// Multiple rule sets, max 4 sets, each 2k in size
// Several device plugins reworked and added to the main branch:
//   P036 Framed OLED
//   P037 MQTT import
//   P038 NeoPixel
//   P039 Thermocouple
//   P040 ID12/RDM6300 RFID
//   P041 NeoPixel WordClock
//   P042 NeoPixel Candle
//   P043 Timer Clock
//   P044 P1 gateway
// Removed savesettings/savecustomsettings from candle plugin, check this later...
// UDP broadcast option prep
// Flash write protection mechanism (aded command 'resetFlashWriteCounter')
// IDX/var no longer mandatory and only requested when the selected controller needs it
// Clock#Time supports 'Wrk' for workdays and 'Wkd' for weekends as wildcards

// R148 02-12-2016
// Fixed a bug in ADS1115 plugin (contributed by Shardan)

// R147 27-11-2016
// Fixed a situation where a large message delay uses background processing without UPD handling, causing network issues.

// R146 20-11-2016
// Fixed a bug in PLUGIN_SERIAL/UDP calls where eventstruct data was not prepared
//   Ser2Net plugin RXWait would only work as expected on task 1

// R145 13-11-2016
// Removed controller specific code from the framework (Domoticz test routines)
// Fixed device table display

// R144 12-11-2016
// Fixed PCA Plugin init (contributed by thexperiments)

// R143 02-11-2016
// Added nodeType for future use

// R142 26-10-2016
// Fixed some more missing I2C scanner devices. Changed 'known' into 'supported'
// Changed help items to www.letscontrolit.com/wiki
// Added a retry mechanisme on NTP response failures, to minimize booting with time set to zero...

// R141 21-10-2016
// Fixed pulsecounter value display style elements
// Fixed some missing I2C scanner 'known devices'

// R140 16-10-2016
// Added PCF8574A at default address as known device

// R139 14-10-2016
// Fixed a bug with Domoticz MQTT protocol. Broken as of R109, due to the new pubsub library
//   It also needs a patched pubsub library!
//   Also fixed a specific crash situation on invalid data

// R138 10-10-2016
// Fixed publish command
// Fixed boot event, triggered after getting the system time

// R137 09-10-2016
// Added formula option for pow calculations like 2^3 in (contributed by pm-cz)

// R136 07-10-2016
// Added NodeMCU/Wemos pin numbers (contributed by nonflammable)
// Added Pressure altitude adjustment to pressure sensors (contributed by adrianmihalko/pm-cz)

// R135 05-10-2016
// Added build and unit name to the node list

// R134 04-10-2016
// Corrected Typo and password field type change
// Added %eventvalue% that can substitute the event value in actions
// Added total to the pulse counter as single value (not persistent counter!) (contributed by fvdpol)
// Added authentication to http controller (contributed by marcfon)
// Added SPI support in hardware tab (contributed by moelski)
// Added NodeMCU/Wemos pin numbering to GPIO list (contributed by moelski)

// R133 26-09-2016
// Added rule events to SerialServer and SerialSend command
// Changed 'wrap on' to 'wrap off' in rules editor

// R132 24-09-2016
// Moved ISR handlers for Pulse, RFID and HCSR04 plugins to iram cache

// R131 11-09-2016
// Added flash write counter (count since boot), displayed in webgui
// Update plugin BME280 for calibration data on multiple sensors (contributed by maxx333)

// R130 10-09-2016
// Fixed some code when using SPIFFS (not recommended!)
// Support DS1825 (contributed by maxx333)

// R129 08-09-2016
// Update plugin BH1750, support multiple sensors (contributed by guslinux & pm-cz)
// Update plugin BME/BMP280, (contributed by pm-cz)

// R128 06-09-2016
// Update FHEM plugin (contributed by ddtlabs)
// Change for decimals on SI7021 humidity (contributed by pm-cz)

// R127 04-09-2016
// Fixed the situation where using the old style MQTT gpio/pwm commands left event-Par3 uninitialized.
//   causing an 'endless' delay using pwm command

// R126 28-08-2016
// Added JSON support on FHEM controller plugin (contributed by ddtlabs)

// R125 27-08-2016
// Update OLED plugin, support for smaller displays (contributed by ToniA)

// R124 20-08-2016
// Corrected minor issue on plugin_027
// Adjustable serial RX Receive timeout in Ser2Net plugin. To collect entire message and send it as one TCP packet.
// Added generic UDP controller, C010
// Added SENSOR_TYPE_QUAD, mainly for dummy device
// Code optimization for controllers C004, C005, C006, C008
// Added default template to C008
// Added IR TX plugin from the playground as P035, (contributed by cherowley)

// R123 19-08-2016
// Added support for the DHT12 sensor, successor of the famous DHT11. (contributed by pm-cz)
//   It can be used as single wire or I2C.

// R122 16-08-2016
// Check on unit number < UNIT_MAX before adding self to the nodelist. This would corrupt the nodelist data structure for units > 31
// Fixed another potential buffer overflow when logging ser2net data. And changed the fix from R119. Extended Ser2Net logging.
// Increased the syslog buffer size to 256 bytes.

// R121 11-08-2016
// Added option for MQTT retain flag. Can be configured in /tools/advanced
// Added internal TaskRun <tasknr> command

// R120 10-08-2016
// Removed formula fields from the Dummy Device. This will not work anyway. Use a formula in TaskValueSet command instead.

// R119 09-08-2016
// Fixed calculation for DS1820 DS18S20 with extended precision for negative temperatures (contributed by saschaludwig)
// Bugfixes for uninitialized variable and potential buffer overflow (contributed by bertrik)
// Bugfix pulse commands for PCF8574 plugin
// Main webpage shows sketch size / free size

// R118 03-08-2016
// Added the Dummy Device to production (used in the testlab in the past, but can also be useful for production purposes)
//   You can use "TaskValueSet <task nr>,<variable nr>,<value>" in the rules section to set values
//   Sending and retrieving values works just like any other sensor, using the template [<name>#<valuename>]
// Prepared framework for SENSOR_TYPE_DUAL

// R117 01-08-2016
// Added support for FHEM using HTTP protocol (contributed by ddtlabs)

// R116 01-08-2016
// Counter plugin can be set to "Delta" (original and default setting for Domoticz incremental counter) or "Delta/Total/Time" for other controllers that can handle this counter type

// R115 31-07-2016
// Provided controller plugin calls CPLUGIN_WEBFORM_SAVE and CPLUGIN_WEBFORM_LOAD to support custom controller configurations stored in flash.

// R114 22-07-2016
// Cleanup code, conditional core 2.0.0/2.1.0 support removed, changed to core 2.3.0
// The web gui now shows the core version used to build the release.

// R113 21-07-2016
// Bugfix in rules processing with multiple value devices, only the first value would work as expected

// R112 19-07-2016
// Bugfix for timezones other than whole hours offset. It is now set in +/- minutes

// R111 18-07-2016
// Bugfix parsing rfid (SENSOR_TYPE_LONG) values in parseTemplate function

// R110 16-07-2016
// Improved Serial Server plugin, reconnect handling
// Improved thingspeak and emoncms plugins (contributed by pm-cz)

// R109 12-07-2016
// Fix HTU21d timing
// Support internal pullup for SHT1X plugin
// Moved to pubsubclient 2.6 library (source no longer compatible with 1.9 library, so use the 2.6 version!)

// R108 07-06-2016
// Support for MS5611 temp/baro sensor (contributed by Battika)
// Support PWM fade (contributed by qubeck). 
//	Use only for small fade times as it's a blocking function!

// R107 21-05-2016
// Support to include VCC reading at compile time, defaults to disabled. (contributed by Battika)

// R106 10-05-2016
// Added support for DS1820/DS18S20 sensor types
// Option to change connection failure threshold, set to 0 to disable.

// R105 26-04-2016
// Bugfix for event names ending with "on "
// Added support for SHT1X sensors like the SHT10 Soil Moisture sensor (contributed by MarkBergsma)

// R104 25-04-2016
// Added a few lowlevel networking commands, to be used in the rules section:
//   SendToUDP <ip>,<port>,<message>  sample: SendToUDP 192.168.0.123,65500,Hello
//   SendToHTTP <ip>,<port>,<path>    sample: SendToHTTP 192.168.0.8,8080,/json.htm
// Reduced flash load size to 2048 in rules engine and use static pointer to save memory on recursive calls
// Max nesting level on rules is 3 levels deep

// R103 19-04-2016
// Upon request, added option for factory default static IP settings.

// R102 19-04-2016
// Added support for BMP280  (contributed by PM-CZ)

// R101 17-04-2016
// bugfix P020, the savetasksettings was obsolete, the loadtasksettings was not...
// Use hostname in C008 protocol if the controller hostname is configured (instead of IP)

// R100 13-04-2016
// Moved UDP handling from background tasks as it could lead to recursive function call issues on globalsync receive
// Some more experimental features:
//  - Added option to send rule events using http control page: /control?cmd=event,<event> (single event buffer limitation applies!)
//  - Added option to send rule events using openhab MQTT protocol: publish event,<event> to <template>/cmd (single event buffer limitation applies!)
//  - Added 'SendTo <unit>,<command>' command for remote unit control commands (uses the internal peer to peer UDP messaging)
//  - Added 'Publish <topic>,<value>' command to send remote commands to other ESP using MQTT broker

// R99 11-04-2016
// Moved PLUGIN_INIT call from webserver form post request after savesettings
// Device table no longer displayed when task is in edit mode, to reduce page size.
// Removed obsolete taskload/save from P020
// Removed obsolete taskload from P004

// R98 10-04-2016
// Fixed boot state send for GPIO for state=1
// Added send state at boot stage for MCP and PCF plugins
// Added option to periodically send input switch states (set timer value to >0)

// R97 09-04-2016
// Bugfix, unable to deselect LED pin when I2C is not configured on any pin.

// R96 07-04-2016
// Bugfix in caching of ExtraTaskSettings

// R95 31-03-2016
// Removed AP config within WifiApMode function, not needed as it is already set at WifiAPConfig
// Added logging to AP mode change
// Added periodic Wifi reconnect check when Wifi connection is lost, AP will be active during this stage
// Set status led pin to -1 on factory reset
// Removed some obsolete debug commands

// R94 30-03-2016
// Fix for clock event rules and added size counter for rules interface (currently 2048 byte max)

// R93 27-03-2016
// Enabled senddata for remote feed sensors and suppressed additional config settings on remote feed tasks.
// Settings save bug, sends 32 sectors instead of only 8 needed. Also the offset 2048 was wrong in the second client.write
// Added Name, Unit number, Build and datestamp to the config file name, sample: Config_DemoESP_U12_R93_2016_3_27.txt

// R92 26-03-2016
// Fixed rounding bug, it seems that String() has a bug with 0 decimals, adding a leading white space. Fixed with toString wrapper.

// R91 20-03-2016
// Added globalsync option to Dallas plugin config

// R90 20-03-2016
// Changed DHT plugin timing, start pulse delay wait was 80/40, should be 80/80. It could result in reading 'half' values.

// R89 20-03-2016
// Added simple load indicator by counting and comparing idle loop counts during initial delay and working mode.
// Moved custom value display of SENSOR_TYPE_LONG from plugins to webserver.ino
// Changed <Base64.h> to <base64.h>, case sensitivity issue on Linux

// R88 19-03-2016
// Option to set decimals for sensor values reported to controller and display. (internal precision remains unchanged)
// Changed DHT plugin timing, 40 -> 20 uSec
// Increased taskname and valuename fieldsize from 25->40
//   IMPORTANT NOTE: Stored settings to file will become invalid, need to resave after upgrade to R88!

// R87 18-03-2016
// Remote sensors also create event for rules
// Fixed IR sensor type, should be SENSOR_TYPE_LONG as it is a unsigned long value.
//   Note: the resulting value will change because it was rounded to a limited precision float before!

// R86 18-03-2016
// Commmand output on tools/command into readonly textarea
// Print "Ok" or "Unknown command" for commands. Most commands report back to source (serial,http,mqtt)
// Enabled experimental globalsync feature (could display values from remote sensors on local OLED...)
// Enabled experimental rules feature (used for testing but may also serve other purposes...)

// R85 13-03-2016
// Preparations for Arduino Core 2.1.0
//   Fixed and tested HTTP authentication in C001
//   Servo library seems to have issues in Core 2.1.0. Using the servo library from 2.0.0.
//   I2C clockstretchlimit can be set using tools/advanced.
// Preparations for global pinstate maintenance and status requests
// Redesign on plugin write output reporting as it was still focussed on HTTP only.

// R84 08-03-2016
// Preparations for Arduino Core 2.1.0
//   base64 encoding for HTTP authentication in C001
//   ConfigSerial casting in ser2net plugin
//   Wire.setClockStretchLimit(2000) for better PN532 support
// Bugfix command handling for openhab MQTT protocol using a non default subscribe
// Added standard logging mechanisme to plugin writes

// R83 03-03-2016
// Bugfix for ExtraTaskSettings.TaskIndex that was not stored in flash
// Optimized parseTemplate performance.

// R82 26-02-2016
// Experimental support for Domoticz MQTT protocol using actuator devices
//   currently limited to On/Off switches and one channel Dimmers and only build-in GPIO's
// Use parseCommandString in Webserver handle_control
// Added option to use plugin commands in tools/commands
// Display flash size in kB.
// Restructured main loop
// Changed sensor timer scheduling, adjust with message delay to spread readings, prevent bursts
// Removed protocol.name from global memory struct to save RAM
// Added global system timers
// Added "LongPulse" commands for build-in GPIO, MCP23017, PCF8574 and PME, using non blocking system timers (inspired by chunter1)

// R81 23-02-2016
// Added SSDP custom version that uses much less RAM than default library, disabled by default, can be enabled in tools/advanced (contributed by mneuron)

// R80 17-02-2016
// Added logging controller TCP port
// Bugfix, set default task timer to main delay if no value entered.
// Added support for all commands on OpenHAB MQTT, using "<system name>/cmd" topic and the message contains the entire command.
// Added Wifi signal strength reporting using the System Info device
// Added network traffic indication to Wifi status led

// R79 16-02-2016
// Bugfix: I2C Scanner mentions PCA9685 as possible device for address 0x40
// Added missing pulse feature for PCF8574
// Added Wifi status LED
// Added option to send boot state on Switch Input device
// Added option to use GPIO-1 and GPIO-3 for other purposes than Serial (must disable serial in tools/advanced!)
// Added individual timers per task

// R78 06-02-2016
// Bugfix for PCF8574 plugin, could not address PCF8574A chip types

// R77 04-02-2016
// Bugfix PN532, code cleanup
//  The PN532 will not work properly with the current ESP Arduino Core 2.0.0. (!)
//  Connect the PN532 RSTPDN pin to a GPIO port and configure this in the device settings as reset pin
//  It will likely reset the PN532 several times a day.
//  Best we can get so far. May be fixed in Arduino ESP core 2.1.0

// R76 31-01-2016
// Added retry to PN532 init step

// R75 31-01-2016
// Fixed a bug for generic http url request variables that needed URLEncoding
// Fixed too aggresive reset action, PN532 now only resets after three consecutive read errors.

// R74 30-01-2016
// Fixed a bug where using both controller IP and (DNS) hostname leads to unwanted behavior
//   more specifically if there is confusion between ESP hostname and controller hostname
//   now you have to select location by IP or DNS and can't enter both fields simultaniously
// Fixed a bug for RFID plugins that provide long values to the controller plugin, did not work for the generic http controller plugin

// R73 28-01-2016
// Removed flashcheck (did not prove much) and many other debug commands
// Update PN532 plugin, should work better now. And added optional reset GPIO pin.

// R72 23-01-2016
// Added flashcheck <start>,<end> command (simple read check)

// R71 23-01-2016
// Changed DHT nul reading logic
// Added connect failure check in MQTT protocol
// Added more info on 'flashdump' serial command
// Factory reset erases entire flash except the sketch and zero fills 64k SPIFFS block
// System halts with error message if no SPIFFS area set.

// R70 22-01-2016
// Avoid reporting DHT nul readings.
// Changed timing SI7021 (contributed by Hallard)

// R69 20-01-2016
// Cosmetic: removed last empty table row in device edit screen
// Added help button to firmware upload
// Added delay to DHT sensor
// Removed interrupt blocking from DHT sensor to avoid Wifi/Network handling issues in the ESP core
// Removed interrupt blocking from Dallas sensor to avoid Wifi/Network handling issues in the ESP core
// Moved UDP check from main loop to background routine
// Webgui now reports ESP.getFlashChipRealSize()
// Added detection of missing BMP085 sensor during init
// Added option to reset target device when the Ser2Net plugin has initialized.

// R68 17-01-2016
// Changed UDP logging
// Added OTA update using bin file upload through the ESP Easy webgui (NOT from arduino IDE!)
//   THIS WILL ONLY WORK ON MODULES WITH MORE THAN 512K FLASH, like the ESP12E modules!

// R67 16-01-2016
// Added plugin for BME280 Temperature/Humidity/Barometric pressure sensor

// R66 16-01-2016
// Added plugin for INA219 voltage & current sensor
// Fixed LCD/OLED padding and now it should actually work in all cases...

// R65 15-01-2016
// Changed UDP transmit delays
// Fixed padding feature that broke the option to skip empty lines for local display
// Added system info plugin (single value device type)
// Added some system info to http json request output

// R64 14-01-2016
// Experimental: Added simple json interface to retrieve sensor data using http
// Experimental: Prepare tasks for remote data feed (Work in progress...)
// Added %uptime% as variable for display
// Added padding to LCD/OLED display
// Added support for local button to turn on LCD/OLED display with configurable timeout
// Changed: no system time displayed when NTP is disabled
// Added protocol option "Standalone" for units without a controller
// Fixed UNIT_MAX in node list
// Added boot config for GPIO pin states

// R63 10-01-2016
// Added Wifi connect setup 'wizard' using captive portal
// Enabled formula for PME plugin

// R62 09-01-2016
// Changed Pro Mini Expander plugin to read 10 bit analog values
//  Pro Mini sketch is also updated, so reprogram your Pro Mini too!

// R61 08-01-2016
// Fix SI7021 plugin for compatibility with HTU21D (contributed by Hallard)
//   added a few more mSec to each settings, because my SI7021 sample still had read errors...
// Added boot cause info for external watchdog

// R60 05-01-2016
// Prepared for plugin numbers ranging between 1-255 (maximum active is still limited to 64!)
// Changed hardcoded page limit 4 to TASK_MAX/4
// Added support for MLX90614 IIR Temperature sensor (contributed by lindeger)
// Added support for ADS1115 ADC (contributed by lindeger)

// R59 02-01-2016
// Replaced delay(10) with yield in backgroundtasks()
// Changed PN532 plugin init stage en reduced sample frequency
// Restored original I2C Watchdog feed for further development
// Fixed bug in HCSR04 and other plugins where buffersize was too short

// R58 28-12-2015
// Fixed bug with right align on LCD 4x20
// Time feature can be disabled with #define at compile time
// No longer using time library, moved relevant parts to Misc tab
// Set system name as DHCP hostname
// Fixed bug with levelcontrol for other values in taskvalue dropdown list

// R57 23-12-2015
// Moved display template handling to generic function parseTemplate() to avoid a lot of similar code in both plugins.
// Added some variable features to LCD/OLED template, you can use %sysname%, %systime% and %ip%
// NTP time can be enabled through advanced settings and defaults to disabled
// Added clear command to LCD/OLED plugins

// R56 22-12-2015
// Added DNS static config option
// Added NTP host name config (optional, defaults to pool.ntp.org)
// Added TimeZone config
// Added Controller Hostname config (optional, use instead of IP)
// Added System time to main info page
// Added switch case to ThingSpeak controller

// R55 21-12-2015
// Experimental development: Added time lib and NTP support
// Bugfix for LCD plugin display size

// R54 20-12-2015
// LCD I2C address and display size can be configured in the webgui
// OLED I2C address and display rotation can be configured in the webgui
// OLEDCMD and LCDCMD commands added to turn the display on or off
// Removed the +15 offset in BH1750 plugin

// R53 19-12-2015
// Added some serial debug commands:
// "load" to load settings from flash while running
// "flashdump <start>, <end>" to show flash contents
// Changed WifiConnect, added retry and changed delays, set static ip before connecting
// Wifi config is no longer persistent in SDK controlled flash memory

// R52 17-12-2015
// Changed send delay routine
// Added feature "Send data" enable/disable to all tasks
// Removed "send data" setting from Level Control plugin since this is now standard
// Check on build changes to fix some changes in tasks
// Removed all urlDecode() since this is handled in the ESP Core as of stable 2.0.0

// R51 13-12-2015
// Changed UDP handling back to a single socket for RX/TX
// Moved LCD init from main tab to LCD plugin
// Added pulse support on MCP23017 (contributed by fgmx85)

// R50 10-12-2015
// First attempt on having a generic HTTP controller interface

// R49 08-12-2015
// Changes to urldecode and WPAKey handling (contributed by ixtrader)
// Added MQTT authentication (contributed by Chrille)
// Changing protocol sets default TCP port

// R48 01-12-2015
// Code cleanup, candidate for Arduino ESP Core 2.0.0.

// R47 29-11-2015
// Support for using http commands to control OLED en LCD displays

// R46 28-11-2015
// Added Wiki help button to Protocol dropdown
// Added basic text message support for I2C OLED SSD1306 display

// R45 22-11-2015
// Bugfix: PCA9685 plugin needs to be listed in the dropdown list
//   else counting/sorting will have issues. But it currently has no real purpose

// R44 22-11-2015
// Added basic support for PCA9685, simple pwm output using control web page

// R43 15-11-2015
// UDP calls to plugins
// Bugfix for TSL2561 plugin

// R42 08-11-2015
// Added support for serial to WiFi bridge applications (very experimental state!)
// Added limited support for local level control like thermostat function (questionable development for a sensor!)
// Device dropdown list is alphabetically ordered

// R41 01-11-2015
// Added support for Sharp GP2Y10 dust sensor
// Added support for PCF8574 IO Expander

// R40 25-10-2015
// Added support to read classic Mifare tags with PN532 NFC module
// Fixed sending of Tag values as a real 32 bit long value (no rounding, no decimal fraction)

// R39 17-10-2015
// Added support for IR signal reception (needs a third party library!)

// R38 13-10-2015
// Added support for I2C TSL2561 Luminosity Sensor (contributed by Hallard)

// R37 12-10-2015
// Added support for I2C SI7021 Temperature Humidity sensor (contributed by Hallard)

// R36 11-10-2015
// Added support to control (max 2) Servo motors using http control commands

// R35 11-10-2015
// Finally finished the LCD plugin with some basic functionality
// Preliminary controller support for EmonCMS

// R34 10-10-2015
// Changed controller password length from 25 to 63 chars.
// If you have set a system password, it will be cleared with this update

// R33 10-10-2015
// Limit flash reset cycle to 3 attempts after cold boot. It will then enter rescue mode.

// R32 09-10-2015
// Fixed a bug in negative values on Dallas DS18b20 and added CRC check

// R31 08-10-2015
// Added more information on dallas plugin and set value to NaN on read errors.
// Changed all serial.print() commands to addlog() where they could conflict with future serial plugins

// R30 06-10-2015
// Moved all controller specific code to CPlugin mechanism (similar to device plugins)
// Added debounce feature to pulsecounter plugin

// R29 03-10-2015
// Fixed bug with idx values > 255. Bug was introduced in R22

// R28 03-10-2015
// Merged Pro Mini extender plugins into one single plugin

// R27 02-10-2015
// Changed MCP23017 plugin to scan inputs realtime instead of using the system timer mechanism
// Changed MCP23017 default pullup resistors on input pins
// Moved MCP23017 output and generic GPIO and PWM to device plugins
// Added simple pulse option for GPIO outputs (only for small pulses less than a few seconds).

// R26 01-10-2015
// Changed internal stylesheet to new skin
// Save configuration settings into SPIFFS reserved area without using SPIFFS
// Reduced code size by disabling SPIFFS to make it work in the latest staging release

// R25 27-09-2015
// Switch plugin supports push button (toggle) switch, selectable for active low or high

// R24 27-09-2015
// Right-aligned device values in device table
// Added periodic MQTT connection check
// Fixed issue with the AP WPA Key configuration

// R23 26-09-2015
// Fix device table layout for some browsers
// Added link to stylesheet "esp.css" located on SPIFFS
// Removed style attibutes from individual tags so it can be arranged with css
// Added delete button in SPIFFS file list

// R22 25-09-2015
// Using Spiffs filesystem support for configuration settings
// All configuration settings are now stored into the ESP filesystem.
// Task string fields are now dynamically loaded from spiffs when needed. Saving RAM...
// Optimize RAM usage by changing device structure array and handling of device strings
// Added option to load and save ESP configuration settings from/to file
// Cleanup all fields when a task is cleared or device is changed
// Moved keys and passwords to a separate file
// Added option to change valuenames (mainly MQTT purpose)
// MQTT publish & subscribe settings can be changed using a template
// Moved some advanced settings to tools/system/advanced button
// Dallas plugin now supports multiple devices on the same bus

// R21 21-09-2015
// Added more logging to MQTT
// Added mcpgpio to valid list of control commands
// Fixed hardcoded MQTT broker port

// R20 13-09-2015
// Changed factory reset, first wipe entire eeprom size to zeros
// Added option to do a factory reset by connecting RX and TX pins during boot
// Added a help button on the device page that leads to the device specific page on our Wiki
// Preliminary support for deep_sleep

// R019 13-09-2015
// Added support for the HC-SR04 Ultrasonic distance sensor
// Input switch can now be set to send switch state or dimvalue
// Other minor internal changes

// R018 09-09-2015
// Fixed issues with special characters (#,% etc) in text fields like SSID, WPA key etc

// R017 08-09-2015
// Fixed factory reset formula settings.
// Preliminary support for PiDome MQTT protocol
// Total rework of all sensor devices, moved code to plugin files for easier maintenance and extensibility
// Plugins can extend the device webform with custom settings
// Changed EEPROM size to 2048
// System no longers needs a reboot after adding/changing specific (IRQ) devices like pulsecounters
// Changed Device table, shows valuenames for devices
// Prevent input switches from firing at boot time, first read current state during init
// Added option to inverse input switch logic
// Changed WPA key limit to 63
// Some code cleanup, removing old debug commands

// R016 29-08-2015
// Preliminary support for IO extender based on Arduino Pro Mini through I2C
// Added GPIO-9 and GPIO-10 for boards equipped with an ESP12E module (NodeMCU V1.0)
// Finished support for PCF8591 4 channel Analog to Digital converter.
// Added support for MCP23017 input ports
// Unit Name is shown in html title
// Show 'well known' devices in the I2C scanner based on their I2C address
// Added option to use simple formulas on device values like "%value%/1000"
// Fixed char string handling that should be 25 chars max
// Added option to turn on/off internal pullup for switch input pins
// Added option to set a name for each task (also used for OpenHAB MQTT publishing)
// Preliminary support for OpenHAB MQTT protocol

// R015 22-08-2015
// Fixed a bug in DHT22 device
// Fixed a bug in setting for default delay
// Changed debug info
// Changed device tab, added more pin info and cancel button
// Fixed syslog loop bug

// R014 20-08-2015
// Redesigned device configuration mechanisme
// Changed password timeout to 5 minutes
// Added direct GPIO output control without password request

// R013 15-08-2015
// Added configurable message delay in ms (delay between messages to controller)
// Changed login password field to type 'password'
// Fixed io pin settings for Dallas/DHT for inconsecutive pin configuration (ESP-01)
// Support for ThingsSpeak "controller" (webservice)

// R012 07-08-2015
// IMPORTANT NOTICE: this release needs ESP Package version 1.6.5-947-g39819f0, built on Jul 23, 2015)
// If udp port = 0 no actions!
// Used sprintf_P to save more RAM
// Added login page and admin password
// Added hardware custom gpio pin selection
// Added sanity check on BMP085 pressure values

// R011 12-07-2015
// Changed javascript link buttons into CSS button styled href links
// Fixed Main button, only worked on IE.
// Changed connectionFailures counting
// Reboot from main loop instead of web event callback
// Changed logging configuration
// BaudRate can be configured through webgui, defaults to 115200
// Added Serial.setDebugOutput(true) if serial loglevel = 4 (debug_more level)
// Added telnet protocol to send variable data to a Nodo Mega controller
// Added webgui I2C scanner
// Added webgui Wifi scanner
// Fixed bug in settingssave on devices
// Added delay(1) in Domoticz_sendData, while available loop

// R010 30-06-2015
// Changed freemem request in root web page
// Some memory reduced by eliminating globals
// Init MQTT at boot only if protocol is set to MQTT
// Added syslog level for detailed logging
// Added menu buttons in webgui and stuctured data as HTML tables

// R009 26-06-2015
// Changed switch pin from GPIO-0 to wired input pin 1
// Added config options for static IP/gateway/subnet
// Using LCD library instead of local functions
// Added MQTT library
// Added JSON parse library
// Needs Domoticz 2560 and Mosquitto 3.1.1 or higher
// Reduced EEPROM from 4096->1024 (needs a RAM buffer from same size!)
// Added basic logging to web interface

// R008 06-06-2015
// UDP listening port web configurable
// If UDP port enabled, also send IDX events to this UDP port
// Added Domotics lightswitch option, input on GPIO-0 pin.

// R007 01-06-2015
// use custom counter for WD instead of millis() (millis does not reset on ESP.Reset)
// Changed BAUDrate to 9600
// DomoticzGet <type>, <idx> test command (via serial)

// R006 23-05-2015
// Stored many constant strings to progmem
// delay(1) into http 'while' client check
// delay(10) in main loop instead of yield()
// DomoticzSend <type>,<idx>,<value> test command (via serial)
// Disabled analogRead, seems broken in g8cd3697 (use millis()/1000 as demo value!)

// R005 21-05-2015
// Some fixes to be compabtible with Arduino 1.6.4 using ESP board addon 1.6.4.-628-g545ffde

// R004 18-05-2015
// Fixed setting ip_octet to 0 on reset
// Fixed missing ; in domoticz send case 3:
// Fixed: Serial Pulse settings command did not work.
// Very experimental stuff for Nodo events on I2C (Nodo 3.8, R818)
// Some code optimization in string/char-array handling within webserver functions
// Replaced 'reboot function call' with ESP.reset
// Added hardware config web page to change io pins on ESP-01 boards
// Added basic stuff for UDP communications and syslog function
// Listens on UDP port 65500 for commands.
// Added connectionFailure counter
// Sends "WD <uptime in seconds> CF <connection failures>" every 30 seconds on serial to reset external watchdog
// Sends "WD <uptime in seconds> CF <connection failures>" to syslog if configured
// Delayed reboot on empty IP adress (in case of DHCP issue)
// Delayed reboot after 30 connection failures

// R003 06-05-2015
// Start AP mode if no connection, leave AP mode 30 seconds after succesfull connection
// Added option to set a fixed last octet of the ESP IP address (remaining config is still done with DHCP)
//   This is the easiest way. You can still change subnet,gw,dns with DHCP but have a fixed IP
//   This is convenient to control ESP with Domoticz http requests (relais, io expander, etc)
// Support for Domoticz multiple value sensor types (temp/hum/baro)
// Added support for pulsecounter

// R002 02-05-2015
// Added configuration webinterface
// Start AP mode is ssid is not configured

// R001 01-05-2015
// First 'stable' edition (meaning that it does not crash during boot of within one minute...)

