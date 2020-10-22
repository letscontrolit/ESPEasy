#include "../DataStructs/Modbus.h"

#include "../DataStructs/ControllerSettingsStruct.h"
#include "../ESPEasyCore/ESPEasy_Log.h"

#ifdef USES_MODBUS

Modbus::Modbus() : ModbusClient(nullptr), errcnt(0), timeout(0),
  TXRXstate(MODBUS_IDLE), RXavailable(0), payLoad(0) {}

bool Modbus::begin(uint8_t function, uint8_t ModbusID, uint16_t ModbusRegister,  MODBUS_registerTypes_t type, char *IPaddress)
{
  currentRegister = ModbusRegister;
  currentFunction = function;
  incomingValue   = type;
  resultReceived  = false;
  ModbusClient    = new WiFiClient();
  ModbusClient->setNoDelay(true);
  ModbusClient->setTimeout(CONTROLLER_CLIENTTIMEOUT_DFLT);
  timeout = millis();
  ModbusClient->flush();

  if (ModbusClient->connected()) {
    LogString += F(" already connected. ");
  } else {
    LogString += F("connect: ");      LogString += IPaddress;

    if (ModbusClient->connect(IPaddress, 502) != 1) {
      LogString += F(" fail. ");
      TXRXstate  = MODBUS_IDLE;
      errcnt++;

      if (LogString.length() > 1) { addLog(LOG_LEVEL_DEBUG, LogString); }
      return false;
    }
  }
  LogString += F(" OK, sending read request: ");

  sendBuffer[6] = ModbusID;
  sendBuffer[7] = function;
  sendBuffer[8] = (ModbusRegister >> 8);
  sendBuffer[9] = (ModbusRegister & 0x00ff);

  if ((incomingValue == signed64) || (incomingValue == unsigned64)) {
    sendBuffer[11] = 4;
  }

  if ((incomingValue == signed32) || (incomingValue == unsigned32)) {
    sendBuffer[11] = 2;
  }

  if ((incomingValue == signed16) || (incomingValue == unsigned16)) {
    sendBuffer[11] = 1;
  }
  ModbusClient->flush();
  ModbusClient->write(&sendBuffer[0], sizeof(sendBuffer));

  for (unsigned int i = 0; i < sizeof(sendBuffer); i++) {
    LogString += ((unsigned int)(sendBuffer[i]));
    LogString += (" ");
  }
  TXRXstate = MODBUS_RECEIVE;

  if (LogString.length() > 1) { addLog(LOG_LEVEL_DEBUG, LogString); }
  return true;
}

bool Modbus::handle() {
  unsigned int RXavailable = 0;

  LogString = "";
  int64_t rxValue = 0;

  switch (TXRXstate) {
    case MODBUS_IDLE:

      // clean up;
      if (ModbusClient) {
        ModbusClient->flush();
        ModbusClient->stop();
        delete (ModbusClient);
        delay(1);
        ModbusClient = nullptr;
      }
      break;

    case MODBUS_RECEIVE:

      if  (hasTimeout()) { break; }

      if  (ModbusClient->available() < 9) { break; }

      LogString += F("reading bytes: ");

      for (int a = 0; a < 9; a++) {
        payLoad    = ModbusClient->read();
        LogString += (payLoad);  LogString += ' ';
      }
      LogString += F("> ");

      if (payLoad > 8) {
        LogString += F("Payload too large !? ");
        errcnt++;
        TXRXstate = MODBUS_IDLE;
      }

    case MODBUS_RECEIVE_PAYLOAD:

      if  (hasTimeout()) { break; }
      RXavailable = ModbusClient->available();

      if (payLoad != RXavailable) {
        TXRXstate = MODBUS_RECEIVE_PAYLOAD;
        break;
      }

      for (unsigned int i = 0; i < RXavailable; i++) {
        rxValue = rxValue << 8;
        char a = ModbusClient->read();
        rxValue    = rxValue | a;
        LogString += ((int)a);  LogString += (" ");
      }

      switch (incomingValue) {
        case signed16:
          result = (int16_t)rxValue;
          break;
        case unsigned16:
          result = (uint16_t)rxValue;
          break;
        case signed32:
          result = (int32_t)rxValue;
          break;
        case unsigned32:
          result = (uint32_t)rxValue;
          break;
        case signed64:
          result = (int64_t)rxValue;
          break;
        case unsigned64:
          result = (uint64_t)rxValue;
          break;
      }

      LogString += "value: "; LogString += result;

      // if ((systemTimePresent()) && (hour() == 0)) errcnt = 0;

      TXRXstate = MODBUS_IDLE;

      resultReceived = true;
      break;

    default:
      LogString += F("default. ");
      TXRXstate  = MODBUS_IDLE;
      break;
  }

  if (LogString.length() > 1) { addLog(LOG_LEVEL_DEBUG, LogString); }
  return true;
}

bool Modbus::hasTimeout()
{
  if   ((millis() - timeout) > 10000) { // too many bytes or timeout
    LogString += F("Modbus RX timeout. "); LogString += String(ModbusClient->available());
    errcnt++;
    TXRXstate = MODBUS_IDLE;
    return true;
  }
  return false;
}

// tryread can be called in a round robin fashion. It will initiate a read if Modbus is idle and update the result once it is available.
// subsequent calls (if Modbus is busy etc. ) will return false and not update the result.
// Use to read multiple values non blocking in an re-entrant function. Not tested yet.
bool Modbus::tryRead(uint8_t ModbusID, uint16_t M_register,  MODBUS_registerTypes_t type, char *IPaddress, double& result) {
  if (isBusy()) { return false; // not done yet
  }

  if (available()) {
    if ((currentFunction == MODBUS_FUNCTION_READ) && (currentRegister == M_register)) {
      result = read(); // result belongs to this request.
      return true;
    }
  } else {
    begin(MODBUS_FUNCTION_READ, ModbusID, M_register, type, IPaddress); // idle and no result -> begin read request
  }
  return false;
}

#endif // USES_MODBUS
