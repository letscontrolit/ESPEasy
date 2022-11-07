# WakeOnLan [![Build Status](https://travis-ci.com/a7md0/WakeOnLan.svg?branch=master)](https://travis-ci.com/a7md0/WakeOnLan)
Wake-On-LAN Library for ( ESP8266 &amp; ESP32 ), Provide an easy way to send/generate magic packet for any MAC Address. Moreover, support SecureOn feature in some motherboard vendors. Finally, support using custom port number than the default one which is port 9.<br /><br />
This library could be used in other environments. Although, (IPAddress, WiFiUDP, delay()) classes should be available.

## **Install**
To install the library to Arduino IDE by downloading this repository as zip file and navigate to Sketch -> Include library -> Add .ZIP library. Alternatively, navigate to Tools -> Library Manager and search for this library name<br /><br />

To install the library to PlatformIO IDE go to and add the repository link to lib_deps variable OR add the library name
`lib_deps = https://github.com/a7md0/WakeOnLan.git`
Or
`lib_deps = WakeOnLan`

#### Include and initialize WiFiUDP
```
#include <WiFiUdp.h>
WiFiUDP UDP;
```

#### Include and initialize WakeOnLan class
```
#include <WakeOnLan.h>
WakeOnLan WOL(UDP); // Pass WiFiUDP class
```

#### Add this line in void setup() (Optinal)

`WOL.setRepeat(3, 100); // Repeat the packet three times with 100ms delay between`

#### Calculate and set the broadcast address, after connecting to WiFi successfully (Optinal)
`WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());`

#### Set the broadcast address manually (Optinal)
`WOL.setBroadcastAddress("192.168.1.255");`
  
## **Usage**

### **Send WOL from char array MAC Address**

#### Set MAC address in variable
```
const char *MACAddress = "01:23:45:67:89:AB";
```

##### Send WOL UDP packet (By default port 9)
`WOL.sendMagicPacket(MACAddress);`

##### Send WOL UDP packet (Use port 7)
`WOL.sendMagicPacket(MACAddress, 7);`


#### Set MAC address and SecureOn in variable
```
const char *MACAddress = "01:23:45:67:89:AB";
const char *secureOn = "FE:DC:BA:98:76:54";
```

##### Send WOL UDP packet with password (By default port 9)
`WOL.sendSecureMagicPacket(MACAddress, secureOn);`

##### Send WOL UDP packet with password (Use port 7)
`WOL.sendSecureMagicPacket(MACAddress, secureOn, 7);`
  
### **Send WOL from byte array MAC Address**

#### Set MAC address in variable
```
uint8_t MAC[6] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB}; // 01:23:45:67:89:AB
```

##### Send WOL UDP packet (By default port 9)
`WOL.sendMagicPacket(MAC, sizeof(MAC));`

##### Send WOL UDP packet (Use port 7)
`WOL.sendMagicPacket(MAC, sizeof(MAC), 7);`


#### Set MAC address and SecureOn in variable
```
uint8_t MAC[6] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB}; // 01:23:45:67:89:AB
uint8_t SECURE_ON[6] = {0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54}; // FE:DC:BA:98:76:54
```

##### Send WOL UDP packet with password (By default port 9)
`WOL.sendSecureMagicPacket(MAC, sizeof(MAC), SECURE_ON, sizeof(SECURE_ON));`

##### Send WOL UDP packet with password (Use port 7)
`WOL.sendSecureMagicPacket(MAC, sizeof(MAC), SECURE_ON, sizeof(SECURE_ON), 7);`


### **Generate WOL packet**

#### Generate
```
size_t magicPacketSize = 6 + (6 * 16);  // FF*6 + MAC*16
uint8_t* magicPacket = new uint8_t[magicPacketSize]; // Magic packet will be stored in this variable

uint8_t MAC[6] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB}; // 01:23:45:67:89:AB

WOL.generateMagicPacket(magicPacket, magicPacketSize, pMacAddress, sizeof(MAC));
```

#### Generate with secure on
```
size_t magicPacketSize = 6 + (6 * 16) + 6;  // FF*6 + MAC*16 + SecureOn
uint8_t* magicPacket = new uint8_t[magicPacketSize]; // Magic packet will be stored in this variable

uint8_t MAC[6] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB}; // MAC Address = 01:23:45:67:89:AB
uint8_t SECURE_ON[6] = {0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54}; // SecureOn = FE:DC:BA:98:76:54

WOL.generateMagicPacket(magicPacket, magicPacketSize, MAC, sizeof(MAC), SECURE_ON, sizeof(SECURE_ON));
```
