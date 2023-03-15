#ifndef PLUGINSTRUCTS_P134_DATA_STRUCT_H
#define PLUGINSTRUCTS_P134_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P134

# define P134_MIN_DISTANCE                   30
# define P134_MAX_DISTANCE                   4500
# define P134_SERIAL_BAUD_RATE               9600
# define P134_SERIAL_HEAD_DATA               0xFF
# define P134_SERIAL_AVAILABLE_CHECK_DELAY   50
# define P134_SERIAL_AVAILABLE_CHECK_CYCLES  10 // SERIAL_AVAILABLE_CHECK_DELAY * SERIAL_AVAILABLE_CHECK_CYCLES = 500ms
# define P134_DISTANCE_DATA_SIZE             4

enum class A02YYUW_status_e : int8_t {
  STATUS_OK              = 0,
  STATUS_ERROR_CHECK_SUM = -1,
  STATUS_ERROR_MAX_LIMIT = -2,
  STATUS_ERROR_MIN_LIMIT = -3,
  STATUS_ERROR_SERIAL    = -4
};

# include <ESPeasySerial.h>

// # define PLUGIN_134_DEBUG true // Set to true to enable extra log output (info)

# ifndef LIMIT_BUILD_SIZE
const __FlashStringHelper* toString(A02YYUW_status_e status);
# endif // ifndef LIMIT_BUILD_SIZE


struct P134_data_struct : public PluginTaskData_base {
public:

  P134_data_struct(uint8_t config_port,
                   int8_t  config_pin1,
                   int8_t  config_pin2);

  P134_data_struct() = delete;
  virtual ~P134_data_struct();

  bool plugin_read(struct EventStruct *event);
  bool isInitialized() const {
    return P134_Serial != nullptr;
  }

private:

  ESPeasySerial *P134_Serial = nullptr;

  const uint8_t _config_port;
  const int8_t  _config_pin1;
  const int8_t  _config_pin2;
};

#endif // ifdef USES_P134
#endif // ifndef PLUGINSTRUCTS_P134_DATA_STRUCT_H
