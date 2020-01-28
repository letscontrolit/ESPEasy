#ifdef USES_P090

//#######################################################################################################
//################################ Plugin 090: Mitsubishi Heat Pump #####################################
//#######################################################################################################

//uncomment one of the following as needed
//#ifdef PLUGIN_BUILD_DEVELOPMENT
//#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_090
#define PLUGIN_ID_090         90
#define PLUGIN_NAME_090       "Mitsubishi Heat Pump"
#define PLUGIN_VALUENAME1_090 "MitsubishiHeatPump"

//#define PLUGIN_xxx_DEBUG  false             //set to true for extra log info in the debug

struct heatpumpSettings {
  const char* power;
  const char* mode;
  float temperature;
  const char* fan;
  const char* vane; //vertical vane, up/down
  const char* wideVane; //horizontal vane, left/right
  bool iSee;   //iSee sensor, at the moment can only detect it, not set it
  //bool connected;
};

struct heatpumpStatus {
  float roomTemperature;
  bool operating; // if true, the heatpump is operating to reach the desired temperature
  //heatpumpTimers timers;
  int compressorFrequency;
};

struct P090_data_struct : public PluginTaskData_base {
  P090_data_struct(const int16_t serialRx, const int16_t serialTx);

  bool read(String& result, bool force);

private:
  void connect(bool retry);

  void createInfoPacket(byte *packet, byte packetType);
  void createPacket(byte *packet, const heatpumpSettings& settings);

  int readPacket();
  void writePacket(const byte *packet, int length);

  bool canRead() const;
  bool canSend(bool isInfo) const;

  void sync(byte packetType);
  bool update();

private:
  ESPeasySerial _serial;
  heatpumpSettings _currentSettings;
  heatpumpSettings _wantedSettings;
  heatpumpStatus _currentStatus;
  bool _isConnected;
  int _bitrate;
  bool _waitForRead;
  unsigned long _lastSend;
  unsigned long _lastRecv;
  bool _tempMode;
  bool _wideVaneAdj;
  bool _firstRun;
  bool _autoUpdate;
  bool _externalUpdate;
  int _infoMode;
};

boolean Plugin_090(byte function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {

    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number = PLUGIN_ID_090;
      Device[deviceCount].Type = DEVICE_TYPE_DUAL;
      Device[deviceCount].VType = SENSOR_TYPE_STRING;
      Device[deviceCount].ValueCount = 1;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption = true;
      Device[deviceCount].TimerOptional = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_090);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_090));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
      serialHelper_getGpioNames(event);
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG: {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      serialHelper_webformLoad(event);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      serialHelper_webformSave(event);
      success = true;
      break;
    }

    case PLUGIN_INIT: {
      initPluginTaskData(event->TaskIndex, new P090_data_struct(CONFIG_PIN1, CONFIG_PIN2));
      success = getPluginTaskData(event->TaskIndex) != nullptr;
      break;
    }

    case PLUGIN_READ: {
      P090_data_struct* heatPump = static_cast<P090_data_struct*>(getPluginTaskData(event->TaskIndex));
      if (heatPump != nullptr) {
        success = heatPump->read(event->String2, true);
      }
      break;
    }

    case PLUGIN_WRITE: {
      //this case defines code to be executed when the plugin executes an action (command).
      //Commands can be accessed via rules or via http.
      //As an example, http://192.168.1.12//control?cmd=dothis
      //implies that there exists the comamnd "dothis"

      /*if (plugin_not_initialised)
        break;

      // FIXME TD-er: This one is not using parseString* function
      //parse string to extract the command
      String tmpString  = string;
      int argIndex = tmpString.indexOf(',');
      if (argIndex)
        tmpString = tmpString.substring(0, argIndex);

      String tmpStr = string;
      int comma1 = tmpStr.indexOf(',');
      if (tmpString.equalsIgnoreCase(F("dothis"))) {
        //do something
        success = true;     //set to true only if plugin has executed a command successfully
      }*/

       break;
    }

    case PLUGIN_EXIT: {
      clearPluginTaskData(event->TaskIndex);
      success = true;
      break;
    }

    case PLUGIN_ONCE_A_SECOND: {
      P090_data_struct* heatPump = static_cast<P090_data_struct*>(getPluginTaskData(event->TaskIndex));
      if (heatPump != nullptr) {
        success = heatPump->read(event->String2, false);
        if (success) {
          sendData(event);
        }
      }
      break;
    }
  }

  return success;
}

/*static void hpPacketDebug(byte* packet, unsigned int length, char* packetDirection) {
  String message = "MHP - ";
  for (unsigned int idx = 0; idx < length; idx++) {
    if (packet[idx] < 16) {
      message += "0"; // pad single hex digits with a 0
    }
    message += String(packet[idx], HEX) + " ";
  }
  addLog(LOG_LEVEL_INFO, message);
}*/

// HeatPump.h

// indexes for INFOMODE array (public so they can be optionally passed to sync())
static const int RQST_PKT_SETTINGS  = 0;
//static const int RQST_PKT_ROOM_TEMP = 1;
//static const int RQST_PKT_TIMERS    = 3;
//static const int RQST_PKT_STATUS    = 4;
//static const int RQST_PKT_STANDBY   = 5;

static const int PACKET_LEN = 22;
static const int PACKET_SENT_INTERVAL_MS = 1000;
static const int PACKET_INFO_INTERVAL_MS = 2000;
static const int PACKET_TYPE_DEFAULT = 99;

static const int CONNECT_LEN = 8;
static const byte CONNECT[CONNECT_LEN] = {0xfc, 0x5a, 0x01, 0x30, 0x02, 0xca, 0x01, 0xa8};
static const int HEADER_LEN  = 8;
static const byte HEADER[HEADER_LEN]  = {0xfc, 0x41, 0x01, 0x30, 0x10, 0x01, 0x00, 0x00};

static const int INFOHEADER_LEN  = 5;
static const byte INFOHEADER[INFOHEADER_LEN]  = {0xfc, 0x42, 0x01, 0x30, 0x10};

static const int INFOMODE_LEN = 6;
static const byte INFOMODE[INFOMODE_LEN] = {
  0x02, // request a settings packet - RQST_PKT_SETTINGS
  0x03, // request the current room temp - RQST_PKT_ROOM_TEMP
  0x04, // unknown
  0x05, // request the timers - RQST_PKT_TIMERS
  0x06, // request status - RQST_PKT_STATUS
  0x09  // request standby mode (maybe?) RQST_PKT_STANDBY
};

static const int RCVD_PKT_FAIL            = 0;
static const int RCVD_PKT_CONNECT_SUCCESS = 1;
static const int RCVD_PKT_SETTINGS        = 2;
static const int RCVD_PKT_ROOM_TEMP       = 3;
static const int RCVD_PKT_UPDATE_SUCCESS  = 4;
static const int RCVD_PKT_STATUS          = 5;
//static const int RCVD_PKT_TIMER           = 6;

static const byte CONTROL_PACKET_1[5] = {0x01,    0x02,  0x04,  0x08, 0x10};
                               //{"POWER","MODE","TEMP","FAN","VANE"};
static const byte CONTROL_PACKET_2[1] = {0x01};
                               //{"WIDEVANE"};
static const byte POWER[2]            = {0x00, 0x01};
static const char* POWER_MAP[2]       = {"OFF", "ON"};
static const byte MODE[5]             = {0x01,   0x02,  0x03, 0x07, 0x08};
static const char* MODE_MAP[5]        = {"HEAT", "DRY", "COOL", "FAN", "AUTO"};
static const byte TEMP[16]            = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
static const int TEMP_MAP[16]         = {31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16};
static const byte FAN[6]              = {0x00,  0x01,   0x02, 0x03, 0x05, 0x06};
static const char* FAN_MAP[6]         = {"AUTO", "QUIET", "1", "2", "3", "4"};
static const byte VANE[7]             = {0x00,  0x01, 0x02, 0x03, 0x04, 0x05, 0x07};
static const char* VANE_MAP[7]        = {"AUTO", "1", "2", "3", "4", "5", "SWING"};
static const byte WIDEVANE[7]         = {0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x0c};
static const char* WIDEVANE_MAP[7]    = {"<<", "<",  "|",  ">",  ">>", "<>", "SWING"};
static const byte ROOM_TEMP[32]       = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                                  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
static const int ROOM_TEMP_MAP[32]    = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                                  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41};
//static const byte TIMER_MODE[4]       = {0x00,  0x01,  0x02, 0x03};
//static const char* TIMER_MODE_MAP[4]  = {"NONE", "OFF", "ON", "BOTH"};


// HeatPump.cpp
static const char* lookupByteMapValue(const char* valuesMap[], const byte byteMap[], int len, byte byteValue) {
  for (int i = 0; i < len; i++) {
    if (byteMap[i] == byteValue) {
      return valuesMap[i];
    }
  }
  return valuesMap[0];
}

static int lookupByteMapValue(const int valuesMap[], const byte byteMap[], int len, byte byteValue) {
  for (int i = 0; i < len; i++) {
    if (byteMap[i] == byteValue) {
      return valuesMap[i];
    }
  }
  return valuesMap[0];
}

static int lookupByteMapIndex(const int valuesMap[], int len, int lookupValue) {
  for (int i = 0; i < len; i++) {
    if (valuesMap[i] == lookupValue) {
      return i;
    }
  }
  return -1;
}

static int lookupByteMapIndex(const char* valuesMap[], int len, const char* lookupValue) {
  for (int i = 0; i < len; i++) {
    if (strcmp(valuesMap[i], lookupValue) == 0) {
      return i;
    }
  }
  return -1;
}

static byte checkSum(byte bytes[], int len) {
  byte sum = 0;
  for (int i = 0; i < len; i++) {
    sum += bytes[i];
  }
  return (0xfc - sum) & 0xff;
}

static bool operator!=(const heatpumpSettings& lhs, const heatpumpSettings& rhs) {
  return lhs.power != rhs.power ||
         lhs.mode != rhs.mode ||
         lhs.temperature != rhs.temperature ||
         lhs.fan != rhs.fan ||
         lhs.vane != rhs.vane ||
         lhs.wideVane != rhs.wideVane ||
         lhs.iSee != rhs.iSee;
}

P090_data_struct::P090_data_struct(const int16_t serialRx, const int16_t serialTx) :
  _serial(serialRx, serialTx),
  _currentSettings({}),
  _wantedSettings({}),
  _currentStatus({}),
  _isConnected(false),
  _bitrate(2400),
  _waitForRead(false),
  _lastSend(0),
  _lastRecv(millis() - (PACKET_SENT_INTERVAL_MS * 10)),
  _tempMode(false),
  _wideVaneAdj(false),
  _firstRun(true),
  _autoUpdate(false),
  _externalUpdate(false),
  _infoMode(0) {

}

void P090_data_struct::connect(bool retry) {
  _isConnected = false;
  _serial.begin(_bitrate, SERIAL_8E1);

//  if(onConnectCallback) {
    //onConnectCallback();
  //}

  writePacket(CONNECT, CONNECT_LEN);
  while(!canRead()) { delay(10); }
  int packetType = readPacket();
  if(packetType != RCVD_PKT_CONNECT_SUCCESS && retry){
	  _bitrate = (_bitrate == 2400 ? 9600 : 2400);
	  connect(false);
  }
}

bool P090_data_struct::read(String& result, bool force) {
  const heatpumpSettings lastSettings = _currentSettings;
  const heatpumpStatus lastStatus = _currentStatus;

  sync(PACKET_TYPE_DEFAULT);

  if (!_isConnected) {
    return false;
  }

  if (force || _currentSettings != lastSettings || _currentStatus.roomTemperature != lastStatus.roomTemperature) {
    //result.reserve(); // TODO:
    result = F("{\"roomTemperature\":");
    result += toString(_currentStatus.roomTemperature, 1);
    result += F(",\"wideVane\":\"");
    result += _currentSettings.wideVane;
    result += F("\",\"power\":\"");
    result += _currentSettings.power;
    result += F("\",\"mode\":\"");
    result += _currentSettings.mode;
    result += F("\",\"fan\":\"");
    result += _currentSettings.fan;
    result += F("\",\"vane\":\"");
    result += _currentSettings.vane;
    result += F("\",\"iSee\":");
    result += boolToString(_currentSettings.iSee);
    result += F(",\"temperature\":");
    result += toString(_currentSettings.temperature, 1) + '}';

    return true;
  }

  return false;
}

void P090_data_struct::sync(byte packetType) {
  if((!_isConnected) || (millis() - _lastRecv > (PACKET_SENT_INTERVAL_MS * 10))) {
    connect(true);
  }
  else if(canRead()) {
    readPacket();
  }
  else if(_autoUpdate && !_firstRun && _wantedSettings != _currentSettings && packetType == PACKET_TYPE_DEFAULT) {
    update();
  }
  else if(canSend(true)) {
    byte packet[PACKET_LEN] = {};
    createInfoPacket(packet, packetType);
    writePacket(packet, PACKET_LEN);
  }
}

bool P090_data_struct::canRead() const {
  return (_waitForRead && (millis() - PACKET_SENT_INTERVAL_MS) > _lastSend);
}

bool P090_data_struct::canSend(bool isInfo) const {
  return (millis() - (isInfo ? PACKET_INFO_INTERVAL_MS : PACKET_SENT_INTERVAL_MS)) > _lastSend;
}

void P090_data_struct::writePacket(const byte *packet, int length) {
  for (int i = 0; i < length; i++) {
     _serial.write(packet[i]);
  }

  //if(packetCallback) {
  //  packetCallback(packet, length, (char*)"packetSent");
  //}

  _waitForRead = true;
  _lastSend = millis();
}

int P090_data_struct::readPacket() {
  byte header[INFOHEADER_LEN] = {};
  byte data[PACKET_LEN] = {};
  bool foundStart = false;
  int dataSum = 0;
  byte checksum = 0;
  byte dataLength = 0;

  _waitForRead = false;

  if(_serial.available() > 0) {
    // read until we get start byte 0xfc
    while(_serial.available() > 0 && !foundStart) {
      header[0] = _serial.read();
      if(header[0] == HEADER[0]) {
        foundStart = true;
        delay(100); // found that this delay increases accuracy when reading, might not be needed though
      }
    }

    if(!foundStart) {
      return RCVD_PKT_FAIL;
    }

    //read header
    for(int i=1;i<5;i++) {
      header[i] = _serial.read();
    }

    //check header
    if(header[0] == HEADER[0] && header[2] == HEADER[2] && header[3] == HEADER[3]) {
      dataLength = header[4];

      for(int i=0;i<dataLength;i++) {
        data[i] = _serial.read();
      }

      // read checksum byte
      data[dataLength] = _serial.read();

      // sum up the header bytes...
      for (int i = 0; i < INFOHEADER_LEN; i++) {
        dataSum += header[i];
      }

      // ...and add to that the sum of the data bytes
      for (int i = 0; i < dataLength; i++) {
        dataSum += data[i];
      }

      // calculate checksum
      checksum = (0xfc - dataSum) & 0xff;

      if(data[dataLength] == checksum) {
        _lastRecv = millis();

        /*if(packetCallback) {
          byte packet[37]; // we are going to put header[5] and data[32] into this, so the whole packet is sent to the callback
          for(int i=0; i<INFOHEADER_LEN; i++) {
            packet[i] = header[i];
          }
          for(int i=0; i<(dataLength+1); i++) { //must be dataLength+1 to pick up checksum byte
            packet[(i+5)] = data[i];
          }
          packetCallback(packet, PACKET_LEN, (char*)"packetRecv");
        }*/

        if(header[1] == 0x62) {
          switch(data[0]) {
            case 0x02: { // setting information
              _currentSettings.power = lookupByteMapValue(POWER_MAP, POWER, 2, data[3]);
              _currentSettings.iSee = data[4] > 0x08 ? true : false;
              _currentSettings.mode = lookupByteMapValue(MODE_MAP, MODE, 5, _currentSettings.iSee  ? (data[4] - 0x08) : data[4]);

              if(data[11] != 0x00) {
                int temp = data[11];
                temp -= 128;
                _currentSettings.temperature = (float)temp / 2;
                _tempMode =  true;
              } else {
                _currentSettings.temperature = lookupByteMapValue(TEMP_MAP, TEMP, 16, data[5]);
              }

              _currentSettings.fan         = lookupByteMapValue(FAN_MAP, FAN, 6, data[6]);
              _currentSettings.vane        = lookupByteMapValue(VANE_MAP, VANE, 7, data[7]);
              _currentSettings.wideVane    = lookupByteMapValue(WIDEVANE_MAP, WIDEVANE, 7, data[10] & 0x0F);
              _wideVaneAdj = (data[10] & 0xF0) == 0x80 ? true : false;

              // if this is the first time we have synced with the heatpump, set wantedSettings to receivedSettings
              if(_firstRun || (_autoUpdate && _externalUpdate)) {
                _wantedSettings = _currentSettings;
                _firstRun = false;
              }

              return RCVD_PKT_SETTINGS;
            }

            case 0x03: { //Room temperature reading
              if(data[6] != 0x00) {
                int temp = data[6];
                temp -= 128;
                _currentStatus.roomTemperature = (float)temp / 2;
              } else {
                _currentStatus.roomTemperature = lookupByteMapValue(ROOM_TEMP_MAP, ROOM_TEMP, 32, data[3]);
              }

              return RCVD_PKT_ROOM_TEMP;
            }

            case 0x04: { // unknown
                break;
            }

            case 0x05: { // timer packet
              /*heatpumpTimers receivedTimers;

              receivedTimers.mode                = lookupByteMapValue(TIMER_MODE_MAP, TIMER_MODE, 4, data[3]);
              receivedTimers.onMinutesSet        = data[4] * TIMER_INCREMENT_MINUTES;
              receivedTimers.onMinutesRemaining  = data[6] * TIMER_INCREMENT_MINUTES;
              receivedTimers.offMinutesSet       = data[5] * TIMER_INCREMENT_MINUTES;
              receivedTimers.offMinutesRemaining = data[7] * TIMER_INCREMENT_MINUTES;

              // callback for status change
              if(statusChangedCallback && currentStatus.timers != receivedTimers) {
                currentStatus.timers = receivedTimers;
                statusChangedCallback(currentStatus);
              } else {
                currentStatus.timers = receivedTimers;
              }

              return RCVD_PKT_TIMER;*/
            }

            case 0x06: { // status
              _currentStatus.operating = data[4];
              _currentStatus.compressorFrequency = data[3];

              return RCVD_PKT_STATUS;
            }

            case 0x09: { // standby mode maybe?
              break;
            }
          }
        }

        if(header[1] == 0x61) { //Last update was successful
          return RCVD_PKT_UPDATE_SUCCESS;
        } else if(header[1] == 0x7a) { //Last update was successful
          _isConnected = true;
          return RCVD_PKT_CONNECT_SUCCESS;
        }
      }
    }
  }

  return RCVD_PKT_FAIL;
}

void P090_data_struct::createInfoPacket(byte *packet, byte packetType) {
  // add the header to the packet
  for (int i = 0; i < INFOHEADER_LEN; i++) {
    packet[i] = INFOHEADER[i];
  }

  // set the mode - settings or room temperature
  if(packetType != PACKET_TYPE_DEFAULT) {
    packet[5] = INFOMODE[packetType];
  } else {
    // request current infoMode, and increment for the next request
    packet[5] = INFOMODE[_infoMode];
    if(_infoMode == (INFOMODE_LEN - 1)) {
      _infoMode = 0;
    } else {
      _infoMode++;
    }
  }

  // pad the packet out
  for (int i = 0; i < 15; i++) {
    packet[i + 6] = 0x00;
  }

  // add the checksum
  byte chkSum = checkSum(packet, 21);
  packet[21] = chkSum;
}

void P090_data_struct::createPacket(byte *packet, const heatpumpSettings& settings) {
  //preset all bytes to 0x00
  for (int i = 0; i < 21; i++) {
    packet[i] = 0x00;
  }
  for (int i = 0; i < HEADER_LEN; i++) {
    packet[i] = HEADER[i];
  }
  if(settings.power != _currentSettings.power) {
    packet[8]  = POWER[lookupByteMapIndex(POWER_MAP, 2, settings.power)];
    packet[6] += CONTROL_PACKET_1[0];
  }
  if(settings.mode!= _currentSettings.mode) {
    packet[9]  = MODE[lookupByteMapIndex(MODE_MAP, 5, settings.mode)];
    packet[6] += CONTROL_PACKET_1[1];
  }
  if(!_tempMode && settings.temperature!= _currentSettings.temperature) {
    packet[10] = TEMP[lookupByteMapIndex(TEMP_MAP, 16, settings.temperature)];
    packet[6] += CONTROL_PACKET_1[2];
  }
  else if(_tempMode && settings.temperature!= _currentSettings.temperature) {
    float temp = (settings.temperature * 2) + 128;
    packet[19] = (int)temp;
    packet[6] += CONTROL_PACKET_1[2];
  }
  if(settings.fan!= _currentSettings.fan) {
    packet[11] = FAN[lookupByteMapIndex(FAN_MAP, 6, settings.fan)];
    packet[6] += CONTROL_PACKET_1[3];
  }
  if(settings.vane!= _currentSettings.vane) {
    packet[12] = VANE[lookupByteMapIndex(VANE_MAP, 7, settings.vane)] | (_wideVaneAdj ? 0x80 : 0x00);
    packet[6] += CONTROL_PACKET_1[4];
  }
  if(settings.wideVane!= _currentSettings.wideVane) {
    packet[18] = WIDEVANE[lookupByteMapIndex(WIDEVANE_MAP, 7, settings.wideVane)];
    packet[7] += CONTROL_PACKET_2[0];
  }
  // add the checksum
  byte chkSum = checkSum(packet, 21);
  packet[21] = chkSum;
}

bool P090_data_struct::update() {
  while(!canSend(false)) { delay(10); }

  byte packet[PACKET_LEN] = {};
  createPacket(packet, _wantedSettings);
  writePacket(packet, PACKET_LEN);

  while(!canRead()) { delay(10); }
  int packetType = readPacket();

  if(packetType == RCVD_PKT_UPDATE_SUCCESS) {
    // call sync() to get the latest settings from the heatpump for autoUpdate, which should now have the updated settings
    if(_autoUpdate) { //this sync will happen regardless, but autoUpdate needs it sooner than later.
	    while(!canSend(true)) {
		    delay(10);
	    }
	    sync(RQST_PKT_SETTINGS);
    }

    return true;
  } else {
    return false;
  }
}

#endif  // USES_P090
