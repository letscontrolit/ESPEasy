#include "_Plugin_Helper.h"

#ifdef USES_P199

// #######################################################################################################
// ############## Plugin 199: Modbus rain sensor                                           ###############
// #######################################################################################################

/*
   Plugin written by: Sergio Faustino sjfaustino__AT__gmail.com

   This plugin reads available values of an Eastron SDM120C SDM120/SDM120CT/220/230/630/72D & also DDM18SD.
 */



# define PLUGIN_199
# define PLUGIN_ID_199         199
# define PLUGIN_NAME_199       "[testing] Rainsensor - Modbus"
# define P199_NR_OUTPUT_VALUES                            1
# define PLUGIN_VALUENAME1_199 "Rain"


# include "src/PluginStructs/P199_data_struct.h"

// These pointers may be used among multiple instances of the same plugin,
// as long as the same serial settings are used.
ESPeasySerial *P199_ESPEasySerial = nullptr;
SDM *Plugin_199_SDM               = nullptr;
boolean P199_init                 = false;

boolean Plugin_199(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number           = PLUGIN_ID_199;
      dev.Type             = DEVICE_TYPE_SERIAL_PLUS1; // connected through 3 datapins
      dev.VType            = Sensor_VType::SENSOR_TYPE_SINGLE; // Only one value
      dev.FormulaOption    = true;
      dev.ValueCount       = P199_NR_OUTPUT_VALUES;
      dev.OutputDataType   = Output_Data_type_t::Simple;
      dev.SendDataOption   = true;
      dev.TimerOption      = true;
      dev.PluginStats      = true;
      dev.TaskLogsOwnPeaks = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_199);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_199));
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_modbus_getGpioNames(event);
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P199_DEV_ID   = P199_DEV_ID_DFLT;
      P199_MODEL    = P199_MODEL_DFLT;
      P199_BAUDRATE = P199_BAUDRATE_DFLT;

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    {
      if ((P199_DEV_ID == 0) || (P199_DEV_ID > 247) || (P199_BAUDRATE >= 6)) {
        // Load some defaults
        P199_DEV_ID   = P199_DEV_ID_DFLT;
        P199_MODEL    = P199_MODEL_DFLT;
        P199_BAUDRATE = P199_BAUDRATE_DFLT;
      }
      {
        String options_baudrate[6];

        for (int i = 0; i < 6; ++i) {
          options_baudrate[i] = P199_storageValueToBaudrate(i);
        }
        constexpr size_t optionCount = NR_ELEMENTS(options_baudrate);
        const FormSelectorOptions selector(optionCount, options_baudrate);
        selector.addFormSelector(F("Baud Rate"), P199_BAUDRATE_LABEL, P199_BAUDRATE);
        addUnit(F("baud"));
      }

      addFormNumericBox(F("Modbus Address"), P199_DEV_ID_LABEL, P199_DEV_ID, 1, 247);

      # ifdef ESP32
      addFormCheckBox(F("Enable Collision Detection"), F(P199_FLAG_COLL_DETECT_LABEL), P199_GET_FLAG_COLL_DETECT);
      addFormNote(F("/RE connected to GND, only supported on hardware serial"));
      # endif // ifdef ESP32

      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {

      P199_DEV_ID   = getFormItemInt(P199_DEV_ID_LABEL);
      P199_MODEL    = getFormItemInt(P199_MODEL_LABEL);
      P199_BAUDRATE = getFormItemInt(P199_BAUDRATE_LABEL);
      # ifdef ESP32
      P199_SET_FLAG_COLL_DETECT(isFormItemChecked(F(P199_FLAG_COLL_DETECT_LABEL)));
      # endif // ifdef ESP32

      P199_init = false; // Force device setup next time
      success         = true;
      break;
    }

    case PLUGIN_INIT:
    {
      P199_init = true;

      if (P199_ESPEasySerial != nullptr) {
        delete P199_ESPEasySerial;
        P199_ESPEasySerial = nullptr;
      }
      P199_ESPEasySerial = new (std::nothrow) ESPeasySerial(static_cast<ESPEasySerialPort>(CONFIG_PORT), CONFIG_PIN1, CONFIG_PIN2);

      if (P199_ESPEasySerial == nullptr) {
        break;
      }

        String log = F("P199: Init serial: ");
        log += F("RX pin ");
        log += CONFIG_PIN1;
        log += F(", TX pin ");
        log += CONFIG_PIN2;
        log += F(", RS485 mode enabled on pin ");
        log += P199_DEPIN;
        log += F(", baudrate ");
        log += P199_storageValueToBaudrate(P199_BAUDRATE); 
        log += F(", collision detection ");
        log += P199_GET_FLAG_COLL_DETECT ? F("enabled") : F("disabled");
        addLogMove(LOG_LEVEL_INFO, log);

      if (P199_ESPEasySerial->setRS485Mode(P199_DEPIN, P199_GET_FLAG_COLL_DETECT)) {
        addLogMove(LOG_LEVEL_INFO, F("Using RS485 mode"));
      } else {
        addLogMove(LOG_LEVEL_INFO, F("Not using RS485 mode, using normal serial mode"));
      }      
      unsigned int baudrate = P199_storageValueToBaudrate(P199_BAUDRATE);

      P199_ESPEasySerial->begin(baudrate);

      success         = true;
      break;
    }

    case PLUGIN_EXIT:
    {
      P199_init = false;

      delete P199_ESPEasySerial;
      P199_ESPEasySerial = nullptr;

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {

      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      uint16_t value = 0;
      P199_modbus_readRegister(0x01, 0x0000, &value); // Example register address 0x0000
      addLogMove(LOG_LEVEL_INFO, concat(F("P199 : debug "), value));
      break;
    }
    
    case PLUGIN_READ:
    {
      uint16_t value = 0;
      P199_modbus_readRegister(0x01, 0x0000, &value); // Example register address 0x0000
      UserVar.setFloat(event->TaskIndex, 0, value);
      break;
    }


  }
  return success;
}

int P199_storageValueToBaudrate(uint8_t baudrate_setting) {
  int baudrate = 9600;

  if (baudrate_setting < 6) {
    baudrate = 1200 << baudrate_setting;
  }
  return baudrate;
}

int P199_modbus_readRegister(uint8_t node_id, uint16_t reg, uint16_t *value)
{
  char buffer[32];

  if (P199_ESPEasySerial == nullptr) {
    addLogMove(LOG_LEVEL_DEBUG, F("P199: Serial not initialized"));
    return -1; // Not initialized
  }

  for (int i = P199_ESPEasySerial->available(); i > 0; --i) {
    P199_ESPEasySerial->read(); // Clear any existing data in the buffer
  }

  buffer[0] = node_id;
  buffer[1] = 0x03; // Function code for reading holding registers
  buffer[2] = highByte(reg); // High byte of register address
  buffer[3] = lowByte(reg); // Low byte of register address
  buffer[4] = 0x00; // Number of registers to read (2 bytes)
  buffer[5] = 0x01; // Number of registers to read (2 bytes)
  uint16_t crc = P199_calculateCRC((uint8_t*)buffer, 6);
  buffer[6] = lowByte(crc); // CRC low byte
  buffer[7] = highByte(crc); // CRC high byte
  P199_dump_buffer((uint8_t*)buffer, 8); // Debug: Dump the buffer content
  P199_ESPEasySerial->write((uint8_t*)buffer, 8);
  unsigned long startTime = millis();
  while (P199_ESPEasySerial->available() < 5 && (millis() - startTime) < 1000) {
    delay(10); // Wait for response
  }
  if (P199_ESPEasySerial->available() >= 7) {
    uint8_t response[7];
    P199_ESPEasySerial->readBytes(response, 7);
    P199_dump_buffer((uint8_t*)response, 8);

    if (response[0] == node_id && response[1] == 0x03 && response[2] == 0x02) {
      *value = (response[3] << 8) | response[4]; // Combine high and low byte
      addLogMove(LOG_LEVEL_DEBUG, concat("P199: received value: ", *value));
      return 0; // Success
    } else {
      addLogMove(LOG_LEVEL_DEBUG, F("P199: Invalid response received"));      
      return -2; // Invalid response
    }
  } else {
    addLogMove(LOG_LEVEL_DEBUG, F("P199: Timeout waiting for response"));
    return -3; // Timeout
  } 
}

uint16_t P199_calculateCRC(const uint8_t *array, uint8_t len)  {
  uint16_t _crc, _flag;
  _crc = 0xFFFF;
  for (uint8_t i = 0; i < len; i++) {
    _crc ^= (uint16_t)array[i];
    for (uint8_t j = 8; j; j--) {
      _flag = _crc & 0x0001;
      _crc >>= 1;
      if (_flag)
        _crc ^= 0xA001;
    }
  }
  return _crc;
}

void P199_dump_buffer(const uint8_t *buffer, size_t length) {
  String log = F("P199: Dumping buffer: ");
  for (size_t i = 0; i < length; ++i) {
    log += String(buffer[i], HEX);
    if (i < length - 1) {
      log += F(", ");
    }
  }
  addLogMove(LOG_LEVEL_DEBUG, log);
}

#endif // USES_P199
