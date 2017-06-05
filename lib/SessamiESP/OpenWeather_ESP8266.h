/**************************************************************************************
  Original Contributor Information
  Sample Arduino Json Web Client
  Downloads and parse http://jsonplaceholder.typicode.com/users/1
  Copyright Benoit Blanchon 2014-2016
  MIT License
  Arduino JSON library
  https://github.com/bblanchon/ArduinoJson
  If you like this project, please add a star!
***************************************************************************************/
/**************************************************************************************
 Title : OpenWeather + ESP8266 Secure WiFi Connection Code using SHA-1 Fingerprint key
 Modified by : Sagar Naikwadi, Kaz Wong
 Date : 7 Dec 2016
 Description : Get weather information from openweather and download with JSON format
***************************************************************************************/
#ifndef OPENWEATHERESP8266_H_
#define OPENWEATHERESP8266_H_


#include <ArduinoJson.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
//#include <WiFiClientSecure.h>
#include <String.h>

extern "C"
{
#include "user_interface.h"
}

class OpenWeather_ESP8266
{
  private:
    unsigned long HTTP_TIMEOUT; //HTTP request/response timeout
    size_t MAX_CONTENT_SIZE; //HTTP response maximum content size


  private:
    //Server Parameters
    WiFiClient client;
    char* server; //Server IP Domain
    char* fingerprint; //SHA-1 Fingerprint Key for the Server
    char* apiReadKey;

    static String description, city, country, icon_name, unit;
	static float temp, temp_min, temp_max;
    static int pressure, humidity;

  public:
    // Constructor and Destructor
    OpenWeather_ESP8266();
    ~OpenWeather_ESP8266();

    //member function
    bool getRequest(char*, char*);
    bool skipResponseHeaders();
    void readResponseContent(char*, size_t);
    bool connect( char*);
    void disconnect();

    void getRequestResponse(char*);
    void parseUserData(char*);
    void wait();
    char* viewWeatherFeeds(String, int*); //byName
    //char* viewWeatherFeeds(String, int*, char*, char*); //byZipCode
    char* viewWeatherFeeds(String, int*, char*, char*); //byCityID
    char* viewWeatherFeeds(String, int*, char*, char*, char*); //byLatLong
    char* getWeatherIcon();
	
	void Update();

    String GetDescription() const {
      return description;
    }
    float GetTemp() const {
      return temp;
    }
    float GetMaxTemp() const {
      return temp_max;
    }
    float GetMinTemp() const {
      return temp_min;
    }
    int GetPressure() const {
      return pressure;
    }
    int GetHumidity() const {
      return humidity;
    }
};
#endif
