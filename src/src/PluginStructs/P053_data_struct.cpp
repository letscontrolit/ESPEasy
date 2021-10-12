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
    case PMSx003_output_selection::ParticlesCount_100ml_cnt1_0_cnt2_5_cnt10:  return F(
        "Particles count/0.1L: cnt1.0, cnt2.5, cnt5, cnt10 (PMS1003/5003(ST)/7003)");
  }
  return F("Unknown");
}

const __FlashStringHelper* toString(PMSx003_event_datatype selection) {
  switch (selection) {
    case PMSx003_event_datatype::Event_None:       return F("None");
    case PMSx003_event_datatype::Event_Particles_TempHum_Formaldehyde:  return F("Particles &micro;g/m3 and Temp/Humi/HCHO");
    case PMSx003_event_datatype::Event_All:  return F("Particles &micro;g/m3, Temp/Humi/HCHO and Particles count/0.1L");
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

void P053_data_struct::sendEvent(const String& baseEvent, const __FlashStringHelper *name, float value) {
  String valueEvent;

  valueEvent.reserve(32);

  // Temperature
  valueEvent  = baseEvent;
  valueEvent += name;
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
    frameData /= 2;          // Each value is 16 bits
    frameData -= 3;          // start markers, length, checksum
  }
  uint16_t data[17] = { 0 }; // uint8_t data_low, data_high;

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


    switch (GET_PLUGIN_053_OUTPUT_SELECTOR) {
      case PMSx003_output_selection::Particles_ug_m3:
      {
        UserVar[event->BaseVarIndex]     = data[PMS_PM1_0_ug_m3_normal];
        UserVar[event->BaseVarIndex + 1] = data[PMS_PM2_5_ug_m3_normal];
        UserVar[event->BaseVarIndex + 2] = data[PMS_PM10_0_ug_m3_normal];
        UserVar[event->BaseVarIndex + 3] = 0.0f;
        break;
      }
      case PMSx003_output_selection::PM2_5_TempHum_Formaldehyde:
      {
        UserVar[event->BaseVarIndex]     = data[PMS_PM2_5_ug_m3_normal];
        UserVar[event->BaseVarIndex + 1] = static_cast<float>(data[PMS_Temp_C]) / 10.0f;               // TEMP
        UserVar[event->BaseVarIndex + 2] = static_cast<float>(data[PMS_Hum_pct]) / 10.0f;              // HUMI
        UserVar[event->BaseVarIndex + 3] = static_cast<float>(data[PMS_Formaldehyde_mg_m3]) / 1000.0f; // HCHO
        break;
      }
      case PMSx003_output_selection::ParticlesCount_100ml_cnt1_0_cnt2_5_cnt10:
      {
        UserVar[event->BaseVarIndex]     = data[PMS_cnt1_0_100ml];
        UserVar[event->BaseVarIndex + 1] = data[PMS_cnt2_5_100ml];
        UserVar[event->BaseVarIndex + 2] = data[PMS_cnt5_0_100ml];
        UserVar[event->BaseVarIndex + 3] = data[PMS_cnt10_0_100ml];
        break;
      }
      default:
        break; // Ignore invalid options
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
      switch (GET_PLUGIN_053_OUTPUT_SELECTOR) {
        case PMSx003_output_selection::Particles_ug_m3:
        {
          if (hasTempHum()) {
            // Temperature
            sendEvent(baseEvent, F("Temp"), static_cast<float>(data[PMS_Temp_C]) / 10.0f);

            // Humidity
            sendEvent(baseEvent, F("Humi"), static_cast<float>(data[PMS_Hum_pct]) / 10.0f);
          }

          if (hasFormaldehyde()) {
            // Formaldehyde (HCHO)
            sendEvent(baseEvent, F("HCHO"), static_cast<float>(data[PMS_Formaldehyde_mg_m3]) / 1000.0f);
          }

          if (GET_PLUGIN_053_EVENT_OUT_SELECTOR == PMSx003_event_datatype::Event_All) {
            // Particle count per 0.1 L > 0.3 micron
            sendEvent(baseEvent, F("cnt0.3"), data[PMS_cnt0_3_100ml]);

            // Particle count per 0.1 L > 0.5 micron
            sendEvent(baseEvent, F("cnt0.5"), data[PMS_cnt0_5_100ml]);

            // Particle count per 0.1 L > 1.0 micron
            sendEvent(baseEvent, F("cnt1.0"), data[PMS_cnt1_0_100ml]);

            // Particle count per 0.1 L > 2.5 micron
            sendEvent(baseEvent, F("cnt2.5"), data[PMS_cnt2_5_100ml]);

            // Particle count per 0.1 L > 5 micron
            sendEvent(baseEvent, F("cnt5"),   data[PMS_cnt5_0_100ml]);

            // Particle count per 0.1 L > 10 micron
            sendEvent(baseEvent, F("cnt10"),  data[PMS_cnt10_0_100ml]);
            break;
          }
        }
        case PMSx003_output_selection::PM2_5_TempHum_Formaldehyde:
        {
          // Particles > 1.0 um/m3
          sendEvent(baseEvent, F("pm1.0"), data[PMS_PM1_0_ug_m3_normal]);

          // Particles > 10 um/m3
          sendEvent(baseEvent, F("pm10"),  data[PMS_PM10_0_ug_m3_normal]);

          if (GET_PLUGIN_053_EVENT_OUT_SELECTOR == PMSx003_event_datatype::Event_All) {
            // Particle count per 0.1 L > 0.3 micron
            sendEvent(baseEvent, F("cnt0.3"), data[PMS_cnt0_3_100ml]);

            // Particle count per 0.1 L > 0.5 micron
            sendEvent(baseEvent, F("cnt0.5"), data[PMS_cnt0_5_100ml]);

            // Particle count per 0.1 L > 1.0 micron
            sendEvent(baseEvent, F("cnt1.0"), data[PMS_cnt1_0_100ml]);

            // Particle count per 0.1 L > 2.5 micron
            sendEvent(baseEvent, F("cnt2.5"), data[PMS_cnt2_5_100ml]);

            // Particle count per 0.1 L > 5 micron
            sendEvent(baseEvent, F("cnt5"),   data[PMS_cnt5_0_100ml]);

            // Particle count per 0.1 L > 10 micron
            sendEvent(baseEvent, F("cnt10"),  data[PMS_cnt10_0_100ml]);
          }
          break;
        }
        case PMSx003_output_selection::ParticlesCount_100ml_cnt1_0_cnt2_5_cnt10:
        {
          // Particles > 1.0 um/m3
          sendEvent(baseEvent, F("pm1.0"), data[PMS_PM1_0_ug_m3_normal]);

          // Particles > 2.5 um/m3
          sendEvent(baseEvent, F("pm2.5"), data[PMS_PM2_5_ug_m3_normal]);

          // Particles > 10 um/m3
          sendEvent(baseEvent, F("pm10"),  data[PMS_PM10_0_ug_m3_normal]);

          if (hasTempHum()) {
            // Temperature
            sendEvent(baseEvent, F("Temp"), static_cast<float>(data[PMS_Temp_C]) / 10.0f);

            // Humidity
            sendEvent(baseEvent, F("Humi"), static_cast<float>(data[PMS_Hum_pct]) / 10.0f);
          }

          if (hasFormaldehyde()) {
            // Formaldehyde (HCHO)
            sendEvent(baseEvent, F("HCHO"), static_cast<float>(data[PMS_Formaldehyde_mg_m3]) / 1000.0f);
          }
          break;
        }
        default:
          break;
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
    const uint8_t command[7] = { 0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74 };
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

#endif // ifdef USES_P053
