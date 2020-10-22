#include "WakeOnLan.h"

WakeOnLan::WakeOnLan(WiFiUDP _udpSock) {
	udpSock = _udpSock;
}

void WakeOnLan::setBroadcastAddress(IPAddress _broadcastAddress) {
	broadcastAddress = _broadcastAddress;
}

void WakeOnLan::setRepeat(uint8_t _repeatPacket, unsigned long _delayPacket) {
	repeatPacket = _repeatPacket;
	delayPacket = _delayPacket;
}

IPAddress WakeOnLan::calculateBroadcastAddress(IPAddress _ipAddress, IPAddress _subnetMask) {
	for (int i = 0; i < 4; i++)
		broadcastAddress[i] = ~_subnetMask[i] | _ipAddress[i];

	return broadcastAddress;
}

bool WakeOnLan::stringToArray(uint8_t* _macAddress, const char* _macString) {
	char macFormat[23 + 1];
	unsigned int tempMACAddress[6];

	if (strlen(_macString) == 12)  // FFFFFFFFFFFF
		sprintf(macFormat, "%%2x%%2x%%2x%%2x%%2x%%2x");
	else if (strlen(_macString) == 14)  // FFFF-FFFF-FFFF
		sprintf(macFormat, "%%2x%%2x%c%%2x%%2x%c%%2x%%2x", _macString[4], _macString[9]);
	else if (strlen(_macString) == 17)  // FF-FF-FF-FF-FF-FF
		sprintf(macFormat, "%%2x%c%%2x%c%%2x%c%%2x%c%%2x%c%%2x", _macString[2], _macString[5], _macString[8], _macString[11], _macString[14]);
	else
		return false;

	int j = sscanf(_macString, (const char*)macFormat, &tempMACAddress[0], &tempMACAddress[1], &tempMACAddress[2], &tempMACAddress[3], &tempMACAddress[4], &tempMACAddress[5]);
	if (j == 6) {
		for (uint8_t i = 0; i < sizeof(tempMACAddress) / sizeof(*tempMACAddress); i++)
			_macAddress[i] = (uint8_t)tempMACAddress[i];

		return true;
	}

	return false;
}

bool WakeOnLan::sendMagicPacket(String _macString, uint16_t _portNum) {
	return sendMagicPacket(_macString.c_str(), _portNum);
}

bool WakeOnLan::sendSecureMagicPacket(String _macString, String _secureOn, uint16_t _portNum) {
	return sendSecureMagicPacket(_macString.c_str(), _secureOn.c_str(), _portNum);
}

bool WakeOnLan::sendMagicPacket(const char* _macAddress, uint16_t _portNum) {
	uint8_t macAddress[6];

	bool res = stringToArray(macAddress, _macAddress);
	if (!res)
		return false;

	return sendMagicPacket(macAddress, sizeof(macAddress), _portNum);
}

bool WakeOnLan::sendSecureMagicPacket(const char* _macAddress, const char* _secureOn, uint16_t _portNum) {
	uint8_t macAddress[6];
	uint8_t secureOn[6];

	bool res = stringToArray(macAddress, _macAddress);
	if (!res)
		return false;

	bool res2 = stringToArray(secureOn, _secureOn);
	if (!res2)
		return false;

	return sendSecureMagicPacket(macAddress, sizeof(macAddress), secureOn, sizeof(secureOn), _portNum);
}

bool WakeOnLan::sendMagicPacket(uint8_t* pMacAddress, size_t sizeOfMacAddress, uint16_t portNum) {
	size_t magicPacketSize = 6 + (6 * 16);  // FF*6 + MAC*16
	uint8_t* magicPacket = new uint8_t[magicPacketSize];

	int sucessNum = 0;

	generateMagicPacket(magicPacket, magicPacketSize, pMacAddress, sizeOfMacAddress);

	for (uint8_t i = 0; i < repeatPacket; i++) {
		udpSock.beginPacket(broadcastAddress, portNum);
		udpSock.write(magicPacket, magicPacketSize);
		sucessNum += udpSock.endPacket();

		if (delayPacket > 0)
			delay(delayPacket);
	}

	delete[] magicPacket;

	if (sucessNum == repeatPacket)
		return true;

	return false;
}

bool WakeOnLan::sendSecureMagicPacket(uint8_t* pMacAddress, size_t sizeOfMacAddress, uint8_t* pSecureOn, size_t sizeOfSecureOn, uint16_t portNum) {
	size_t magicPacketSize = 6 + (6 * 16) + 6;  // FF*6 + MAC*16 + SecureOn
	uint8_t* magicPacket = new uint8_t[magicPacketSize];

	int sucessNum = 0;

	generateSecureMagicPacket(magicPacket, magicPacketSize, pMacAddress, sizeOfMacAddress, pSecureOn, sizeOfSecureOn);

	for (uint8_t i = 0; i < repeatPacket; i++) {
		udpSock.beginPacket(broadcastAddress, portNum);
		udpSock.write(magicPacket, magicPacketSize);
		sucessNum += udpSock.endPacket();

		if (delayPacket > 0)
			delay(delayPacket);
	}

	delete[] magicPacket;

	if (sucessNum == repeatPacket)
		return true;

	return false;
}

void WakeOnLan::generateMagicPacket(uint8_t*& pMagicPacket, size_t& sizeOfMagicPacket, uint8_t* pMacAddress, size_t sizeOfMacAddress) {
	uint8_t macAddress[6];

	memcpy(macAddress, pMacAddress, sizeOfMacAddress);
	memset(pMagicPacket, 0xFF, 6);

	for (uint8_t i = 0; i < 16; i++) {
		uint8_t indx = (i + 1) * sizeOfMacAddress;
		memcpy(&pMagicPacket[indx], &macAddress, sizeOfMacAddress);
	}
}

void WakeOnLan::generateSecureMagicPacket(uint8_t*& pMagicPacket, size_t& sizeOfMagicPacket, uint8_t* pMacAddress, size_t sizeOfMacAddress, uint8_t* pSecureOn, size_t sizeOfSecureOn) {
	uint8_t macAddress[6];
	uint8_t secureOn[6];

	memcpy(macAddress, pMacAddress, sizeOfMacAddress);
	memcpy(secureOn, pSecureOn, sizeOfSecureOn);

	memset(pMagicPacket, 0xFF, 6);

	for (uint8_t i = 0; i < 16; i++) {
		uint8_t indx = (i + 1) * sizeOfMacAddress;
		memcpy(&pMagicPacket[indx], &macAddress, sizeOfMacAddress);
	}

	memcpy(&pMagicPacket[17 * sizeOfSecureOn], &secureOn, sizeOfSecureOn);
}
