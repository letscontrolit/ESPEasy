#ifdef USES_P031
//#######################################################################################################
//#################### Plugin 031: SHT10/SHT11/SHT15 Temp/Humidity Sensor ###############################
//#######################################################################################################

#define PLUGIN_031
#define PLUGIN_ID_031         31
#define PLUGIN_NAME_031       "Environment - SHT1X"
#define PLUGIN_VALUENAME1_031 "Temperature"
#define PLUGIN_VALUENAME2_031 "Humidity"

boolean Plugin_031_init = false;
byte Plugin_031_DATA_Pin = 0;
byte Plugin_031_CLOCK_Pin = 0;
int input_mode;

enum {
  SHT1X_CMD_MEASURE_TEMP  = B00000011,
  SHT1X_CMD_MEASURE_RH    = B00000101,
  SHT1X_CMD_READ_STATUS   = B00000111,
  SHT1X_CMD_SOFT_RESET    = B00011110
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

    case PLUGIN_INIT:
      {
        Plugin_031_init = true;
        Plugin_031_DATA_Pin = Settings.TaskDevicePin1[event->TaskIndex];
        Plugin_031_CLOCK_Pin = Settings.TaskDevicePin2[event->TaskIndex];
        if (Settings.TaskDevicePin1PullUp[event->TaskIndex]) {
          input_mode = INPUT_PULLUP;
          String log = F("SHT1X: Setting PullUp on pin ");
          log += String(Plugin_031_DATA_Pin);
          addLog(LOG_LEVEL_DEBUG, log);
        }
        else {
          input_mode = INPUT;
        }
        pinMode(Plugin_031_DATA_Pin, input_mode); /* Keep Hi-Z except when sending data */
        pinMode(Plugin_031_CLOCK_Pin, OUTPUT);
        Plugin_031_reset();
        byte status = Plugin_031_readStatus();
        String log = F("SHT1X : Status byte: ");
        log += String(status, HEX);
        log += F(" - resolution: ");
        log += (status & 1 ? F("low") : F("high"));
        log += F(" reload from OTP: ");
        log += ((status >> 1) & 1 ? F("yes") : F("no"));
        log += F(", heater: ");
        log += ((status >> 2) & 1 ? F("on") : F("off"));
        addLog(LOG_LEVEL_DEBUG, log);
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        if (!Plugin_031_init) {
          addLog(LOG_LEVEL_ERROR, F("SHT1X : not yet initialized!"));
          break;
        }
        UserVar[event->BaseVarIndex] = Plugin_031_readTemperature();
        UserVar[event->BaseVarIndex+1] = Plugin_031_readRelHumidity(UserVar[event->BaseVarIndex]);
        success = true;
        break;
      }
  }
  return success;
}

float Plugin_031_readTemperature()
{
  float tempRaw, tempC;

  Plugin_031_sendCommand(SHT1X_CMD_MEASURE_TEMP);
  Plugin_031_awaitResult();
  tempRaw = Plugin_031_readData(16);

  // Temperature conversion coefficients from SHT1X datasheet for version 4
  const float d1 = -39.7;  // 3.5V
  const float d2 = 0.01;   // 14-bit

  tempC = d1 + (tempRaw * d2);

  String log = F("SHT1X : Read temperature (raw): ");
  log += String(tempRaw);
  log += F(" (Celcius): ");
  log += String(tempC);
  addLog(LOG_LEVEL_DEBUG, log);

  return tempC;
}

float Plugin_031_readRelHumidity(float tempC)
{
  float raw, rhLinear, rhTrue;

  Plugin_031_sendCommand(SHT1X_CMD_MEASURE_RH);
  Plugin_031_awaitResult();
  raw = Plugin_031_readData(16);

  // Temperature conversion coefficients from SHT1X datasheet for version 4
  const float c1 = -2.0468;
  const float c2 = 0.0367;
  const float c3 = -1.5955E-6;
  const float t1 = 0.01;
  const float t2 = 0.00008;

  rhLinear = c1 + c2 * raw + c3 * raw * raw;
  rhTrue = (tempC - 25) * (t1 + t2 * raw) + rhLinear;

  String log = F("SHT1X : Read humidity (raw): ");
  log += String(raw);
  log += F(" (Linear): ");
  log += String(rhLinear);
  log += F(" (True): ");
  log += String(rhTrue);
  addLog(LOG_LEVEL_DEBUG, log);

  return rhTrue;
}


void Plugin_031_reset()
{
  delay(11);
  for (int i=0; i<9; i++) {
    digitalWrite(Plugin_031_CLOCK_Pin, HIGH);
    digitalWrite(Plugin_031_CLOCK_Pin, LOW);
  }
  Plugin_031_sendCommand(SHT1X_CMD_SOFT_RESET);
  delay(11);
}

byte Plugin_031_readStatus()
{
  Plugin_031_sendCommand(SHT1X_CMD_READ_STATUS);
  return Plugin_031_readData(8);
}

void Plugin_031_sendCommand(const byte cmd)
{
  pinMode(Plugin_031_DATA_Pin, OUTPUT);

  // Transmission Start sequence
  digitalWrite(Plugin_031_DATA_Pin, HIGH);
  digitalWrite(Plugin_031_CLOCK_Pin, HIGH);
  digitalWrite(Plugin_031_DATA_Pin, LOW);
  digitalWrite(Plugin_031_CLOCK_Pin, LOW);
  digitalWrite(Plugin_031_CLOCK_Pin, HIGH);
  digitalWrite(Plugin_031_DATA_Pin, HIGH);
  digitalWrite(Plugin_031_CLOCK_Pin, LOW);

  // Send the command (address must be 000b)
  shiftOut(Plugin_031_DATA_Pin, Plugin_031_CLOCK_Pin, MSBFIRST, cmd);

  // Wait for ACK
  bool ackerror = false;
  digitalWrite(Plugin_031_CLOCK_Pin, HIGH);
  pinMode(Plugin_031_DATA_Pin, input_mode);
  if (digitalRead(Plugin_031_DATA_Pin) != LOW) ackerror = true;
  digitalWrite(Plugin_031_CLOCK_Pin, LOW);

  if (cmd == SHT1X_CMD_MEASURE_TEMP || cmd == SHT1X_CMD_MEASURE_RH) {
    delayMicroseconds(1); /* Give the sensor time to release the data line */
    if (digitalRead(Plugin_031_DATA_Pin) != HIGH) ackerror = true;
  }

  if (ackerror) {
    addLog(LOG_LEVEL_ERROR, F("SHT1X : Sensor did not ACK command"));
  }
}

void Plugin_031_awaitResult()
{
  // Maximum 320ms for 14 bit measurement
  for (int i=0; i<16; i++) {
    if (digitalRead(Plugin_031_DATA_Pin) == LOW) return;
    delay(20);
  }
  if (digitalRead(Plugin_031_DATA_Pin) != LOW) {
    addLog(LOG_LEVEL_ERROR, F("SHT1X : Data not ready"));
  }
}

int Plugin_031_readData(const int bits)
{
  int val = 0;

  if (bits == 16) {
    // Read most significant byte
    val = shiftIn(Plugin_031_DATA_Pin, Plugin_031_CLOCK_Pin, 8);
    val <<= 8;

    // Send ACK
    pinMode(Plugin_031_DATA_Pin, OUTPUT);
    digitalWrite(Plugin_031_DATA_Pin, LOW);
    digitalWrite(Plugin_031_CLOCK_Pin, HIGH);
    digitalWrite(Plugin_031_CLOCK_Pin, LOW);
    pinMode(Plugin_031_DATA_Pin, input_mode);
  }

  // Read least significant byte
  val |= shiftIn(Plugin_031_DATA_Pin, Plugin_031_CLOCK_Pin, 8);

  // Keep DATA pin high to skip CRC
  digitalWrite(Plugin_031_CLOCK_Pin, HIGH);
  digitalWrite(Plugin_031_CLOCK_Pin, LOW);

  return val;
}
#endif // USES_P031
