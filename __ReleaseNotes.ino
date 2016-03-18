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

