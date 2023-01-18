#include "../PluginStructs/P134_data_struct.h"

#ifdef USES_P134

# ifndef LIMIT_BUILD_SIZE
const __FlashStringHelper* toString(A02YYUW_status_e status) {
  switch (status) {
    case A02YYUW_status_e::STATUS_OK: return F("OK");
    case A02YYUW_status_e::STATUS_ERROR_CHECK_SUM: return F("Checksum error");
    case A02YYUW_status_e::STATUS_ERROR_MAX_LIMIT: return F("Distance over max. limit");
    case A02YYUW_status_e::STATUS_ERROR_MIN_LIMIT: return F("Distance under min. limit");
    case A02YYUW_status_e::STATUS_ERROR_SERIAL: return F("Serial communication error");
  }
  return F("Unknown");
}

# endif // ifndef LIMIT_BUILD_SIZE

/**************************************************************************
* Constructor
**************************************************************************/
P134_data_struct::P134_data_struct(uint8_t config_port,
                                   int8_t  config_pin1,
                                   int8_t  config_pin2)
  : _config_port(config_port), _config_pin1(config_pin1), _config_pin2(config_pin2)
{
  const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(_config_port);

  P134_Serial = new (std::nothrow) ESPeasySerial(port, _config_pin1, _config_pin2);

  if (P134_Serial != nullptr) {
    P134_Serial->begin(P134_SERIAL_BAUD_RATE);
    P134_Serial->flush();
    addLog(LOG_LEVEL_INFO, F("A02YYUW: Initialization OK"));
  } else {
    addLog(LOG_LEVEL_ERROR, F("A02YYUW: Initialization FAILED"));
  }
}

/*****************************************************
* Destructor
*****************************************************/
P134_data_struct::~P134_data_struct() {
  delete P134_Serial;
  P134_Serial = nullptr;
}

/*****************************************************
* plugin_read
*****************************************************/
bool P134_data_struct::plugin_read(struct EventStruct *event)           {
  bool success = false;

  if (isInitialized()) {
    uint8_t  data[P134_DISTANCE_DATA_SIZE] = { 0 };
    int16_t  i                             = 0;
    uint16_t measuredDistance              = P134_MIN_DISTANCE;
    A02YYUW_status_e measurementStatus;
    P134_Serial->flush();

    while (!P134_Serial->available() && i < P134_SERIAL_AVAILABLE_CHECK_CYCLES) {
      i++;
      // FIXME TD-er: Why making such a lousy check for data holding up everything running on the ESP node.
      // Repeat after me: "Thou shall not use delay()!"
      delay(P134_SERIAL_AVAILABLE_CHECK_DELAY);
    }
    i = 0;

    while (P134_Serial->available() && i < P134_DISTANCE_DATA_SIZE) {
      // FIXME TD-er: This does not have a timeout => Will crash the node when only data is received != P134_SERIAL_HEAD_DATA
      // Reading bytes should be done from the PLUGIN_TEN_PER_SECOND or something similar.
      data[i] = P134_Serial->read();
      i++;

      if (data[0] != P134_SERIAL_HEAD_DATA) {
        i = 0;
      }
    }

    if (i != P134_DISTANCE_DATA_SIZE) {
      measurementStatus = A02YYUW_status_e::STATUS_ERROR_SERIAL;
    } else {
      if (!(((data[0] + data[1] + data[2]) & 0xFF) == data[3])) {
        measurementStatus = A02YYUW_status_e::STATUS_ERROR_CHECK_SUM;
      } else {
        measuredDistance = ((data[1] << 8) + data[2]);

        if (measuredDistance < P134_MIN_DISTANCE) {
          measurementStatus = A02YYUW_status_e::STATUS_ERROR_MIN_LIMIT;
        }
        else if (measuredDistance > P134_MAX_DISTANCE) {
          measurementStatus = A02YYUW_status_e::STATUS_ERROR_MAX_LIMIT;
        } else {
          measurementStatus = A02YYUW_status_e::STATUS_OK;
        }
      }
    }

    if (measurementStatus == A02YYUW_status_e::STATUS_OK) {
      UserVar[event->BaseVarIndex] = static_cast<float>(measuredDistance);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLogMove(LOG_LEVEL_INFO, concat(F("A02YYUW: Distance value = "), static_cast<int>(measuredDistance)));
      }
      success = true;
    } else {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String log;
        log += concat(F("A02YYUW: Error status = "), static_cast<int>(measurementStatus));
        # ifndef LIMIT_BUILD_SIZE
        log += concat(F(", "), toString(measurementStatus));
        # endif // ifndef LIMIT_BUILD_SIZE
        addLogMove(LOG_LEVEL_ERROR, log);
      }
    }
  }
  return success;
}

#endif // ifdef USES_P134
