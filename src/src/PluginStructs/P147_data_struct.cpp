#include "../PluginStructs/P147_data_struct.h"

#ifdef USES_P147

/**************************************************************************
* Constructor
**************************************************************************/
P147_data_struct::P147_data_struct(struct EventStruct *event)
{
  _sensorType      = static_cast<P147_sensor_e>(P147_SENSOR_TYPE);
  _initialCounter  = P147_LOW_POWER_MEASURE == 0 ? P147_SHORT_COUNTER : P147_LONG_COUNTER;
  _secondsCounter  = _initialCounter;
  _ignoreFirstRead = P147_LOW_POWER_MEASURE == 1;
  _useCompensation = P147_GET_USE_COMPENSATION;
  # if P147_FEATURE_GASINDEXALGORITHM
  _rawOnly = P147_GET_RAW_DATA_ONLY;
  # endif // if P147_FEATURE_GASINDEXALGORITHM

  if (validTaskIndex(P147_TEMPERATURE_TASK) && validTaskVarIndex(P147_TEMPERATURE_VALUE)) {
    _temperatureValueIndex = P147_TEMPERATURE_TASK * VARS_PER_TASK + P147_TEMPERATURE_VALUE;
  }

  if (validTaskIndex(P147_HUMIDITY_TASK) && validTaskVarIndex(P147_HUMIDITY_VALUE)) {
    _humidityValueIndex = P147_HUMIDITY_TASK * VARS_PER_TASK + P147_HUMIDITY_VALUE;
  }
  _initialized = false;
}

/*****************************************************
 * Init sensor and supporting objects
 ****************************************************/
bool P147_data_struct::init(struct EventStruct *event) {
  if (I2C_wakeup(P147_I2C_ADDRESS) == 0) {
    _initialized = true;

    // TODO Add initialization of IndexAlgorithm objects
    # if P147_FEATURE_GASINDEXALGORITHM
    vocGasIndexAlgorithm = new VOCGasIndexAlgorithm((P147_LOW_POWER_MEASURE == 0 ? P147_SHORT_COUNTER : P147_LONG_COUNTER) * 1.0f);

    if (nullptr == vocGasIndexAlgorithm) {
      _initialized = false;
    }

    if (_initialized && (_sensorType == P147_sensor_e::SGP41)) {
      noxGasIndexAlgorithm = new NOxGasIndexAlgorithm(); // Algorithm doesn't support a sampling interval

      if (nullptr == noxGasIndexAlgorithm) {
        _initialized = false;
      }
    }
    # endif // if P147_FEATURE_GASINDEXALGORITHM

    // Read serial number
    if (_initialized && I2C_write8_reg(P147_I2C_ADDRESS, P147_CMD_READ_SERIALNR_A, P147_CMD_READ_SERIALNR_B)) {
      uint16_t serialNumber[3] = { 0 };
      bool     is_ok           = true;
      delay(1);

      for (uint8_t d = 0; d < 3 && is_ok; d++) {
        serialNumber[d] = readCheckedWord(is_ok);
      }

      if (is_ok) {
        _serial   = static_cast<uint64_t>(serialNumber[0]);
        _serial <<= 16;
        _serial  |= static_cast<uint64_t>(serialNumber[1]);
        _serial <<= 16;
        _serial  |= static_cast<uint64_t>(serialNumber[2]);
      } else {
        _initialized = false;
      }
      addLog(LOG_LEVEL_INFO, concat(F("SGP4x: Serial number: 0x"), ull2String(_serial, 16u)));
      # if P147_FEATURE_GASINDEXALGORITHM

      if (!_rawOnly) {
        addLog(LOG_LEVEL_INFO, F("SGP4x: Attention: First values will be available after initial indexing!"));
      }
      # endif // if P147_FEATURE_GASINDEXALGORITHM
    }

    if (_initialized && I2C_write8_reg(P147_I2C_ADDRESS, P147_CMD_SELF_TEST_A, P147_CMD_SELF_TEST_B)) {
      _state = P147_state_e::MeasureTest;
      Scheduler.setPluginTaskTimer(P147_DELAY_SELFTEST, event->TaskIndex, 0); // Retrieve selftest result after 320 msec
    }

    // addLog(LOG_LEVEL_INFO,
    //        concat(F("P147 : INIT State: "), static_cast<int>(_state)) +
    //        boolToString(_initialized));
  }
  return isInitialized();
}

/*****************************************************
* Destructor
*****************************************************/
P147_data_struct::~P147_data_struct() {
  # if P147_FEATURE_GASINDEXALGORITHM
  delete vocGasIndexAlgorithm;
  delete noxGasIndexAlgorithm;
  # endif // if P147_FEATURE_GASINDEXALGORITHM
}

/*****************************************************
* plugin_tasktimer_in : Handle several delay related tasks
*****************************************************/
bool P147_data_struct::plugin_tasktimer_in(struct EventStruct *event) {
  bool success = false;
  bool is_ok;

  if (isInitialized()) {
    switch (_state)
    {
      case P147_state_e::MeasureTest:
      {
        uint16_t result = readCheckedWord(is_ok);

        // addLog(LOG_LEVEL_INFO, concat(F("P147 : Selftest result: "), formatToHex(result)) + (is_ok ? F(" ok") : F(" error")));
        bool checkOk = false;

        if (_sensorType == P147_sensor_e::SGP40) {
          checkOk = ((result >> 8) & 0xFF) == 0xD4; // 0xD4xx = OK, 0x4Bxx = Error
        } else {
          checkOk = (result & 0xFF) == 0x00;        // 0xxx00 = OK 01..03 = Error
        }

        if (is_ok && checkOk) {
          success = true;
          _state  = P147_state_e::MeasureStart; // Start sequence
        } else {
          _state = P147_state_e::Uninitialized;
        }
        _initialized = success; // Failing selftest will disable the plugin
        break;
      }

      case P147_state_e::MeasureStart:
      case P147_state_e::MeasureTrigger:
      {
        uint16_t compensationT  = 0x6666; // Default values
        uint16_t compensationRh = 0x8000;
        float    temperature    = 25.0f;  // Default values
        float    humidity       = 50.0f;

        // addLog(LOG_LEVEL_INFO, F("P147 : MeasureStart"));

        if (_useCompensation && ((_temperatureValueIndex > -1) || (_humidityValueIndex > -1))) {
          if (_temperatureValueIndex > -1) {
            temperature = UserVar[_temperatureValueIndex];
          }

          if (_humidityValueIndex > -1) {
            humidity = UserVar[_humidityValueIndex];
          }

          // Sanity checks
          if (definitelyLessThan(temperature, -45.0f)) { temperature = -45.0f; }

          if (definitelyGreaterThan(temperature, 130.0f)) { temperature = 130.0f; }

          if (definitelyLessThan(humidity, 0.0f)) { humidity = 0.0f; }

          if (definitelyGreaterThan(humidity, 100.0f)) { humidity = 100.0f; }

          // Calculate ticks
          compensationT  = static_cast<uint16_t>((temperature + 45) * 65535 / 175);
          compensationRh = static_cast<uint16_t>(humidity * 65535 / 100);
        }

        if (startSensorRead(compensationRh, compensationT)) {
          if ((_readLoop == 0) && (P147_LOW_POWER_MEASURE == 1) && (P147_state_e::MeasureTrigger != _state)) {
            _readLoop = 1;                       // Skip first read after waking up
          }
          _state = P147_state_e::MeasureReading; // Starting a measurement also turns the heater on, just needs extra time

          Scheduler.setPluginTaskTimer((P147_LOW_POWER_MEASURE == 0 || _readLoop == 0)
                                       ? (_sensorType == P147_sensor_e::SGP40 ? P147_DELAY_REGULAR : P147_DELAY_REGULAR_SGP41)
                                       : P147_DELAY_LOW_POWER,
                                       event->TaskIndex,
                                       0);
        }
        break;
      }

      case P147_state_e::MeasureReading: // Get raw data
      {
        _rawVOC = readCheckedWord(is_ok);

        if (is_ok && (_lastCommand == P147_CMD_SGP41_READ_B) && (_sensorType == P147_sensor_e::SGP41)) {
          _rawNOx = readCheckedWord(is_ok);
        }

        // addLog(LOG_LEVEL_INFO,
        //        concat(F("P147 : MeasureReading raw VOC: "), _rawVOC) +
        //        concat(F(", raw NOx: "),                     _rawNOx) +
        //        concat(F(", loop: "),                        _readLoop) +
        //        (is_ok ? F(" ok") : F(" error")));

        if (is_ok && (_readLoop == 0)) {
          success = true;
          _state  = P147_state_e::Ready;

          # if P147_FEATURE_GASINDEXALGORITHM

          // Feed to normalizers
          _vocIndex = vocGasIndexAlgorithm->process(_rawVOC);

          if (_vocIndex == 0) {
            _skipCount++;
          }

          if ((_lastCommand == P147_CMD_SGP41_READ_B) && (_sensorType == P147_sensor_e::SGP41)) {
            _noxIndex = noxGasIndexAlgorithm->process(_rawNOx);
          }
          # endif // if P147_FEATURE_GASINDEXALGORITHM

          // Startup delay check for NOx measurement/normalizer
          if ((_startupNOxCounter == 0) || (_sensorType == P147_sensor_e::SGP40)) {
            _dataAvailable = true;                                               // Data can be read
          }

          if (P147_LOW_POWER_MEASURE == 1) {                                     // Turn  off heater
            I2C_write8_reg(P147_I2C_ADDRESS, P147_CMD_HEATER_OFF_A, P147_CMD_HEATER_OFF_B);
          }
          Scheduler.setPluginTaskTimer(P147_DELAY_MINIMAL, event->TaskIndex, 0); // Next step
        } else {
          _state = P147_state_e::MeasureStart;                                   // Restart from once_a_second

          if (_readLoop > 0) {
            _state = P147_state_e::MeasureTrigger;                               // Trigger only
            Scheduler.setPluginTaskTimer(_sensorType == P147_sensor_e::SGP40 ? P147_DELAY_REGULAR : P147_DELAY_REGULAR_SGP41,
                                         event->TaskIndex, 0);                   // Trigger actual read after heating up
          }
        }

        if (_readLoop > 0) { _readLoop--; }
        break;
      }

      case P147_state_e::Ready:
        _state = P147_state_e::MeasureStart; // When ready, start a new sequence from plugin_once_a_second
        break;

      case P147_state_e::Uninitialized:      // Keep compiler happy
        break;
    }
  }
  return success;
}

/*****************************************************
* plugin_once_a_second
*****************************************************/
bool P147_data_struct::plugin_once_a_second(struct EventStruct *event) {
  bool success = false;

  // addLog(LOG_LEVEL_INFO,
  //        concat(F("P147 : State: "),   static_cast<int>(_state)) +
  //        concat(F(", Last _rawVOC: "), _rawVOC) +
  //        concat(F(", Last _rawNOx: "), _rawNOx) +
  //        concat(F(", count: "),        _secondsCounter));

  if (isInitialized()) {
    _secondsCounter--;

    if (_startupNOxCounter > 0) { _startupNOxCounter--; }

    if (_secondsCounter == 0) {
      // Execute a measurement cycle
      if (_state == P147_state_e::MeasureStart) {
        // Trigger a cycle
        Scheduler.setPluginTaskTimer(P147_DELAY_MINIMAL, event->TaskIndex, 0); // Next step
        success = true;
      }

      // Reset counter
      _secondsCounter = _initialCounter;
    }
  }

  return success;
}

/*****************************************************
* plugin_read
*****************************************************/
bool P147_data_struct::plugin_read(struct EventStruct *event) {
  bool success = false;

  if (isInitialized()) {
    if (_dataAvailable) {
      if (_ignoreFirstRead) {
        _ignoreFirstRead = false;
      } else {
        # if P147_FEATURE_GASINDEXALGORITHM

        if (_rawOnly)
        # endif // if P147_FEATURE_GASINDEXALGORITHM
        {
          UserVar.setFloat(event->TaskIndex, 0, _rawVOC);
        }
        # if P147_FEATURE_GASINDEXALGORITHM
        else {
          UserVar.setFloat(event->TaskIndex, 0, _vocIndex); // Use normalized VOC index
        }
        # endif // if P147_FEATURE_GASINDEXALGORITHM

        if (_sensorType == P147_sensor_e::SGP41) {
          # if P147_FEATURE_GASINDEXALGORITHM

          if (_rawOnly)
          # endif // if P147_FEATURE_GASINDEXALGORITHM
          {
            UserVar.setFloat(event->TaskIndex, 1, _rawNOx);
          }
          # if P147_FEATURE_GASINDEXALGORITHM
          else {
            UserVar.setFloat(event->TaskIndex, 1, _noxIndex); // Use normalized NOx index
          }
          # endif // if P147_FEATURE_GASINDEXALGORITHM
        }
        # if P147_FEATURE_GASINDEXALGORITHM
        success = (_rawOnly || _vocIndex != 0); // Accepted if the VOC index is no longer 0 (NOx index ignored for now)

        if (success && !_rawOnly && (_skipCount > 0)) {
          addLog(LOG_LEVEL_INFO, concat(F("SGP4x: Valid values found, skipped samples: "), _skipCount));
          _skipCount = 0;
        }
        # else // if P147_FEATURE_GASINDEXALGORITHM
        success = true;
        # endif // if P147_FEATURE_GASINDEXALGORITHM
      }
    }
  }
  return success;
}

/*****************************************************
* plugin_write
*****************************************************/
bool P147_data_struct::plugin_write(struct EventStruct *event,
                                    String            & string) {
  bool success = false;

  const String command = parseString(string, 1);

  if (equals(command, F("sgp4x"))) {
    // const String sub = parseString(string, 2);
  }
  return success;
}

/*****************************************************
* plugin_get_config_value
*****************************************************/
bool P147_data_struct::plugin_get_config_value(struct EventStruct *event,
                                               String            & string) {
  bool success = false;

  const String var = parseString(string, 1);

  if (equals(var, F("serialnumber"))) { // [<taskname>#serialnumber] = the devices electronic serial number
    string  = ull2String(_serial);
    success = true;
  } else
  if (equals(var, F("rawvoc"))) { // [<taskname>#rawVOC] = the last raw VOC value retrieved from the sensor
    string  = _rawVOC;
    success = true;
  } else
  if (equals(var, F("rawnox")) &&
      (_sensorType == P147_sensor_e::SGP41)) { // [<taskname>#rawNOx] = the last raw NOx value retrieved from the sensor
    string  = _rawNOx;
    success = true;
  }
  return success;
}

// Private

/*****************************************************
 * readCheckedWord : Read 2 data bytes from I2C and validate checksum (3rd byte)
 ****************************************************/
uint16_t P147_data_struct::readCheckedWord(bool& is_ok, long extraDelay) {
  uint16_t result  = 0;
  uint8_t  data[3] = { 0 };
  uint32_t timeOut = millis();

  is_ok = false;
  Wire.requestFrom(P147_I2C_ADDRESS, 3);

  while (Wire.available() != 3 && timePassedSince(timeOut) < extraDelay) { // Wait extra 5 msec.
    delay(1);
  }

  if (Wire.available() == 3) {
    for (uint8_t d = 0; d < 3; d++) {
      data[d] = Wire.read();
    }

    if (calc_CRC8(data, 2) == data[2]) { // valid checksum?
      result = (data[0] << 8) | data[1];
      is_ok  = true;
    }
  }

  return result;
}

bool P147_data_struct::startSensorRead(uint16_t compensationRh, uint16_t compensationT) {
  uint8_t data[2] = { 0 };

  Wire.beginTransmission(P147_I2C_ADDRESS);       // Start

  if (_sensorType == P147_sensor_e::SGP40) {
    Wire.write((uint8_t)P147_CMD_SGP40_READ_A);   // SGP40 Command
    Wire.write((uint8_t)P147_CMD_SGP40_READ_B);
    _lastCommand = P147_CMD_SGP40_READ_B;         // Read only VOC
  } else {
    if (_startupNOxCounter == 0) {
      Wire.write((uint8_t)P147_CMD_SGP41_READ_A); // SGP41 regular read Command
      Wire.write((uint8_t)P147_CMD_SGP41_READ_B);
      _lastCommand = P147_CMD_SGP41_READ_B;       // Read VOC and NOx
    } else {
      Wire.write((uint8_t)P147_CMD_SGP41_COND_A); // SGP41 NOx Conditioning Command
      Wire.write((uint8_t)P147_CMD_SGP41_COND_B); // Only raw VOC is returned
      _lastCommand = P147_CMD_SGP41_COND_B;       // Conditioning, read only VOC
    }
  }
  data[0] = (compensationRh >> 8);
  data[1] = (compensationRh & 0xFF);
  Wire.write(data[0]);            // Rel. humidity compensation
  Wire.write(data[1]);
  Wire.write(calc_CRC8(data, 2)); // crc
  data[0] = (compensationT >> 8);
  data[1] = (compensationT & 0xFF);
  Wire.write(data[0]);            // Temperature compensation
  Wire.write(data[1]);
  Wire.write(calc_CRC8(data, 2)); // crc

  return Wire.endTransmission() == 0;
}

#endif // ifdef USES_P147
