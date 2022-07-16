#include "../PluginStructs/P075_data_struct.h"

#ifdef USES_P075


P075_data_struct::P075_data_struct(ESPEasySerialPort port, int rx, int tx, uint32_t baud) : rxPin(rx), txPin(tx), baudrate(baud) {
  if ((baudrate < 9600) || (baudrate > 115200)) {
    baudrate = 9600;
  }
  easySerial = new (std::nothrow) ESPeasySerial(port, rx, tx, false, RXBUFFSZ);

  if (easySerial != nullptr) {
    easySerial->begin(baudrate);
    easySerial->flush();
  }
}

P075_data_struct::~P075_data_struct() {
  if (easySerial != nullptr) {
    easySerial->flush();
    delete easySerial;
    easySerial = nullptr;
  }
}

void P075_data_struct::loadDisplayLines(taskIndex_t taskIndex) {
  LoadCustomTaskSettings(taskIndex, displayLines, P75_Nlines, P75_Nchars);
}

String P075_data_struct::getLogString() const {
  String result;

  if (easySerial != nullptr) {
    result = easySerial->getLogString();
  }
  return result;
}

#endif // ifdef USES_P075
