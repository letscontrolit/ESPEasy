/*
    Modbus.h - Header for Modbus Core Library
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017-2020 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#pragma once

#include "Arduino.h"
#include <vector>
#include <algorithm>
#ifdef ARDUINO_ARCH_ESP32
 #include <byteswap.h>
#endif

static inline uint16_t __swap_16(uint16_t num) { return (num >> 8) | (num << 8); }

#define MB_GLOBAL_REGS
//#define MB_MAX_REGS     32
#define MODBUS_MAX_FRAME   256
#define COIL(n) (TAddress){TAddress::COIL, n}
#define ISTS(n) (TAddress){TAddress::ISTS, n}
#define IREG(n) (TAddress){TAddress::IREG, n}
#define HREG(n) (TAddress){TAddress::HREG, n}
#define BIT_VAL(v) (v?0xFF00:0x0000)
#define BIT_BOOL(v) (v==0xFF00)
#define COIL_VAL(v) (v?0xFF00:0x0000)
#define COIL_BOOL(v) (v==0xFF00)
#define ISTS_VAL(v) (v?0xFF00:0x0000)
#define ISTS_BOOL(v) (v==0xFF00)

// For depricated (v1.xx) onSet/onGet format compatibility
#define cbDefault nullptr

struct TRegister;

typedef uint16_t (*cbModbus)(TRegister* reg, uint16_t val); // Callback function Type

struct TAddress {
    enum RegType {COIL, ISTS, IREG, HREG};
    RegType type;
    uint16_t address;
    bool operator==(const TAddress &obj) const { // TAddress == TAddress
	    return type == obj.type && address == obj.address;
	}
    bool operator!=(const TAddress &obj) const { // TAddress != TAddress
        return type != obj.type || address != obj.address;
    }
    TAddress& operator++() {     // ++TAddress
        address++;
        return *this;
    }
    TAddress  operator++(int) {  // TAddress++
        TAddress result(*this);
         ++(*this);
        return result;
    }
    TAddress& operator+=(const int& inc) {  // TAddress += integer
        address += inc;
        return *this;
    }
    const TAddress operator+(const int& inc) const {    // TAddress + integer
        TAddress result(*this);
        result.address += inc;
        return result;
    }
    bool isCoil() {
       return type == COIL;
    }
    bool isIsts() {
       return type == ISTS;
    }
    bool isIreg() {
        return type == IREG;
    }
    bool isHreg() {
        return type == HREG;
    }
};

struct TCallback {
    enum CallbackType {ON_SET, ON_GET};
    CallbackType type;
    TAddress    address;
    cbModbus    cb;
};

struct TRegister {
    TAddress    address;
    uint16_t value;
    bool operator ==(const TRegister &obj) const {
	    return address == obj.address;
	}
};

class Modbus {
    public:
        //Function Codes
        enum FunctionCode {
            FC_READ_COILS       = 0x01, // Read Coils (Output) Status
            FC_READ_INPUT_STAT  = 0x02, // Read Input Status (Discrete Inputs)
            FC_READ_REGS        = 0x03, // Read Holding Registers
            FC_READ_INPUT_REGS  = 0x04, // Read Input Registers
            FC_WRITE_COIL       = 0x05, // Write Single Coil (Output)
            FC_WRITE_REG        = 0x06, // Preset Single Register
            FC_DIAGNOSTICS      = 0x08, // Not implemented. Diagnostics (Serial Line only)
            FC_WRITE_COILS      = 0x0F, // Write Multiple Coils (Outputs)
            FC_WRITE_REGS       = 0x10, // Write block of contiguous registers
            FC_READ_FILE_REC    = 0x14, // Not implemented. Read File Record
            FC_WRITE_FILE_REC   = 0x15, // Not implemented. Write File Record
            FC_MASKWRITE_REG    = 0x16, // Not implemented. Mask Write Register
            FC_READWRITE_REGS   = 0x17  // Not implemented. Read/Write Multiple registers
        };
        //Exception Codes
        //Custom result codes used internally and for callbacks but never used for Modbus responce
        enum ResultCode {
            EX_SUCCESS              = 0x00, // Custom. No error
            EX_ILLEGAL_FUNCTION     = 0x01, // Function Code not Supported
            EX_ILLEGAL_ADDRESS      = 0x02, // Output Address not exists
            EX_ILLEGAL_VALUE        = 0x03, // Output Value not in Range
            EX_SLAVE_FAILURE        = 0x04, // Slave or Master Device Fails to process request
            EX_ACKNOWLEDGE          = 0x05, // Not used
            EX_SLAVE_DEVICE_BUSY    = 0x06, // Not used
            EX_MEMORY_PARITY_ERROR  = 0x08, // Not used
            EX_PATH_UNAVAILABLE     = 0x0A, // Not used
            EX_DEVICE_FAILED_TO_RESPOND = 0x0B, // Not used
            EX_GENERAL_FAILURE      = 0xE1, // Custom. Unexpected master error
            EX_DATA_MISMACH         = 0xE2, // Custom. Inpud data size mismach
            EX_UNEXPECTED_RESPONSE  = 0xE3, // Custom. Returned result doesn't mach transaction
            EX_TIMEOUT              = 0xE4, // Custom. Operation not finished within reasonable time
            EX_CONNECTION_LOST      = 0xE5, // Custom. Connection with device lost
            EX_CANCEL               = 0xE6  // Custom. Transaction/request canceled
        };
        ~Modbus();
        bool addHreg(uint16_t offset, uint16_t value = 0, uint16_t numregs = 1);
        bool Hreg(uint16_t offset, uint16_t value);
        uint16_t Hreg(uint16_t offset);
        bool removeHreg(uint16_t offset, uint16_t numregs = 1);
        bool addCoil(uint16_t offset, bool value = false, uint16_t numregs = 1);
        bool addIsts(uint16_t offset, bool value = false, uint16_t numregs = 1);
        bool addIreg(uint16_t offset, uint16_t value = 0, uint16_t numregs = 1);
        bool Coil(uint16_t offset, bool value);
        bool Ists(uint16_t offset, bool value);
        bool Ireg(uint16_t offset, uint16_t value);
        bool Coil(uint16_t offset);
        bool Ists(uint16_t offset);
        uint16_t Ireg(uint16_t offset);
        bool removeCoil(uint16_t offset, uint16_t numregs = 1);
        bool removeIsts(uint16_t offset, uint16_t numregs = 1);
        bool removeIreg(uint16_t offset, uint16_t numregs = 1);
        /*
        bool Hreg(uint16_t offset, uint16_t* value);
        bool Coil(uint16_t offset, bool* value);
        bool Ists(uint16_t offset, bool* value);
        bool Ireg(uint16_t offset, uint16_t* value);
        */
        void cbEnable(bool state = true);
        void cbDisable();
        
        bool onGetCoil(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool onSetCoil(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool onGetHreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool onSetHreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool onGetIsts(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool onSetIsts(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool onGetIreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool onSetIreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);

        bool removeOnGetCoil(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool removeOnSetCoil(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool removeOnGetHreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool removeOnSetHreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool removeOnGetIsts(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool removeOnSetIsts(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool removeOnGetIreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool removeOnSetIreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);

    private:
	    void readBits(TAddress startreg, uint16_t numregs, FunctionCode fn);
	    void readWords(TAddress startreg, uint16_t numregs, FunctionCode fn);
        
        void setMultipleBits(uint8_t* frame, TAddress startreg, uint16_t numoutputs);
        void setMultipleWords(uint16_t* frame, TAddress startreg, uint16_t numoutputs);
        
        void getMultipleBits(uint8_t* frame, TAddress startreg, uint16_t numregs);
        void getMultipleWords(uint16_t* frame, TAddress startreg, uint16_t numregs);

        void bitsToBool(bool* dst, uint8_t* src, uint16_t numregs);
        void boolToBits(uint8_t* dst, bool* src, uint16_t numregs);
    
    protected:
        //Reply Types
        enum ReplyCode {
            REPLY_OFF            = 0x01,
            REPLY_ECHO           = 0x02,
            REPLY_NORMAL         = 0x03,
            REPLY_ERROR          = 0x04,
            REPLY_UNEXPECTED     = 0x05
        };
    #ifndef MB_GLOBAL_REGS
        std::vector<TRegister> _regs;
        std::vector<TCallback> _callbacks;
    #endif
        uint8_t*  _frame = nullptr;
        uint16_t  _len = 0;
        uint8_t   _reply = 0;
        bool cbEnabled = true;
        uint16_t callback(TRegister* reg, uint16_t val, TCallback::CallbackType t);
        TRegister* searchRegister(TAddress addr);
        void exceptionResponse(FunctionCode fn, ResultCode excode); // Fills _frame with response
        void successResponce(TAddress startreg, uint16_t numoutputs, FunctionCode fn);  // Fills frame with response
        void slavePDU(uint8_t* frame);    //For Slave
        void masterPDU(uint8_t* frame, uint8_t* sourceFrame, TAddress startreg, void* output = nullptr);   //For Master
        // frame - data received form slave
        // sourceFrame - data have sent fo slave
        // startreg - local register to start put data to
        // output - if not null put data to the buffer insted local registers. output assumed to by array of uint16_t or boolean

        bool readSlave(uint16_t address, uint16_t numregs, FunctionCode fn);
        bool writeSlaveBits(TAddress startreg, uint16_t to, uint16_t numregs, FunctionCode fn, bool* data = nullptr);
        bool writeSlaveWords(TAddress startreg, uint16_t to, uint16_t numregs, FunctionCode fn, uint16_t* data = nullptr);
        // startreg - local register to get data from
        // to - slave register to write data to
        // numregs - number of registers
        // fn - Modbus function
        // data - if null use local registers. Otherwise use data from array to erite to slave

        bool addReg(TAddress address, uint16_t value = 0, uint16_t numregs = 1);
        bool Reg(TAddress address, uint16_t value);
        uint16_t Reg(TAddress address);
        bool removeReg(TAddress address, uint16_t numregs = 1);

        bool onGet(TAddress address, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool onSet(TAddress address, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool removeOnSet(TAddress address, cbModbus cb = nullptr, uint16_t numregs = 1);
        bool removeOnGet(TAddress address, cbModbus cb = nullptr, uint16_t numregs = 1);

        virtual uint32_t eventSource() {return 0;}
};

typedef bool (*cbTransaction)(Modbus::ResultCode event, uint16_t transactionId, void* data); // Callback skeleton for requests
