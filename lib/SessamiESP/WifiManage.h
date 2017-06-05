/**************************************************************************************
 Title : Wifi setting Class
 Modified by : Kaz Wong
 Date : 7 Dec 2016
 Description : Save Wifi setting and connect to internet
 ***************************************************************************************/

#ifndef WIFIMANAGE_H_
#define WIFIMANAGE_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <String.h>

class WifiManage {
private:
	String ssid; //Wi-Fi SSID for ESP8266
	String password; //Wi-Fi Password for ESP8266

public:
	void SetSSID(String _ssid) { ssid = _ssid; }
	void SetPW(String _password) { password = PWCry(_password); }
	const char* PWCry(String pw) { return pw.c_str(); }
	const char* PWCry() { return password.c_str(); }

	String GetSSID() const { return ssid; }
	String GetPW() const { return password; }
	//Static member function
	bool initWiFiConnection();

	WifiManage();
	~WifiManage();
};

#endif
