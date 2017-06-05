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
#include "OpenWeather_ESP8266.h"

String OpenWeather_ESP8266::description, OpenWeather_ESP8266::city = "KwunTong", OpenWeather_ESP8266::country, OpenWeather_ESP8266::icon_name, OpenWeather_ESP8266::unit = "metric";
float OpenWeather_ESP8266::temp = 0.0, OpenWeather_ESP8266::temp_min = 0.0, OpenWeather_ESP8266::temp_max = 0.0;
int OpenWeather_ESP8266::pressure = 0, OpenWeather_ESP8266::humidity = 0;
	
char* OpenWeather_ESP8266::viewWeatherFeeds(String unit,int* queryLength)
{
            String query;
            query="/data/2.5/weather?q="+city+"&units="+unit+"&APPID="+String(apiReadKey);
            char *char_query=  new char[query.length()+1];
            strcpy(char_query,query.c_str());
            char_query[query.length()+1] = '\0';
            *queryLength= query.length()+1;
            return char_query;
}

/*char* OpenWeather_ESP8266::viewWeatherFeedsbyZipCode(String& unit,int* queryLength, char* apiReadKey, char* parameter)
{
            String query;
            query="/data/2.5/weather?zip="+String(parameter)+"&units="+unit+"&APPID="+String(apiReadKey);
            char *char_query=  new char[query.length()+1];
            strcpy(char_query,query.c_str());
            char_query[query.length()+1] = '\0';
            *queryLength= query.length()+1;
            return char_query;
}*/

char* OpenWeather_ESP8266::viewWeatherFeeds(String unit,int* queryLength, char* apiReadKey, char* parameter)
{
            String query;
            query="/data/2.5/weather?id="+String(parameter)+"&units="+unit+"&APPID="+String(apiReadKey);
            char *char_query=  new char[query.length()+1];
            strcpy(char_query,query.c_str());
            char_query[query.length()+1] = '\0';
            *queryLength= query.length()+1;
            return char_query;
}

char* OpenWeather_ESP8266::viewWeatherFeeds(String unit,int* queryLength, char* apiReadKey, char* parameter1,char* parameter2)
{
            String query;
            query="/data/2.5/weather?lat="+String(parameter1)+"&lon="+String(parameter2)+"&units="+unit+"&APPID="+String(apiReadKey);
            char *char_query=  new char[query.length()+1];
            strcpy(char_query,query.c_str());
            char_query[query.length()+1] = '\0';
            *queryLength= query.length()+1;
            return char_query;
}

// Single function for HTTP GET response calling request to the server
void OpenWeather_ESP8266::getRequestResponse(char* getresource)
{
            if (connect(server)) 
            {
                      if (getRequest(server, getresource) && skipResponseHeaders()) 
                      {
                                  char response[MAX_CONTENT_SIZE];
                                  readResponseContent(response, sizeof(response));
                                  parseUserData(response);    
                      }
                      disconnect();
           }
           //wait();
}

void OpenWeather_ESP8266::parseUserData(char* content) 
 {   
			float ftemp, ftemp_min, ftemp_max;
			int itemp, itemp_min, itemp_max;
            DynamicJsonBuffer jsonBuffer;
            
            Serial.println("--------------JSON Content-------------------");
            Serial.println(content);

            //JsonObject operator overload bug 
            JsonObject& root = jsonBuffer.parseObject(content);
            Serial.println("---------JSON Parse Start-----------");
            
             if (!root.success()) 
             {
                        Serial.println("JSON parsing failed!");
             }
             else
                        Serial.println("JSON parsing success!");
                        
            Serial.println("---------JSON Parse End-----------");

            ftemp = root["main"]["temp"];
			if (ftemp==0) {
				itemp = root["main"]["temp"];
				temp = (float)itemp;
			} else
				temp = ftemp;
			ftemp_min = root["main"]["temp_min"];
			if (ftemp_min==0) {
				itemp_min = root["main"]["temp"];
				temp_min = (float)itemp_min;
			} else
				temp_min = ftemp_min;
            ftemp_max = root["main"]["temp_max"];
			if (ftemp_max==0) {
				itemp_max = root["main"]["temp"];
				temp_max = (float)itemp_max;
			} else
				temp_max = ftemp_max;
            pressure = root["main"]["pressure"];
            humidity = root["main"]["humidity"];
            
            
            description = String ((const char*)root["weather"][0]["main"]);
            country = String ((const char*)root["sys"]["country"]);
            icon_name = String ((const char*)root["weather"][0]["icon"]);
            //city= root["name"];
}

char* OpenWeather_ESP8266::getWeatherIcon()
{
  String str = "/" + icon_name + ".bmp";
  return (char*)str.c_str(); 
}

// Anyhow needs 15 sec for openweather API to respond
void OpenWeather_ESP8266::wait() 
{
            Serial.println("Wait 1 second");
            delay(1000);
}


// Send the HTTP GET request to the server
bool OpenWeather_ESP8266::getRequest(char* host,char* resource) 
{

            Serial.println();
            Serial.println("Sending request HTTP "); // Request from the server information about the weather
            Serial.print("GET ");
            client.print("GET ");
            Serial.print(resource);
            client.print(resource);
            Serial.print(" HTTP/1.1\r\n");
            client.print(" HTTP/1.1\r\n");
            Serial.print("Host: "+String(host)+"\r\n"); 
            client.print("Host: "+String(host)+"\r\n"); 
            Serial.print("Connection: close\r\n\r\n"); 
            client.print("Connection: close\r\n\r\n"); 

            return true;
}

// Read the body of the response from the HTTP server
void OpenWeather_ESP8266::readResponseContent(char* content, size_t maxSize) 
{
           size_t length = client.readBytes(content, maxSize);
           content[length] = 0;
}

// Skip HTTP headers so that we are at the beginning of the response's body
bool OpenWeather_ESP8266::skipResponseHeaders() 
{
// HTTP headers end with an empty line
           //OpenWeather_ESP8266 openweatherskipheaders;
           char endOfHeaders[] = "\r\n\r\n";
           client.setTimeout(HTTP_TIMEOUT);
           bool ok = client.find(endOfHeaders);
           
           if (!ok) 
           {
                      Serial.println("No response or invalid response!");
           }
           return ok;
}

// Open connection to the HTTP server
bool OpenWeather_ESP8266::connect( char* hostName) 
{
            Serial.println();
            Serial.println();
            Serial.println("------------------------------------------");
            Serial.print("Connect to ");
            Serial.println(hostName);
            
            bool ok = client.connect(hostName, 80);
            Serial.println(ok ? "Connected" : "Connection Failed!");
            return ok;
}

// Close the connection with the HTTP server
void OpenWeather_ESP8266::disconnect() 
{
           Serial.println("Disconnect");
           client.stop();
}

void OpenWeather_ESP8266::Update() {
	char* char_query;
	int query_length;   
	
	char_query= viewWeatherFeeds(unit.c_str(),&query_length);
	char resource_array[query_length];
	
	strcpy(resource_array, char_query); 
	getRequestResponse(resource_array);
}

OpenWeather_ESP8266::OpenWeather_ESP8266() : 
	apiReadKey("3b559fda3a30a6b25ef27e2545392ed1"), server("api.openweathermap.org"), 
	fingerprint("AA 3B D4 93 7C E6 91 A5 61 89 75 F2 CB C8 BA B0 42 B3 0C 74")	{
  HTTP_TIMEOUT = 10000;
  MAX_CONTENT_SIZE = 2048;
}

OpenWeather_ESP8266::~OpenWeather_ESP8266() {
}
