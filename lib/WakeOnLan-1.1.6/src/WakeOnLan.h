#ifndef WakeOnLan_h
#define WakeOnLan_h

#include <WiFiUdp.h>
#include <Arduino.h>

class WakeOnLan {
   private:
	WiFiUDP udpSock;
	IPAddress broadcastAddress = IPAddress(255, 255, 255, 255);

	uint8_t repeatPacket = 1;
	unsigned long delayPacket = 0;

   public:
	WakeOnLan(WiFiUDP _udpSock);

	void setBroadcastAddress(IPAddress _broadcastAddress);
	void setRepeat(uint8_t _repeatPacket, unsigned long _delayPacket);
	
	IPAddress calculateBroadcastAddress(IPAddress _ipAddress, IPAddress _subnetMask);
	
	bool stringToArray(uint8_t* _macAddress, const char* _macString);

	bool sendMagicPacket(String _macString, uint16_t _portNum = 9);
	bool sendSecureMagicPacket(String _macString, String _secureOn, uint16_t _portNum = 9);

	bool sendMagicPacket(const char* _macAddress, uint16_t _portNum = 9);
	bool sendSecureMagicPacket(const char* _macAddress, const char* _secureOn, uint16_t _portNum = 9);

	bool sendMagicPacket(uint8_t* pMacAddress, size_t sizeOfMacAddress, uint16_t portNum = 9);
	bool sendSecureMagicPacket(uint8_t* pMacAddress, size_t sizeOfMacAddress, uint8_t* pSecureOn, size_t sizeOfSecureOn, uint16_t portNum = 9);

	void generateMagicPacket(uint8_t*& pMagicPacket, size_t& sizeOfMagicPacket, uint8_t* pMacAddress, size_t sizeOfMacAddress);
	void generateSecureMagicPacket(uint8_t*& pMagicPacket, size_t& sizeOfMagicPacket, uint8_t* pMacAddress, size_t sizeOfMacAddress, uint8_t* pSecureOn, size_t sizeOfSecureOn);
};

#endif
