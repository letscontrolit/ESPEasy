// R001 01-05-2015
// First 'stable' edition (meaning that it does not crash during boot of within one minute...)

// R002 02-05-2015
// Added configuration webinterface
// Start AP mode is ssid is not configured

// R003 06-05-2015
// Start AP mode if no connection, leave AP mode 30 seconds after succesfull connection
// Added option to set a fixed last octet of the ESP IP address (remaining config is still done with DHCP)
//   This is the easiest way. You can still change subnet,gw,dns with DHCP but have a fixed IP
//   This is convenient to control ESP with Domoticz http requests (relais, io expander, etc)
// Support for Domoticz multiple value sensor types (temp/hum/baro)
// Added support for pulsecounter

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

// R005 21-05-2015
// Some fixes to be compabtible with Arduino 1.6.4 using ESP board addon 1.6.4.-628-g545ffde

// R006 23-05-2015
// Stored many constant strings to progmem
// delay(1) into http 'while' client check
// delay(10) in main loop instead of yield()
// DomoticzSend <type>,<idx>,<value> test command (via serial)
// Disabled analogRead, seems broken in g8cd3697 (use millis()/1000 as demo value!)

