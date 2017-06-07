/**************************************************************************************
 Title : Wifi setting Class
 Modified by : Kaz Wong
 Date : 7 Dec 2016
 Description : Save Wifi setting and connect to internet
 ***************************************************************************************/

#include "WifiManage.h" 

//Initialize WiFi Connection
bool WifiManage::initWiFiConnection()
{                 
             Serial.println("---------Wi-Fi Connection Start-----------");     
               
             Serial.print("Using Wi-Fi SSID :");
             Serial.println(ssid);
   
            WiFi.begin(ssid.c_str(), PWCry());
            Serial.println("\nAttempting Connection!!!");
            
			int attempt = 0;
            while (WiFi.status() != WL_CONNECTED) 
            {
					  if (attempt >= 30)
						return false;
                      delay(500);
                      Serial.print(".");
					  attempt++;
            }
            Serial.println("");
            Serial.println("Wi-Fi connected to SSID :");
            Serial.println(ssid.c_str());
            Serial.println("IP address :");
            Serial.println(WiFi.localIP());

            Serial.println("---------Wi-Fi Connection End-----------");   
            Serial.println();
            return true;    
}

WifiManage::WifiManage() : ssid(""), password("") {
	ssid = "Coolsure";
	password = "abcdeabcde";
}

WifiManage::~WifiManage() {
}