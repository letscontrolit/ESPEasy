/*
    ModbusRTU Library for ESP8266/ESP32
    Copyright (C) 2019-2020 Alexander Emelianov (a.m.emelianov@gmail.com)
	https://github.com/emelianov/modbus-esp8266
	This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/
#pragma once
#include "src/Modbus/ModbusRTU.h"

// Table of CRC values
static const uint16_t _auchCRC[] PROGMEM = {
	0x0000, 0xC1C0, 0x81C1, 0x4001, 0x01C3, 0xC003, 0x8002, 0x41C2, 0x01C6, 0xC006, 0x8007, 0x41C7, 0x0005, 0xC1C5, 0x81C4,
	0x4004, 0x01CC, 0xC00C, 0x800D, 0x41CD, 0x000F, 0xC1CF, 0x81CE, 0x400E, 0x000A, 0xC1CA, 0x81CB, 0x400B, 0x01C9, 0xC009,
	0x8008, 0x41C8, 0x01D8, 0xC018, 0x8019, 0x41D9, 0x001B, 0xC1DB, 0x81DA, 0x401A, 0x001E, 0xC1DE, 0x81DF, 0x401F, 0x01DD,
	0xC01D, 0x801C, 0x41DC, 0x0014, 0xC1D4, 0x81D5, 0x4015, 0x01D7, 0xC017, 0x8016, 0x41D6, 0x01D2, 0xC012, 0x8013, 0x41D3,
	0x0011, 0xC1D1, 0x81D0, 0x4010, 0x01F0, 0xC030, 0x8031, 0x41F1, 0x0033, 0xC1F3, 0x81F2, 0x4032, 0x0036, 0xC1F6, 0x81F7,
	0x4037, 0x01F5, 0xC035, 0x8034, 0x41F4, 0x003C, 0xC1FC, 0x81FD, 0x403D, 0x01FF, 0xC03F, 0x803E, 0x41FE, 0x01FA, 0xC03A,
	0x803B, 0x41FB, 0x0039, 0xC1F9, 0x81F8, 0x4038, 0x0028, 0xC1E8, 0x81E9, 0x4029, 0x01EB, 0xC02B, 0x802A, 0x41EA, 0x01EE,
	0xC02E, 0x802F, 0x41EF, 0x002D, 0xC1ED, 0x81EC, 0x402C, 0x01E4, 0xC024, 0x8025, 0x41E5, 0x0027, 0xC1E7, 0x81E6, 0x4026,
	0x0022, 0xC1E2, 0x81E3, 0x4023, 0x01E1, 0xC021, 0x8020, 0x41E0, 0x01A0, 0xC060, 0x8061, 0x41A1, 0x0063, 0xC1A3, 0x81A2,
	0x4062, 0x0066, 0xC1A6, 0x81A7, 0x4067, 0x01A5, 0xC065, 0x8064, 0x41A4, 0x006C, 0xC1AC, 0x81AD, 0x406D, 0x01AF, 0xC06F,
	0x806E, 0x41AE, 0x01AA, 0xC06A, 0x806B, 0x41AB, 0x0069, 0xC1A9, 0x81A8, 0x4068, 0x0078, 0xC1B8, 0x81B9, 0x4079, 0x01BB,
	0xC07B, 0x807A, 0x41BA, 0x01BE, 0xC07E, 0x807F, 0x41BF, 0x007D, 0xC1BD, 0x81BC, 0x407C, 0x01B4, 0xC074, 0x8075, 0x41B5,
	0x0077, 0xC1B7, 0x81B6, 0x4076, 0x0072, 0xC1B2, 0x81B3, 0x4073, 0x01B1, 0xC071, 0x8070, 0x41B0, 0x0050, 0xC190, 0x8191,
	0x4051, 0x0193, 0xC053, 0x8052, 0x4192, 0x0196, 0xC056, 0x8057, 0x4197, 0x0055, 0xC195, 0x8194, 0x4054, 0x019C, 0xC05C,
	0x805D, 0x419D, 0x005F, 0xC19F, 0x819E, 0x405E, 0x005A, 0xC19A, 0x819B, 0x405B, 0x0199, 0xC059, 0x8058, 0x4198, 0x0188,
	0xC048, 0x8049, 0x4189, 0x004B, 0xC18B, 0x818A, 0x404A, 0x004E, 0xC18E, 0x818F, 0x404F, 0x018D, 0xC04D, 0x804C, 0x418C,
	0x0044, 0xC184, 0x8185, 0x4045, 0x0187, 0xC047, 0x8046, 0x4186, 0x0182, 0xC042, 0x8043, 0x4183, 0x0041, 0xC181, 0x8180,
	0x4040, 0x0000
};

uint16_t ModbusRTU::crc16(uint8_t address, uint8_t* frame, uint8_t pduLen) {
	uint8_t i = 0xFF ^ address;
	uint16_t val = pgm_read_word(_auchCRC + i);
    uint8_t CRCHi = 0xFF ^ highByte(val);	// Hi
    uint8_t CRCLo = lowByte(val);	//Low
    while (pduLen--) {
        i = CRCHi ^ *frame++;
        val = pgm_read_word(_auchCRC + i);
        CRCHi = CRCLo ^ highByte(val);	// Hi
        CRCLo = lowByte(val);	//Low
    }
    return (CRCHi << 8) | CRCLo;
}

void ModbusRTU::setBaudrate(uint32_t baud) {
    if (baud > 19200) {
        _t = 2;
    } else {
        _t = (35000/baud) + 1;
    }
}

bool ModbusRTU::begin(Stream* port) {
    _port = port;
    _t = 2;
    return true;
}

bool ModbusRTU::begin(HardwareSerial* port, int16_t txPin, bool direct) {
    uint32_t baud = 0;
    #if defined(ESP32) || defined(ESP8266)
    // baudRate() only available with ESP32+ESP8266
    baud = port->baudRate();
    #else
    baud = 9600;
    #endif
	setBaudrate(baud);
    #if defined(ESP8266)
    maxRegs = port->setRxBufferSize(MODBUS_MAX_FRAME) / 2 - 3;
    #endif
    _port = port;
    if (txPin >= 0) {
	    _txPin = txPin;
		_direct = direct;
        pinMode(_txPin, OUTPUT);
        digitalWrite(_txPin, _direct?LOW:HIGH);
    }
    return true;
}

#if defined(ESP8266)
bool ModbusRTU::begin(SoftwareSerial* port, int16_t txPin, bool direct) {
	uint32_t baud = port->baudRate();
    _port = port;
    if (txPin >= 0) {
		if (direct)	// If direct logic use SoftwareSerial transmit control
        	port->setTransmitEnablePin(txPin);
		else {
    		_txPin = txPin;
			_direct = direct;
        	pinMode(_txPin, OUTPUT);
        	digitalWrite(_txPin, _direct?LOW:HIGH);
		}
	}
	setBaudrate(baud);
    return true;
}
#endif

bool ModbusRTU::rawSend(uint8_t slaveId, uint8_t* frame, uint8_t len) {
    uint16_t newCrc = crc16(slaveId, frame, len);
    if (_txPin >= 0) {
        digitalWrite(_txPin, _direct?HIGH:LOW);
        delay(1);
    }
	#ifdef ESP32
	portENTER_CRITICAL(&mux);
	#endif
    _port->write(slaveId);  	//Send slaveId
    _port->write(frame, len); 	// Send PDU
    _port->write(newCrc >> 8);	//Send CRC
    _port->write(newCrc & 0xFF);//Send CRC
    _port->flush();
    if (_txPin >= 0)
        digitalWrite(_txPin, _direct?LOW:HIGH);
	#ifdef ESP32
    portEXIT_CRITICAL(&mux);
 	#endif
	return true;
}

bool ModbusRTU::send(uint8_t slaveId, TAddress startreg, cbTransaction cb, void* data, bool waitResponse) {
    bool result = false;
	if (!_slaveId) { // Check if waiting for previous request result
		rawSend(slaveId, _frame, _len);
		if (waitResponse) {
        	_slaveId = slaveId;
			_timestamp = millis();
			_cb = cb;
			_data = data;
			_sentFrame = _frame;
			_sentReg = startreg;
			_frame = nullptr;
		}
		result = true;
	}
	free(_frame);
	_frame = nullptr;
	_len = 0;
	return result;
}

void ModbusRTU::task() {
	#ifdef ESP32
	portENTER_CRITICAL(&mux);
	#endif
    if (_port->available() > _len) {
        _len = _port->available();
        t = millis();
		#ifdef ESP32
    	portEXIT_CRITICAL(&mux);
 		#endif
		return;
    }

	if (_len == 0) {
		#ifdef ESP32
    	portEXIT_CRITICAL(&mux);
 		#endif
		if (isMaster) cleanup();
		return;
	}

    if (millis() - t < _t) { // Wait data whitespace if there is data
		#ifdef ESP32
    	portEXIT_CRITICAL(&mux);
 		#endif
		return;
	}
	#ifdef ESP32
    portEXIT_CRITICAL(&mux);
 	#endif

    uint8_t address = _port->read(); //first byte of frame = address
    _len--; // Decrease by slaveId byte
    if (isMaster && _slaveId == 0) {    // Check if slaveId is set
        for (uint8_t i=0 ; i < _len ; i++) _port->read();   // Skip packet if is not expected
        _len = 0;
		if (isMaster) cleanup();
        return;
    }
    if (address != MODBUSRTU_BROADCAST && address != _slaveId) {     // SlaveId Check
        for (uint8_t i=0 ; i < _len ; i++) _port->read();   // Skip packet if SlaveId doesn't mach
        _len = 0;
		if (isMaster) cleanup();
        return;
    }

	free(_frame);	//Just in case
    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {  // Fail to allocate buffer
      for (uint8_t i=0 ; i < _len ; i++) _port->read(); // Skip packet if can't allocate buffer
      _len = 0;
	  if (isMaster) cleanup();
      return;
    }
    for (uint8_t i=0 ; i < _len ; i++) {
		_frame[i] = _port->read();   // read data + crc
		#if defined(MODBUSRTU_DEBUG)
		Serial.printf("%02X ", _frame[i]);
		#endif
	}
	#if defined(MODBUSRTU_DEBUG)
	Serial.println();
	#endif
	//_port->readBytes(_frame, _len);
    u_int frameCrc = ((_frame[_len - 2] << 8) | _frame[_len - 1]); // Last two byts = crc
    _len = _len - 2;    // Decrease by CRC 2 bytes
    if (frameCrc != crc16(address, _frame, _len)) {  // CRC Check
        free(_frame);
        _frame = nullptr;
		_len = 0;
		if (isMaster) cleanup();
        return;
    }
    if (isMaster) {
        _reply = EX_SUCCESS;
        if ((_frame[0] & 0x7F) == _sentFrame[0]) { // Check if function code the same as requested
			// Procass incoming frame as master
			masterPDU(_frame, _sentFrame, _sentReg, _data);
            if (_cb) {
			    _cb((ResultCode)_reply, 0, nullptr);
				_cb = nullptr;
		    }
            free(_sentFrame);
            _sentFrame = nullptr;
            _data = nullptr;
		    _slaveId = 0;
		}
        _reply = Modbus::REPLY_OFF;    // No reply if master
    } else {
        slavePDU(_frame);
        if (address == MODBUSRTU_BROADCAST)
			_reply = Modbus::REPLY_OFF;    // No reply for Broadcasts
    }
    if (_reply != Modbus::REPLY_OFF)
		rawSend(_slaveId, _frame, _len);
    // Cleanup
    free(_frame);
    _frame = nullptr;
    _len = 0;
	if (isMaster) cleanup();
}


bool ModbusRTU::cleanup() {
	// Remove timeouted request and forced event
	if (_slaveId && (millis() - _timestamp > MODBUSRTU_TIMEOUT)) {
		if (_cb) {
			_cb(Modbus::EX_TIMEOUT, 0, nullptr);
			_cb = nullptr;
		}
		free(_sentFrame);
        _sentFrame = nullptr;
        _data = nullptr;
		_slaveId = 0;
        return true;
	}
    return false;
}


uint16_t ModbusRTU::writeHreg(uint8_t slaveId, uint16_t offset, uint16_t value, cbTransaction cb) {
    readSlave(offset, value, FC_WRITE_REG);
	return send(slaveId, HREG(offset), cb, nullptr);
}

uint16_t ModbusRTU::writeCoil(uint8_t slaveId, uint16_t offset, bool value, cbTransaction cb) {
	readSlave(offset, COIL_VAL(value), FC_WRITE_COIL);
	return send(slaveId, COIL(offset), cb, nullptr);
}

uint16_t ModbusRTU::readCoil(uint8_t slaveId, uint16_t offset, bool* value, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > maxRegs << 4) return false;
	readSlave(offset, numregs, FC_READ_COILS);
	return send(slaveId, COIL(offset), cb, value);
}


uint16_t ModbusRTU::writeCoil(uint8_t slaveId, uint16_t offset, bool* value, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > 0x07D0) return false;
	writeSlaveBits(COIL(offset), offset, numregs, FC_WRITE_COILS, value);
	return send(slaveId, COIL(offset), cb, nullptr);
}


uint16_t ModbusRTU::writeHreg(uint8_t slaveId, uint16_t offset, uint16_t* value, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > 0x007D) return false;
	writeSlaveWords(HREG(offset), offset, numregs, FC_WRITE_REGS, value);
	return send(slaveId, HREG(offset), cb, nullptr);
}


uint16_t ModbusRTU::readHreg(uint8_t slaveId, uint16_t offset, uint16_t* value, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > maxRegs) return false;
	readSlave(offset, numregs, FC_READ_REGS);
	return send(slaveId, HREG(offset), cb, value);
}


uint16_t ModbusRTU::readIsts(uint8_t slaveId, uint16_t offset, bool* value, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > maxRegs << 4) return false;
	readSlave(offset, numregs, FC_READ_INPUT_STAT);
	return send(slaveId, ISTS(offset), cb, value);
}


uint16_t ModbusRTU::readIreg(uint8_t slaveId, uint16_t offset, uint16_t* value, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > maxRegs) return false;
	readSlave(offset, numregs, FC_READ_INPUT_REGS);
	return send(slaveId, IREG(offset), cb, value);
}


uint16_t ModbusRTU::pushCoil(uint8_t slaveId, uint16_t to, uint16_t from, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > 0x07D0) return false;
	if (!searchRegister(COIL(from))) return false;
	if (numregs == 1) {
		readSlave(to, COIL_VAL(Coil(from)), FC_WRITE_COIL);
	} else {
		writeSlaveBits(COIL(from), to, numregs, FC_WRITE_COILS);
	}
	return send(slaveId, COIL(from), cb);
}


uint16_t ModbusRTU::pullCoil(uint8_t slaveId, uint16_t from, uint16_t to, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > maxRegs << 4) return false;
	#ifdef MODBUSRTU_ADD_REG
	 addCoil(to, false, numregs);
	#endif
	readSlave(from, numregs, FC_READ_COILS);
	return send(slaveId, COIL(to), cb);
}


uint16_t ModbusRTU::pullIsts(uint8_t slaveId, uint16_t from, uint16_t to, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > maxRegs << 4) return false;
	#ifdef MODBUSRTU_ADD_REG
	 addIsts(to, false, numregs);
	#endif
	readSlave(from, numregs, FC_READ_INPUT_STAT);
	return send(slaveId, ISTS(to), cb);
}


uint16_t ModbusRTU::pushHreg(uint8_t slaveId, uint16_t to, uint16_t from, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > 0x007D) return false;
	if (!searchRegister(HREG(from))) return false;
	if (numregs == 1) {
		readSlave(to, Hreg(from), FC_WRITE_REG);
	} else {
		writeSlaveWords(HREG(from), to, numregs, FC_WRITE_REGS);
	}
	return send(slaveId, HREG(from), cb);
}


uint16_t ModbusRTU::pullHreg(uint8_t slaveId, uint16_t from, uint16_t to, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > maxRegs) return false;
	#ifdef MODBUSRTU_ADD_REG
	 addHreg(to, 0, numregs);
	#endif
	readSlave(from, numregs, FC_READ_REGS);
	return send(slaveId, HREG(to), cb);
}


uint16_t ModbusRTU::pullIreg(uint8_t slaveId, uint16_t from, uint16_t to, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > maxRegs) return false;
	#ifdef MODBUSRTU_ADD_REG
	 addIreg(to, 0, numregs);
	#endif
	readSlave(from, numregs, FC_READ_INPUT_REGS);
	return send(slaveId, IREG(to), cb);
}


uint16_t ModbusRTU::pushIregToHreg(uint8_t slaveId, uint16_t to, uint16_t from, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > 0x007D) return false;
	if (!searchRegister(IREG(from))) return false;
	if (numregs == 1) {
		readSlave(to, Ireg(from), FC_WRITE_REG);
	} else {
		writeSlaveWords(IREG(from), to, numregs, FC_WRITE_REGS);
	}
	return send(slaveId, IREG(from), cb);
}


uint16_t ModbusRTU::pushIstsToCoil(uint8_t slaveId, uint16_t to, uint16_t from, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > maxRegs << 4) return false;
	if (!searchRegister(ISTS(from))) return false;
	if (numregs == 1) {
		readSlave(to, ISTS_VAL(Ists(from)), FC_WRITE_COIL);
	} else {
		writeSlaveBits(ISTS(from), to, numregs, FC_WRITE_COILS);
	}
	return send(slaveId, ISTS(from), cb);
}


uint16_t ModbusRTU::pullHregToIreg(uint8_t slaveId, uint16_t from, uint16_t to, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > maxRegs) return false;
	#ifdef MODBUSRTU_ADD_REG
	 addIreg(to, 0, numregs);
	#endif
	readSlave(from, numregs, FC_READ_REGS);
	return send(slaveId, IREG(to), cb);
}


uint16_t ModbusRTU::pullCoilToIsts(uint8_t slaveId, uint16_t from, uint16_t to, uint16_t numregs, cbTransaction cb) {
	if (numregs < 0x0001 || numregs > maxRegs << 4) return false;
	#ifdef MODBUSRTU_ADD_REG
	 addIsts(to, false, numregs);
	#endif
	readSlave(from, numregs, FC_READ_COILS);
	return send(slaveId, ISTS(to), cb);
}