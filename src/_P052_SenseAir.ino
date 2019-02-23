#ifdef USES_P052
//#######################################################################################################
//############################# Plugin 052: Senseair CO2 Sensors ########################################
//#######################################################################################################
/*
  Plugin originally written by: Daniel Tedenljung
  info__AT__tedenljungconsulting.com
  Rewritten by: Mikael Trieb mikael__AT__triebconsulting.se

  This plugin reads available values of Senseair Co2 Sensors.
  Datasheet can be found here:
  S8: http://www.senseair.com/products/oem-modules/senseair-s8/
  K30: http://www.senseair.com/products/oem-modules/k30/
  K70/tSENSE: http://www.senseair.com/products/wall-mount/tsense/

  Circuit wiring
    GPIO Setting 1 -> RX
    GPIO Setting 2 -> TX
    Use 1kOhm in serie on datapins!
*/

#define PLUGIN_052
#define PLUGIN_ID_052 52
#define PLUGIN_NAME_052 "Gases - CO2 Senseair"
#define PLUGIN_VALUENAME1_052 ""

#define P052_MEASUREMENT_INTERVAL 60000L

#define P052_READ_HOLDING_REGISTERS 0x03
#define P052_READ_INPUT_REGISTERS 0x04
#define P052_WRITE_SINGLE_REGISTER 0x06

#define P052_CMD_READ_RAM 0x44
#define P052_CMD_READ_EEPROM 0x46
#define P052_CMD_WRITE_RAM 0x41
#define P052_CMD_WRITE_EEPROM 0x43

// For layout and status flags in RAM/EEPROM, see document
// "CO2-Engine-BLG_ELG configuration guide Rev 1_02.docx"

// RAM layout
#define P052_RAM_ADDR_ERROR_STATUS 0x1E      // U8 (error flags)
#define P052_RAM_ADDR_METER_STATUS 0x1D      // U8 (status flags)
#define P052_RAM_ADDR_ALARM_STATUS 0x1C      // U8 (alarm flags)
#define P052_RAM_ADDR_CO2 0x08               // S16 BLG: x.xxx%  ELG: x ppm
#define P052_RAM_ADDR_SPACE_TEMPERATURE 0x12 // S16 x.xx °C
#define P052_RAM_ADDR_RELATIVE_HUMIDITY 0x14 // S16 x.xx %
#define P052_RAM_ADDR_MIXING_RATIO 0x16      // S16 x.xx g/kg
#define P052_RAM_ADDR_HR1 0x40               // U16
#define P052_RAM_ADDR_HR2 0x42               // U16
#define P052_RAM_ADDR_ANIN4 0x69             // U16 x.xxx volt
#define P052_RAM_ADDR_RTC 0x65 // U32 x seconds   (virtual real time clock)
#define P052_RAM_ADDR_SCR 0x60 // U8 (special control register)

// EEPROM layout
#define P052_EEPROM_ADDR_METERCONTROL 0x03    // U8
#define P052_EEPROM_ADDR_METERCONFIG 0x06     // U16
#define P052_EEPROM_ADDR_ABC_PERIOD 0x40      // U16 ABC period in hours
#define P052_EEPROM_ADDR_HEARTBEATPERIOD 0xA2 // U8 Period in seconds
#define P052_EEPROM_ADDR_PUMPPERIOD 0xA3      // U8 Period in seconds
#define P052_EEPROM_ADDR_MEASUREMENT_SLEEP_PERIOD  0xB0 // U24 Measurement period (unit = seconds)
#define P052_EEPROM_ADDR_LOGGER_STRUCTURE_ADDRESS  0x200 // 16b Described in “BLG_ELG Logger Structure”

// SCR (Special Control Register) commands
#define P052_SCR_FORCE_START_MEASUREMENT 0x30
#define P052_SCR_FORCE_STOP_MEASUREMENT 0x31
#define P052_SCR_RESTART_LOGGER 0x32      // (logger data erased)
#define P052_SCR_REINITIALIZE_LOGGER 0x33 // (logger data unaffected)
#define P052_SCR_WRITE_TIMESTAMP_TO_LOGGER 0x34
#define P052_SCR_SINGLE_MEASUREMENT 0x35

#define P052_IR_METERSTATUS 0
#define P052_IR_ALARMSTATUS 1
#define P052_IR_OUTPUTSTATUS 2
#define P052_IR_SPACE_CO2 3

#define P052_HR_ACK_REG 0
#define P052_HR_SPACE_CO2 3
#define P052_HR_ABC_PERIOD 31

#define P052_MODBUS_RECEIVE_BUFFER 256
#define P052_MODBUS_SLAVE_ADDRESS 0xFE // Modbus "any address"

#define MODBUS_EXCEPTION_ILLEGAL_FUNCTION        1
#define MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS    2
#define MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE      3
#define MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE 4
#define MODBUS_EXCEPTION_ACKNOWLEDGE             5
#define MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY    6
#define MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE    7
#define MODBUS_EXCEPTION_MEMORY_PARITY           8
#define MODBUS_EXCEPTION_NOT_DEFINED             9
#define MODBUS_EXCEPTION_GATEWAY_PATH            10
#define MODBUS_EXCEPTION_GATEWAY_TARGET          11

/* Additional error codes for the Plugin_052_processCommand return values */
#define Plugin_052_MODBUS_BADCRC   (MODBUS_EXCEPTION_GATEWAY_TARGET + 1)
#define Plugin_052_MODBUS_BADDATA  (MODBUS_EXCEPTION_GATEWAY_TARGET + 2)
#define Plugin_052_MODBUS_BADEXC   (MODBUS_EXCEPTION_GATEWAY_TARGET + 3)
#define Plugin_052_MODBUS_UNKEXC   (MODBUS_EXCEPTION_GATEWAY_TARGET + 4)
#define Plugin_052_MODBUS_MDATA    (MODBUS_EXCEPTION_GATEWAY_TARGET + 5)
#define Plugin_052_MODBUS_BADSLAVE (MODBUS_EXCEPTION_GATEWAY_TARGET + 6)

#include <ESPeasySerial.h>

byte _plugin_052_sendframe[8] = {0};
byte _plugin_052_sendframe_used = 0;
byte _plugin_052_recv_buf[P052_MODBUS_RECEIVE_BUFFER] = {0xff};
byte _plugin_052_recv_buf_used = 0;
boolean Plugin_052_init = false;
String detected_device_description;

unsigned int _plugin_052_last_measurement = 0;
uint32_t _plugin_052_reads_pass = 0;
uint32_t _plugin_052_reads_crc_failed = 0;

ESPeasySerial *P052_easySerial;

boolean Plugin_052(byte function, struct EventStruct *event, String &string) {
  boolean success = false;
  switch (function) {

  case PLUGIN_DEVICE_ADD: {
    Device[++deviceCount].Number = PLUGIN_ID_052;
    Device[deviceCount].Type = DEVICE_TYPE_DUAL;
    Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
    Device[deviceCount].Ports = 0;
    Device[deviceCount].PullUpOption = false;
    Device[deviceCount].InverseLogicOption = false;
    Device[deviceCount].FormulaOption = true;
    Device[deviceCount].ValueCount = 1;
    Device[deviceCount].SendDataOption = true;
    Device[deviceCount].TimerOption = true;
    Device[deviceCount].GlobalSyncOption = true;
    break;
  }

  case PLUGIN_GET_DEVICENAME: {
    string = F(PLUGIN_NAME_052);
    break;
  }

  case PLUGIN_GET_DEVICEVALUENAMES: {
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0],
             PSTR(PLUGIN_VALUENAME1_052));
    break;
  }

  case PLUGIN_GET_DEVICEGPIONAMES: {
    serialHelper_getGpioNames(event);
    break;
  }

  case PLUGIN_WRITE: {
    String cmd = parseString(string, 1);
    String param1 = parseString(string, 2);

    if (cmd.equalsIgnoreCase(F("senseair_setrelay"))) {
      int par1;
      if (validIntFromString(param1, par1)) {
        if (par1 == 0 || par1 == 1 || par1 == -1) {
          Plugin_052_setRelayStatus(par1);
          addLog(LOG_LEVEL_INFO,
                 String(F("Senseair command: relay=")) + param1);
        }
      }
      success = true;
    }

    /*
    // ABC functionality disabled for now, due to a bug in the firmware.
    // See https://github.com/letscontrolit/ESPEasy/issues/759
    if (cmd.equalsIgnoreCase(F("senseair_setABCperiod")))
    {
      if (param1.toInt() >= 0) {
        Plugin_052_setABCperiod(param1.toInt());
        addLog(LOG_LEVEL_INFO, String(F("Senseair command: ABCperiod=")) +
    param1);
      }
      success = true;
    }
    */

    break;
  }

  case PLUGIN_WEBFORM_LOAD: {
    serialHelper_webformLoad(event);
    byte choiceSensor = PCONFIG(0);

    String optionsSensor[7] = {F("Error Status"), F("Carbon Dioxide"),
                               F("Temperature"),  F("Humidity"),
                               F("Relay Status"), F("Temperature Adjustment"),
                               F("ABC period")};
    addFormSelector(F("Sensor"), F("p052_sensor"), 7, optionsSensor, NULL,
                    choiceSensor);

    /*
    // ABC functionality disabled for now, due to a bug in the firmware.
    // See https://github.com/letscontrolit/ESPEasy/issues/759
    byte choiceABCperiod = PCONFIG(1);
    String optionsABCperiod[9] = { F("disable"), F("1 h"), F("12 h"), F("1
    day"), F("2 days"), F("4 days"), F("7 days"), F("14 days"), F("30 days") };
    addFormSelector(F("ABC period"), F("p052_ABC_period"), 9, optionsABCperiod,
    NULL, choiceABCperiod);
    */

    if (detected_device_description.length() > 0) {
      String detectedString;
      detectedString += detected_device_description;
      detectedString += F("detected");
      addFormNote(detectedString);
    }
    addRowLabel(F("Checksum (pass/fail)"));
    String chksumStats;
    chksumStats = _plugin_052_reads_pass;
    chksumStats += '/';
    chksumStats += _plugin_052_reads_crc_failed;
    addHtml(chksumStats);
    success = true;
    break;
  }

  case PLUGIN_WEBFORM_SAVE: {
    serialHelper_webformSave(event);
    PCONFIG(0) = getFormItemInt(F("p052_sensor"));
    /*
    // ABC functionality disabled for now, due to a bug in the firmware.
    // See https://github.com/letscontrolit/ESPEasy/issues/759
    PCONFIG(1) = getFormItemInt(F("p052_ABC_period"));
    */

    success = true;
    break;
  }

  case PLUGIN_INIT: {
    Plugin_052_init = true;
    P052_easySerial = new ESPeasySerial(CONFIG_PIN1, CONFIG_PIN2);

    /*
    // ABC functionality disabled for now, due to a bug in the firmware.
    // See https://github.com/letscontrolit/ESPEasy/issues/759
    const int periodInHours[9] = {0, 1, 12, (24*1), (24*2), (24*4), (24*7),
    (24*14), (24*30) };
    byte choiceABCperiod = PCONFIG(1);

    Plugin_052_setABCperiod(periodInHours[choiceABCperiod]);
    */

    success = true;
    Plugin_052_modbus_log_MEI(P052_MODBUS_SLAVE_ADDRESS);
    detected_device_description =
        Plugin_052_getDevice_description(P052_MODBUS_SLAVE_ADDRESS);
    addLog(LOG_LEVEL_INFO, detected_device_description);

    break;
  }

  case PLUGIN_READ: {

    if (Plugin_052_init) {

      String log = F("Senseair: ");
      switch (PCONFIG(0)) {
      case 0: {
        int errorWord = Plugin_052_readErrorStatus();
        for (size_t i = 0; i < 9; i++) {
          if (bitRead(errorWord, i)) {
            UserVar[event->BaseVarIndex] = i;
            log += F("error code = ");
            log += i;
            break;
          }
        }

        UserVar[event->BaseVarIndex] = -1;
        log += F("error code = ");
        log += -1;
        break;
      }
      case 1: {
        int co2 = Plugin_052_readCo2();
        UserVar[event->BaseVarIndex] = co2;
        log += F("co2 = ");
        log += co2;
        break;
      }
      case 2: {
        float temperature = Plugin_052_readTemperature();
        UserVar[event->BaseVarIndex] = (float)temperature;
        log += F("temperature = ");
        log += (float)temperature;
        break;
      }
      case 3: {
        float relativeHumidity = Plugin_052_readRelativeHumidity();
        UserVar[event->BaseVarIndex] = (float)relativeHumidity;
        log += F("humidity = ");
        log += (float)relativeHumidity;
        break;
      }
      case 4: {
        int relayStatus = Plugin_052_readRelayStatus();
        UserVar[event->BaseVarIndex] = relayStatus;
        log += F("relay status = ");
        log += relayStatus;
        break;
      }
      case 5: {
        int temperatureAdjustment = Plugin_052_readTemperatureAdjustment();
        UserVar[event->BaseVarIndex] = temperatureAdjustment;
        log += F("temperature adjustment = ");
        log += temperatureAdjustment;
        break;
      }
      }
      addLog(LOG_LEVEL_INFO, log);

      success = true;
      break;
    }
    break;
  }
  }
  return success;
}

String Plugin_052_getDevice_description(byte slaveAddress) {
  bool more_follows = true;
  byte next_object_id = 0;
  byte conformity_level = 0;
  unsigned int object_value_int;
  String description;
  String obj_text;
//  description = F("address: ");
//  description += String(Plugin_052_readModbusAddress());
  for (byte object_id = 0; object_id < 0x84; ++object_id) {
    if (object_id == 6) {
      object_id = 0x82; // Skip to the serialnr/sensor type
    }
    int result = Plugin_052_modbus_get_MEI(slaveAddress, object_id, obj_text,
                                       object_value_int, next_object_id,
                                       more_follows, conformity_level);
    String label;
    switch (object_id) {
    case 0x01:
      if (result == 0) label = F("Pcode");
      break;
    case 0x02:
      if (result == 0) label = F("Rev");
      break;
    case 0x82:
    {
      if (result != 0) {
        uint32_t sensorId = Plugin_052_readSensorId();
        obj_text = String(sensorId, HEX);
        result = 0;

      }
      if (result == 0) label = F("S/N");
      break;
    }
    case 0x83:
    {
      if (result != 0) {
        uint32_t sensorId = Plugin_052_readTypeId();
        obj_text = String(sensorId, HEX);
        result = 0;
      }
      if (result == 0) label = F("Type");
      break;
    }
    default:
      break;
    }
    if (result == 0) {
      if (label.length() > 0) {
        // description += Plugin_052_MEI_objectid_to_name(object_id);
        description += label;
        description += F(": ");
      }
      if (obj_text.length() > 0) {
        description += obj_text;
        description += F(" - ");
      }
    }
  }
  return description;
}

// Read from RAM or EEPROM
void Plugin_052_buildRead_RAM_EEPROM(byte slaveAddress, byte functionCode,
                                     short startAddress, byte number_bytes) {
  _plugin_052_sendframe[0] = slaveAddress;
  _plugin_052_sendframe[1] = functionCode;
  _plugin_052_sendframe[2] = (byte)(startAddress >> 8);
  _plugin_052_sendframe[3] = (byte)(startAddress & 0xFF);
  _plugin_052_sendframe[4] = number_bytes;
  _plugin_052_sendframe_used = 5;
}

// Write to the Special Control Register (SCR)
void Plugin_052_buildWriteCommandRegister(byte slaveAddress, byte value) {
  _plugin_052_sendframe[0] = slaveAddress;
  _plugin_052_sendframe[1] = P052_CMD_WRITE_RAM;
  _plugin_052_sendframe[2] = 0;    // Address-Hi SCR  (0x0060)
  _plugin_052_sendframe[3] = 0x60; // Address-Lo SCR
  _plugin_052_sendframe[4] = 1;    // Count
  _plugin_052_sendframe[5] = value;
  _plugin_052_sendframe_used = 6;
}

void Plugin_052_buildFrame(byte slaveAddress, byte functionCode,
                           short startAddress, short parameter) {
  _plugin_052_sendframe[0] = slaveAddress;
  _plugin_052_sendframe[1] = functionCode;
  _plugin_052_sendframe[2] = (byte)(startAddress >> 8);
  _plugin_052_sendframe[3] = (byte)(startAddress & 0xFF);
  _plugin_052_sendframe[4] = (byte)(parameter >> 8);
  _plugin_052_sendframe[5] = (byte)(parameter & 0xFF);
  _plugin_052_sendframe_used = 6;
}

void Plugin_052_build_modbus_MEI_frame(byte slaveAddress, byte device_id,
                                       byte object_id) {
  _plugin_052_sendframe[0] = slaveAddress;
  _plugin_052_sendframe[1] = 0x2B;
  _plugin_052_sendframe[2] = 0x0E;

  // The parameter "Read Device ID code" allows to define four access types :
  // 01: request to get the basic device identification (stream access)
  // 02: request to get the regular device identification (stream access)
  // 03: request to get the extended device identification (stream access)
  // 04: request to get one specific identification object (individual access)
  _plugin_052_sendframe[3] = device_id;
  _plugin_052_sendframe[4] = object_id;
  _plugin_052_sendframe_used = 5;
}

String Plugin_052_MEI_objectid_to_name(byte object_id) {
  switch (object_id) {
  case 0:
    return F("VendorName");
  case 1:
    return F("ProductCode");
  case 2:
    return F("MajorMinorRevision");
  case 3:
    return F("VendorUrl");
  case 4:
    return F("ProductName");
  case 5:
    return F("ModelName");
  case 6:
    return F("UserApplicationName");
  case 0x80:
    return F("MemoryMapVersion");
  case 0x81:
    return F("Firmware Rev.");
  case 0x82:
    return F("Sensor S/N");
  case 0x83:
    return F("Sensor type");
  default:
    return String(F("0x")) + String(object_id, HEX);
  }
}

String Plugin_052_parse_modbus_MEI_response(unsigned int &object_value_int,
                                            byte &next_object_id,
                                            bool &more_follows,
                                            byte &conformity_level) {
  String result;
  if (_plugin_052_recv_buf_used < 8) {
    // Too small.
    addLog(LOG_LEVEL_INFO,
           String(F("MEI response too small: ")) + _plugin_052_recv_buf_used);
    next_object_id = 0xFF;
    more_follows = false;
    return result;
  }
  int pos = 4; // Data skipped: slave_address, FunctionCode, MEI type, ReadDevId
  // See http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
  // Page 45
  conformity_level             = _plugin_052_recv_buf[pos++];
  more_follows                 = _plugin_052_recv_buf[pos++] != 0;
  next_object_id               = _plugin_052_recv_buf[pos++];
  const byte number_objects    = _plugin_052_recv_buf[pos++];
  byte object_id = 0;
  for (int i = 0; i < number_objects; ++i) {
    if ((pos + 3) < _plugin_052_recv_buf_used) {
      object_id                = _plugin_052_recv_buf[pos++];
      const byte object_length = _plugin_052_recv_buf[pos++];
      if ((pos + object_length) < _plugin_052_recv_buf_used) {
        String object_value;
        if (object_id < 0x80) {
          // Parse as type String
          object_value.reserve(object_length);
          object_value_int = static_cast<unsigned int>(-1);
          for (int c = 0; c < object_length; ++c) {
            object_value += char(_plugin_052_recv_buf[pos++]);
          }
        } else {
          object_value.reserve(2 * object_length + 2);
          object_value_int = 0;
          for (int c = 0; c < object_length; ++c) {
            object_value_int =
                object_value_int << 8 | _plugin_052_recv_buf[pos++];
          }
          object_value = F("0x");
          String hexvalue(object_value_int, HEX);
          hexvalue.toUpperCase();
          object_value += hexvalue;
        }
        if (i != 0) {
          // Append to existing description
          result += String(F(",  "));
        }
        result += object_value;
      }
    }
  }
  return result;
}

String Plugin_052_log_buffer(byte *buffer, int length) {
  String log;
  log.reserve(3 * length + 5);
  for (int i = 0; i < length; ++i) {
    String hexvalue(buffer[i], HEX);
    hexvalue.toUpperCase();
    log += hexvalue;
    log += F(" ");
  }
  log += F("(");
  log += length;
  log += F(")");
  return log;
}

bool Plugin_052_check_error_status() {
  byte error_status = Plugin_052_read_RAM_EEPROM(P052_CMD_READ_RAM,
                                                 P052_RAM_ADDR_ERROR_STATUS, 1);
  if (error_status == 0)
    return true;
  String log = F("P052 Error status:");
  for (int i = 0; i < 8; ++i) {
    if (getBitOfInt(error_status, i)) {
      log += F(" (");
      switch (i) {
      case 0:
        log += F("fatal");
        break;
      case 1:
        log += F("CO2");
        break;
      case 2:
        log += F("T/H comm");
        break;
      case 4:
        log += F("detector temp range");
        break;
      case 5:
        log += F("CO2 range");
        break;
      case 6:
        log += F("mem err.");
        break;
      case 7:
        log += F("room temp range");
        break;
      default:
        log += F("unknown");
        break;
      }
      log += F(")");
    }
  }
  addLog(LOG_LEVEL_INFO, log);
  return false;
}

void Plugin_052_logModbusException(byte value) {
  if (value == 0)
    return;
  // Exception Response, see:
  // http://digital.ni.com/public.nsf/allkb/E40CA0CFA0029B2286256A9900758E06?OpenDocument
  String log = F("Modbus Exception - ");
  switch (value) {
  case MODBUS_EXCEPTION_ILLEGAL_FUNCTION: {
    // The function code received in the query is not an allowable action for
    // the slave.
    // If a Poll Program Complete command was issued, this code indicates that
    // no program function preceded it.
    log += F("Illegal Function (not allowed by client)");
    break;
  }
  case MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS: {
    // The data address received in the query is not an allowable address for
    // the slave.
    log += F("Illegal Data Address");
    break;
  }
  case MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE: {
    // A value contained in the query data field is not an allowable value for
    // the slave
    log += F("Illegal Data Value");
    break;
  }
  case MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE: {
    // An unrecoverable error occurred while the slave was attempting to perform
    // the requested action
    log += F("Slave Device Failure");
    break;
  }
  case MODBUS_EXCEPTION_ACKNOWLEDGE: {
    // The slave has accepted the request and is processing it, but a long
    // duration of time will be
    // required to do so. This response is returned to prevent a timeout error
    // from occurring in the master.
    // The master can next issue a Poll Program Complete message to determine if
    // processing is completed.
    log += F("Acknowledge");
    break; // Is this an error?
  }
  case MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY: {
    // The slave is engaged in processing a long-duration program command.
    // The master should retransmit the message later when the slave is free.
    log += F("Slave Device Busy");
    break;
  }
  case MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE:
    log += F("Negative acknowledge");
    break;
  case MODBUS_EXCEPTION_MEMORY_PARITY:
    log += F("Memory parity error");
    break;
  case MODBUS_EXCEPTION_GATEWAY_PATH:
    log += F("Gateway path unavailable");
    break;
  case MODBUS_EXCEPTION_GATEWAY_TARGET:
    log += F("Target device failed to respond");
    break;
  case Plugin_052_MODBUS_BADCRC:
    log += F("Invalid CRC");
    break;
  case Plugin_052_MODBUS_BADDATA:
    log += F("Invalid data");
    break;
  case Plugin_052_MODBUS_BADEXC:
    log += F("Invalid exception code");
    break;
  case Plugin_052_MODBUS_MDATA:
    log += F("Too many data");
    break;
  case Plugin_052_MODBUS_BADSLAVE:
    log += F("Response not from requested slave");
    break;
  default:
    log += String(F("Unknown Exception code: ")) + value;
    break;
  }
  log += F(" - sent: ");
  log +=
      Plugin_052_log_buffer(_plugin_052_sendframe, _plugin_052_sendframe_used);
  log += F(" - received: ");
  log += Plugin_052_log_buffer(_plugin_052_recv_buf, _plugin_052_recv_buf_used);
  addLog(LOG_LEVEL_DEBUG_MORE, log);
}

byte Plugin_052_processCommand() {
  // CRC-calculation
  unsigned int crc =
      Plugin_052_ModRTU_CRC(_plugin_052_sendframe, _plugin_052_sendframe_used);
  // Note, this number has low and high bytes swapped, so use it accordingly (or
  // swap bytes)
  byte checksumHi = (byte)((crc >> 8) & 0xFF);
  byte checksumLo = (byte)(crc & 0xFF);
  _plugin_052_sendframe[_plugin_052_sendframe_used++] = checksumLo;
  _plugin_052_sendframe[_plugin_052_sendframe_used++] = checksumHi;

  int nrRetriesLeft = 2;
  byte return_value = 0;
  while (nrRetriesLeft > 0) {
    return_value = 0;
    // Send the byte array
    P052_easySerial->write(_plugin_052_sendframe, _plugin_052_sendframe_used);
    delay(50);

    // Read answer from sensor
    _plugin_052_recv_buf_used = 0;
    while (P052_easySerial->available() &&
           _plugin_052_recv_buf_used < P052_MODBUS_RECEIVE_BUFFER) {
      _plugin_052_recv_buf[_plugin_052_recv_buf_used++] =
          P052_easySerial->read();
    }

    // Check for MODBUS exception
    const byte received_functionCode = _plugin_052_recv_buf[1];
    if ((received_functionCode & 0x80) != 0) {
      return_value = _plugin_052_recv_buf[2];
    }
    // Check checksum
    crc = Plugin_052_ModRTU_CRC(_plugin_052_recv_buf, _plugin_052_recv_buf_used);
    if (crc != 0u) {
      ++_plugin_052_reads_crc_failed;
      return_value = Plugin_052_MODBUS_BADCRC;
    } else {
      ++_plugin_052_reads_pass;
    }
    switch (return_value) {
    case MODBUS_EXCEPTION_ACKNOWLEDGE:
    case MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY:
    case Plugin_052_MODBUS_BADCRC:
      // Bad communication, makes sense to retry.
      break;
    default:
      nrRetriesLeft = 0; // When not supported, does not make sense to retry.
      break;
    }
    --nrRetriesLeft;
  }
  return return_value;
}

uint32_t Plugin_052_read_32b_InputRegister(short address) {
  uint32_t result = 0;
  int idHigh = Plugin_052_readInputRegister(address);
  int idLow = Plugin_052_readInputRegister(address + 1);
  if (idHigh >= 0 && idLow >= 0) {
    result = idHigh;
    result = result << 16;
    result += idLow;
  }
  return result;
}

int Plugin_052_readInputRegister(short address) {
  // Only read 1 register
  return Plugin_052_process_16b_register(P052_MODBUS_SLAVE_ADDRESS,
                                         P052_READ_INPUT_REGISTERS, address, 1);
}

int Plugin_052_readHoldingRegister(short address) {
  // Only read 1 register
  return Plugin_052_process_16b_register(
      P052_MODBUS_SLAVE_ADDRESS, P052_READ_HOLDING_REGISTERS, address, 1);
}

// Write to holding register.
int Plugin_052_writeSingleRegister(short address, short value) {
  // GN: Untested, will probably not work
  return Plugin_052_process_16b_register(
      P052_MODBUS_SLAVE_ADDRESS, P052_WRITE_SINGLE_REGISTER, address, value);
}

byte Plugin_052_modbus_get_MEI(byte slaveAddress, byte object_id,
                               String &result, unsigned int &object_value_int,
                               byte &next_object_id, bool &more_follows,
                               byte &conformity_level) {
  // Force device_id to 4 = individual access (reading one ID object per call)
  Plugin_052_build_modbus_MEI_frame(slaveAddress, 4, object_id);
  const byte process_result = Plugin_052_processCommand();
  if (process_result == 0) {
    result = Plugin_052_parse_modbus_MEI_response(object_value_int,
                                                  next_object_id, more_follows,
                                                  conformity_level);
  } else {
    more_follows = false;
  }
  return process_result;
}

void Plugin_052_modbus_log_MEI(byte slaveAddress) {
  // Iterate over all Device identification items, using
  // Modbus command (0x2B / 0x0E) Read Device Identification
  // And add to log.
  bool more_follows = true;
  byte conformity_level = 0;
  byte object_id = 0;
  byte next_object_id = 0;
  while (more_follows) {
    String result;
    unsigned int object_value_int;
    const byte process_result = Plugin_052_modbus_get_MEI(
        slaveAddress, object_id, result, object_value_int, next_object_id,
        more_follows, conformity_level);
    if (process_result == 0) {
      if (result.length() > 0) {
        String log = Plugin_052_MEI_objectid_to_name(object_id);
        log += F(": ");
        log += result;
        addLog(LOG_LEVEL_INFO, log);
      }
    } else {
      switch (process_result) {
      case MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS:
        // No need to log this exception when scanning.
        break;
      default:
        Plugin_052_logModbusException(process_result);
        break;
      }
    }
    // If more parts are needed, collect them or iterate over the known list.
    // For example with "individual access" a new request has to be sent for each single item
    if (more_follows) {
      object_id = next_object_id;
    } else if (object_id < 0x84) {
      // Allow for scanning only the usual object ID's
      // This range is vendor specific
      more_follows = true;
      object_id++;
      if (object_id == 7) {
        // Skip range 0x07...0x7F
        object_id = 0x80;
      }
    }
  }
}

int Plugin_052_process_16b_register(byte slaveAddress, byte functionCode,
                                    short startAddress, short parameter) {
  Plugin_052_buildFrame(slaveAddress, functionCode, startAddress, parameter);
  const byte process_result = Plugin_052_processCommand();
  if (process_result == 0) {
    return (_plugin_052_recv_buf[3] << 8) | (_plugin_052_recv_buf[4]);
  }
  Plugin_052_logModbusException(process_result);
  return -1;
}

int Plugin_052_writeSpecialCommandRegister(byte command) {
  Plugin_052_buildWriteCommandRegister(P052_MODBUS_SLAVE_ADDRESS, command);
  const byte process_result = Plugin_052_processCommand();
  if (process_result == 0)
    return 0;
  Plugin_052_logModbusException(process_result);
  return -1;
}

unsigned int Plugin_052_read_RAM_EEPROM(byte command, byte startAddress,
                                        byte nrBytes) {
  Plugin_052_buildRead_RAM_EEPROM(P052_MODBUS_SLAVE_ADDRESS, command,
                                  startAddress, nrBytes);
  const byte process_result = Plugin_052_processCommand();
  if (process_result == 0) {
    unsigned int result = 0;
    for (int i = 0; i < _plugin_052_recv_buf[2]; ++i) {
      // Most significant byte at lower address
      result = (result << 8) | _plugin_052_recv_buf[i + 3];
    }
    return result;
  }
  Plugin_052_logModbusException(process_result);
  return -1;
}

int Plugin_052_readErrorStatus(void) {
  return Plugin_052_readInputRegister(0x00);
}

uint32_t Plugin_052_readTypeId() {
  return Plugin_052_read_32b_InputRegister(25);
}

uint32_t Plugin_052_readSensorId() {
  return Plugin_052_read_32b_InputRegister(29);
}

int Plugin_052_readCo2(void) { return Plugin_052_readInputRegister(0x03); }

int Plugin_052_readCo2_from_RAM(void) {
  bool valid_measurement = Plugin_052_prepare_single_measurement_from_RAM();
  short co2 =
      Plugin_052_read_RAM_EEPROM(P052_CMD_READ_RAM, P052_RAM_ADDR_CO2, 2);
  short temperature = Plugin_052_read_RAM_EEPROM(
      P052_CMD_READ_RAM, P052_RAM_ADDR_SPACE_TEMPERATURE, 2);
  short humidity = Plugin_052_read_RAM_EEPROM(
      P052_CMD_READ_RAM, P052_RAM_ADDR_RELATIVE_HUMIDITY, 2);
  String log = F("P052: ");
  log += F("CO2: ");
  log += co2;
  log += F(" ppm Temp: ");
  log += (float)temperature / 100.0;
  log += F(" C Hum: ");
  log += (float)humidity / 100.0;
  log += F("%");
  if (!valid_measurement)
    log += F(" (old)");
  addLog(LOG_LEVEL_INFO, log);
  return co2;
}

bool Plugin_052_measurement_active() {
  unsigned int meter_status = Plugin_052_read_RAM_EEPROM(
      P052_CMD_READ_RAM, P052_RAM_ADDR_METER_STATUS, 1);
  // Meter Status bit 5 indicates single cycle measurement active
  return getBitOfInt(meter_status, 5);
}

// Perform a single measurement.
// return value indicates a successful measurement update.
bool Plugin_052_prepare_single_measurement_from_RAM() {
  Plugin_052_check_error_status();
  if (timeOutReached(_plugin_052_last_measurement +
                     P052_MEASUREMENT_INTERVAL)) {
    // Last measurement taken is still valid.
    return true;
  }
  int retry_count = 2;
  addLog(LOG_LEVEL_INFO, F("P052: Start perform measurement"));
  while (!Plugin_052_measurement_active() && retry_count > 0) {
    // Trigger new measurement and make sure it is set active.
    --retry_count;
    addLog(LOG_LEVEL_INFO, F("P052: Write to SCR: perform measurement"));
    Plugin_052_writeSpecialCommandRegister(P052_SCR_SINGLE_MEASUREMENT);
    delay(50);
  }
  if (!Plugin_052_measurement_active()) {
    // Could not start measurement.
    addLog(LOG_LEVEL_INFO, F("P052: Could not start single measurement"));
    return false;
  }
  retry_count = 30;
  while (retry_count > 0) {
    --retry_count;
    if (retry_count < 14) {
      // Just wait for 16 seconds.
      if (!Plugin_052_measurement_active()) {
        _plugin_052_last_measurement = millis();
        String log = F("P052: Measurement complete after ");
        log += (30 - retry_count);
        addLog(LOG_LEVEL_INFO, log);
        return true;
      }
    }
    delay(1000);
  }
  return false;
}

float Plugin_052_readTemperature(void) {
  int temperatureX100 = Plugin_052_readInputRegister(0x04);
  float temperature = (float)temperatureX100 / 100.0;
  return temperature;
}

float Plugin_052_readRelativeHumidity(void) {
  int rhX100 = Plugin_052_readInputRegister(0x05);
  float rh = 0.0;
  rh = (float)rhX100 / 100;
  return rh;
}

int Plugin_052_readRelayStatus(void) {
  int status = Plugin_052_readInputRegister(0x1C);
  bool result = status >> 8 & 0x1;
  return result;
}

int Plugin_052_readTemperatureAdjustment(void) {
  return Plugin_052_readInputRegister(0x0A);
}

void Plugin_052_setRelayStatus(int status) {
  short relaystatus = 0; // 0x3FFF represents 100% output.
  //  Refer to sensor model’s specification for voltage at 100% output.
  switch (status) {
  case 0:
    relaystatus = 0;
    break;
  case 1:
    relaystatus = 0x3FFF;
    break;
  default:
    relaystatus = 0x7FFF;
    break;
  }
  Plugin_052_writeSingleRegister(0x18, relaystatus);
}

int Plugin_052_readABCperiod(void) {
  return Plugin_052_readHoldingRegister(0x1F);
}

int Plugin_052_readModbusAddress(void) {
  return Plugin_052_readHoldingRegister(63); // HR64 MAC address Modbus address, valid range 1 - 253
}

// Compute the MODBUS RTU CRC
unsigned int Plugin_052_ModRTU_CRC(byte *buf, int len) {
  unsigned int crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++) {
    crc ^= (unsigned int)buf[pos]; // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) { // Loop over each bit
      if ((crc & 0x0001) != 0) {   // If the LSB is set
        crc >>= 1;                 // Shift right and XOR 0xA001
        crc ^= 0xA001;
      } else       // Else LSB is not set
        crc >>= 1; // Just shift right
    }
  }
  return crc;
}

bool getBitOfInt(int reg, int pos) {
  // Create a mask
  int mask = 0x01 << pos;

  // Mask the status register
  int masked_register = mask & reg;

  // Shift the result of masked register back to position 0
  int result = masked_register >> pos;
  return (result == 1);
}
#endif // USES_P052
