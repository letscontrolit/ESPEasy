#include "../PluginStructs/P053_data_struct.h"

#ifdef USES_P053

P053_data_struct::P053_data_struct(
  int8_t                  rxPin,
  int8_t                  txPin,
  const ESPEasySerialPort port,
  int8_t                  resetPin,
  uint8_t                 sensortype)
  : Plugin_053_sensortype(sensortype) {

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


  // Hardware serial is RX on 3 and TX on 1
  if ((rxPin == 3) && (txPin == 1)) {
    addLog(LOG_LEVEL_INFO, F("PMSx003 : using hardware serial"));
  } else {
    addLog(LOG_LEVEL_INFO, F("PMSx003: using software serial"));
  }
  easySerial = new (std::nothrow) ESPeasySerial(port, rxPin, txPin, false, 96); // 96 Bytes buffer, enough for up to 3 packets.

  if (easySerial != nullptr) {
    easySerial->begin(9600);
    easySerial->flush();

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
  if (easySerial != nullptr) {
    // Regardless the set pins, the software serial must be deleted.
    delete easySerial;
    easySerial = nullptr;
  }
}

bool P053_data_struct::initialized() const
{
  return easySerial != nullptr;
}

// Read 2 bytes from serial and make an uint16 of it. Additionally calculate
// checksum for PMSx003. Assumption is that there is data available, otherwise
// this function is blocking.
void P053_data_struct::SerialRead16(uint16_t *value, uint16_t *checksum)
{
  uint8_t data_high, data_low;

  if (!initialized()) { return; }
  data_high = easySerial->read();
  data_low  = easySerial->read();

  *value  = data_low;
  *value |= (data_high << 8);

  if (checksum != nullptr)
  {
    *checksum += data_high;
    *checksum += data_low;
  }

# if 0

  // Low-level logging to see data from sensor
  String log = F("PMSx003 : uint8_t high=0x");
  log += String(data_high, HEX);
  log += F(" uint8_t low=0x");
  log += String(data_low, HEX);
  log += F(" result=0x");
  log += String(*value, HEX);
  addLog(LOG_LEVEL_INFO, log);
# endif // if 0
}

void P053_data_struct::SerialFlush() {
  if (easySerial != nullptr) {
    easySerial->flush();
  }
}

uint8_t P053_data_struct::packetSize(uint8_t sensorType) {
  switch (sensorType) {
    case PMSx003_TYPE:    return PMSx003_SIZE;
    # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    case PMSx003_TYPE_S:  return PMSx003S_SIZE;
    case PMSx003_TYPE_T:  return PMSx003T_SIZE;
    case PMSx003_TYPE_ST: return PMSx003ST_SIZE;
    case PMS3003_TYPE:    return PMS3003_SIZE;
    # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  }
  return 0u;
}

bool P053_data_struct::packetAvailable()
{
  if (easySerial != nullptr) // Software serial
  {
    // When there is enough data in the buffer, search through the buffer to
    // find header (buffer may be out of sync)
    if (!easySerial->available()) { return false; }

    while ((easySerial->peek() != PMSx003_SIG1) && easySerial->available()) {
      easySerial->read(); // Read until the buffer starts with the
      // first uint8_t of a message, or buffer
      // empty.
    }

    if (easySerial->available() < packetSize(Plugin_053_sensortype)) { return false; // Not enough yet for a complete packet
    }
  }
  return true;
}

void P053_data_struct::sendEvent(const String& baseEvent, const String& name, double value) {
  String valueEvent;

  valueEvent.reserve(32);

  // Temperature
  valueEvent  = baseEvent;
  valueEvent += name;
  valueEvent += '=';
  valueEvent += value;
  eventQueue.addMove(std::move(valueEvent));
}

bool P053_data_struct::processData(struct EventStruct *event) {
  uint16_t checksum = 0, checksum2 = 0;
  uint16_t framelength   = 0;
  uint16_t packet_header = 0;

  SerialRead16(&packet_header, &checksum); // read PMSx003_SIG1 + PMSx003_SIG2

  if (packet_header != ((PMSx003_SIG1 << 8) | PMSx003_SIG2)) {
    // Not the start of the packet, stop reading.
    return false;
  }

  SerialRead16(&framelength, &checksum);

  if (framelength != (packetSize(Plugin_053_sensortype) - 4)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log;
      log.reserve(34);
      log  = F("PMSx003 : invalid framelength - ");
      log += framelength;
      addLog(LOG_LEVEL_ERROR, log);
    }
    return false;
  }

  uint8_t frameData = 0;

  switch (Plugin_053_sensortype) {
    case PMSx003_TYPE:    frameData = 13; break; // PMS1003/PMS5003/PMS7003
    case PMS3003_TYPE:    frameData =  9; break; // PMS2003/PMS3003
    case PMSx003_TYPE_ST: frameData = 17; break; // PMS5003ST
    default: break;
  }
  uint16_t data[17] = { 0 };                     // uint8_t data_low, data_high;

  for (uint8_t i = 0; i < frameData; i++) {
    SerialRead16(&data[i], &checksum);
  }

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) { // Available on all supported sensor models
    String log;
    log.reserve(87);
    log  = F("PMSx003 : pm1.0=");
    log += data[0];
    log += F(", pm2.5=");
    log += data[1];
    log += F(", pm10=");
    log += data[2];
    log += F(", pm1.0a=");
    log += data[3];
    log += F(", pm2.5a=");
    log += data[4];
    log += F(", pm10a=");
    log += data[5];
    addLog(LOG_LEVEL_DEBUG, log);
  }

  if (loglevelActiveFor(LOG_LEVEL_DEBUG) && (PCONFIG(0) != PMS3003_TYPE)) { // 'Count' values not available on PMS2003/PMS3003 models
                                                                            // (handled as 1 model in code)
    String log;
    log.reserve(96);
    log  = F("PMSx003 : count/0.1L : 0.3um=");
    log += data[6];
    log += F(", 0.5um=");
    log += data[7];
    log += F(", 1.0um=");
    log += data[8];
    log += F(", 2.5um=");
    log += data[9];
    log += F(", 5.0um=");
    log += data[10];
    log += F(", 10um=");
    log += data[11];
    addLog(LOG_LEVEL_DEBUG, log);
  }

  # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  if (loglevelActiveFor(LOG_LEVEL_DEBUG) && (PCONFIG(0) == PMSx003_TYPE_ST)) { // Values only available on PMS5003ST
    String log;
    log.reserve(45);
    log  = F("PMSx003 : temp=");
    log += static_cast<float>(data[13]) / 10.0f;
    log += F(", humi=");
    log += static_cast<float>(data[14]) / 10.0f;
    log += F(", hcho=");
    log += static_cast<float>(data[12]) / 1000.0f;
    addLog(LOG_LEVEL_DEBUG, log);
  }
  # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  // Compare checksums
  SerialRead16(&checksum2, nullptr);
  SerialFlush(); // Make sure no data is lost due to full buffer.

  if (checksum == checksum2)
  {
    // Data is checked and good, fill in output
    # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

    switch (PCONFIG(1)) {
      case PLUGIN_053_OUTPUT_PART:
      {
    # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    UserVar[event->BaseVarIndex]     = data[3];
    UserVar[event->BaseVarIndex + 1] = data[4];
    UserVar[event->BaseVarIndex + 2] = data[5];
    # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    UserVar[event->BaseVarIndex + 3] = 0.0;
    break;
  }
  case PLUGIN_053_OUTPUT_THC:
  {
    UserVar[event->BaseVarIndex]     = data[4];
    UserVar[event->BaseVarIndex + 1] = static_cast<float>(data[13]) / 10.0f;   // TEMP
    UserVar[event->BaseVarIndex + 2] = static_cast<float>(data[14]) / 10.0f;   // HUMI
    UserVar[event->BaseVarIndex + 3] = static_cast<float>(data[12]) / 1000.0f; // HCHO
    break;
  }
  case PLUGIN_053_OUTPUT_CNT:
  {
    UserVar[event->BaseVarIndex]     = data[8];
    UserVar[event->BaseVarIndex + 1] = data[9];
    UserVar[event->BaseVarIndex + 2] = data[10];
    UserVar[event->BaseVarIndex + 3] = data[11];
    break;
  }
  default:
    break;                                                                   // Ignore invalid options
}

if (Settings.UseRules && (PCONFIG(2) > 0) && (PCONFIG(0) != PMS3003_TYPE)) { // Events not applicable to PMS2003 & PMS3003 models
  String baseEvent;
  baseEvent.reserve(21);
  baseEvent  = getTaskDeviceName(event->TaskIndex);
  baseEvent += '#';

  switch (PCONFIG(1)) {
    case PLUGIN_053_OUTPUT_PART:
    {
      // Temperature
      sendEvent(baseEvent, F("Temp"), static_cast<float>(data[13]) / 10.0f);

      // Humidity
      sendEvent(baseEvent, F("Humi"), static_cast<float>(data[14]) / 10.0f);

      // Formaldebyde (HCHO)
      sendEvent(baseEvent, F("HCHO"), static_cast<float>(data[12]) / 1000.0f);

      if (PCONFIG(2) == PLUGIN_053_OUTPUT_CNT) {
        // Particle count per 0.1 L > 1.0 micron
        sendEvent(baseEvent, F("cnt1.0"), data[8]);

        // Particle count per 0.1 L > 2.5 micron
        sendEvent(baseEvent, F("cnt2.5"), data[9]);

        // Particle count per 0.1 L > 5 micron
        sendEvent(baseEvent, F("cnt5"),   data[10]);

        // Particle count per 0.1 L > 10 micron
        sendEvent(baseEvent, F("cnt10"),  data[11]);
        break;
      }
    }
    case PLUGIN_053_OUTPUT_THC:
    {
      // Particles > 1.0 um/m3
      sendEvent(baseEvent, F("pm1.0"), data[3]);

      // Particles > 10 um/m3
      sendEvent(baseEvent, F("pm10"),  data[5]);

      if (PCONFIG(2) == PLUGIN_053_OUTPUT_CNT) {
        // Particle count per 0.1 L > 1.0 micron
        sendEvent(baseEvent, F("cnt1.0"), data[8]);

        // Particle count per 0.1 L > 2.5 micron
        sendEvent(baseEvent, F("cnt2.5"), data[9]);

        // Particle count per 0.1 L > 5 micron
        sendEvent(baseEvent, F("cnt5"),   data[10]);

        // Particle count per 0.1 L > 10 micron
        sendEvent(baseEvent, F("cnt10"),  data[11]);
      }
      break;
    }
    case PLUGIN_053_OUTPUT_CNT:
    {
      // Particles > 1.0 um/m3
      sendEvent(baseEvent, F("pm1.0"), data[3]);

      // Particles > 2.5 um/m3
      sendEvent(baseEvent, F("pm2.5"), data[4]);

      // Particles > 10 um/m3
      sendEvent(baseEvent, F("pm10"),  data[5]);

      // Temperature
      sendEvent(baseEvent, F("Temp"),  static_cast<float>(data[13]) / 10.0f);

      // Humidity
      sendEvent(baseEvent, F("Humi"),  static_cast<float>(data[14]) / 10.0f);

      // Formaldebyde (HCHO)
      sendEvent(baseEvent, F("HCHO"),  static_cast<float>(data[12]) / 1000.0f);
      break;
    }
    default:
      break;
  }
}
    # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    values_received = true;
    return true;
  }
  return false;
}

bool P053_data_struct::checkAndClearValuesReceived() {
  const bool ret = values_received;

  values_received = false;
  return ret;
}

#endif // ifdef USES_P053
