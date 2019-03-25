#ifdef USES_P031
//#######################################################################################################
//#################### Plugin 031: SHT10/SHT11/SHT15 Temp/Humidity Sensor ###############################
//#######################################################################################################

#define PLUGIN_031
#define PLUGIN_ID_031         31
#define PLUGIN_NAME_031       "Environment - SHT1X"
#define PLUGIN_VALUENAME1_031 "Temperature"
#define PLUGIN_VALUENAME2_031 "Humidity"

class P031_data_struct: public PluginTaskData_base
{
public:
  enum {
    SHT1X_CMD_MEASURE_TEMP  = B00000011,
    SHT1X_CMD_MEASURE_RH    = B00000101,
    SHT1X_CMD_READ_STATUS   = B00000111,
    SHT1X_CMD_SOFT_RESET    = B00011110
  };

	P031_data_struct() {}

  byte init(byte data_pin, byte clock_pin, bool pullUp) {
    _dataPin = data_pin;
    _clockPin = clock_pin;
    input_mode = pullUp ? INPUT_PULLUP : INPUT;

    pinMode(_dataPin, input_mode); /* Keep Hi-Z except when sending data */
    pinMode(_clockPin, OUTPUT);
    resetSensor();
    return readStatus();
  }


  float readTemperature()
  {
    float tempRaw, tempC;

    sendCommand(SHT1X_CMD_MEASURE_TEMP);
    awaitResult();
    tempRaw = readData(16);

    // Temperature conversion coefficients from SHT1X datasheet for version 4
    const float d1 = -39.7;  // 3.5V
    const float d2 = 0.01;   // 14-bit

    tempC = d1 + (tempRaw * d2);
/*
    String log = F("SHT1X : Read temperature (raw): ");
    log += String(tempRaw);
    log += F(" (Celcius): ");
    log += String(tempC);
    addLog(LOG_LEVEL_DEBUG, log);
*/
    return tempC;
  }

  float readRelHumidity(float tempC)
  {
    float raw, rhLinear, rhTrue;

    sendCommand(SHT1X_CMD_MEASURE_RH);
    awaitResult();
    raw = readData(16);

    // Temperature conversion coefficients from SHT1X datasheet for version 4
    const float c1 = -2.0468;
    const float c2 = 0.0367;
    const float c3 = -1.5955E-6;
    const float t1 = 0.01;
    const float t2 = 0.00008;

    rhLinear = c1 + c2 * raw + c3 * raw * raw;
    rhTrue = (tempC - 25) * (t1 + t2 * raw) + rhLinear;
/*
    String log = F("SHT1X : Read humidity (raw): ");
    log += String(raw);
    log += F(" (Linear): ");
    log += String(rhLinear);
    log += F(" (True): ");
    log += String(rhTrue);
    addLog(LOG_LEVEL_DEBUG, log);
*/
    return rhTrue;
  }


  void resetSensor()
  {
    delay(11);
    for (int i=0; i<9; i++) {
      digitalWrite(_clockPin, HIGH);
      digitalWrite(_clockPin, LOW);
    }
    sendCommand(SHT1X_CMD_SOFT_RESET);
    delay(11);
  }

  byte readStatus()
  {
    sendCommand(SHT1X_CMD_READ_STATUS);
    return readData(8);
  }

  void sendCommand(const byte cmd)
  {
    pinMode(_dataPin, OUTPUT);

    // Transmission Start sequence
    digitalWrite(_dataPin, HIGH);
    digitalWrite(_clockPin, HIGH);
    digitalWrite(_dataPin, LOW);
    digitalWrite(_clockPin, LOW);
    digitalWrite(_clockPin, HIGH);
    digitalWrite(_dataPin, HIGH);
    digitalWrite(_clockPin, LOW);

    // Send the command (address must be 000b)
    shiftOut(_dataPin, _clockPin, MSBFIRST, cmd);

    // Wait for ACK
    bool ackerror = false;
    digitalWrite(_clockPin, HIGH);
    pinMode(_dataPin, input_mode);
    if (digitalRead(_dataPin) != LOW) ackerror = true;
    digitalWrite(_clockPin, LOW);

    if (cmd == SHT1X_CMD_MEASURE_TEMP || cmd == SHT1X_CMD_MEASURE_RH) {
      delayMicroseconds(1); /* Give the sensor time to release the data line */
      if (digitalRead(_dataPin) != HIGH) ackerror = true;
    }

    if (ackerror) {
//      addLog(LOG_LEVEL_ERROR, F("SHT1X : Sensor did not ACK command"));
    }
  }

  void awaitResult()
  {
    // Maximum 320ms for 14 bit measurement
    for (int i=0; i<16; i++) {
      if (digitalRead(_dataPin) == LOW) return;
      delay(20);
    }
    if (digitalRead(_dataPin) != LOW) {
//      addLog(LOG_LEVEL_ERROR, F("SHT1X : Data not ready"));
    }
  }

  int readData(const int bits)
  {
    int val = 0;

    if (bits == 16) {
      // Read most significant byte
      val = shiftIn(_dataPin, _clockPin, 8);
      val <<= 8;

      // Send ACK
      pinMode(_dataPin, OUTPUT);
      digitalWrite(_dataPin, LOW);
      digitalWrite(_clockPin, HIGH);
      digitalWrite(_clockPin, LOW);
      pinMode(_dataPin, input_mode);
    }

    // Read least significant byte
    val |= shiftIn(_dataPin, _clockPin, 8);

    // Keep DATA pin high to skip CRC
    digitalWrite(_clockPin, HIGH);
    digitalWrite(_clockPin, LOW);

    return val;
  }

  byte _dataPin = 0;
  byte _clockPin = 0;
  int input_mode;
};



boolean Plugin_031(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_031;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = SENSOR_TYPE_TEMP_HUM;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = true;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_031);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_031));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_031));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_bidirectional(F("Data"));
        event->String2 = formatGpioName_output(F("SCK"));
        break;
      }

    case PLUGIN_INIT:
      {
        initPluginTaskData(event->TaskIndex, new P031_data_struct());
        P031_data_struct *P031_data =
            static_cast<P031_data_struct *>(getPluginTaskData(event->TaskIndex));
        if (nullptr == P031_data) {
          return success;
        }
        byte status = P031_data->init(CONFIG_PIN1, CONFIG_PIN2, Settings.TaskDevicePin1PullUp[event->TaskIndex]);
        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("SHT1X : Status byte: ");
          log += String(status, HEX);
          log += F(" - resolution: ");
          log += (status & 1 ? F("low") : F("high"));
          log += F(" reload from OTP: ");
          log += ((status >> 1) & 1 ? F("yes") : F("no"));
          log += F(", heater: ");
          log += ((status >> 2) & 1 ? F("on") : F("off"));
          addLog(LOG_LEVEL_DEBUG, log);
        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        P031_data_struct *P031_data =
            static_cast<P031_data_struct *>(getPluginTaskData(event->TaskIndex));
        if (nullptr != P031_data) {
          UserVar[event->BaseVarIndex] = P031_data->readTemperature();
          UserVar[event->BaseVarIndex+1] = P031_data->readRelHumidity(UserVar[event->BaseVarIndex]);
          success = true;
        }
        break;
      }
  }
  return success;
}
#endif // USES_P031
