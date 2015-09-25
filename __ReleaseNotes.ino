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

