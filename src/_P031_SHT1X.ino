#include "_Plugin_Helper.h"
#ifdef USES_P031
//#######################################################################################################
//#################### Plugin 031: SHT10/SHT11/SHT15 Temp/Humidity Sensor ###############################
//#######################################################################################################



#define PLUGIN_031
#define PLUGIN_ID_031         31
#define PLUGIN_NAME_031       "Environment - SHT1X"
#define PLUGIN_VALUENAME1_031 "Temperature"
#define PLUGIN_VALUENAME2_031 "Humidity"

#define P031_IDLE            0
#define P031_WAIT_TEMP       1
#define P031_WAIT_HUM        2
#define P031_MEAS_READY      3
#define P031_COMMAND_NO_ACK  4
#define P031_NO_DATA         5

// see https://github.com/letscontrolit/ESPEasy/issues/2444
#define P031_DELAY_LONGER_CABLES  delayMicroseconds(_clockdelay);
#define P031_MAX_CLOCK_DELAY  30   // delay of 10 usec is enough for a 30m CAT6 UTP cable.

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

  byte init(byte data_pin, byte clock_pin, bool pullUp, byte clockdelay) {
    _dataPin = data_pin;
    _clockPin = clock_pin;
    _clockdelay = clockdelay;
    if (_clockdelay > P031_MAX_CLOCK_DELAY) {
      _clockdelay = P031_MAX_CLOCK_DELAY;
    }
    input_mode = pullUp ? INPUT_PULLUP : INPUT;
    state = P031_IDLE;

    pinMode(_dataPin, input_mode); /* Keep Hi-Z except when sending data */
    pinMode(_clockPin, OUTPUT);
    resetSensor();
    return readStatus();
  }

  bool process() {
    switch (state) {
      case P031_IDLE: return false; // Nothing changed, nothing to do
      case P031_WAIT_TEMP: {
        if (digitalRead(_dataPin) == LOW) {
          float tempRaw = readData(16);
          // Temperature conversion coefficients from SHT1X datasheet for version 4
          const float d1 = -39.7f;  // 3.5V
          const float d2 = 0.01f;   // 14-bit
          tempC = d1 + (tempRaw * d2);
          state = P031_WAIT_HUM; // Wait for humidity
          sendCommand(SHT1X_CMD_MEASURE_RH);
        }
        break;
      }
      case P031_WAIT_HUM:
      {
        if (digitalRead(_dataPin) == LOW) {
          float raw = readData(16);

          // Temperature conversion coefficients from SHT1X datasheet for version 4
          const float c1 = -2.0468f;
          const float c2 = 0.0367f;
          const float c3 = -1.5955E-6f;
          const float t1 = 0.01f;
          const float t2 = 0.00008f;

          float rhLinear = c1 + c2 * raw + c3 * raw * raw;
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
          state = P031_MEAS_READY; // Measurement ready
          return true;
        }
        break;
      }
      case P031_MEAS_READY: return true;
      default:
        // It is already an error state, just return.
        return false;
    }
    // Compute timeout
    if (timePassedSince(sendCommandTime) > 320) {
      state = P031_NO_DATA; // No data after 320 msec
    }
    return false;
  }

  void startMeasurement() {
    state = P031_WAIT_TEMP; // Wait for temperature
    sendCommand(SHT1X_CMD_MEASURE_TEMP);
  }

  bool measurementReady() {
    return state == P031_MEAS_READY;
  }

  bool hasError() {
    return state > P031_MEAS_READY;
  }

  void resetSensor()
  {
    state = P031_IDLE;
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
    sendCommandTime = millis();
    pinMode(_dataPin, OUTPUT);

    // Transmission Start sequence
    digitalWrite(_dataPin, HIGH);
    digitalWrite(_clockPin, HIGH);
    P031_DELAY_LONGER_CABLES
    digitalWrite(_dataPin, LOW);
    digitalWrite(_clockPin, LOW);
    P031_DELAY_LONGER_CABLES
    digitalWrite(_clockPin, HIGH);
    P031_DELAY_LONGER_CABLES
    digitalWrite(_dataPin, HIGH);
    digitalWrite(_clockPin, LOW);
    P031_DELAY_LONGER_CABLES

    // Send the command (address must be 000b)
    p031_shiftOut(_dataPin, _clockPin, MSBFIRST, cmd);

    // Wait for ACK
    bool ackerror = false;
    digitalWrite(_clockPin, HIGH);
    P031_DELAY_LONGER_CABLES
    pinMode(_dataPin, input_mode);
    if (digitalRead(_dataPin) != LOW) ackerror = true;
    digitalWrite(_clockPin, LOW);
    P031_DELAY_LONGER_CABLES

    if (cmd == SHT1X_CMD_MEASURE_TEMP || cmd == SHT1X_CMD_MEASURE_RH) {
      delayMicroseconds(1); /* Give the sensor time to release the data line */
      if (digitalRead(_dataPin) != HIGH) ackerror = true;
    }
    if (ackerror) {
      state = P031_COMMAND_NO_ACK;
    }
  }

  int readData(const int bits)
  {
    int val = 0;

    if (bits == 16) {
      // Read most significant byte
      val = p031_shiftIn(_dataPin, _clockPin, MSBFIRST);
      val <<= 8;

      // Send ACK
      pinMode(_dataPin, OUTPUT);
      digitalWrite(_dataPin, LOW);
      digitalWrite(_clockPin, HIGH);
      P031_DELAY_LONGER_CABLES
      digitalWrite(_clockPin, LOW);
      P031_DELAY_LONGER_CABLES
      pinMode(_dataPin, input_mode);
    }

    // Read least significant byte
    val |= p031_shiftIn(_dataPin, _clockPin, MSBFIRST);

    // Keep DATA pin high to skip CRC
    digitalWrite(_clockPin, HIGH);
    P031_DELAY_LONGER_CABLES
    digitalWrite(_clockPin, LOW);
    P031_DELAY_LONGER_CABLES

    return val;
  }

  uint8_t p031_shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
  	uint8_t value = 0;
  	uint8_t i;

  	for (i = 0; i < 8; ++i) {
  		digitalWrite(clockPin, HIGH);
      P031_DELAY_LONGER_CABLES
  		if (bitOrder == LSBFIRST)
  			value |= digitalRead(dataPin) << i;
  		else
  			value |= digitalRead(dataPin) << (7 - i);
  		digitalWrite(clockPin, LOW);
      P031_DELAY_LONGER_CABLES
  	}
  	return value;
  }

  void p031_shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
  {
  	uint8_t i;

  	for (i = 0; i < 8; i++)  {
  		if (bitOrder == LSBFIRST)
  			digitalWrite(dataPin, !!(val & (1 << i)));
  		else
  			digitalWrite(dataPin, !!(val & (1 << (7 - i))));

  		digitalWrite(clockPin, HIGH);
      P031_DELAY_LONGER_CABLES
  		digitalWrite(clockPin, LOW);
      P031_DELAY_LONGER_CABLES
  	}
  }


  float tempC = 0.0f;
  float rhTrue = 0.0f;
  unsigned long sendCommandTime = 0;

  int input_mode = 0;
  byte _dataPin = 0;
  byte _clockPin = 0;
  byte state = P031_IDLE;
  byte _clockdelay = 0;
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
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
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

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Clock Delay"), F("p031_delay"), PCONFIG(0), 0, P031_MAX_CLOCK_DELAY);
      addUnit(F("usec"));
      addFormNote(F("Reduce clock/data frequency to allow for longer cables"));
      success = true;
      break;
    }
    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = getFormItemInt(F("p031_delay"));
        success = true;
        break;
      }


    case PLUGIN_INIT:
      {
        initPluginTaskData(event->TaskIndex, new (std::nothrow) P031_data_struct());
        P031_data_struct *P031_data =
            static_cast<P031_data_struct *>(getPluginTaskData(event->TaskIndex));
        if (nullptr == P031_data) {
          return success;
        }
        byte status = P031_data->init(
          CONFIG_PIN1, CONFIG_PIN2,
          Settings.TaskDevicePin1PullUp[event->TaskIndex],
          PCONFIG(0));
        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("SHT1X : Status byte: ");
          log += String(status, HEX);
          log += F(" - resolution: ");
          log += ((status & 1) ? F("low") : F("high"));
          log += F(" reload from OTP: ");
          log += (((status >> 1) & 1) ? F("yes") : F("no"));
          log += F(", heater: ");
          log += (((status >> 2) & 1) ? F("on") : F("off"));
          addLog(LOG_LEVEL_DEBUG, log);
        }
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
    {
      P031_data_struct *P031_data =
          static_cast<P031_data_struct *>(getPluginTaskData(event->TaskIndex));
      if (nullptr != P031_data) {
        if (P031_data->process()) {
          // Measurement ready, schedule new read.
          Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
        }
      }
      success = true;
      break;
    }

    case PLUGIN_READ:
      {
        P031_data_struct *P031_data =
            static_cast<P031_data_struct *>(getPluginTaskData(event->TaskIndex));
        if (nullptr != P031_data) {
          if (P031_data->measurementReady()) {
            UserVar[event->BaseVarIndex] = P031_data->tempC;
            UserVar[event->BaseVarIndex+1] = P031_data->rhTrue;
            success = true;
            P031_data->state = P031_IDLE;
          } else if (P031_data->state == P031_IDLE) {
            P031_data->startMeasurement();
          } else if (P031_data->hasError()) {
            // Log error
            switch (P031_data->state) {
              case P031_COMMAND_NO_ACK:
                addLog(LOG_LEVEL_ERROR, F("SHT1X : Sensor did not ACK command"));
                break;
              case P031_NO_DATA:
                addLog(LOG_LEVEL_ERROR, F("SHT1X : Data not ready"));
                break;
              default:
                break;
            }
            P031_data->state = P031_IDLE;
          }
        }
        break;
      }
  }
  return success;
}


#endif // USES_P031
