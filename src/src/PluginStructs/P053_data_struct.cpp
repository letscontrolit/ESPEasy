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
  }
  return F("Unknown");
}

const __FlashStringHelper* toString(PMSx003_output_selection selection) {
  switch (selection) {
    case PMSx003_output_selection::Particles_ug_m3: return F("Particles &micro;g/m3: pm1.0, pm2.5, pm10");
    case PMSx003_output_selection::PM2_5_TempHum_Formaldehyde:  return F("Particles &micro;g/m3: pm2.5; Other: Temp, Humi, HCHO (PMS5003ST)");
    case PMSx003_output_selection::ParticlesCount_100ml_cnt1_0_cnt2_5_cnt10:
      return F("Particles count/0.1L: cnt1.0, cnt2.5, cnt5, cnt10 (PMS1003/5003(ST)/7003)");
    case PMSx003_output_selection::ParticlesCount_100ml_cnt0_3__cnt_2_5:
      return F("Particles count/0.1L: cnt0.3, cnt0.5, cnt1.0, cnt2.5 (PMS1003/5003(ST)/7003)");
  }
  return F("Unknown");
}

const __FlashStringHelper* toString(PMSx003_event_datatype selection) {
  switch (selection) {
    case PMSx003_event_datatype::Event_None:       return F("None");
    case PMSx003_event_datatype::Event_PMxx_TempHum_Formaldehyde:  return F("Particles &micro;g/m3 and Temp/Humi/HCHO");
    case PMSx003_event_datatype::Event_All:  return F("Particles &micro;g/m3, Temp/Humi/HCHO and Particles count/0.1L");
    case PMSx003_event_datatype::Event_All_count_bins: return F("Particles count/0.1L");
  }
  return F("Unknown");
}

P053_data_struct::P053_data_struct(
  int8_t                  rxPin,
  int8_t                  txPin,
  const ESPEasySerialPort port,
  int8_t                  resetPin,
  int8_t                  pwrPin,
  PMSx003_type            sensortype)
  : _sensortype(sensortype), _resetPin(resetPin), _pwrPin(pwrPin) {
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    log.reserve(25);
    log  = F("PMSx003 : config ");
    log += rxPin;
    log += ' ';
    log += txPin;
    log += ' ';
    log += _resetPin;
    addLog(LOG_LEVEL_DEBUG, log);
  }
  # endif // ifndef BUILD_NO_DEBUG


  // Hardware serial is RX on 3 and TX on 1
  if (port == ESPEasySerialPort::software) {
    addLog(LOG_LEVEL_INFO, F("PMSx003 : using software serial"));
  } else {
    addLog(LOG_LEVEL_INFO, F("PMSx003 : using hardware serial"));
  }
  _easySerial = new (std::nothrow) ESPeasySerial(port, rxPin, txPin, false, 96); // 96 Bytes buffer, enough for up to 3 packets.

  if (_easySerial != nullptr) {
    _easySerial->begin(9600);
    _easySerial->flush();

    wakeSensor();

    resetSensor();

    // Make sure to set the mode to active reading mode.
    // This is the default, but not sure if passive mode can be set persistant.
    setActiveReadingMode();
  }
}

P053_data_struct::~P053_data_struct() {
  if (_easySerial != nullptr) {
    // Regardless the set pins, the software serial must be deleted.
    delete _easySerial;
    _easySerial = nullptr;
  }
}

bool P053_data_struct::initialized() const
{
  return _easySerial != nullptr;
}

// Read 2 bytes from serial and make an uint16 of it. Additionally calculate
// checksum for PMSx003. Assumption is that there is data available, otherwise
// this function is blocking.
void P053_data_struct::SerialRead16(uint16_t& value, uint16_t *checksum)
{
  if (!initialized()) { return; }
  const uint8_t data_high = _easySerial->read();
  const uint8_t data_low  = _easySerial->read();

  value  = data_low;
  value |= (data_high << 8);

  if (checksum != nullptr)
  {
    *checksum += data_high;
    *checksum += data_low;
  }

# ifdef P053_LOW_LEVEL_DEBUG

  // Low-level logging to see data from sensor
  String log = F("PMSx003 : uint8_t high=0x");
  log += String(data_high, HEX);
  log += F(" uint8_t low=0x");
  log += String(data_low, HEX);
  log += F(" result=0x");
  log += String(value, HEX);
  addLog(LOG_LEVEL_INFO, log);
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
  }
  return 0u;
}

bool P053_data_struct::packetAvailable()
{
  if (_easySerial != nullptr) // Software serial
  {
    // When there is enough data in the buffer, search through the buffer to
    // find header (buffer may be out of sync)
    if (!_easySerial->available()) { return false; }

    while ((_easySerial->peek() != PMSx003_SIG1) && _easySerial->available()) {
      _easySerial->read(); // Read until the buffer starts with the
      // first uint8_t of a message, or buffer
      // empty.
    }

    if (_easySerial->available() < packetSize()) {
      return false; // Not enough yet for a complete packet
    }
  }
  return true;
}

void P053_data_struct::sendEvent(const String & baseEvent,
                                 uint8_t        index,
                                 const uint16_t data[]) {
  float value = 0.0f;

  if (!getValue(data, index, value)) { return; }


  String valueEvent;

  valueEvent.reserve(32);

  // Temperature
  valueEvent  = baseEvent;
  valueEvent += getEventString(index);
  valueEvent += '=';
  valueEvent += value;
  eventQueue.addMove(std::move(valueEvent));
}

bool P053_data_struct::hasFormaldehyde() const {
  # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  return _sensortype == PMSx003_type::PMS5003_S ||
         _sensortype == PMSx003_type::PMS5003_ST;
  # else // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  return false;
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
}

bool P053_data_struct::hasTempHum() const {
  # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  return _sensortype == PMSx003_type::PMS5003_T ||
         _sensortype == PMSx003_type::PMS5003_ST;
  # else // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  return false;
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
}

bool P053_data_struct::processData(struct EventStruct *event) {
  uint16_t checksum = 0, checksum2 = 0;
  uint16_t framelength   = 0;
  uint16_t packet_header = 0;

  SerialRead16(packet_header, &checksum); // read PMSx003_SIG1 + PMSx003_SIG2

  if (packet_header != ((PMSx003_SIG1 << 8) | PMSx003_SIG2)) {
    // Not the start of the packet, stop reading.
    return false;
  }

  SerialRead16(framelength, &checksum);

  if (framelength != (packetSize() - 4)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log;
      log.reserve(34);
      log  = F("PMSx003 : invalid framelength - ");
      log += framelength;
      addLog(LOG_LEVEL_ERROR, log);
    }
    return false;
  }

  uint8_t frameData = packetSize();

  if (frameData > 0u) {
    frameData /= 2;                               // Each value is 16 bits
    frameData -= 3;                               // start markers, length, checksum
  }
  uint16_t data[PMS_RECEIVE_BUFFER_SIZE] = { 0 }; // uint8_t data_low, data_high;

  for (uint8_t i = 0; i < frameData; i++) {
    SerialRead16(data[i], &checksum);
  }

# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) { // Available on all supported sensor models
    String log;
    log.reserve(87);
    log  = F("PMSx003 : pm1.0=");
    log += data[PMS_PM1_0_ug_m3_factory];
    log += F(", pm2.5=");
    log += data[PMS_PM2_5_ug_m3_factory];
    log += F(", pm10=");
    log += data[PMS_PM10_0_ug_m3_factory];
    log += F(", pm1.0a=");
    log += data[PMS_PM1_0_ug_m3_normal];
    log += F(", pm2.5a=");
    log += data[PMS_PM2_5_ug_m3_normal];
    log += F(", pm10a=");
    log += data[PMS_PM10_0_ug_m3_normal];
    addLog(LOG_LEVEL_DEBUG, log);
  }

#  ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)
      && (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR != PMSx003_type::PMS2003_3003)) { // 'Count' values not available on
    // PMS2003/PMS3003 models
    // (handled as 1 model in code)
    String log;
    log.reserve(96);
    log  = F("PMSx003 : count/0.1L : 0.3um=");
    log += data[PMS_cnt0_3_100ml];
    log += F(", 0.5um=");
    log += data[PMS_cnt0_5_100ml];
    log += F(", 1.0um=");
    log += data[PMS_cnt1_0_100ml];
    log += F(", 2.5um=");
    log += data[PMS_cnt2_5_100ml];
    log += F(", 5.0um=");
    log += data[PMS_cnt5_0_100ml];
    log += F(", 10um=");
    log += data[PMS_cnt10_0_100ml];
    addLog(LOG_LEVEL_DEBUG, log);
  }

  #   ifdef PLUGIN_053_ENABLE_S_AND_T

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)
      && (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR == PMSx003_type::PMS5003_ST)) { // Values only available on PMS5003ST
    String log;
    log.reserve(45);
    log  = F("PMSx003 : temp=");
    log += static_cast<float>(data[PMS_Temp_C]) / 10.0f;
    log += F(", humi=");
    log += static_cast<float>(data[PMS_Hum_pct]) / 10.0f;
    log += F(", hcho=");
    log += static_cast<float>(data[PMS_Formaldehyde_mg_m3]) / 1000.0f;
    addLog(LOG_LEVEL_DEBUG, log);
  }
  #   endif // ifdef PLUGIN_053_ENABLE_S_AND_T
  #  endif  // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

# endif     // ifndef BUILD_NO_DEBUG

  // Compare checksums
  SerialRead16(checksum2, nullptr);
  SerialFlush(); // Make sure no data is lost due to full buffer.

  if (checksum == checksum2)
  {
    if (checksum == _last_checksum) {
      // Duplicate message
      # ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) { // Available on all supported sensor models
        addLog(LOG_LEVEL_DEBUG, F("PMSx003 : Duplicate message"));
      }
      # endif // ifndef BUILD_NO_DEBUG
      return false;
    }

    // Store new checksum, to help detect duplicates.
    _last_checksum = checksum;

    // Data is checked and good, fill in output
    # ifndef PLUGIN_053_ENABLE_EXTRA_SENSORS
    UserVar[event->BaseVarIndex]     = data[PMS_PM1_0_ug_m3_normal];
    UserVar[event->BaseVarIndex + 1] = data[PMS_PM2_5_ug_m3_normal];
    UserVar[event->BaseVarIndex + 2] = data[PMS_PM10_0_ug_m3_normal];
    # else // ifndef PLUGIN_053_ENABLE_EXTRA_SENSORS

    _value_mask = 0;

    switch (GET_PLUGIN_053_OUTPUT_SELECTOR) {
      case PMSx003_output_selection::Particles_ug_m3:
      {
        UserVar[event->BaseVarIndex]     = getValue(data, PMS_PM1_0_ug_m3_normal);
        UserVar[event->BaseVarIndex + 1] = getValue(data, PMS_PM2_5_ug_m3_normal);
        UserVar[event->BaseVarIndex + 2] = getValue(data, PMS_PM10_0_ug_m3_normal);
        UserVar[event->BaseVarIndex + 3] = 0.0f;
        break;
      }
      case PMSx003_output_selection::PM2_5_TempHum_Formaldehyde:
      {
        UserVar[event->BaseVarIndex]     = getValue(data, PMS_PM2_5_ug_m3_normal);
        UserVar[event->BaseVarIndex + 1] = getValue(data, PMS_Temp_C);
        UserVar[event->BaseVarIndex + 2] = getValue(data, PMS_Hum_pct);
        UserVar[event->BaseVarIndex + 3] = getValue(data, PMS_Formaldehyde_mg_m3);
        break;
      }
      case PMSx003_output_selection::ParticlesCount_100ml_cnt0_3__cnt_2_5:
      {
        UserVar[event->BaseVarIndex]     = getValue(data, PMS_cnt0_3_100ml);
        UserVar[event->BaseVarIndex + 1] = getValue(data, PMS_cnt0_5_100ml);
        UserVar[event->BaseVarIndex + 2] = getValue(data, PMS_cnt1_0_100ml);
        UserVar[event->BaseVarIndex + 3] = getValue(data, PMS_cnt2_5_100ml);
        break;
      }
      case PMSx003_output_selection::ParticlesCount_100ml_cnt1_0_cnt2_5_cnt10:
      {
        UserVar[event->BaseVarIndex]     = getValue(data, PMS_cnt1_0_100ml);
        UserVar[event->BaseVarIndex + 1] = getValue(data, PMS_cnt2_5_100ml);
        UserVar[event->BaseVarIndex + 2] = getValue(data, PMS_cnt5_0_100ml);
        UserVar[event->BaseVarIndex + 3] = getValue(data, PMS_cnt10_0_100ml);
        break;
      }
    }

    if (Settings.UseRules
        && (GET_PLUGIN_053_EVENT_OUT_SELECTOR != PMSx003_event_datatype::Event_None)
        && (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR != PMSx003_type::PMS2003_3003)) {
      // Events not applicable to PMS2003 & PMS3003 models
      String baseEvent;
      baseEvent.reserve(21);
      baseEvent  = getTaskDeviceName(event->TaskIndex);
      baseEvent += '#';


      // Send out events for those values not present in the task output
      switch (GET_PLUGIN_053_EVENT_OUT_SELECTOR) {
        case PMSx003_event_datatype::Event_All:
        {
          // Send all remaining
          for (uint8_t i = PMS_PM1_0_ug_m3_normal; i < PMS_RECEIVE_BUFFER_SIZE; ++i) {
            sendEvent(baseEvent, i, data);
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
            sendEvent(baseEvent, indices[i], data);
          }
          break;
        }
        case PMSx003_event_datatype::Event_All_count_bins:
        {
          // Thexe values are sequential, so just use a simple for loop.
          for (uint8_t i = PMS_cnt0_3_100ml; i <= PMS_cnt10_0_100ml; ++i) {
            sendEvent(baseEvent, i, data);
          }
          break;
        }
      }
    }
    # endif // ifndef PLUGIN_053_ENABLE_EXTRA_SENSORS
    _values_received = true;
    return true;
  }
  return false;
}

bool P053_data_struct::checkAndClearValuesReceived() {
  const bool ret = _values_received;

  _values_received = false;
  return ret;
}

bool P053_data_struct::resetSensor() {
  if (_resetPin >= 0) { // Reset if pin is configured
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

  if (_pwrPin >= 0) {
    // Make sure the sensor is "on"
    addLog(LOG_LEVEL_INFO, F("PMSx003: Set PWR_SET high"));
    pinMode(_pwrPin, OUTPUT);
    digitalWrite(_pwrPin, HIGH);
    pinMode(_pwrPin, INPUT_PULLUP);
  } else {
    const uint8_t command[7] = { 0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74
    };
    _easySerial->write(command, 7);
  }
  return true;
}

bool P053_data_struct::sleepSensor() {
  if (!initialized()) {
    return false;
  }

  if (_pwrPin >= 0) {
    // Put the sensor to sleep
    addLog(LOG_LEVEL_INFO, F("PMSx003: Set PWR_SET low"));
    pinMode(_pwrPin, OUTPUT);
    digitalWrite(_pwrPin, LOW);
  } else {
    const uint8_t command[7] = { 0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73 };
    _easySerial->write(command, 7);
  }
  return true;
}

void P053_data_struct::setActiveReadingMode() {
  if (initialized()) {
    const uint8_t command[7] = { 0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71 };
    _easySerial->write(command, 7);
    _activeReadingModeEnabled = true;
  }
}

void P053_data_struct::setPassiveReadingMode() {
  if (initialized()) {
    const uint8_t command[7] = { 0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70 };
    _easySerial->write(command, 7);
    _activeReadingModeEnabled = false;
  }
}

void P053_data_struct::requestData() {
  if (initialized() && !_activeReadingModeEnabled) {
    const uint8_t command[7] = { 0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71 };
    _easySerial->write(command, 7);
  }
}

float P053_data_struct::getValue(const uint16_t data[], uint8_t index) {
  float value = 0.0f;

  getValue(data, index, value);
  return value;
}

bool P053_data_struct::getValue(const uint16_t data[], uint8_t index, float& value) {
  if (bitRead(_value_mask, index) || (index >= PMS_RECEIVE_BUFFER_SIZE)) {
    // Already read
    return false;
  }

  switch (index) {
    case PMS_PM1_0_ug_m3_factory:
    case PMS_PM2_5_ug_m3_factory:
    case PMS_PM10_0_ug_m3_factory:
    case PMS_FW_rev_error:
      return false;

    case PMS_Temp_C:
    case PMS_Hum_pct:

      if (!hasTempHum()) { return false; }
      value = static_cast<float>(data[index]) / 10.0f;
      break;
    case PMS_Formaldehyde_mg_m3:

      if (!hasFormaldehyde()) { return false; }
      value = static_cast<float>(data[index]) / 1000.0f;
      break;
    default:
      value = static_cast<float>(data[index]);
      break;
  }
  bitSet(_value_mask, index);
  return true;
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

void P053_data_struct::setTaskValueNames(ExtraTaskSettingsStruct& settings, const uint8_t indices[], uint8_t nrElements) {
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    settings.TaskDeviceValueDecimals[i] = 0;

    if (i < nrElements) {
      safe_strncpy(
        settings.TaskDeviceValueNames[i],
        P053_data_struct::getEventString(indices[i]),
        sizeof(settings.TaskDeviceValueNames[i]));

      switch (indices[i]) {
        case PMS_Temp_C:
        case PMS_Hum_pct:
          settings.TaskDeviceValueDecimals[i] = 1;
          break;
        case PMS_Formaldehyde_mg_m3:
          settings.TaskDeviceValueDecimals[i] = 3;
          break;
        default:
          break;
      }
    } else {
      ZERO_FILL(settings.TaskDeviceValueNames[i]);
    }
  }
}

#endif // ifdef USES_P053
