#include "../PluginStructs/P053_data_struct.h"

#ifdef USES_P053

const __FlashStringHelper* toString(PMSx003_type sensorType) {
  switch (sensorType) {
    case PMSx003_type::PMS1003_5003_7003: return F("PMS1003 / PMS5003 / PMS7003");
    # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    case PMSx003_type::PMS2003_3003: return F("PMS2003 / PMS3003");
    case PMSx003_type::PMS5003_S:    return F("PMS5003S");
    case PMSx003_type::PMS5003_T:    return F("PMS5003T");
    case PMSx003_type::PMS5003_ST:   return F("PMS5003ST");
    # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    # ifdef USES_P175
    case PMSx003_type::PMSA003i:   return F("PMSA003i");
    # endif // ifdef USES_P175
  }
  return F("Unknown");
}

const __FlashStringHelper* toString(PMSx003_output_selection selection) {
  switch (selection) {
    case PMSx003_output_selection::Particles_ug_m3: return F("Particles &micro;g/m3: pm1.0, pm2.5, pm10");
    case PMSx003_output_selection::PM2_5_TempHum_Formaldehyde:  return F("Particles &micro;g/m3: pm2.5; Other: Temp, Humi, HCHO (PMS5003ST)");
    case PMSx003_output_selection::ParticlesCount_100ml_cnt1_0_cnt2_5_cnt10:
      return F("Particles count/0.1L: cnt1.0, cnt2.5, cnt5, cnt10 (PMS1003/5003(ST)/7003"
               # ifdef USES_P175
               "/A003i"
               # endif // ifdef USES_P175
               ")");
    case PMSx003_output_selection::ParticlesCount_100ml_cnt0_3__cnt_2_5:
      return F("Particles count/0.1L: cnt0.3, cnt0.5, cnt1.0, cnt2.5 (PMS1003/5003(ST)/7003"
               # ifdef USES_P175
               "/A003i"
               # endif // ifdef USES_P175
               ")");
  }
  return F("Unknown");
}

const __FlashStringHelper* toString(PMSx003_event_datatype selection) {
  switch (selection) {
    case PMSx003_event_datatype::Event_None: return F("None");
    case PMSx003_event_datatype::Event_PMxx_TempHum_Formaldehyde: return F("Particles &micro;g/m3 and Temp/Humi/HCHO");
    case PMSx003_event_datatype::Event_All: return F("Particles &micro;g/m3, Temp/Humi/HCHO and Particles count/0.1L");
    case PMSx003_event_datatype::Event_All_count_bins: return F("Particles count/0.1L");
  }
  return F("Unknown");
}

P053_data_struct::P053_data_struct(
  taskIndex_t             TaskIndex,
  int8_t                  rxPin,
  int8_t                  txPin,
  const ESPEasySerialPort port,
  int8_t                  resetPin,
  int8_t                  pwrPin,
  PMSx003_type            sensortype,
  uint32_t                delay_read_after_wakeup_ms
  # ifdef                 PLUGIN_053_ENABLE_EXTRA_SENSORS
  , bool                  oversample
  , bool                  splitCntBins
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  )
  : _taskIndex(TaskIndex),
  _rxPin(rxPin),
  _txPin(txPin),
  _port(port),
  _sensortype(sensortype),
  # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  _oversample(oversample),
  _splitCntBins(splitCntBins),
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  _delay_read_after_wakeup_ms(delay_read_after_wakeup_ms),
  _resetPin(resetPin), _pwrPin(pwrPin)
{
  # ifdef USES_P175
  _P053_for_P175 = PMSx003_type::PMSA003i == sensortype;
  # endif // ifdef USES_P175
}

bool P053_data_struct::init() {
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLogMove(LOG_LEVEL_DEBUG, strformat(F("PMSx003 : config %d %d %d %d"), _rxPin, _txPin, _resetPin, _pwrPin));
  }
  # endif // ifndef BUILD_NO_DEBUG

  # ifdef USES_P175

  if (!_P053_for_P175)
  # endif // ifdef USES_P175
  {
    if (_easySerial != nullptr) {
      delete _easySerial;
      _easySerial = nullptr;
    }

    _easySerial = new (std::nothrow) ESPeasySerial(_port, _rxPin, _txPin, false, 96); // 96 Bytes buffer, enough for up to 3 packets.
  }

  if ((_easySerial != nullptr)
      # ifdef USES_P175
      || _P053_for_P175
      # endif // ifdef USES_P175
      ) {
    # ifdef USES_P175

    if (_P053_for_P175) {
      // Initialize I2C sensor
      _i2c_init = true;

      if (I2C_wakeup(P175_I2C_ADDR) != 0) {
        addLog(LOG_LEVEL_INFO, F("PMSx003 : I2C sensor not found"));
        _i2c_init = false;
      }
    } else
    # endif // ifdef USES_P175
    {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO, concat(F("PMSx003 : "), _easySerial->getLogString()));
      }

      _easySerial->begin(9600);
      _easySerial->flush();
    }

    wakeSensor();

    resetSensor();

    // Make sure to set the mode to active reading mode.
    // This is the default, but not sure if passive mode can be set persistant.
    setActiveReadingMode();
  }
  clearReceivedData();
  return initialized();
}

P053_data_struct::~P053_data_struct() {
  if (_easySerial != nullptr) {
    delete _easySerial;
    _easySerial = nullptr;
  }
}

bool P053_data_struct::initialized() const
{
  return _easySerial != nullptr
         # ifdef USES_P175
         || _i2c_init
         # endif // ifdef USES_P175
  ;
}

// Read 2 bytes from serial and make an uint16 of it. Additionally calculate
// checksum for PMSx003. Assumption is that there is data available, otherwise
// this function is blocking.
void P053_data_struct::PacketRead16(uint16_t& value, uint16_t *checksum)
{
  if (!initialized()) { return; }

  if (_packetPos > (PMSx003_PACKET_BUFFER_SIZE - 2)) { return; }
  const uint8_t data_high = _packet[_packetPos++];
  const uint8_t data_low  = _packet[_packetPos++];

  value  = data_low;
  value |= (data_high << 8);

  if (checksum != nullptr)
  {
    *checksum += data_high;
    *checksum += data_low;
  }

  # ifdef P053_LOW_LEVEL_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    // Low-level logging to see data from sensor
    addLog(LOG_LEVEL_INFO,
           strformat(F("PMSx003 : uint8_t high=0x%02x uint8_t low=0x%02x result=0x%04x"), data_high, data_low, value));
  }
  # endif // ifdef P053_LOW_LEVEL_DEBUG
}

void P053_data_struct::SerialFlush() {
  if (_easySerial != nullptr) {
    _easySerial->flush();
  }
}

uint8_t P053_data_struct::packetSize() const {
  switch (_sensortype) {
    case PMSx003_type::PMS1003_5003_7003:    return PMS1003_5003_7003_SIZE;
    # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    case PMSx003_type::PMS5003_S:    return PMS5003_S_SIZE;
    case PMSx003_type::PMS5003_T:    return PMS5003_T_SIZE;
    case PMSx003_type::PMS5003_ST:   return PMS5003_ST_SIZE;
    case PMSx003_type::PMS2003_3003: return PMS2003_3003_SIZE;
    # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    # ifdef USES_P175
    case PMSx003_type::PMSA003i:     return PMSA003i_SIZE;
    # endif // ifdef USES_P175
  }
  return 0u;
}

bool P053_data_struct::packetAvailable()
{
  const uint8_t expectedSize = packetSize();

  if (expectedSize == 0) { return false; }

  if (_easySerial != nullptr)
  {
    if (_packetPos < expectedSize) {
      // When there is enough data in the buffer, search through the buffer to
      // find header (buffer may be out of sync)
      if (!_easySerial->available()) { return false; }

      if (_packetPos == 0) {
        while ((_easySerial->peek() != PMSx003_SIG1) && _easySerial->available()) {
          _easySerial->read(); // Read until the buffer starts with the
          // first uint8_t of a message, or buffer
          // empty.
        }

        if (_easySerial->peek() == PMSx003_SIG1) {
          _packet[_packetPos++] = _easySerial->read();
        }
      }

      if (_packetPos > 0) {
        while (_packetPos < expectedSize) {
          if (_easySerial->available() == 0) {
            return false;
          } else {
            _packet[_packetPos++] = _easySerial->read();
          }
        }
      }
    }
  }
  # ifdef USES_P175

  if (_i2c_init) {
    if (I2C_wakeup(P175_I2C_ADDR) == 0) {
      _packetPos = expectedSize; // We always have the data ready for reading
    }
  }
  # endif // ifdef USES_P175
  return _packetPos >= expectedSize;
}

# ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
void P053_data_struct::sendEvent(taskIndex_t TaskIndex,
                                 uint8_t     index) {
  float value = 0.0f;

  if (!getValue(index, value)) { return; }

  const unsigned char nrDecimals = getNrDecimals(index, _oversample);

  // Temperature
  eventQueue.add(TaskIndex, getEventString(index), toString(value, nrDecimals));
}

bool P053_data_struct::hasFormaldehyde() const {
  #  ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  return _sensortype == PMSx003_type::PMS5003_S ||
         _sensortype == PMSx003_type::PMS5003_ST;
  #  else // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  return false;
  #  endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
}

bool P053_data_struct::hasTempHum() const {
  #  ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  return _sensortype == PMSx003_type::PMS5003_T ||
         _sensortype == PMSx003_type::PMS5003_ST;
  #  else // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  return false;
  #  endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
}

# endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

bool P053_data_struct::processData(struct EventStruct *event) {
  uint16_t checksum = 0, checksum2 = 0;
  uint16_t framelength   = 0;
  uint16_t packet_header = 0;
  uint16_t data[PMS_I2C_BUFFER_SIZE]{}; // uint8_t data_low, data_high;

  _packetPos = 0;

  # ifdef USES_P175

  if (_i2c_init) {
    // Read data from I2C
    if (I2C_wakeup(P175_I2C_ADDR) == 0) {
      const uint8_t nrBytes = packetSize();

      if (Wire.requestFrom((uint8_t)P175_I2C_ADDR, nrBytes) == nrBytes) {
        // Read all bytes into word-buffer, and calculate the checksum
        uint8_t hbyte = Wire.read(); // Skip the ID bytes SIG1 = 0x42, SIG2 = 0x4d
        checksum += hbyte;           // but still have to count them in the checksum
        uint8_t lbyte = Wire.read();
        checksum += lbyte;

        if ((hbyte != PMSx003_SIG1) && (lbyte != PMSx003_SIG2)) {
          return false;
        }
        hbyte     = Wire.read(); // Skip the length bytes
        checksum += hbyte;       // but still have to count them in the checksum
        lbyte     = Wire.read();
        checksum += lbyte;

        // Read the data, (packetSize / 2) - 3, excluding SIG, Length and checksum
        const uint8_t nrBytes2 = (nrBytes / 2) - 3;

        for (uint8_t i = 0; i < nrBytes2; ++i) {
          const uint8_t hbyte = Wire.read(); // Read data bytes, counting them into the checksum
          checksum += hbyte;
          const uint8_t lbyte = Wire.read();
          checksum += lbyte;
          data[i]   = (hbyte << 8) | lbyte;
        }

        hbyte     = Wire.read();          // Read the checksum bytes, not counting them into the checksum
        lbyte     = Wire.read();
        checksum2 = (hbyte << 8) | lbyte; // Received checksum
      }
    } else {
      return false;                       // No device
    }
  } else if (!_P053_for_P175)
  # endif // ifdef USES_P175
  {
    PacketRead16(packet_header, &checksum); // read PMSx003_SIG1 + PMSx003_SIG2

    if (packet_header != ((PMSx003_SIG1 << 8) | PMSx003_SIG2)) {
      // Not the start of the packet, stop reading.
      return false;
    }

    PacketRead16(framelength, &checksum);

    if ((framelength + 4) != packetSize()) {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        addLog(LOG_LEVEL_ERROR, concat(F("PMSx003 : invalid framelength - "), framelength));
      }
      return false;
    }

    uint8_t frameData = packetSize();

    if (frameData > 0u) {
      frameData /= 2; // Each value is 16 bits
      frameData -= 3; // start markers, length, checksum
    }

    for (uint8_t i = 0; i < frameData && i < PMS_RECEIVE_BUFFER_SIZE; ++i) {
      PacketRead16(data[i], &checksum);
    }
  }

  # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  if (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR == PMSx003_type::PMS5003_T) {
    data[PMS_Temp_C]  = data[PMS_T_Temp_C]; // Move data to the 'usual' location for Temp/Hum
    data[PMS_Hum_pct] = data[PMS_T_Hum_pct];
  }
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  # ifndef BUILD_NO_DEBUG
  #  ifdef P053_LOW_LEVEL_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) { // Available on all supported sensor models
    addLog(LOG_LEVEL_DEBUG,
           strformat(F("PMSx003 : pm1.0=%d, pm2.5=%d, pm10=%d, pm1.0a=%d, pm2.5a=%d, pm10a=%d"),
                     data[PMS_PM1_0_ug_m3_factory],
                     data[PMS_PM2_5_ug_m3_factory],
                     data[PMS_PM10_0_ug_m3_factory],
                     data[PMS_PM1_0_ug_m3_normal],
                     data[PMS_PM2_5_ug_m3_normal],
                     data[PMS_PM10_0_ug_m3_normal]));
  }

  #   ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)
      && (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR != PMSx003_type::PMS2003_3003)) { // 'Count' values not available on
    // PMS2003/PMS3003 models
    // (handled as 1 model in code)
    addLog(LOG_LEVEL_DEBUG,
           strformat(F("PMSx003 : count/0.1L : 0.3um=%d, 0.5um=%d, 1.0um=%d, 2.5um=%d, 5.0um=%d, 10um=%d"),
                     data[PMS_cnt0_3_100ml],
                     data[PMS_cnt0_5_100ml],
                     data[PMS_cnt1_0_100ml],
                     data[PMS_cnt2_5_100ml],
                     data[PMS_cnt5_0_100ml],
                     data[PMS_cnt10_0_100ml]));
  }


  if (loglevelActiveFor(LOG_LEVEL_DEBUG)
      && ((GET_PLUGIN_053_SENSOR_MODEL_SELECTOR == PMSx003_type::PMS5003_ST)
          || (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR == PMSx003_type::PMS5003_T))) { // Values only available on PMS5003ST & PMS5003T
    String log;

    if (log.reserve(45)) {
      log = strformat(F("PMSx003 : temp=%.2f, humi=%.2f"),
                      static_cast<float>(data[PMS_Temp_C]) / 10.0f,
                      static_cast<float>(data[PMS_Hum_pct]) / 10.0f);

      if (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR == PMSx003_type::PMS5003_ST) {
        log += strformat(F(", hcho=%.4f"), static_cast<float>(data[PMS_Formaldehyde_mg_m3]) / 1000.0f);
      }
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  }
  #   endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  #  endif     // ifdef P053_LOW_LEVEL_DEBUG
  # endif      // ifndef BUILD_NO_DEBUG

  # ifdef USES_P175

  if (!_P053_for_P175)
  # endif // ifdef USES_P175
  {
    // Compare checksums
    PacketRead16(checksum2, nullptr);
    SerialFlush(); // Make sure no data is lost due to full buffer.
  }

  if (checksum != checksum2) {
    addLog(LOG_LEVEL_ERROR, strformat(F("PMSx003 : Checksum error (0x%x expected: 0x%x)"), checksum, checksum2));
    return false;
  }

  if (_last_wakeup_moment.isSet() && !_last_wakeup_moment.timeReached()) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;

      if (log.reserve(80)) {
        addLog(LOG_LEVEL_INFO,
               strformat(F("PMSx003 : Less than %d sec since sensor wakeup => Ignoring sample"),
                         _delay_read_after_wakeup_ms / 1000ul));
      }
    }
    return false;
  }

  if (checksum == _last_checksum) {
    // Duplicate message
    # ifndef BUILD_NO_DEBUG

    addLog(LOG_LEVEL_DEBUG, F("PMSx003 : Duplicate message"));
    # endif // ifndef BUILD_NO_DEBUG
    return false;
  }
  # ifndef PLUGIN_053_ENABLE_EXTRA_SENSORS

  // Data is checked and good, fill in output
  UserVar.setFloat(event->TaskIndex, 0, data[PMS_PM1_0_ug_m3_normal]);
  UserVar.setFloat(event->TaskIndex, 1, data[PMS_PM2_5_ug_m3_normal]);
  UserVar.setFloat(event->TaskIndex, 2, data[PMS_PM10_0_ug_m3_normal]);
  _values_received = 1;
  # else // ifndef PLUGIN_053_ENABLE_EXTRA_SENSORS

  // Store in the averaging buffer to process later
  if (!_oversample) {
    clearReceivedData();
  }

  for (uint8_t i = 0; i < PMS_RECEIVE_BUFFER_SIZE; ++i) {
    _data[i] += data[i];
  }
  ++_values_received;
  # endif // ifndef PLUGIN_053_ENABLE_EXTRA_SENSORS


  // Store new checksum, to help detect duplicates.
  _last_checksum = checksum;

  return true;
}

bool P053_data_struct::checkAndClearValuesReceived(struct EventStruct *event) {
  if (_values_received == 0) { return false; }

  # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  // Data is checked and good, fill in output

  _value_mask = 0;

  switch (GET_PLUGIN_053_OUTPUT_SELECTOR) {
    case PMSx003_output_selection::Particles_ug_m3:
    {
      UserVar.setFloat(event->TaskIndex, 0, getValue(PMS_PM1_0_ug_m3_normal));
      UserVar.setFloat(event->TaskIndex, 1, getValue(PMS_PM2_5_ug_m3_normal));
      UserVar.setFloat(event->TaskIndex, 2, getValue(PMS_PM10_0_ug_m3_normal));
      UserVar.setFloat(event->TaskIndex, 3, 0.0f);
      break;
    }
    case PMSx003_output_selection::PM2_5_TempHum_Formaldehyde:
    {
      UserVar.setFloat(event->TaskIndex, 0, getValue(PMS_PM2_5_ug_m3_normal));
      UserVar.setFloat(event->TaskIndex, 1, getValue(PMS_Temp_C));
      UserVar.setFloat(event->TaskIndex, 2, getValue(PMS_Hum_pct));
      UserVar.setFloat(event->TaskIndex, 3, getValue(PMS_Formaldehyde_mg_m3));
      break;
    }
    case PMSx003_output_selection::ParticlesCount_100ml_cnt0_3__cnt_2_5:
    {
      UserVar.setFloat(event->TaskIndex, 0, getValue(PMS_cnt0_3_100ml));
      UserVar.setFloat(event->TaskIndex, 1, getValue(PMS_cnt0_5_100ml));
      UserVar.setFloat(event->TaskIndex, 2, getValue(PMS_cnt1_0_100ml));
      UserVar.setFloat(event->TaskIndex, 3, getValue(PMS_cnt2_5_100ml));
      break;
    }
    case PMSx003_output_selection::ParticlesCount_100ml_cnt1_0_cnt2_5_cnt10:
    {
      UserVar.setFloat(event->TaskIndex, 0, getValue(PMS_cnt1_0_100ml));
      UserVar.setFloat(event->TaskIndex, 1, getValue(PMS_cnt2_5_100ml));
      UserVar.setFloat(event->TaskIndex, 2, getValue(PMS_cnt5_0_100ml));
      UserVar.setFloat(event->TaskIndex, 3, getValue(PMS_cnt10_0_100ml));
      break;
    }
  }

  if (Settings.UseRules
      && (GET_PLUGIN_053_EVENT_OUT_SELECTOR != PMSx003_event_datatype::Event_None)
      && (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR != PMSx003_type::PMS2003_3003)) {
    // Events not applicable to PMS2003 & PMS3003 models
    // Send out events for those values not present in the task output
    switch (GET_PLUGIN_053_EVENT_OUT_SELECTOR) {
      case PMSx003_event_datatype::Event_None: break;
      case PMSx003_event_datatype::Event_All:
      {
        // Send all remaining
        for (uint8_t i = PMS_PM1_0_ug_m3_normal; i < PMS_RECEIVE_BUFFER_SIZE; ++i) {
          sendEvent(event->TaskIndex, i);
        }
        break;
      }
      case PMSx003_event_datatype::Event_PMxx_TempHum_Formaldehyde:
      {
        const uint8_t indices[] =
        {
          PMS_PM1_0_ug_m3_normal,
          PMS_PM2_5_ug_m3_normal,
          PMS_PM10_0_ug_m3_normal,
          PMS_Temp_C,
          PMS_Hum_pct,
          PMS_Formaldehyde_mg_m3
        };

        for (uint8_t i = 0; i < 6; ++i) {
          sendEvent(event->TaskIndex, indices[i]);
        }
        break;
      }
      case PMSx003_event_datatype::Event_All_count_bins:
      {
        // Thexe values are sequential, so just use a simple for loop.
        for (uint8_t i = PMS_cnt0_3_100ml; i <= PMS_cnt10_0_100ml; ++i) {
          sendEvent(event->TaskIndex, i);
        }
        break;
      }
    }
  }

  if (_oversample) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO,
                 strformat(F("PMSx003: Oversampling using %d samples"), _values_received));
    }
  }
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  clearReceivedData();
  return true;
}

bool P053_data_struct::resetSensor() {
  if (validGpio(_resetPin)) { // Reset if pin is configured
    // Toggle 'reset' to assure we start reading header
    addLog(LOG_LEVEL_INFO, F("PMSx003: resetting module"));
    pinMode(_resetPin, OUTPUT);
    digitalWrite(_resetPin, LOW);
    delay(250);
    digitalWrite(_resetPin, HIGH);
    pinMode(_resetPin, INPUT_PULLUP);
    return true;
  }
  return false;
}

bool P053_data_struct::wakeSensor() {
  if (!initialized()) {
    return false;
  }
  addLog(LOG_LEVEL_INFO, F("PMSx003: Wake sensor"));

  if (validGpio(_pwrPin)) {
    // Make sure the sensor is "on"
    pinMode(_pwrPin, OUTPUT);
    digitalWrite(_pwrPin, HIGH);
    pinMode(_pwrPin, INPUT_PULLUP);
  } else
  # ifdef USES_P175
  if (!_P053_for_P175)
  # endif // ifdef USES_P175
  {
    const uint8_t command[7] = { 0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74 };
    _easySerial->write(command, 7);
  }

  // No idea how old this data is, so clear it.
  // Otherwise oversampling may give weird results
  clearReceivedData();

  if (_delay_read_after_wakeup_ms != 0) {
    _last_wakeup_moment.setMillisFromNow(_delay_read_after_wakeup_ms);
  } else {
    _last_wakeup_moment.clear();
  }
  return true;
}

bool P053_data_struct::sleepSensor() {
  if (!initialized()) {
    return false;
  }

  // Put the sensor to sleep
  addLog(LOG_LEVEL_INFO, F("PMSx003: Sleep sensor"));

  if (_pwrPin >= 0) {
    pinMode(_pwrPin, OUTPUT);
    digitalWrite(_pwrPin, LOW);
  } else
  # ifdef USES_P175
  if (!_P053_for_P175)
  # endif // ifdef USES_P175
  {
    const uint8_t command[7] = { 0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73 };
    _easySerial->write(command, 7);
  }

  if (_values_received > 0) {
    // Going to sleep, so flush whatever is read.
    Scheduler.schedule_task_device_timer(_taskIndex, millis() + 10);
  }
  return true;
}

void P053_data_struct::setActiveReadingMode() {
  if (initialized()) {
    # ifdef USES_P175

    if (!_P053_for_P175)
    # endif // ifdef USES_P175
    {
      const uint8_t command[7] = { 0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71 };
      _easySerial->write(command, 7);
    }
    _activeReadingModeEnabled = true;
  }
}

void P053_data_struct::setPassiveReadingMode() {
  if (initialized()) {
    # ifdef USES_P175

    if (!_P053_for_P175)
    # endif // ifdef USES_P175
    {
      const uint8_t command[7] = { 0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70 };
      _easySerial->write(command, 7);
    }
    _activeReadingModeEnabled = false;
  }
}

void P053_data_struct::requestData() {
  if (initialized() && !_activeReadingModeEnabled
      # ifdef USES_P175
      && !_P053_for_P175
      # endif // ifdef USES_P175
      ) {
    const uint8_t command[7] = { 0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71 };
    _easySerial->write(command, 7);
  }
}

# ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
float P053_data_struct::getValue(uint8_t index) {
  float value = 0.0f;

  getValue(index, value);
  return value;
}

bool P053_data_struct::getValue(uint8_t index, float& value) {
  if (bitRead(_value_mask, index) || (index >= PMS_RECEIVE_BUFFER_SIZE)) {
    // Already read
    return false;
  }

  switch (index) {
    case PMS_PM1_0_ug_m3_factory:
    case PMS_PM2_5_ug_m3_factory:
    case PMS_PM10_0_ug_m3_factory:
    case PMS_FW_rev_error:
    case PMS_Reserved:
      return false;

    case PMS_Temp_C:
    case PMS_Hum_pct:

      if (!hasTempHum()) { return false; }
      value = _data[index] / 10.0f;
      break;
    case PMS_Formaldehyde_mg_m3:

      if (!hasFormaldehyde()) { return false; }
      value = _data[index] / 1000.0f;
      break;
    case PMS_cnt5_0_100ml:
    case PMS_cnt10_0_100ml: // this option was missing :-|

      if (_sensortype == PMSx003_type::PMS5003_T) { return false; } // else: fall through
    case PMS_cnt0_3_100ml:
    case PMS_cnt0_5_100ml:
    case PMS_cnt1_0_100ml:
    case PMS_cnt2_5_100ml:
      value = _data[index];

      if (_splitCntBins) {
        value -= _data[index + 1];

        if (value < 0.0f) { value = 0.0f; }
      }
      break;

    default:
      value = _data[index];
      break;
  }

  if (_values_received > 1) {
    value /= static_cast<float>(_values_received);
  }

  bitSet(_value_mask, index);
  return true;
}

# endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

void P053_data_struct::clearReceivedData() {
  # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  for (uint8_t i = 0; i < PMS_RECEIVE_BUFFER_SIZE; ++i) {
    _data[i] = 0.0f;
  }
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  _values_received = 0;
}

void P053_data_struct::clearPacket() {
  _packetPos = 0;
  ZERO_FILL(_packet);
}

const __FlashStringHelper * P053_data_struct::getEventString(uint8_t index) {
  switch (index) {
    // case PMS_PM1_0_ug_m3_factory   : return F("");
    // case PMS_PM2_5_ug_m3_factory   : return F("");
    // case PMS_PM10_0_ug_m3_factory  : return F("");
    case PMS_PM1_0_ug_m3_normal: return F("pm1.0");
    case PMS_PM2_5_ug_m3_normal: return F("pm2.5");
    case PMS_PM10_0_ug_m3_normal: return F("pm10");
    case PMS_cnt0_3_100ml: return F("cnt0.3");
    case PMS_cnt0_5_100ml: return F("cnt0.5");
    case PMS_cnt1_0_100ml: return F("cnt1.0");
    case PMS_cnt2_5_100ml: return F("cnt2.5");
    case PMS_cnt5_0_100ml: return F("cnt5.0");
    case PMS_cnt10_0_100ml: return F("cnt10");
    case PMS_Formaldehyde_mg_m3: return F("HCHO");
    case PMS_Temp_C: return F("Temp");
    case PMS_Hum_pct: return F("Humi");

      // case PMS_FW_rev_error          : return F("");
  }
  return F("Unknown");
}

void P053_data_struct::setTaskValueNames(ExtraTaskSettingsStruct& settings, const uint8_t indices[], uint8_t nrElements,
                                         bool oversample) {
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    settings.TaskDeviceValueDecimals[i] = 0;

    if (i < nrElements) {
      safe_strncpy(
        settings.TaskDeviceValueNames[i],
        P053_data_struct::getEventString(indices[i]),
        sizeof(settings.TaskDeviceValueNames[i]));

      settings.TaskDeviceValueDecimals[i] = getNrDecimals(indices[i], oversample);
    } else {
      ZERO_FILL(settings.TaskDeviceValueNames[i]);
    }
  }
}

unsigned char P053_data_struct::getNrDecimals(uint8_t index, bool oversample) {
  switch (index) {
    case PMS_Temp_C:
    case PMS_Hum_pct:
      return 1;
    case PMS_Formaldehyde_mg_m3:
      return 3;
  }

  if (oversample) { return 1; }
  return 0;
}

#endif // ifdef USES_P053
