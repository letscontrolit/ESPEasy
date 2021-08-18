/*
    Modbus.cpp - Modbus Core Library Implementation
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017-2020 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "src/Modbus/Modbus.h"

#ifdef MB_GLOBAL_REGS
 std::vector<TRegister> _regs;
 std::vector<TCallback> _callbacks;
#endif

uint16_t Modbus::callback(TRegister* reg, uint16_t val, TCallback::CallbackType t) {
    uint16_t newVal = val;
    std::vector<TCallback>::iterator it = _callbacks.begin();
    do {
        it = std::find_if(it, _callbacks.end(), [reg, t](TCallback& cb){return cb.address == reg->address && cb.type == t;});
        if (it != _callbacks.end()) {
            newVal = it->cb(reg, newVal);
            it++;
        }
    } while (it != _callbacks.end());
    return newVal;
}

TRegister* Modbus::searchRegister(TAddress address) {
    std::vector<TRegister>::iterator it = std::find_if(_regs.begin(), _regs.end(), [address](TRegister& addr){return addr.address == address;});
    if (it != _regs.end()) return &*it;
    return nullptr;
}

bool Modbus::addReg(TAddress address, uint16_t value, uint16_t numregs) {
   #ifdef MB_MAX_REGS
    if (_regs.size() + numregs > MB_MAX_REGS) return false;
   #endif
    for (uint16_t i = 0; i < numregs; i++) {
        if (!searchRegister(address + i))
            _regs.push_back({address + i, value});
    }
    //std::sort(_regs.begin(), _regs.end());
    return true;
}

bool Modbus::Reg(TAddress address, uint16_t value) {
    TRegister* reg;
    reg = searchRegister(address); //search for the register address
    if (reg) { //if found then assign the register value to the new value.
        if (cbEnabled) {
            reg->value = callback(reg, value, TCallback::ON_SET);
        } else {
            reg->value = value;
        }
        return true;
    } else 
        return false;
}

uint16_t Modbus::Reg(TAddress address) {
    TRegister* reg;
    reg = searchRegister(address);
    if(reg)
        if (cbEnabled) {
            return callback(reg, reg->value, TCallback::ON_GET);
        } else {
            return reg->value;
        }
    else
        return 0;
}

bool Modbus::removeReg(TAddress address, uint16_t numregs) {
    TRegister* reg;
    bool atLeastOne = false;
    for (uint16_t i = 0; i < numregs; i++) {
        reg = searchRegister(address + i);
        if (reg) {
            atLeastOne = true;
            removeOnSet(address + i);
            removeOnGet(address + i);
            _regs.erase(std::remove( _regs.begin(), _regs.end(), *reg), _regs.end() );
        }
    }
    return atLeastOne;
}

void Modbus::slavePDU(uint8_t* frame) {
    FunctionCode fcode  = (FunctionCode)frame[0];
    uint16_t field1 = (uint16_t)frame[1] << 8 | (uint16_t)frame[2];
    uint16_t field2 = (uint16_t)frame[3] << 8 | (uint16_t)frame[4];
    uint16_t bytecount_calc;
    uint16_t k;
    switch (fcode) {
        case FC_WRITE_REG:
            //field1 = reg, field2 = value
            if (!Hreg(field1, field2)) { //Check Address and execute (reg exists?)
                exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                break;
            }
            if (Hreg(field1) != field2) { //Check for failure
                exceptionResponse(fcode, EX_SLAVE_FAILURE);
                break;
            }
            _reply = REPLY_ECHO;
        break;

        case FC_READ_REGS:
            //field1 = startreg, field2 = numregs, header len = 3
            readWords(HREG(field1), field2, fcode);
        break;

        case FC_WRITE_REGS:
            //field1 = startreg, field2 = numregs, frame[5] = data lenght, header len = 6
            if (field2 < 0x0001 || field2 > 0x007B || frame[5] != 2 * field2) { //Check value
                exceptionResponse(fcode, EX_ILLEGAL_VALUE);
                break;
            }
            for (k = 0; k < field2; k++) { //Check Address (startreg...startreg + numregs)
                if (!searchRegister(HREG(field1) + k)) {
                    exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                    break;
                }
            }
            if (k >= field2) {
                setMultipleWords((uint16_t*)(frame + 6), HREG(field1), field2);
                successResponce(HREG(field1), field2, fcode);
                _reply = REPLY_NORMAL;
            }
        break;

        case FC_READ_COILS:
            //field1 = startreg, field2 = numregs
            readBits(COIL(field1), field2, fcode);
        break;

        case FC_READ_INPUT_STAT:
            //field1 = startreg, field2 = numregs
            readBits(ISTS(field1), field2, fcode);
        break;

        case FC_READ_INPUT_REGS:
            //field1 = startreg, field2 = numregs
            readWords(IREG(field1), field2, fcode);
        break;

        case FC_WRITE_COIL:
            //field1 = reg, field2 = status, header len = 3
            if (field2 != 0xFF00 && field2 != 0x0000) { //Check value (status)
                exceptionResponse(fcode, EX_ILLEGAL_VALUE);
                break;
            }
            if (!Coil(field1, COIL_BOOL(field2))) { //Check Address and execute (reg exists?)
                exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                break;
            }
            if (Coil(field1) != COIL_BOOL(field2)) { //Check for failure
                exceptionResponse(fcode, EX_SLAVE_FAILURE);
                break;
            }
            _reply = REPLY_ECHO;
        break;

        case FC_WRITE_COILS:
            //field1 = startreg, field2 = numregs, frame[5] = bytecount, header len = 6
            bytecount_calc = field2 / 8;
            if (field2%8) bytecount_calc++;
            if (field2 < 0x0001 || field2 > 0x07B0 || frame[5] != bytecount_calc) { //Check registers range and data size maches
                exceptionResponse(fcode, EX_ILLEGAL_VALUE);
                break;
            }
            for (k = 0; k < field2; k++) { //Check Address (startreg...startreg + numregs)
                if (!searchRegister(COIL(field1) + k)) {
                    exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
                    break;
                }
            }
            if (k >= field2) {
                setMultipleBits(frame + 6, COIL(field1), field2);
                successResponce(COIL(field1), field2, fcode);
                _reply = REPLY_NORMAL;
            }
        break;

        default:
            exceptionResponse(fcode, EX_ILLEGAL_FUNCTION);
    }
}

void Modbus::successResponce(TAddress startreg, uint16_t numoutputs, FunctionCode fn) {
    free(_frame);
	_len = 5;
    _frame = (uint8_t*) malloc(_len);
    _frame[0] = fn;
    _frame[1] = startreg.address >> 8;
    _frame[2] = startreg.address & 0x00FF;
    _frame[3] = numoutputs >> 8;
    _frame[4] = numoutputs & 0x00FF;
}

void Modbus::exceptionResponse(FunctionCode fn, ResultCode excode) {
    free(_frame);
    _len = 2;
    _frame = (uint8_t*) malloc(_len);
    _frame[0] = fn + 0x80;
    _frame[1] = excode;
    _reply = REPLY_NORMAL;
}

void Modbus::getMultipleBits(uint8_t* frame, TAddress startreg, uint16_t numregs) {
    uint8_t bitn = 0;
    uint16_t i = 0;
	while (numregs--) {
		if (BIT_BOOL(Reg(startreg)))
			bitSet(frame[i], bitn);
        else
			bitClear(frame[i], bitn);
		bitn++; //increment the bit index
		if (bitn == 8)  {
            i++;
            bitn = 0;
        }
		startreg++; //increment the register
	}
}

void Modbus::getMultipleWords(uint16_t* frame, TAddress startreg, uint16_t numregs) {
    for (uint8_t i = 0; i < numregs; i++) {
        frame[i] = __swap_16(Reg(startreg + i));
    }
}

void Modbus::readBits(TAddress startreg, uint16_t numregs, FunctionCode fn) {
    if (numregs < 0x0001 || numregs > 0x07D0) { //Check value (numregs)
        exceptionResponse(fn, EX_ILLEGAL_VALUE);
        return;
    }
    //Check Address
    //Check only startreg. Is this correct?
    //When I check all registers in range I got errors in ScadaBR
    //I think that ScadaBR request more than one in the single request
    //when you have more then one datapoint configured from same type.
    if (!searchRegister(startreg)) {
        exceptionResponse(fn, EX_ILLEGAL_ADDRESS);
        return;
    }
    free(_frame);
    //Determine the message length = function type, byte count and
	//for each group of 8 registers the message length increases by 1
	_len = 2 + numregs/8;
	if (numregs % 8) _len++; //Add 1 to the message length for the partial byte.
    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {
        exceptionResponse(fn, EX_SLAVE_FAILURE);
        return;
    }
    _frame[0] = fn;
    _frame[1] = _len - 2; //byte count (_len - function code and byte count)
	_frame[_len - 1] = 0;  //Clean last probably partial byte
    getMultipleBits(_frame+2, startreg, numregs);
    _reply = REPLY_NORMAL;
}

void Modbus::readWords(TAddress startreg, uint16_t numregs, FunctionCode fn) {
    //Check value (numregs)
    if (numregs < 0x0001 || numregs > 0x007D) {
        exceptionResponse(fn, EX_ILLEGAL_VALUE);
        return;
    }
    if (!searchRegister(startreg)) { //Check Address
        exceptionResponse(fn, EX_ILLEGAL_ADDRESS);
        return;
    }
    free(_frame);
	_len = 2 + numregs * 2; //calculate the query reply message length. 2 bytes per register + 2 bytes for header
    _frame = (uint8_t*) malloc(_len);
    if (!_frame) {
        exceptionResponse(fn, EX_SLAVE_FAILURE);
        return;
    }
    _frame[0] = fn;
    _frame[1] = _len - 2;   //byte count
    getMultipleWords((uint16_t*)(_frame + 2), startreg, numregs);
    _reply = REPLY_NORMAL;
}

void Modbus::setMultipleBits(uint8_t* frame, TAddress startreg, uint16_t numoutputs) {
    uint8_t bitn = 0;
    uint16_t i = 0;
	while (numoutputs--) {
        Reg(startreg, BIT_VAL(bitRead(frame[i], bitn)));
        bitn++;     //increment the bit index
        if (bitn == 8) {
            i++;
            bitn = 0;
        }
        startreg++; //increment the register
	}
}

void Modbus::setMultipleWords(uint16_t* frame, TAddress startreg, uint16_t numregs) {
    for (uint8_t i = 0; i < numregs; i++) {
        Reg(startreg + i, __swap_16(frame[i]));
    }
}

bool Modbus::onGet(TAddress address, cbModbus cb, uint16_t numregs) {
	TRegister* reg;
	bool atLeastOne = false;
    if (!cb) {
        return removeOnGet(address);
    }
	while (numregs > 0) {
		reg = searchRegister(address);
		if (reg) {
            _callbacks.push_back({TCallback::ON_GET, address, cb});
			atLeastOne = true;
		}
		address++;
		numregs--;
	}
	return atLeastOne;
}
bool Modbus::onSet(TAddress address, cbModbus cb, uint16_t numregs) {
	TRegister* reg;
	bool atLeastOne = false;
    if (!cb) {
        return removeOnGet(address);
    }
	while (numregs > 0) {
		reg = searchRegister(address);
		if (reg) {
            _callbacks.push_back({TCallback::ON_SET, address, cb});
			atLeastOne = true;
		}
		address++;
		numregs--;
	}
	return atLeastOne;
}

bool Modbus::removeOnSet(TAddress address, cbModbus cb, uint16_t numregs) {
    while(numregs--) {
        _callbacks.erase(remove_if(_callbacks.begin(), _callbacks.end(), [address, cb](TCallback entry){
                        return entry.type == TCallback::ON_SET && entry.address == address && (!cb || entry.cb == cb);} ), _callbacks.end() );
        address++;
    }
    return false;
}
bool Modbus::removeOnGet(TAddress address, cbModbus cb, uint16_t numregs) {
    while(numregs--) {
        _callbacks.erase(remove_if(_callbacks.begin(), _callbacks.end(), [address, cb](TCallback entry){
                        return entry.type == TCallback::ON_GET && entry.address == address && (!cb || entry.cb == cb);} ), _callbacks.end() );
        address++;
    }
    return false;
}

bool Modbus::readSlave(uint16_t address, uint16_t numregs, FunctionCode fn) {
	free(_frame);
	_len = 5;
	_frame = (uint8_t*) malloc(_len);
	_frame[0] = fn;
	_frame[1] = address >> 8;
	_frame[2] = address & 0x00FF;
	_frame[3] = numregs >> 8;
	_frame[4] = numregs & 0x00FF;
	return true;
}

bool Modbus::writeSlaveBits(TAddress startreg, uint16_t to, uint16_t numregs, FunctionCode fn, bool* data) {
	free(_frame);
	_len = 6 + numregs/8;
	if (numregs % 8) _len++; //Add 1 to the message length for the partial byte.
    _frame = (uint8_t*) malloc(_len);
    if (_frame) {
	    _frame[0] = fn;
	    _frame[1] = to >> 8;
	    _frame[2] = to & 0x00FF;
	    _frame[3] = numregs >> 8;
	    _frame[4] = numregs & 0x00FF;
        _frame[5] = _len - 6;
        _frame[_len - 1] = 0;  //Clean last probably partial byte
        if (data) {
            boolToBits(_frame + 6, data, numregs);
        } else {
            getMultipleBits(_frame + 6, startreg, numregs);
        }
        _reply = REPLY_NORMAL;
        return true;
    }
    _reply = REPLY_OFF;
	return false;
}

bool Modbus::writeSlaveWords(TAddress startreg, uint16_t to, uint16_t numregs, FunctionCode fn, uint16_t* data) {
	free(_frame);
	_len = 6 + 2 * numregs;
	_frame = (uint8_t*) malloc(_len);
    if (_frame) {
	    _frame[0] = fn;
	    _frame[1] = to >> 8;
	    _frame[2] = to & 0x00FF;
	    _frame[3] = numregs >> 8;
	    _frame[4] = numregs & 0x00FF;
        _frame[5] = _len - 6;
        if (data) {
            uint16_t* frame = (uint16_t*)(_frame + 6);
            for (uint8_t i = 0; i < numregs; i++) {
                frame[i] = __swap_16(data[i]);
            }
        } else {
            getMultipleWords((uint16_t*)(_frame + 6), startreg, numregs);
        }
        return true;
    }
    _reply = REPLY_OFF;
	return false;    
}

void Modbus::boolToBits(uint8_t* dst, bool* src, uint16_t numregs) {
    uint8_t bitn = 0;
    uint16_t i = 0;
    uint16_t j = 0;
	while (numregs--) {
		if (src[j])
			bitSet(dst[i], bitn);
        else
			bitClear(dst[i], bitn);
		bitn++; //increment the bit index
		if (bitn == 8)  {
            i++;
            bitn = 0;
        }
		j++; //increment the register
	}
}

void Modbus::bitsToBool(bool* dst, uint8_t* src, uint16_t numregs) {
    uint8_t bitn = 0;
    uint16_t i = 0;
    uint16_t j = 0;
	while (numregs--) {
        dst[j] = bitRead(src[i], bitn);
        bitn++;     //increment the bit index
        if (bitn == 8) {
            i++;
            bitn = 0;
        }
        j++; //increment the register
	}
}

void Modbus::masterPDU(uint8_t* frame, uint8_t* sourceFrame, TAddress startreg, void* output) {
    uint8_t fcode  = frame[0];
    _reply = EX_SUCCESS;
    if ((fcode & 0x80) != 0) {
	    _reply = frame[1];
	    return;
    }
    uint16_t field2 = (uint16_t)sourceFrame[3] << 8 | (uint16_t)sourceFrame[4];
    uint8_t bytecount_calc;
    switch (fcode) {
        case FC_READ_REGS:
        case FC_READ_INPUT_REGS:
            //field2 = numregs, frame[1] = data lenght, header len = 2
            if (frame[1] != 2 * field2) { //Check if data size matches
                _reply = EX_DATA_MISMACH;
                break;
            }
            if (output) {
                frame += 2;
                while(field2) {
                    *((uint16_t*)output) = __swap_16(*((uint16_t*)frame));
                    frame += 2;
                    output += 2;
                    field2--;
                }
            } else {
                setMultipleWords((uint16_t*)(frame + 2), startreg, field2);
            }
        break;
        case FC_READ_COILS:
        case FC_READ_INPUT_STAT:
            //field2 = numregs, frame[1] = data length, header len = 2
            bytecount_calc = field2 / 8;
            if (field2 % 8) bytecount_calc++;
            if (frame[1] != bytecount_calc) { // check if data size matches
                _reply = EX_DATA_MISMACH;
                break;
            }
            if (output) {
                bitsToBool((bool*)output, frame + 2, field2);
            } else {
                setMultipleBits(frame + 2, startreg, field2);
            }
        break;
        case FC_WRITE_REG:
        case FC_WRITE_REGS:
        case FC_WRITE_COIL:
        case FC_WRITE_COILS:
        break;
        default:
		    _reply = EX_GENERAL_FAILURE;
    }
}

void Modbus::cbEnable(bool state) {
    cbEnabled = state;
}
void Modbus::cbDisable() {
    cbEnable(false);
}

bool Modbus::addHreg(uint16_t offset, uint16_t value, uint16_t numregs) {
    return addReg(HREG(offset), value, numregs);
}
bool Modbus::Hreg(uint16_t offset, uint16_t value) {
    return Reg(HREG(offset), value);
}
uint16_t Modbus::Hreg(uint16_t offset) {
    return Reg(HREG(offset));
}
bool Modbus::removeHreg(uint16_t offset, uint16_t numregs) {
    return removeReg(HREG(offset), numregs);
}
bool Modbus::addCoil(uint16_t offset, bool value, uint16_t numregs) {
    return addReg(COIL(offset), COIL_VAL(value), numregs);
}
bool Modbus::addIsts(uint16_t offset, bool value, uint16_t numregs) {
    return addReg(ISTS(offset), ISTS_VAL(value), numregs);
}
bool Modbus::addIreg(uint16_t offset, uint16_t value, uint16_t numregs) {
    return addReg(IREG(offset), value, numregs);
}
bool Modbus::Coil(uint16_t offset, bool value) {
    return Reg(COIL(offset), COIL_VAL(value));
}
bool Modbus::Ists(uint16_t offset, bool value) {
    return Reg(ISTS(offset), ISTS_VAL(value));
}
bool Modbus::Ireg(uint16_t offset, uint16_t value) {
    return Reg(IREG(offset), value);
}
bool Modbus::Coil(uint16_t offset) {
    return COIL_BOOL(Reg(COIL(offset)));
}
bool Modbus::Ists(uint16_t offset) {
    return ISTS_BOOL(Reg(ISTS(offset)));
}
uint16_t Modbus::Ireg(uint16_t offset) {
    return Reg(IREG(offset));
}
bool Modbus::removeCoil(uint16_t offset, uint16_t numregs) {
    return removeReg(COIL(offset), numregs);
}
bool Modbus::removeIsts(uint16_t offset, uint16_t numregs) {
    return removeReg(ISTS(offset), numregs);
}
bool Modbus::removeIreg(uint16_t offset, uint16_t numregs) {
    return removeReg(IREG(offset), numregs);
}
bool Modbus::onGetCoil(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return onGet(COIL(offset), cb, numregs);
}
bool Modbus::onSetCoil(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return onSet(COIL(offset), cb, numregs);
}
bool Modbus::onGetHreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return onGet(HREG(offset), cb, numregs);
}
bool Modbus::onSetHreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return onSet(HREG(offset), cb, numregs);
}
bool Modbus::onGetIsts(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return onGet(ISTS(offset), cb, numregs);
}
bool Modbus::onSetIsts(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return onSet(ISTS(offset), cb, numregs);
}
bool Modbus::onGetIreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return onGet(IREG(offset), cb, numregs);
}
bool Modbus::onSetIreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return onSet(IREG(offset), cb, numregs);
}

bool Modbus::removeOnGetCoil(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return removeOnGet(COIL(offset), cb, numregs);
}
bool Modbus::removeOnSetCoil(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return removeOnSet(COIL(offset), cb, numregs);
}
bool Modbus::removeOnGetHreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return removeOnGet(HREG(offset), cb, numregs);
}
bool Modbus::removeOnSetHreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return removeOnSet(HREG(offset), cb, numregs);
}
bool Modbus::removeOnGetIsts(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return removeOnGet(ISTS(offset), cb, numregs);
}
bool Modbus::removeOnSetIsts(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return removeOnSet(ISTS(offset), cb, numregs);
}
bool Modbus::removeOnGetIreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return removeOnGet(IREG(offset), cb, numregs);
}
bool Modbus::removeOnSetIreg(uint16_t offset, cbModbus cb, uint16_t numregs) {
    return removeOnSet(IREG(offset), cb, numregs);
}

Modbus::~Modbus() {
    free(_frame);
}