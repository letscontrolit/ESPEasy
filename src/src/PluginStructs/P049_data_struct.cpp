#include "../PluginStructs/P049_data_struct.h"

#ifdef USES_P049


// 9 uint8_t commands:
// mhzCmdReadPPM[]              = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
// mhzCmdCalibrateZero[]        = {0xFF,0x01,0x87,0x00,0x00,0x00,0x00,0x00,0x78};
// mhzCmdABCEnable[]            = {0xFF,0x01,0x79,0xA0,0x00,0x00,0x00,0x00,0xE6};
// mhzCmdABCDisable[]           = {0xFF,0x01,0x79,0x00,0x00,0x00,0x00,0x00,0x86};
// mhzCmdReset[]                = {0xFF,0x01,0x8d,0x00,0x00,0x00,0x00,0x00,0x72};

/* It seems the offsets [3]..[4] for the detection range setting (command uint8_t 0x99) are wrong in the latest
 * online data sheet: http://www.winsen-sensor.com/d/files/infrared-gas-sensor/mh-z19b-co2-ver1_0.pdf
 * According to the MH-Z19B datasheet version 1.2, valid from: 2017.03.22 (received 2018-03-07)
 * the offset should be [6]..[7] instead.
 * 0x99 - Detection range setting, send command:
 * /---------+---------+---------+---------+---------+---------+---------+---------+---------\
 * | Byte 0  | Byte 1  | Byte 2  | Byte 3  | Byte 4  | Byte 5  | Byte 6  | Byte 7  | Byte 8  |
 * |---------+---------+---------+---------+---------+---------+---------+---------+---------|
 * | Start   | Reserved| Command | Reserved|Detection|Detection|Detection|Detection| Checksum|
 * | Byte    |         |         |         |range    |range    |range    |range    |         |
 * |         |         |         |         |24~32 bit|16~23 bit|8~15 bit |0~7 bit  |         |
 * |---------+---------+---------+---------+---------+---------+---------+---------+---------|
 * | 0xFF    | 0x01    | 0x99    | 0x00    | Data 1  | Data 2  | Data 3  | Data 4  | Checksum|
 * \---------+---------+---------+---------+---------+---------+---------+---------+---------/
 * Note: Detection range should be 0~2000, 0~5000, 0~10000 ppm.
 * For example: set 0~2000 ppm  detection range, send command: FF 01 99 00 00 00 07 D0 8F
 *              set 0~10000 ppm detection range, send command: FF 01 99 00 00 00 27 10 8F
 * The latter, updated version above is implemented here.
 */

// mhzCmdMeasurementRange1000[] = {0xFF,0x01,0x99,0x00,0x00,0x00,0x03,0xE8,0x7B};
// mhzCmdMeasurementRange2000[] = {0xFF,0x01,0x99,0x00,0x00,0x00,0x07,0xD0,0x8F};
// mhzCmdMeasurementRange3000[] = {0xFF,0x01,0x99,0x00,0x00,0x00,0x0B,0xB8,0xA3};
// mhzCmdMeasurementRange5000[] = {0xFF,0x01,0x99,0x00,0x00,0x00,0x13,0x88,0xCB};
// Removing redundant data, just keeping offsets [2], [6]..[7]:
const PROGMEM uint8_t mhzCmdData[][3] = {
  { 0x86, 0x00, 0x00 },
  { 0x87, 0x00, 0x00 },
  { 0x79, 0xA0, 0x00 },
  { 0x79, 0x00, 0x00 },
  { 0x8d, 0x00, 0x00 },
# ifdef ENABLE_DETECTION_RANGE_COMMANDS
  { 0x99, 0x03, 0xE8 },
  { 0x99, 0x07, 0xD0 },
  { 0x99, 0x0B, 0xB8 },
  { 0x99, 0x13, 0x88 }
# endif // ifdef ENABLE_DETECTION_RANGE_COMMANDS
};



P049_data_struct::~P049_data_struct() {
  if (easySerial != nullptr) {
    delete easySerial;
    easySerial = nullptr;
  }
}

void P049_data_struct::reset() {
  if (easySerial != nullptr) {
    delete easySerial;
    easySerial = nullptr;
  }
  linesHandled       = 0;
  checksumFailed     = 0;
  nrUnknownResponses = 0;
  ++sensorResets;

  // Default of the sensor is to run ABC
  ABC_Disable     = false;
  ABC_MustApply   = false;
  modelA_detected = false;
}

bool P049_data_struct::init(ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx, bool setABCdisabled) {
  if ((serial_rx < 0) || (serial_tx < 0)) {
    return false;
  }
  reset();
  easySerial = new (std::nothrow) ESPeasySerial(port, serial_rx, serial_tx);

  if (easySerial == nullptr) {
    return false;
  }
  easySerial->begin(9600);
  ABC_Disable = setABCdisabled;

  if (ABC_Disable) {
    // No guarantee the correct state is active on the sensor after reboot.
    ABC_MustApply = true;
  }
  lastInitTimestamp = millis();
  initTimePassed    = false;
  return isInitialized();
}

bool P049_data_struct::plugin_write(struct EventStruct *event, const String& string)
{
  String command = parseString(string, 1);

  if (equals(command, F("mhzcalibratezero")))
  {
    send_mhzCmd(mhzCmdCalibrateZero);
    addLog(LOG_LEVEL_INFO, F("MHZ19: Calibrated zero point!"));
    return true;
  }
  else if (equals(command, F("mhzreset")))
  {
    send_mhzCmd(mhzCmdReset);
    addLog(LOG_LEVEL_INFO, F("MHZ19: Sent sensor reset!"));
    return true;
  }
  else if (equals(command, F("mhzabcenable")))
  {
    send_mhzCmd(mhzCmdABCEnable);
    addLog(LOG_LEVEL_INFO, F("MHZ19: Sent sensor ABC Enable!"));
    return true;
  }
  else if (equals(command, F("mhzabcdisable")))
  {
    send_mhzCmd(mhzCmdABCDisable);
    addLog(LOG_LEVEL_INFO, F("MHZ19: Sent sensor ABC Disable!"));
    return true;
  }

# ifdef ENABLE_DETECTION_RANGE_COMMANDS
  else if (command.startsWith(F("mhzmeasurementrange"))) {
    if (equals(command, F("mhzmeasurementrange1000")))
    {
      send_mhzCmd(mhzCmdMeasurementRange1000);
      addLog(LOG_LEVEL_INFO, F("MHZ19: Sent measurement range 0-1000PPM!"));
      return true;
    }
    else if (equals(command, F("mhzmeasurementrange2000")))
    {
      send_mhzCmd(mhzCmdMeasurementRange2000);
      addLog(LOG_LEVEL_INFO, F("MHZ19: Sent measurement range 0-2000PPM!"));
      return true;
    }
    else if (equals(command, F("mhzmeasurementrange3000")))
    {
      send_mhzCmd(mhzCmdMeasurementRange3000);
      addLog(LOG_LEVEL_INFO, F("MHZ19: Sent measurement range 0-3000PPM!"));
      return true;
    }
    else if (equals(command, F("mhzmeasurementrange5000")))
    {
      send_mhzCmd(mhzCmdMeasurementRange5000);
      addLog(LOG_LEVEL_INFO, F("MHZ19: Sent measurement range 0-5000PPM!"));
      return true;
    }
  }
# endif // ENABLE_DETECTION_RANGE_COMMANDS
  return false;
}

void P049_data_struct::setABCmode(int abcDisableSetting) {
  const bool new_ABC_disable = (abcDisableSetting == P049_ABC_disabled);

  if (ABC_Disable != new_ABC_disable) {
    // Setting changed in the webform.
    ABC_MustApply = true;
    ABC_Disable   = new_ABC_disable;
  }
}

uint8_t P049_data_struct::calculateChecksum() const {
  uint8_t checksum = 0;

  for (uint8_t i = 1; i < 8; i++) {
    checksum += mhzResp[i];
  }
  checksum = 0xFF - checksum;
  return checksum + 1;
}

size_t P049_data_struct::send_mhzCmd(uint8_t CommandId)
{
  if (!isInitialized()) { return 0; }

  // The receive buffer "mhzResp" is re-used to send a command here:
  mhzResp[0] = 0xFF; // Start uint8_t, fixed
  mhzResp[1] = 0x01; // Sensor number, 0x01 by default
  memcpy_P(&mhzResp[2], mhzCmdData[CommandId], sizeof(mhzCmdData[0]));
  mhzResp[6] = mhzResp[3]; mhzResp[7] = mhzResp[4];
  mhzResp[3] = mhzResp[4] = mhzResp[5] = 0x00;
  mhzResp[8] = calculateChecksum();

  if (!initTimePassed) {
    // Allow for 3 minutes of init time.
    initTimePassed = timePassedSince(lastInitTimestamp) > 180000;
  }

  return easySerial->write(mhzResp, sizeof(mhzResp));
}

bool P049_data_struct::read_ppm(unsigned int& ppm, signed int& temp, unsigned int& s, float& u) {
  if (!isInitialized()) { return false; }

  // send read PPM command
  uint8_t nbBytesSent = send_mhzCmd(mhzCmdReadPPM);

  if (nbBytesSent != 9) {
    return false;
  }

  // get response
  memset(mhzResp, 0, sizeof(mhzResp));

  long timer   = millis() + PLUGIN_READ_TIMEOUT;
  int  counter = 0;

  while (!timeOutReached(timer) && (counter < 9)) {
    if (easySerial->available() > 0) {
      uint8_t value = easySerial->read();

      if (((counter == 0) && (value == 0xFF)) || (counter > 0)) {
        mhzResp[counter++] = value;
      }
    } else {
      delay(10);
    }
  }

  if (counter < 9) {
    // Timeout
    return false;
  }
  ++linesHandled;

  if (!(mhzResp[8] == calculateChecksum())) {
    ++checksumFailed;
    return false;
  }

  if ((mhzResp[0] == 0xFF) && (mhzResp[1] == 0x86)) {
    // calculate CO2 PPM
    ppm = (static_cast<unsigned int>(mhzResp[2]) << 8) + mhzResp[3];

    // set temperature (offset 40)
    unsigned int mhzRespTemp = (unsigned int)mhzResp[4];
    temp = mhzRespTemp - 40;

    // set 's' (stability) value
    s = mhzResp[5];

    if (s != 0) {
      modelA_detected = true;
    }

    // calculate 'u' value
    u = (static_cast<unsigned int>(mhzResp[6]) << 8) + mhzResp[7];
    return true;
  }
  return false;
}

bool P049_data_struct::receivedCommandAcknowledgement(bool& expectReset) {
  expectReset = false;

  if (mhzResp[0] == 0xFF)  {
    switch (mhzResp[1]) {
      case 0x86: // Read CO2 concentration
      case 0x79: // ON/OFF Auto Calibration
        break;
      case 0x87: // Calibrate Zero Point (ZERO)
      case 0x88: // Calibrate Span Point (SPAN)
      case 0x99: // Detection range setting
        expectReset = true;
        break;
      default:
        ++nrUnknownResponses;
        return false;
    }
    uint8_t checksum = calculateChecksum();
    return mhzResp[8] == checksum;
  }
  ++nrUnknownResponses;
  return false;
}

String P049_data_struct::getBufferHexDump() const {
  String result;

  result.reserve(27);

  for (int i = 0; i < 9; ++i) {
    result += ' ';
    result += String(mhzResp[i], HEX);
  }
  return result;
}

MHZ19Types P049_data_struct::getDetectedDevice() const {
  if (linesHandled > checksumFailed) {
    return modelA_detected ? MHZ19_A : MHZ19_B;
  }
  return MHZ19_notDetected;
}

bool Plugin_049_Check_and_ApplyFilter(unsigned int prevVal, unsigned int& newVal, uint32_t s, const int filterValue, String& log) {
  if (s == 1) {
    // S==1 => "A" version sensor bootup, do not use values.
    return false;
  }

  if ((prevVal < 400) || (prevVal > 5000)) {
    // Prevent unrealistic values during start-up with filtering enabled.
    // Just assume the entered value is correct.
    return true;
  }
  bool filterApplied = filterValue > PLUGIN_049_FILTER_OFF_ALLSAMPLES;
  int32_t difference = newVal - prevVal;

  if ((s > 0) && (s < 64) && (filterValue != PLUGIN_049_FILTER_OFF)) {
    // Not the "B" version of the sensor, S value is used.
    // S==0 => "B" version, else "A" version
    // The S value is an indication of the stability of the reading.
    // S == 64 represents a stable reading and any lower value indicates (unusual) fast change.
    // Now we increase the delay filter for low values of S and increase response time when the
    // value is more stable.
    // This will make the reading useful in more turbulent environments,
    // where the sensor would report more rapid change of measured values.
    difference    = difference * s;
    difference   /= 64;
    log          += F("Compensate Unstable ");
    filterApplied = true;
  }

  switch (filterValue) {
    case PLUGIN_049_FILTER_OFF: {
      if ((s != 0) && (s != 64)) {
        log += F("Skip Unstable ");
        return false;
      }
      filterApplied = false;
      break;
    }

    // #Samples to reach >= 75% of step response
    case PLUGIN_049_FILTER_OFF_ALLSAMPLES: filterApplied = false; break; // No Delay
    case PLUGIN_049_FILTER_FAST:    difference          /= 2; break;     // Delay: 2 samples
    case PLUGIN_049_FILTER_MEDIUM:  difference          /= 4; break;     // Delay: 5 samples
    case PLUGIN_049_FILTER_SLOW:    difference          /= 8; break;     // Delay: 11 samples
  }

  if (filterApplied) {
    log += F("Raw PPM: ");
    log += newVal;
    log += F(" Filtered ");
  }
  newVal = static_cast<unsigned int>(prevVal + difference);
  return true;
}

void P049_html_show_stats(struct EventStruct *event) {
  P049_data_struct *P049_data =
    static_cast<P049_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P049_data) {
    return;
  }

  addRowLabel(F("Checksum (pass/fail/reset)"));
  addHtmlInt(P049_data->linesHandled);
  addHtml('/');
  addHtmlInt(P049_data->checksumFailed);
  addHtml('/');
  addHtmlInt(P049_data->sensorResets);

  addRowLabel(F("Detected"));

  switch (P049_data->getDetectedDevice()) {
    case MHZ19_A: addHtml(F("MH-Z19A")); break;
    case MHZ19_B: addHtml(F("MH-Z19B")); break;
    default: addHtml(F("---")); break;
  }
}

bool P049_perform_init(struct EventStruct *event) {
  bool success                 = false;
  const int16_t serial_rx      = CONFIG_PIN1;
  const int16_t serial_tx      = CONFIG_PIN2;
  const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
  P049_data_struct *P049_data  =
    static_cast<P049_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr != P049_data) {
    if (P049_data->init(port, serial_rx, serial_tx, (PCONFIG(0) == P049_ABC_disabled))) {
      success = true;
      addLog(LOG_LEVEL_INFO, F("MHZ19: Init OK "));

      // delay first read, because hardware needs to initialize on cold boot
      // otherwise we get a weird value or read error
      Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 15000);
    }
  }
  return success;
}

#endif // ifdef USES_P049
