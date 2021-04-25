#include "_Plugin_Helper.h"
#ifdef USES_P053
//#######################################################################################################
//#################################### Plugin 053: Plantower PMSx003 ####################################
//#######################################################################################################
//
// http://www.aqmd.gov/docs/default-source/aq-spec/resources-page/plantower-pms5003-manual_v2-3.pdf?sfvrsn=2
//
// The PMSx003 are particle sensors. Particles are measured by blowing air through the enclosure and,
// together with a laser, count the amount of particles. These sensors have an integrated microcontroller
// that counts particles and transmits measurement data over the serial connection.


#include <ESPeasySerial.h>


#define PLUGIN_053
#define PLUGIN_ID_053 53
#define PLUGIN_NAME_053 "Dust - PMSx003"
#define PLUGIN_VALUENAME1_053 "pm1.0"
#define PLUGIN_VALUENAME2_053 "pm2.5"
#define PLUGIN_VALUENAME3_053 "pm10"
#define PMSx003_SIG1 0X42
#define PMSx003_SIG2 0X4d
#define PMSx003_SIZE 32

ESPeasySerial *P053_easySerial = nullptr;
boolean Plugin_053_init = false;
boolean values_received = false;

// Read 2 bytes from serial and make an uint16 of it. Additionally calculate
// checksum for PMSx003. Assumption is that there is data available, otherwise
// this function is blocking.
void SerialRead16(uint16_t* value, uint16_t* checksum)
{
  uint8_t data_high, data_low;

  // If P053_easySerial is initialized, we are using soft serial
  if (P053_easySerial == nullptr) return;
  data_high = P053_easySerial->read();
  data_low = P053_easySerial->read();

  *value = data_low;
  *value |= (data_high << 8);

  if (checksum != nullptr)
  {
    *checksum += data_high;
    *checksum += data_low;
  }

#if 0
  // Low-level logging to see data from sensor
  String log = F("PMSx003 : byte high=0x");
  log += String(data_high,HEX);
  log += F(" byte low=0x");
  log += String(data_low,HEX);
  log += F(" result=0x");
  log += String(*value,HEX);
  addLog(LOG_LEVEL_INFO, log);
#endif
}

void SerialFlush() {
  if (P053_easySerial != nullptr) {
    P053_easySerial->flush();
  }
}

boolean PacketAvailable(void)
{
  if (P053_easySerial != nullptr) // Software serial
  {
    // When there is enough data in the buffer, search through the buffer to
    // find header (buffer may be out of sync)
    if (!P053_easySerial->available()) return false;
    while ((P053_easySerial->peek() != PMSx003_SIG1) && P053_easySerial->available()) {
      P053_easySerial->read(); // Read until the buffer starts with the first byte of a message, or buffer empty.
    }
    if (P053_easySerial->available() < PMSx003_SIZE) return false; // Not enough yet for a complete packet
  }
  return true;
}

boolean Plugin_053_process_data(struct EventStruct *event) {
  uint16_t checksum = 0, checksum2 = 0;
  uint16_t framelength = 0;
  uint16_t packet_header = 0;
  SerialRead16(&packet_header, &checksum); // read PMSx003_SIG1 + PMSx003_SIG2
  if (packet_header != ((PMSx003_SIG1 << 8) | PMSx003_SIG2)) {
    // Not the start of the packet, stop reading.
    return false;
  }

  SerialRead16(&framelength, &checksum);
  if (framelength != (PMSx003_SIZE - 4))
  {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("PMSx003 : invalid framelength - ");
      log += framelength;
      addLog(LOG_LEVEL_ERROR, log);
    }
    return false;
  }

  uint16_t data[13]; // byte data_low, data_high;
  for (int i = 0; i < 13; i++)
    SerialRead16(&data[i], &checksum);

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("PMSx003 : pm1.0=");
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

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    String log = F("PMSx003 : count/0.1L : 0.3um=");
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
    addLog(LOG_LEVEL_DEBUG_MORE, log);
  }

  // Compare checksums
  SerialRead16(&checksum2, nullptr);
  SerialFlush(); // Make sure no data is lost due to full buffer.
  if (checksum == checksum2)
  {
    // Data is checked and good, fill in output
    UserVar[event->BaseVarIndex]     = data[3];
    UserVar[event->BaseVarIndex + 1] = data[4];
    UserVar[event->BaseVarIndex + 2] = data[5];
    values_received = true;
    return true;
  }
  return false;
}

boolean Plugin_053(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_053;
        Device[deviceCount].Type = DEVICE_TYPE_SERIAL_PLUS1;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_TRIPLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        success = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_053);
        success = true;
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_053));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_053));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_053));
        success = true;
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        serialHelper_getGpioNames(event);
        event->String3 = formatGpioName_output(F("Reset"));
        break;
      }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
      {
        string += serialHelper_getSerialTypeLabel(event);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_LOAD: {
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      success = true;
      break;
    }

    case PLUGIN_INIT:
      {
        int rxPin = CONFIG_PIN1;
        int txPin = CONFIG_PIN2;
        const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
        int resetPin = CONFIG_PIN3;

        String log = F("PMSx003 : config ");
        log += rxPin;
        log += txPin;
        log += resetPin;
        addLog(LOG_LEVEL_DEBUG, log);

        if (P053_easySerial != nullptr) {
          // Regardless the set pins, the software serial must be deleted.
          delete P053_easySerial;
          P053_easySerial = nullptr;
        }

        // Hardware serial is RX on 3 and TX on 1
        if (rxPin == 3 && txPin == 1)
        {
          log = F("PMSx003 : using hardware serial");
          addLog(LOG_LEVEL_INFO, log);
        }
        else
        {
          log = F("PMSx003: using software serial");
          addLog(LOG_LEVEL_INFO, log);
        }
        P053_easySerial = new (std::nothrow) ESPeasySerial(port, rxPin, txPin, false, 96); // 96 Bytes buffer, enough for up to 3 packets.
        if (P053_easySerial == nullptr) {
          break;
        }
        P053_easySerial->begin(9600);
        P053_easySerial->flush();

        if (resetPin >= 0) // Reset if pin is configured
        {
          // Toggle 'reset' to assure we start reading header
          log = F("PMSx003: resetting module");
          addLog(LOG_LEVEL_INFO, log);
          pinMode(resetPin, OUTPUT);
          digitalWrite(resetPin, LOW);
          delay(250);
          digitalWrite(resetPin, HIGH);
          pinMode(resetPin, INPUT_PULLUP);
        }

        Plugin_053_init = true;
        success = true;
        break;
      }

    case PLUGIN_EXIT:
      {
          if (P053_easySerial)
          {
            delete P053_easySerial;
            P053_easySerial=nullptr;
          }
          break;
      }

    // The update rate from the module is 200ms .. multiple seconds. Practise
    // shows that we need to read the buffer many times per seconds to stay in
    // sync.
    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_053_init)
        {
          // Check if a complete packet is available in the UART FIFO.
          if (PacketAvailable())
          {
            addLog(LOG_LEVEL_DEBUG_MORE, F("PMSx003 : Packet available"));
            success = Plugin_053_process_data(event);
          }
        }
        break;
      }
    case PLUGIN_READ:
      {
        // When new data is available, return true
        success = values_received;
        values_received = false;
        break;
      }
  }
  return success;
}
#endif // USES_P053
