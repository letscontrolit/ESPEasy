#ifndef PLUGINSTRUCTS_P031_DATA_STRUCT_H
#define PLUGINSTRUCTS_P031_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P031

// #######################################################################################################
// #################### Plugin 031: SHT10/SHT11/SHT15 Temp/Humidity Sensor ###############################
// #######################################################################################################

# define P031_IDLE            0
# define P031_WAIT_TEMP       1
# define P031_WAIT_HUM        2
# define P031_MEAS_READY      3
# define P031_COMMAND_NO_ACK  4
# define P031_NO_DATA         5

// see https://github.com/letscontrolit/ESPEasy/issues/2444
# define P031_DELAY_LONGER_CABLES  delayMicroseconds(_clockdelay);
# define P031_MAX_CLOCK_DELAY  30 // delay of 10 usec is enough for a 30m CAT6 UTP cable.

class P031_data_struct : public PluginTaskData_base {
public:

  enum {
    SHT1X_CMD_MEASURE_TEMP = B00000011,
    SHT1X_CMD_MEASURE_RH   = B00000101,
    SHT1X_CMD_READ_STATUS  = B00000111,
    SHT1X_CMD_SOFT_RESET   = B00011110
  };

  P031_data_struct() = default;
  virtual ~P031_data_struct() = default;


  uint8_t init(uint8_t data_pin,
               uint8_t clock_pin,
               bool    pullUp,
               uint8_t clockdelay);

  bool process();
  void startMeasurement();

  bool measurementReady() const {
    return state == P031_MEAS_READY;
  }

  bool hasError() const {
    return state > P031_MEAS_READY;
  }

  void    resetSensor();

  uint8_t readStatus();

  void    sendCommand(const uint8_t cmd);

  int     readData(const int bits) const;

  uint8_t shiftIn(uint8_t dataPin,
                  uint8_t clockPin,
                  uint8_t bitOrder) const;

  void shiftOut(uint8_t dataPin,
                uint8_t clockPin,
                uint8_t bitOrder,
                uint8_t val) const;

  float tempC                   = 0.0f;
  float rhTrue                  = 0.0f;
  unsigned long sendCommandTime = 0;

  int input_mode      = 0;
  uint8_t _dataPin    = 0;
  uint8_t _clockPin   = 0;
  uint8_t state       = P031_IDLE;
  uint8_t _clockdelay = 0;
};


#endif // ifdef USES_P031

#endif // ifndef PLUGINSTRUCTS_P031_DATA_STRUCT_H
