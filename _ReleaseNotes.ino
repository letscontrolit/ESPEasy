// R001 01-05-2015
// First stable edition

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

