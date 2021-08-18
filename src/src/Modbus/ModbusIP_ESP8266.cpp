/*
    ModbusIP_ESP8266.cpp - ModbusIP Library Implementation
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017-2019 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#if defined(ESP32) || defined(ESP8266)

#include "ModbusIP_ESP8266.h"

ModbusIP::ModbusIP() {
	//_trans.reserve(MODBUSIP_MAX_TRANSACIONS);
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		tcpclient[i] = nullptr;
}

void ModbusIP::client() {

}

void ModbusIP::server(uint16_t port) {
	serverPort = port;
	tcpserver = new WiFiServer(serverPort);
	tcpserver->begin();
}

bool ModbusIP::connect(IPAddress ip, uint16_t port) {
	//cleanupConnections();
	if(getSlave(ip) != -1)
		return true;
	int8_t p = getFreeClient();
	if (p == -1)
		return false;
	tcpclient[p] = new WiFiClient();
	return tcpclient[p]->connect(ip, port);
}

uint32_t ModbusIP::eventSource() {		// Returns IP of current processing client query
	if (n >= 0 && n < MODBUSIP_MAX_CLIENTS && tcpclient[n])
		return (uint32_t)tcpclient[n]->remoteIP();
	return (uint32_t)INADDR_NONE;
}

TTransaction* ModbusIP::searchTransaction(uint16_t id) {
	std::vector<TTransaction>::iterator it = std::find_if(_trans.begin(), _trans.end(), [id](TTransaction& trans){return trans.transactionId == id;});
   	if (it != _trans.end()) return &*it;
   	return nullptr;
}

void ModbusIP::task() {
	MBAP_t _MBAP;
	cleanupConnections();
	if (tcpserver) {
		while (tcpserver->hasClient()) {
			WiFiClient* currentClient = new WiFiClient(tcpserver->available());
			if (!currentClient || !currentClient->connected())
				continue;
			if (cbConnect == nullptr || cbConnect(currentClient->remoteIP())) {
				#ifdef MODBUSIP_UNIQUE_CLIENTS
				// Disconnect previous connection from same IP if present
				n = getMaster(currentClient->remoteIP());
				if (n != -1) {
					tcpclient[n]->flush();
					delete tcpclient[n];
					tcpclient[n] = nullptr;
				}
				#endif
				n = getFreeClient();
				if (n > -1) {
					tcpclient[n] = currentClient;
					continue; // while
				}
			}
			// Close connection if callback returns false or MODBUSIP_MAX_CLIENTS reached
			delete currentClient;
		}
	}
	for (n = 0; n < MODBUSIP_MAX_CLIENTS; n++) {
		if (!tcpclient[n]) continue;
		if (!tcpclient[n]->connected()) continue;
		uint32_t readStart = millis();
		while (millis() - readStart < MODBUSIP_MAX_READMS &&  tcpclient[n]->available() > sizeof(_MBAP)) {
			tcpclient[n]->readBytes(_MBAP.raw, sizeof(_MBAP.raw));	// Get MBAP
		
			if (__swap_16(_MBAP.protocolId) != 0) {   // Check if MODBUSIP packet. __bswap is usless there.
				while (tcpclient[n]->available())	// Drop all incoming if wrong packet
					tcpclient[n]->read();
					continue;
			}
			_len = __swap_16(_MBAP.length);
			_len--; // Do not count with last byte from MBAP
			if (_len > MODBUSIP_MAXFRAME) {	// Length is over MODBUSIP_MAXFRAME
				exceptionResponse((FunctionCode)tcpclient[n]->read(), EX_SLAVE_FAILURE);
				_len--;	// Subtract for read byte
				for (uint8_t i = 0; tcpclient[n]->available() && i < _len; i++)	// Drop rest of packet
					tcpclient[n]->read();
			} else {
				free(_frame);
				_frame = (uint8_t*) malloc(_len);
				if (!_frame) {
					exceptionResponse((FunctionCode)tcpclient[n]->read(), EX_SLAVE_FAILURE);
					for (uint8_t i = 0; tcpclient[n]->available() && i < _len; i++)	// Drop packet
						tcpclient[n]->read();
				} else {
					if (tcpclient[n]->readBytes(_frame, _len) < _len) {	// Try to read MODBUS frame
						exceptionResponse((FunctionCode)_frame[0], EX_ILLEGAL_VALUE);
						//while (tcpclient[n]->available())	// Drop all incoming (if any)
						//	tcpclient[n]->read();
					} else {
						if (tcpclient[n]->localPort() == serverPort) {
							// Process incoming frame as slave
							slavePDU(_frame);
						} else {
							// Process reply to master request
							_reply = EX_SUCCESS;
							TTransaction* trans = searchTransaction(__swap_16(_MBAP.transactionId));
							if (trans) { // if valid transaction id
								if ((_frame[0] & 0x7F) == trans->_frame[0]) { // Check if function code the same as requested
									// Procass incoming frame as master
									masterPDU(_frame, trans->_frame, trans->startreg, trans->data);
								} else {
									_reply = EX_UNEXPECTED_RESPONSE;
								}
								if (trans->cb) {
									trans->cb((ResultCode)_reply, trans->transactionId, nullptr);
								}
								free(trans->_frame);
								//_trans.erase(std::remove(_trans.begin(), _trans.end(), *trans), _trans.end() );
								std::vector<TTransaction>::iterator it = std::find(_trans.begin(), _trans.end(), *trans);
								if (it != _trans.end())
									_trans.erase(it);
							}
						}
					}
				}
			}
			if (tcpclient[n]->localPort() != serverPort) _reply = REPLY_OFF;	// No replay if it was responce to master
			if (_reply != REPLY_OFF) {
				_MBAP.length = __swap_16(_len+1);     // _len+1 for last byte from MBAP					
				size_t send_len = (uint16_t)_len + sizeof(_MBAP.raw);
				uint8_t sbuf[send_len];				
				memcpy(sbuf, _MBAP.raw, sizeof(_MBAP.raw));
				memcpy(sbuf + sizeof(_MBAP.raw), _frame, _len);
				tcpclient[n]->write(sbuf, send_len);
				//tcpclient[n]->flush();
			}
			if (_frame) {
				free(_frame);
				_frame = nullptr;
			}
			_len = 0;
		}
	}
	n = -1;
	cleanupTransactions();
}

uint16_t ModbusIP::send(IPAddress ip, TAddress startreg, cbTransaction cb, uint8_t unit, void* data, bool waitResponse) {
	MBAP_t _MBAP;
#ifdef MODBUSIP_MAX_TRANSACIONS
	if (_trans.size() >= MODBUSIP_MAX_TRANSACIONS) return false;
#endif
	int8_t p = getSlave(ip);
	if (p == -1 || !tcpclient[p]->connected())
		return autoConnectMode?connect(ip):false;
	transactionId++;
	if (!transactionId) transactionId = 1;
	_MBAP.transactionId	= __swap_16(transactionId);
	_MBAP.protocolId	= __swap_16(0);
	_MBAP.length		= __swap_16(_len+1);     //_len+1 for last byte from MBAP
	_MBAP.unitId		= unit;
	size_t send_len = _len + sizeof(_MBAP.raw);
	uint8_t sbuf[send_len];
	memcpy(sbuf, _MBAP.raw, sizeof(_MBAP.raw));
	memcpy(sbuf + sizeof(_MBAP.raw), _frame, _len);
	if (tcpclient[p]->write(sbuf, send_len) != send_len)
		return false;
	//tcpclient[p]->flush();
	if (waitResponse) {
		TTransaction tmp;
		tmp.transactionId = transactionId;
		tmp.timestamp = millis();
		tmp.cb = cb;
		tmp.data = data;	// BUG: Should data be saved? It may lead to memory leak or double free.
		tmp._frame = _frame;
		tmp.startreg = startreg;
		_trans.push_back(tmp);
		_frame = nullptr;
		_len = 0;
	}
	return transactionId;
}


void ModbusIP::onConnect(cbModbusConnect cb) {
	cbConnect = cb;
}

void ModbusIP::onDisconnect(cbModbusConnect cb) {
		cbDisconnect = cb;
}

void ModbusIP::cleanupConnections() {
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
		if (tcpclient[i] && !tcpclient[i]->connected()) {
			//IPAddress ip = tcpclient[i]->remoteIP();
			//tcpclient[i]->stop();
			delete tcpclient[i];
			tcpclient[i] = nullptr;
			if (cbDisconnect && cbEnabled) 
				cbDisconnect(IPADDR_NONE);
		}
	}
}

void ModbusIP::cleanupTransactions() {
	for (auto it = _trans.begin(); it != _trans.end();) {
		if (millis() - it->timestamp > MODBUSIP_TIMEOUT || it->forcedEvent != Modbus::EX_SUCCESS) {
			Modbus::ResultCode res = (it->forcedEvent != Modbus::EX_SUCCESS)?it->forcedEvent:Modbus::EX_TIMEOUT;
			if (it->cb)
				it->cb(res, it->transactionId, nullptr);
			free(it->_frame);
			it = _trans.erase(it);
		} else
			it++;
	}
}

int8_t ModbusIP::getFreeClient() {
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		if (!tcpclient[i])
			return i;
	return -1;
}

int8_t ModbusIP::getSlave(IPAddress ip) {
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		if (tcpclient[i] && tcpclient[i]->connected() && tcpclient[i]->remoteIP() == ip && tcpclient[i]->localPort() != serverPort)
			return i;
	return -1;
}

int8_t ModbusIP::getMaster(IPAddress ip) {
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		if (tcpclient[i] && tcpclient[i]->connected() && tcpclient[i]->remoteIP() == ip && tcpclient[i]->localPort() == serverPort)
			return i;
	return -1;
}

uint16_t ModbusIP::writeCoil(IPAddress ip, uint16_t offset, bool value, cbTransaction cb, uint8_t unit) {
	readSlave(offset, COIL_VAL(value), FC_WRITE_COIL);
	return send(ip, COIL(offset), cb, unit, nullptr, cb);
}

uint16_t ModbusIP::writeCoil(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x07D0) return false;
	writeSlaveBits(COIL(offset), offset, numregs, FC_WRITE_COILS, value);
	return send(ip, COIL(offset), cb, unit, nullptr, cb);
}

uint16_t ModbusIP::readCoil(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x07D0) return false;
	readSlave(offset, numregs, FC_READ_COILS);
	return send(ip, COIL(offset), cb, unit, value);
}

uint16_t ModbusIP::writeHreg(IPAddress ip, uint16_t offset, uint16_t value, cbTransaction cb, uint8_t unit) {
	readSlave(offset, value, FC_WRITE_REG);
	return send(ip, HREG(offset), cb, unit, nullptr, cb);
}

uint16_t ModbusIP::writeHreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x007D) return false;
	writeSlaveWords(HREG(offset), offset, numregs, FC_WRITE_REGS, value);
	return send(ip, HREG(offset), cb, unit, nullptr, cb);
}

uint16_t ModbusIP::readHreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x007D) return false;
	readSlave(offset, numregs, FC_READ_REGS);
	return send(ip, HREG(offset), cb, unit, value);
}

uint16_t ModbusIP::readIsts(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x07D0) return false;
	readSlave(offset, numregs, FC_READ_INPUT_STAT);
	return send(ip, ISTS(offset), cb, unit, value);
}

uint16_t ModbusIP::readIreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x007D) return false;
	readSlave(offset, numregs, FC_READ_INPUT_REGS);
	return send(ip, IREG(offset), cb, unit, value);
}

uint16_t ModbusIP::pushCoil(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x07D0) return false;
	if (!searchRegister(COIL(from))) return false;
	if (numregs == 1) {
		readSlave(to, COIL_VAL(Coil(from)), FC_WRITE_COIL);
	} else {
		writeSlaveBits(COIL(from), to, numregs, FC_WRITE_COILS);
	}
	return send(ip, COIL(from), cb, unit);
}

uint16_t ModbusIP::pullCoil(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x07D0) return false;
	#ifdef MODBUSIP_ADD_REG
	 addCoil(to, false, numregs);
	#endif
	readSlave(from, numregs, FC_READ_COILS);
	return send(ip, COIL(to), cb, unit);
}

uint16_t ModbusIP::pullIsts(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x07D0) return false;
	#ifdef MODBUSIP_ADD_REG
	 addIsts(to, false, numregs);
	#endif
	readSlave(from, numregs, FC_READ_INPUT_STAT);
	return send(ip, ISTS(to), cb, unit);
}

uint16_t ModbusIP::pushHreg(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x007D) return false;
	if (!searchRegister(HREG(from))) return false;
	if (numregs == 1) {
		readSlave(to, Hreg(from), FC_WRITE_REG);
	} else {
		writeSlaveWords(HREG(from), to, numregs, FC_WRITE_REGS);
	}
	return send(ip, HREG(from), cb, unit);
}

uint16_t ModbusIP::pullHreg(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x007D) return false;
	#ifdef MODBUSIP_ADD_REG
	 addHreg(to, 0, numregs);
	#endif
	readSlave(from, numregs, FC_READ_REGS);
	return send(ip, HREG(to), cb, unit);
}

uint16_t ModbusIP::pullIreg(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x007D) return false;
	#ifdef MODBUSIP_ADD_REG
	 addIreg(to, 0, numregs);
	#endif
	readSlave(from, numregs, FC_READ_INPUT_REGS);
	return send(ip, IREG(to), cb, unit);
}

uint16_t ModbusIP::pushIregToHreg(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x007D) return false;
	if (!searchRegister(IREG(from))) return false;
	if (numregs == 1) {
		readSlave(to, Ireg(from), FC_WRITE_REG);
	} else {
		writeSlaveWords(IREG(from), to, numregs, FC_WRITE_REGS);
	}
	return send(ip, IREG(from), cb, unit);
}

uint16_t ModbusIP::pushIstsToCoil(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x07D0) return false;
	if (!searchRegister(ISTS(from))) return false;
	if (numregs == 1) {
		readSlave(to, ISTS_VAL(Ists(from)), FC_WRITE_COIL);
	} else {
		writeSlaveBits(ISTS(from), to, numregs, FC_WRITE_COILS);
	}
	return send(ip, ISTS(from), cb, unit);
}

uint16_t ModbusIP::pullHregToIreg(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x007D) return false;
	#ifdef MODBUSIP_ADD_REG
	 addIreg(to, 0, numregs);
	#endif
	readSlave(from, numregs, FC_READ_REGS);
	return send(ip, IREG(to), cb, unit);
}

uint16_t ModbusIP::pullCoilToIsts(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs, cbTransaction cb, uint8_t unit) {
	if (numregs < 0x0001 || numregs > 0x07D0) return false;
	#ifdef MODBUSIP_ADD_REG
	 addIsts(to, false, numregs);
	#endif
	readSlave(from, numregs, FC_READ_COILS);
	return send(ip, ISTS(to), cb, unit);
}

bool ModbusIP::isTransaction(uint16_t id) {
	return searchTransaction(id) != nullptr;
}
bool ModbusIP::isConnected(IPAddress ip) {
	int8_t p = getSlave(ip);
	return  p != -1 && tcpclient[p]->connected();
}

void ModbusIP::autoConnect(bool enabled) {
	autoConnectMode = enabled;
}

bool ModbusIP::disconnect(IPAddress ip) {
	int8_t p = getSlave(ip);
	if (p != -1) {
		delete tcpclient[p];
		tcpclient[p] = nullptr;
	}
	return true;
}

void ModbusIP::dropTransactions() {
	for (auto &t : _trans) t.forcedEvent = EX_CANCEL;
}

ModbusIP::~ModbusIP() {
	free(_frame);
	dropTransactions();
	cleanupConnections();
	cleanupTransactions();
	delete tcpserver;
	tcpserver = nullptr;
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
		delete tcpclient[i];
		tcpclient[i] = nullptr;
	}
}

#endif