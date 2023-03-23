// ----------------------------------------------------------------------------
// P144 "Dust - PM1006(K) (Vindriktning)"
// Header file for sensor abstraction
// 2022 By flashmark
// ----------------------------------------------------------------------------
#ifndef PLUGINSTRUCTS_P144_DATA_STRUCT_H
#define PLUGINSTRUCTS_P144_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P144
#define PLUGIN_144_DEBUG  false   // set to true for extra log info

// States for statemachine used to decode received message
typedef enum {
    PM1006_HEADER,    // Waiting for the message header character
    PM1006_LENGTH,    // Waiting for the message length
    PM1006_DATA,      // waiting for/receiving data
    PM1006_CHECK      // waiting for the checksum
} pm1006_state_t;

const int P144_bufferSize = 20;
struct P144_data_struct : public PluginTaskData_base {
  P144_data_struct() = default;
  virtual ~P144_data_struct() = default;
  bool     setSerial(ESPEasySerialPort portType, int8_t rxPin, int8_t txPin);  // setup serial port connected to the sensor
  void     disconnectSerial();  // Disconnect the serial port connected to the sensor
  bool     processSensor();     // Process received characters from sensor, must be called regularly
  int      getValue();          // Get the last value received from the senso

  private:
  bool     processRx(char c);   // Handle one received character according to protocol
  void     dump();              // Diagnostics, dump the serialRxBuffer
  char*    toHex(char c, char * ptr);           // Print an int in hexadecimal

  ESPeasySerial *easySerial = nullptr;          // Setial port object
  char serialRxBuffer[P144_bufferSize] = {0};   // Receive buffer for serial RX characters
  #ifdef PLUGIN_144_DEBUG
  char debugBuffer[3*P144_bufferSize] = {0};    // Buffer to build debugging string during reception
  #endif
  int rxChecksum = 0;     // Build checksum value during message processing
  int rxIndex = 0;        // Index in serialRxBuffer during message processing
  int rxlen = 0;          // Size of the received message as stated in the received message
  int pm25Value = 0;      // Last decoded pm25Value
  pm1006_state_t rxState = PM1006_HEADER;   // Message decoding state during message processing
};

#endif  // USES_P144
#endif  // PLUGINSTRUCTS_P144_DATA_STRUCT_H