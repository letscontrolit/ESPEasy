#include "../PluginStructs/P153_data_struct.h"

#ifdef USES_P153

/**************************************************************************
* Constructor
**************************************************************************/
P153_data_struct::P153_data_struct(uint8_t              address,
                                   float                tempOffset,
                                   P153_configuration_e startupConfiguration,
                                   P153_configuration_e normalConfiguration,
                                   uint16_t             intervalLoops) :
  _address(address), _tempOffset(tempOffset), _startupConfiguration(startupConfiguration),
  _normalConfiguration(normalConfiguration), _intervalLoops(intervalLoops), initialized(false)
{}

bool P153_data_struct::init() {
  // - Read sensor serial number
  if (I2C_wakeup(_address) == 0) {
    if (I2C_write8(_address, P153_SHT4X_RESET)) {
      delay(1);

      uint8_t data[6]{};

      if (I2C_write8(_address, P153_SHT4X_READ_SERIAL)) {
        delay(10);

        Wire.requestFrom(_address, (uint8_t)6);

        for (uint8_t d = 0; d < 6; d++) {
          data[d] = Wire.read();
        }

        if (CRC8(data[0], data[1], data[2]) && CRC8(data[3], data[4], data[5])) {
          serialNumber   = data[0];
          serialNumber <<= 8;
          serialNumber  |= data[1];
          serialNumber <<= 8;
          serialNumber  |= data[3];
          serialNumber <<= 8;
          serialNumber  |= data[4];
          addLog(LOG_LEVEL_INFO, concat(F("SHT4x: Serial number: "), formatToHex_decimal(serialNumber)));
          initialized = true;
        } else {
          // crc error
          addLog(LOG_LEVEL_ERROR, F("SHT4x: Error reading serial number."));
        }
      }
    }
  }

  return isInitialized();
}

/*****************************************************
* plugin_read
*****************************************************/
bool P153_data_struct::plugin_read(struct EventStruct *event)           {
  bool success = false;

  if (isInitialized()) {
    int16_t timeDelay = -1;

    // read in stages
    if (P153_read_mode_e::Idle == readMode) {
      // Determine delay per command

      switch ((_intervalLoops > 0) ? _startupConfiguration : _normalConfiguration) {
        case P153_configuration_e::LowResolution:
          timeDelay = P153_DELAY_LOW_RESOLUTION;
          break;
        case P153_configuration_e::MediumResolution:
          timeDelay = P153_DELAY_MEDIUM_RESOLUTION;
          break;
        case P153_configuration_e::HighResolution:
          timeDelay = P153_DELAY_HIGH_RESOLUTION;
          break;
        case P153_configuration_e::HighResolution200mW100msec:
        case P153_configuration_e::HighResolution110mW100msec:
        case P153_configuration_e::HighResolution20mW100msec:
          timeDelay = P153_DELAY_100MS_HEATER;
          break;
        case P153_configuration_e::HighResolution200mW1000msec:
        case P153_configuration_e::HighResolution110mW1000msec:
        case P153_configuration_e::HighResolution20mW1000msec:
          timeDelay = P153_DELAY_1S_HEATER;
          break;
      }

      // Start measurement
      if (!I2C_write8(_address, static_cast<uint8_t>((_intervalLoops > 0) ? _startupConfiguration : _normalConfiguration))) {
        timeDelay = -1; // Don't continue if writing command fails

        UserVar.setFloat(event->TaskIndex, 0, NAN);
        UserVar.setFloat(event->TaskIndex, 1, NAN);

        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log;

          if (log.reserve(40)) {
            log  = getTaskDeviceName(event->TaskIndex);
            log += F(": Error writing command to sensor");
            addLogMove(LOG_LEVEL_ERROR, log);
          }
        }
      }
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, concat(F("P153 : READ delay: "), timeDelay));
      # endif // ifndef BUILD_NO_DEBUG

      measurementStart = millis();

      if (timeDelay > P153_DELAY_HIGH_RESOLUTION) { // Short delays are handled locally, longer uses the task device timer
        Scheduler.schedule_task_device_timer(event->TaskIndex, measurementStart + timeDelay);
      }
    }

    if ((P153_read_mode_e::Reading == readMode) || ((timeDelay >= 0) && (timeDelay <= P153_DELAY_HIGH_RESOLUTION))) {
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("P153 : READ execute Readmode = Reading"));
      # endif // ifndef BUILD_NO_DEBUG

      // Handle short delays immediately
      if ((timeDelay > 0) && (timeDelay <= P153_DELAY_HIGH_RESOLUTION)) {
        delay(timeDelay);
      }

      // Read the sensor data
      uint8_t data[6]{};

      Wire.requestFrom(_address, (uint8_t)6);

      if (Wire.available() == 6) {
        for (uint8_t d = 0; d < 6; d++) {
          data[d] = Wire.read();
        }
      } else {
        UserVar.setFloat(event->TaskIndex, 0, NAN); // Read error or I/O error
        UserVar.setFloat(event->TaskIndex, 1, NAN);

        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log;

          if (log.reserve(40)) {
            log  = getTaskDeviceName(event->TaskIndex);
            log += F(": Error reading sensor");
            addLogMove(LOG_LEVEL_ERROR, log);
          }
        }
      }

      // Data valid?
      if (CRC8(data[0], data[1], data[2]) && CRC8(data[3], data[4], data[5])) {
        float temp = static_cast<float>(((uint16_t)data[0] << 8) | (uint16_t)data[1]);
        float hum  = static_cast<float>(((uint16_t)data[3] << 8) | (uint16_t)data[4]);
        temperature = -45.0f + 175.0f * temp / 65535.0f;
        humidity    = -6.0f + 125.0f * hum / 65535.0f;

        if (definitelyLessThan(humidity, 0.0f)) { humidity = 0.0f; }

        if (definitelyGreaterThan(humidity, 100.0f)) { humidity = 100.0f; }

        UserVar.setFloat(event->TaskIndex, 0, temperature + _tempOffset); // Apply offset
        UserVar.setFloat(event->TaskIndex, 1, humidity);

        success    = true;
        errorCount = 0;

        if (_intervalLoops > 0) {
          _intervalLoops--;

          if ((_intervalLoops == 0) && (_startupConfiguration != _normalConfiguration)) {
            addLog(LOG_LEVEL_INFO, F("SHT4x: Switching from Startup to Normal Configuration."));
          }
        }

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;

          if (log.reserve(40)) {
            log  = getTaskDeviceName(event->TaskIndex);
            log += F(": Temperature: ");
            log += formatUserVarNoCheck(event->TaskIndex, 0);
            addLogMove(LOG_LEVEL_INFO, log);

            log  = getTaskDeviceName(event->TaskIndex);
            log += F(": Humidity: ");
            log += formatUserVarNoCheck(event->TaskIndex, 1);
            addLogMove(LOG_LEVEL_INFO, log);
          }
        }
      } else {
        UserVar.setFloat(event->TaskIndex, 0, NAN);
        UserVar.setFloat(event->TaskIndex, 1, NAN);
        addLog(LOG_LEVEL_ERROR, concat(F("SHT4x: READ CRC Error, data: 0x"), formatToHex_array(data, 6)));
        errorCount++;

        if (errorCount > P153_MAX_ERRORCOUNT) {
          I2C_write8(_address, P153_SHT4X_RESET);
          delay(1);
          _intervalLoops = 0;
          addLog(LOG_LEVEL_ERROR, F("SHT4x: READ Error count reached, reset to Normal Configuration."));
        }
      }

      Scheduler.reschedule_task_device_timer(event->TaskIndex, measurementStart); // Sync with schedule
      readMode  = P153_read_mode_e::Idle;
      timeDelay = -1;                                                             // Reset
    }

    if ((P153_read_mode_e::Idle == readMode) && (timeDelay >= 0)) {
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("P153 : READ set Readmode = Reading"));
      # endif // ifndef BUILD_NO_DEBUG
      readMode = P153_read_mode_e::Reading;
    }
  }

  return success;
}

/*****************************************************
* plugin_write
*****************************************************/
bool P153_data_struct::plugin_write(struct EventStruct *event,
                                    String            & string) {
  bool success = false;

  const String command = parseString(string, 1);

  if (equals(command, F("sht4x"))) {
    const String subCommand = parseString(string, 2);

    if (equals(subCommand, F("startup")) && (_startupConfiguration != _normalConfiguration) && (P153_INTERVAL_LOOPS > 0)) {
      _intervalLoops = P153_INTERVAL_LOOPS;
      success        = true;
    }
  }
  return success;
}

/*****************************************************
* plugin_get_config_value
*****************************************************/
bool P153_data_struct::plugin_get_config_value(struct EventStruct *event,
                                               String            & string) {
  bool success = false;

  const String var = parseString(string, 1);

  if (equals(var, F("serialnumber"))) { // [<taskname>#serialnumber] = the devices electronic serial number
    string  = String(serialNumber);
    success = true;
  }
  return success;
}

bool P153_data_struct::CRC8(uint8_t MSB, uint8_t LSB, uint8_t CRC)
{
  /*
   *	Name           : CRC-8
   * Polynomial     : 0x31 (x8 + x5 + x4 + 1)
   * Initialization : 0xFF
   * Reflect input  : False
   * Reflect output : False
   * Final          : XOR 0x00
   *	Example        : CRC8( 0xBE, 0xEF, 0x92) should be true
   */
  uint8_t crc = 0xFF;

  for (uint8_t bytenr = 0; bytenr < 2; ++bytenr) {
    crc ^= (bytenr == 0) ? MSB : LSB;

    for (uint8_t i = 0; i < 8; ++i) {
      crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
    }
  }
  return crc == CRC;
}

#endif // ifdef USES_P153
