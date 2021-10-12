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

P053_data_struct::P053_data_struct(
  int8_t                  rxPin,
  int8_t                  txPin,
  const ESPEasySerialPort port,
  int8_t                  resetPin,
  PMSx003_type            sensortype)
  : _sensortype(sensortype) {
  #ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    log.reserve(25);
    log  = F("PMSx003 : config ");
    log += rxPin;
    log += ' ';
    log += txPin;
    log += ' ';
    log += resetPin;
    addLog(LOG_LEVEL_DEBUG, log);
  }
  #endif


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

    if (resetPin >= 0) { // Reset if pin is configured
      // Toggle 'reset' to assure we start reading header
      addLog(LOG_LEVEL_INFO, F("PMSx003: resetting module"));
      pinMode(resetPin, OUTPUT);
      digitalWrite(resetPin, LOW);
      delay(250);
      digitalWrite(resetPin, HIGH);
      pinMode(resetPin, INPUT_PULLUP);
    }
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
void P053_data_struct::SerialRead16(uint16_t &value, uint16_t *checksum)
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

#ifdef P053_LOW_LEVEL_DEBUG
  // Low-level logging to see data from sensor
  String log = F("PMSx003 : uint8_t high=0x");
  log += String(data_high, HEX);
  log += F(" uint8_t low=0x");
  log += String(data_low, HEX);
  log += F(" result=0x");
  log += String(value, HEX);
  addLog(LOG_LEVEL_INFO, log);
#endif
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
  return _sensortype == PMSx003_type::PMS5003_S ||
         _sensortype == PMSx003_type::PMS5003_ST;
}

bool P053_data_struct::hasTempHum() const {
  return _sensortype == PMSx003_type::PMS5003_T ||
         _sensortype == PMSx003_type::PMS5003_ST;
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
    frameData /= 2; // Each value is 16 bits
    frameData -= 3; // start markers, length, checksum
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
    log += data[PMS_PM0_3_100ml_normal];
    log += F(", 0.5um=");
    log += data[PMS_PM0_5_100ml_normal];
    log += F(", 1.0um=");
    log += data[PMS_PM1_0_100ml_normal];
    log += F(", 2.5um=");
    log += data[PMS_PM2_5_100ml_normal];
    log += F(", 5.0um=");
    log += data[PMS_PM5_0_100ml_normal];
    log += F(", 10um=");
    log += data[PMS_PM10_0_100ml_normal];
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
  #  endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

# endif // ifndef BUILD_NO_DEBUG

  // Compare checksums
  SerialRead16(checksum2, nullptr);
  SerialFlush(); // Make sure no data is lost due to full buffer.

  if (checksum == checksum2)
  {
    // Data is checked and good, fill in output
    # ifndef PLUGIN_053_ENABLE_EXTRA_SENSORS
    UserVar[event->BaseVarIndex]     = data[PMS_PM1_0_ug_m3_normal];
    UserVar[event->BaseVarIndex + 1] = data[PMS_PM2_5_ug_m3_normal];
    UserVar[event->BaseVarIndex + 2] = data[PMS_PM10_0_ug_m3_normal];
    # else // ifndef PLUGIN_053_ENABLE_EXTRA_SENSORS


    switch (PLUGIN_053_OUTPUT_SELECTOR) {
      case PLUGIN_053_OUTPUT_PART:
      {
        UserVar[event->BaseVarIndex]     = data[PMS_PM1_0_ug_m3_normal];
        UserVar[event->BaseVarIndex + 1] = data[PMS_PM2_5_ug_m3_normal];
        UserVar[event->BaseVarIndex + 2] = data[PMS_PM10_0_ug_m3_normal];
        UserVar[event->BaseVarIndex + 3] = 0.0;
        break;
      }
      case PLUGIN_053_OUTPUT_THC:
      {
        UserVar[event->BaseVarIndex]     = data[PMS_PM2_5_ug_m3_normal];
        UserVar[event->BaseVarIndex + 1] = static_cast<float>(data[PMS_Temp_C]) / 10.0f;   // TEMP
        UserVar[event->BaseVarIndex + 2] = static_cast<float>(data[PMS_Hum_pct]) / 10.0f;   // HUMI
        UserVar[event->BaseVarIndex + 3] = static_cast<float>(data[PMS_Formaldehyde_mg_m3]) / 1000.0f; // HCHO
        break;
      }
      case PLUGIN_053_OUTPUT_CNT:
      {
        UserVar[event->BaseVarIndex]     = data[PMS_PM1_0_100ml_normal];
        UserVar[event->BaseVarIndex + 1] = data[PMS_PM2_5_100ml_normal];
        UserVar[event->BaseVarIndex + 2] = data[PMS_PM5_0_100ml_normal];
        UserVar[event->BaseVarIndex + 3] = data[PMS_PM10_0_100ml_normal];
        break;
      }
      default:
        break; // Ignore invalid options
    }

    if (Settings.UseRules && (PLUGIN_053_EVENT_OUT_SELECTOR > 0)
        && (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR != PMSx003_type::PMS2003_3003)) {
      // Events not applicable to PMS2003 & PMS3003 models
      String baseEvent;
      baseEvent.reserve(21);
      baseEvent  = getTaskDeviceName(event->TaskIndex);
      baseEvent += '#';

      // Send out events for those values not present in the task output
      switch (PLUGIN_053_OUTPUT_SELECTOR) {
        case PLUGIN_053_OUTPUT_PART:
        {
          if (hasTempHum()) {
            // Temperature
            sendEvent(baseEvent, F("Temp"), static_cast<float>(data[PMS_Temp_C]) / 10.0f);

            // Humidity
            sendEvent(baseEvent, F("Humi"), static_cast<float>(data[PMS_Hum_pct]) / 10.0f);
          }

          if (hasFormaldehyde()) {
            // Formaldebyde (HCHO)
            sendEvent(baseEvent, F("HCHO"), static_cast<float>(data[PMS_Formaldehyde_mg_m3]) / 1000.0f);
          }

          if (PLUGIN_053_EVENT_OUT_SELECTOR == PLUGIN_053_OUTPUT_CNT) {
            // Particle count per 0.1 L > 1.0 micron
            sendEvent(baseEvent, F("cnt1.0"), data[PMS_PM1_0_100ml_normal]);

            // Particle count per 0.1 L > 2.5 micron
            sendEvent(baseEvent, F("cnt2.5"), data[PMS_PM2_5_100ml_normal]);

            // Particle count per 0.1 L > 5 micron
            sendEvent(baseEvent, F("cnt5"),   data[PMS_PM5_0_100ml_normal]);

            // Particle count per 0.1 L > 10 micron
            sendEvent(baseEvent, F("cnt10"),  data[PMS_PM10_0_100ml_normal]);
            break;
          }
        }
        case PLUGIN_053_OUTPUT_THC:
        {
          // Particles > 1.0 um/m3
          sendEvent(baseEvent, F("pm1.0"), data[PMS_PM1_0_ug_m3_normal]);

          // Particles > 10 um/m3
          sendEvent(baseEvent, F("pm10"),  data[PMS_PM10_0_ug_m3_normal]);

          if (PLUGIN_053_EVENT_OUT_SELECTOR == PLUGIN_053_OUTPUT_CNT) {
            // Particle count per 0.1 L > 1.0 micron
            sendEvent(baseEvent, F("cnt1.0"), data[PMS_PM1_0_100ml_normal]);

            // Particle count per 0.1 L > 2.5 micron
            sendEvent(baseEvent, F("cnt2.5"), data[PMS_PM2_5_100ml_normal]);

            // Particle count per 0.1 L > 5 micron
            sendEvent(baseEvent, F("cnt5"),   data[PMS_PM5_0_100ml_normal]);

            // Particle count per 0.1 L > 10 micron
            sendEvent(baseEvent, F("cnt10"),  data[PMS_PM10_0_100ml_normal]);
          }
          break;
        }
        case PLUGIN_053_OUTPUT_CNT:
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
            // Formaldebyde (HCHO)
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

#endif // ifdef USES_P053
