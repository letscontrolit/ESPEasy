#include "../PluginStructs/P031_data_struct.h"

#ifdef USES_P031

uint8_t P031_data_struct::init(uint8_t data_pin, uint8_t clock_pin, bool pullUp, uint8_t clockdelay) {
  _dataPin    = data_pin;
  _clockPin   = clock_pin;
  _clockdelay = clockdelay;

  if (_clockdelay > P031_MAX_CLOCK_DELAY) {
    _clockdelay = P031_MAX_CLOCK_DELAY;
  }
  input_mode = pullUp ? INPUT_PULLUP : INPUT;
  state      = P031_IDLE;

  pinMode(_dataPin,  input_mode); /* Keep Hi-Z except when sending data */
  pinMode(_clockPin, OUTPUT);
  resetSensor();
  return readStatus();
}

bool P031_data_struct::process() {
  switch (state) {
    case P031_IDLE: return false; // Nothing changed, nothing to do
    case P031_WAIT_TEMP: {
      if (digitalRead(_dataPin) == LOW) {
        float tempRaw = readData(16);

        // Temperature conversion coefficients from SHT1X datasheet for version 4
        const float d1 = -39.7f; // 3.5V
        const float d2 = 0.01f;  // 14-bit
        tempC = d1 + (tempRaw * d2);
        state = P031_WAIT_HUM;   // Wait for humidity
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

        const float rhLinear = c1 + c2 * raw + c3 * raw * raw;
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

void P031_data_struct::startMeasurement() {
  state = P031_WAIT_TEMP; // Wait for temperature
  sendCommand(SHT1X_CMD_MEASURE_TEMP);
}

void P031_data_struct::resetSensor()
{
  state = P031_IDLE;
  delay(11);

  for (int i = 0; i < 9; i++) {
    digitalWrite(_clockPin, HIGH);
    digitalWrite(_clockPin, LOW);
  }
  sendCommand(SHT1X_CMD_SOFT_RESET);
  delay(11);
}

uint8_t P031_data_struct::readStatus()
{
  sendCommand(SHT1X_CMD_READ_STATUS);
  return readData(8);
}

void P031_data_struct::sendCommand(const uint8_t cmd)
{
  sendCommandTime = millis();

  // Transmission Start sequence
  digitalWrite(_dataPin, HIGH);
  pinMode(_dataPin, OUTPUT);
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
    shiftOut(_dataPin, _clockPin, MSBFIRST, cmd);

  // Wait for ACK
  pinMode(_dataPin, input_mode);
  bool ackerror = false;

  digitalWrite(_clockPin, HIGH);
  P031_DELAY_LONGER_CABLES

  if (digitalRead(_dataPin) != LOW) { ackerror = true; }
  digitalWrite(_clockPin, LOW);
  P031_DELAY_LONGER_CABLES

  if ((cmd == SHT1X_CMD_MEASURE_TEMP) || (cmd == SHT1X_CMD_MEASURE_RH)) {
    delayMicroseconds(1); /* Give the sensor time to release the data line */

    if (digitalRead(_dataPin) != HIGH) { ackerror = true; }
  }

  if (ackerror) {
    state = P031_COMMAND_NO_ACK;
  }
}

int P031_data_struct::readData(const int bits) const
{
  int val = 0;

  if (bits == 16) {
    // Read most significant uint8_t
    val   = shiftIn(_dataPin, _clockPin, MSBFIRST);
    val <<= 8;

    // Send ACK
    pinMode(_dataPin, OUTPUT);
    digitalWrite(_dataPin,  LOW);
    digitalWrite(_clockPin, HIGH);
    P031_DELAY_LONGER_CABLES
      digitalWrite(_clockPin, LOW);
    P031_DELAY_LONGER_CABLES
      pinMode(_dataPin, input_mode);
  }

  // Read least significant uint8_t
  val |= shiftIn(_dataPin, _clockPin, MSBFIRST);

  // Keep DATA pin high to skip CRC
  digitalWrite(_clockPin, HIGH);
  P031_DELAY_LONGER_CABLES
    digitalWrite(_clockPin, LOW);

  P031_DELAY_LONGER_CABLES

  return val;
}

uint8_t P031_data_struct::shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) const
{
  uint8_t value = 0;
  uint8_t i;

  for (i = 0; i < 8; ++i) {
    digitalWrite(clockPin, HIGH);
    P031_DELAY_LONGER_CABLES

    if (bitOrder == LSBFIRST) {
      value |= digitalRead(dataPin) << i;
    }
    else {
      value |= digitalRead(dataPin) << (7 - i);
    }
    digitalWrite(clockPin, LOW);
    P031_DELAY_LONGER_CABLES
  }
  return value;
}

void P031_data_struct::shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val) const
{
  uint8_t i;

  for (i = 0; i < 8; i++)  {
    if (bitOrder == LSBFIRST) {
      digitalWrite(dataPin, !!(val & (1 << i)));
    }
    else {
      digitalWrite(dataPin, !!(val & (1 << (7 - i))));
    }

    digitalWrite(clockPin, HIGH);
    P031_DELAY_LONGER_CABLES
      digitalWrite(clockPin, LOW);
    P031_DELAY_LONGER_CABLES
  }
}

#endif // ifdef USES_P031
