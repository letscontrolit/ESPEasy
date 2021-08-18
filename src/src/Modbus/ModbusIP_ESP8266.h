/*
    ModbusIP_ESP8266.h - Header for ModbusIP Library
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017-2020 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#pragma once

#if defined(ESP32) || defined(ESP8266)

#include "src/Modbus/Modbus.h"
#ifdef ESP8266
 #include "ESP8266WiFi.h"
#else
 #include "WiFi.h"
#endif

#define MODBUSIP_PORT 	  502
#define MODBUSIP_MAXFRAME 200
#define MODBUSIP_TIMEOUT 1000
#define MODBUSIP_UNIT	  255
#define MODBUSIP_MAX_TRANSACIONS 16
#define MODBUSIP_MAX_CLIENTS	  4
#define MODBUSIP_ADD_REG  1
#define MODBUSIP_UNIQUE_CLIENTS
#define MODBUSIP_MAX_READMS 100

// Callback function Type
typedef bool (*cbModbusConnect)(IPAddress ip);

typedef struct TTransaction {
	uint16_t	transactionId;
	uint32_t	timestamp;
	cbTransaction cb = nullptr;
	uint8_t*	_frame = nullptr;
	void*		data = nullptr;
	TAddress	startreg;
	Modbus::ResultCode forcedEvent = Modbus::EX_SUCCESS;	// EX_SUCCESS means no forced event here. Forced EX_SUCCESS is not possible.
	bool operator ==(const TTransaction &obj) const {
		    return transactionId == obj.transactionId;
	}
};

class ModbusIP : public Modbus {
	protected:
	typedef union MBAP_t {
		struct {
			uint16_t transactionId;
			uint16_t protocolId;
			uint16_t length;
			uint8_t	 unitId;
		};
		uint8_t  raw[7];
	};
	cbModbusConnect cbConnect = nullptr;
	cbModbusConnect cbDisconnect = nullptr;
	WiFiServer* tcpserver = nullptr;
	WiFiClient* tcpclient[MODBUSIP_MAX_CLIENTS];
	std::vector<TTransaction> _trans;
	int16_t		transactionId = 0;  // Last started transaction. Increments on unsuccessful transaction start too.
	int8_t n = -1;
	bool autoConnectMode = false;
	uint16_t serverPort = 0;

	TTransaction* searchTransaction(uint16_t id);
	void cleanupConnections();	// Free clients if not connected
	void cleanupTransactions();	// Remove timedout transactions and forced event

	int8_t getFreeClient();    // Returns free slot position
	int8_t getSlave(IPAddress ip);
	int8_t getMaster(IPAddress ip);
	uint16_t send(IPAddress ip, TAddress startreg, cbTransaction cb, uint8_t unit = MODBUSIP_UNIT, void* data = nullptr, bool waitResponse = true);
	// Prepare and send ModbusIP frame. _frame buffer and _len should be filled with Modbus data
	// ip - slave ip address
	// startreg - first local register to save returned data to (miningless for write to slave operations)
	// cb - transaction callback function
	// unit - slave modbus unit id
	// data - if not null use buffer to save returned data instead of local registers
	public:
	ModbusIP();
	~ModbusIP();
	bool isTransaction(uint16_t id);
	bool isConnected(IPAddress ip);
	bool connect(IPAddress ip, uint16_t port = MODBUSIP_PORT);
	bool disconnect(IPAddress ip);
	void server(uint16_t port = MODBUSIP_PORT);
	inline void slave(uint16_t port = MODBUSIP_PORT) { server(port); }	// Depricated
	void client();
	inline void master() { client(); }		// Depricated
	void task();
	inline void begin() { server(); }; 	// Depricated
	void onConnect(cbModbusConnect cb = nullptr);
	void onDisconnect(cbModbusConnect cb = nullptr);
	uint32_t eventSource() override;
	void autoConnect(bool enabled = true);
	void dropTransactions();

	uint16_t writeCoil(IPAddress ip, uint16_t offset, bool value, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t writeHreg(IPAddress ip, uint16_t offset, uint16_t value, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t writeCoil(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t writeHreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t readCoil(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t readIsts(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t readHreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t readIreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);

	uint16_t pushCoil(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pullCoil(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pullIsts(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pushHreg(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pullHreg(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pullIreg(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);

	uint16_t pullHregToIreg(IPAddress ip, uint16_t offset, uint16_t startreg, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pullCoilToIsts(IPAddress ip, uint16_t offset, uint16_t startreg, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pushIstsToCoil(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pushIregToHreg(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	/*
	uint16_t maskHreg(IPAddress ip, uint16_t offset, uint16_t andMask, uint16_t orMask, cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t pushPullIreg
	uint16_t pushPullHreg
	uint16_t pushIregPullToHreg
	uint16_t pushHregPullToIreg
	uint16_t pushPullHreg(IPAddress ip,
		uint16_t from, uint16_t to, uint16_t numregs = 1,
		uint16_t to, uint16_t from, uint16_t numregs = 1,
		cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	uint16_t readWriteHreg(IPAddress ip,
		uint16_t readOffset, uint16_t* value, uint16_t numregs = 1,
		uint16_t writeOffset, uint16_t* value, uint16_t numregs = 1,
		cbTransaction cb = nullptr, uint8_t unit = MODBUSIP_UNIT);
	*/
};

#endif