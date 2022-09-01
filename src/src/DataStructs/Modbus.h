#ifndef DATASTRUCTS_MODBUS_H
#define DATASTRUCTS_MODBUS_H

#include "../../ESPEasy_common.h"

#if FEATURE_MODBUS

enum MODBUS_states_t { MODBUS_IDLE, MODBUS_RECEIVE, MODBUS_RECEIVE_PAYLOAD };
enum MODBUS_registerTypes_t { signed16, unsigned16, signed32, unsigned32, signed64, unsigned64 };

# define MODBUS_FUNCTION_READ 4

class Modbus {
public:

  Modbus(void);
  ~Modbus();
  bool handle();
  bool begin(uint8_t                function,
             uint8_t                ModbusID,
             uint16_t               ModbusRegister,
             MODBUS_registerTypes_t type,
             char                  *IPaddress);
  double read() {
    if (resultReceived) {
      resultReceived = false;
      return result;
    }
    else {
      return -1;
    }
  }

  bool available() {
    return resultReceived;
  }

  unsigned int getReadErrors() {
    return errcnt;
  }

  void resetReadErrors() {
    errcnt = 0;
  }

  void stop() {
    TXRXstate = MODBUS_IDLE;
    handle();
  }

  bool tryRead(uint8_t                ModbusID,
               uint16_t               M_register,
               MODBUS_registerTypes_t type,
               char                  *IPaddress,
               double               & result);

private:

  WiFiClient *ModbusClient = nullptr;             // pointer to tcp client
  unsigned int errcnt = 0;
  char sendBuffer[12] =  { 0, 1, 0, 0, 0, 6, 0x7e, 4, 0x9d, 7, 0, 1 };
  #ifndef BUILD_NO_DEBUG
  String LogString;                     // for debug logging
  #endif
  unsigned long timeout = 0;                // send and read timeout
  MODBUS_states_t TXRXstate = MODBUS_IDLE;            // state for handle() state machine
  unsigned int RXavailable = 0;
  unsigned int payLoad = 0;                 // number of bytes to receive as payload. Payload may come as seperate frame.
  bool hasTimeout();
  MODBUS_registerTypes_t incomingValue = signed16; // how to interpret the incoming value
  double result = 0.0;                        // incoming value, converted to double
  bool resultReceived = false;                  // incoming value is valid ?
  bool isBusy(void) {
    return TXRXstate != MODBUS_IDLE;
  }

  uint16_t currentRegister = 0;
  uint8_t currentFunction = 0;
};


#endif // if FEATURE_MODBUS

#endif // ifndef DATASTRUCTS_MODBUS_H
