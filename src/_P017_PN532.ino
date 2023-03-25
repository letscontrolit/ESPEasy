#include "_Plugin_Helper.h"
#ifdef USES_P017

// #######################################################################################################
// #################################### Plugin-017: PN532 RFID reader ####################################
// #######################################################################################################


/*
 ################## WARNING!!!!! ################
   See this conversation whenever new issues are reported for this plugin:
   https://github.com/letscontrolit/ESPEasy/issues/4170#issuecomment-1211901726
 */

# define PLUGIN_017
# define PLUGIN_ID_017         17
# define PLUGIN_NAME_017       "RFID - PN532"
# define PLUGIN_VALUENAME1_017 "Tag"

# define PN532_I2C_ADDRESS             0x24

# define PN532_PREAMBLE                (0x00)
# define PN532_STARTCODE1              (0x00)
# define PN532_STARTCODE2              (0xFF)
# define PN532_POSTAMBLE               (0x00)
# define PN532_HOSTTOPN532             (0xD4)
# define PN532_PN532TOHOST             (0xD5)
# define PN532_ACK_WAIT_TIME           (5)
# define PN532_INVALID_ACK             (-1)
# define PN532_TIMEOUT                 (-2)
# define PN532_INVALID_FRAME           (-3)
# define PN532_NO_SPACE                (-4)

# define PN532_COMMAND_GETFIRMWAREVERSION    (0x02)
# define PN532_COMMAND_SAMCONFIGURATION      (0x14)
# define PN532_COMMAND_INLISTPASSIVETARGET   (0x4A)
# define PN532_RESPONSE_INLISTPASSIVETARGET  (0x4B)
# define PN532_MIFARE_ISO14443A              (0x00)
# define PN532_COMMAND_POWERDOWN             (0x16)

// This plugin uses PLUGIN_TASKTIMER_IN to perform stages at specific intervals
// The PLUGIN_TEN_PER_SECOND etc. are too generic for this
# define PN532_TIMER_TYPE_REMOVE_TAG         1
# define PN532_TIMER_TYPE_START_READ_TAG     2
# define PN532_TIMER_TYPE_READ_TAG_RESPONSE  3

# define PN532_INTERVAL_BETWEEN_READS   300 // Perform a read roughly every 300 msec.
// Delay between PN532_TIMER_TYPE_START_READ_TAG and PN532_TIMER_TYPE_READ_TAG_RESPONSE
// 20 msec seems to work fine, but just to be sure set to 30 msec.
# define PN532_DELAY_READ_TAG_RESPONSE  30

# define P017_AUTO_TAG_REMOVAL      PCONFIG(0)
# define P017_EVENT_ON_TAG_REMOVAL  PCONFIG(1)
# define P017_NO_TAG_DETECTED_VALUE PCONFIG_LONG(0)
# define P017_REMOVAL_TIMEOUT       PCONFIG_LONG(1)


// DEBUG code using logic analyzer for timings
//# define P017_DEBUG_LOGIC_ANALYZER_PIN       25
//# define P017_DEBUG_LOGIC_ANALYZER_PIN_INIT  33

# include <GPIO_Direct_Access.h>


// Forward declarations
bool     P017_handle_timer_in(struct EventStruct *event);
boolean  Plugin_017_Init(int8_t resetPin);
uint8_t  Plugin_017_readPassiveTargetID(uint8_t *uid,
                                        uint8_t *uidLength);
uint8_t  Plugin_017_StartReadPassiveTargetID(uint8_t cardbaudrate);
uint32_t getFirmwareVersion(void);
void     Plugin_017_powerDown(void);
int8_t   Plugin_017_writeCommand(const uint8_t *header,
                                 uint8_t        hlen);
int16_t  Plugin_017_readResponse(uint8_t command,
                                 uint8_t buf[],
                                 uint8_t len);

boolean  Plugin_017(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_017;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_LONG;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_017);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_017));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = (event->Par1 == 0x24);
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = 0x24;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      string  = F("RST: ");
      string += formatGpioLabel(CONFIG_PIN3, false);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // FIXME TD-er: Why is this using pin3 and not pin1? And why isn't this using the normal pin selection functions?
      addFormPinSelect(PinSelectPurpose::Generic, F("Reset Pin"), F("taskdevicepin3"), CONFIG_PIN3);

      const bool autoTagRemoval = P017_AUTO_TAG_REMOVAL == 0; // Inverted state!
      addFormCheckBox(F("Automatic Tag removal"), F("tagremove"), autoTagRemoval);

      if (P017_REMOVAL_TIMEOUT == 0) { 
        P017_REMOVAL_TIMEOUT = 500; // Defaulty 500 mSec (was hardcoded value)
      }
      // 0.25 to 60 seconds
      addFormNumericBox(F("Automatic Tag removal after"), F("removetime"), P017_REMOVAL_TIMEOUT, 250, 60000); 
      addUnit(F("mSec."));

      
      addFormNumericBox(F("Value to set on Tag removal"), F("removevalue"), P017_NO_TAG_DETECTED_VALUE, 0, 2147483647); 
      // Max allowed is int
      // =
      // 0x7FFFFFFF ...

      const bool eventOnRemoval = P017_EVENT_ON_TAG_REMOVAL == 1; // Normal state!
      addFormCheckBox(F("Event on Tag removal"), F("eventremove"), eventOnRemoval);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P017_AUTO_TAG_REMOVAL      = isFormItemChecked(F("tagremove")) ? 0 : 1; // Inverted logic!
      P017_EVENT_ON_TAG_REMOVAL  = isFormItemChecked(F("eventremove")) ? 1 : 0;
      P017_NO_TAG_DETECTED_VALUE = getFormItemInt(F("removevalue"));
      P017_REMOVAL_TIMEOUT       = getFormItemInt(F("removetime"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // set clock stretch to 2000, if its not set via advanced settings yet
      // something that Martinus figured out and added: https://github.com/esp8266/Arduino/issues/1541
      // if (!Settings.WireClockStretchLimit)
      //   Wire.setClockStretchLimit(2000);

      # ifdef P017_DEBUG_LOGIC_ANALYZER_PIN

      // DEBUG code using logic analyzer for timings
      pinMode(P017_DEBUG_LOGIC_ANALYZER_PIN, OUTPUT);
      # endif // ifdef P017_DEBUG_LOGIC_ANALYZER_PIN
      # ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT

      // DEBUG code using logic analyzer for timings
      pinMode(P017_DEBUG_LOGIC_ANALYZER_PIN_INIT, OUTPUT);
      # endif // ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT


      for (uint8_t x = 0; x < 3; x++)
      {
        if (Plugin_017_Init(CONFIG_PIN3)) {
          success = true;
          break;
        }
        delay(100);
      }
      Scheduler.setPluginTaskTimer(PN532_INTERVAL_BETWEEN_READS, event->TaskIndex, PN532_TIMER_TYPE_START_READ_TAG);
      break;
    }

    case PLUGIN_TASKTIMER_IN:
    {
      success = P017_handle_timer_in(event);
      break;
    }
  }
  return success;
}

bool P017_handle_timer_in(struct EventStruct *event)
{
  bool success = false;

  static unsigned long tempcounter = 0;
  static uint8_t errorCount        = 0;

  switch (event->Par1) {
    case PN532_TIMER_TYPE_REMOVE_TAG:
    {
# ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT

      // DEBUG code using logic analyzer for timings
      // Mark we cleared the card anyway.
      DIRECT_pinWrite(P017_DEBUG_LOGIC_ANALYZER_PIN_INIT, 0);
# endif // ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT

      // Reset card id on timeout
      if (P017_AUTO_TAG_REMOVAL == 0) {
        UserVar.setSensorTypeLong(event->TaskIndex, P017_NO_TAG_DETECTED_VALUE);
        addLog(LOG_LEVEL_INFO, F("RFID : Removed Tag"));

        if (P017_EVENT_ON_TAG_REMOVAL == 1) {
          sendData(event);
        }
        success = true;
      }
      break;
    }
    case PN532_TIMER_TYPE_START_READ_TAG:
    {
        # ifdef P017_DEBUG_LOGIC_ANALYZER_PIN

      // DEBUG code using logic analyzer for timings
      DIRECT_pinWrite(P017_DEBUG_LOGIC_ANALYZER_PIN, 1);
        # endif // ifdef P017_DEBUG_LOGIC_ANALYZER_PIN

      // TODO: Clock stretching issue https://github.com/esp8266/Arduino/issues/1541
      if (Settings.isI2CEnabled()
          && ((DIRECT_pinRead(Settings.Pin_i2c_sda) == 0) || (DIRECT_pinRead(Settings.Pin_i2c_scl) == 0)))
      {
        addLog(LOG_LEVEL_ERROR, F("PN532: BUS error"));
        Plugin_017_Init(CONFIG_PIN3);
      }

      if (Plugin_017_StartReadPassiveTargetID(PN532_MIFARE_ISO14443A) == 0) {
        // Successful start
        // Schedule to fetch the data
        Scheduler.setPluginTaskTimer(PN532_DELAY_READ_TAG_RESPONSE, event->TaskIndex, PN532_TIMER_TYPE_READ_TAG_RESPONSE);
      }

      // Schedule the next start read
      Scheduler.setPluginTaskTimer(PN532_INTERVAL_BETWEEN_READS, event->TaskIndex, PN532_TIMER_TYPE_START_READ_TAG);

      # ifdef P017_DEBUG_LOGIC_ANALYZER_PIN

      // DEBUG code using logic analyzer for timings
      DIRECT_pinWrite(P017_DEBUG_LOGIC_ANALYZER_PIN, 0);
      # endif // ifdef P017_DEBUG_LOGIC_ANALYZER_PIN

      break;
    }
    case PN532_TIMER_TYPE_READ_TAG_RESPONSE:
    {
              # ifdef P017_DEBUG_LOGIC_ANALYZER_PIN

      // DEBUG code using logic analyzer for timings
      DIRECT_pinWrite(P017_DEBUG_LOGIC_ANALYZER_PIN, 1);
        # endif // ifdef P017_DEBUG_LOGIC_ANALYZER_PIN

      uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
      uint8_t uidLength;
      uint8_t error = Plugin_017_readPassiveTargetID(uid, &uidLength);

      # ifdef P017_DEBUG_LOGIC_ANALYZER_PIN

      // DEBUG code using logic analyzer for timings
      DIRECT_pinWrite(P017_DEBUG_LOGIC_ANALYZER_PIN, 0);
      # endif // ifdef P017_DEBUG_LOGIC_ANALYZER_PIN


      if (error == 1)
      {
        errorCount++;

        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log = F("PN532: Read error: ");
          log += errorCount;
          addLogMove(LOG_LEVEL_ERROR, log);
        }
      }
      else {
        errorCount = 0;
      }

      if (errorCount > 2) // if three consecutive I2C errors, reset PN532
      {
        Plugin_017_Init(CONFIG_PIN3);
      }


      if (error == 0) {
# ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT

        // DEBUG code using logic analyzer for timings
        // Mark we read a card
        DIRECT_pinWrite(P017_DEBUG_LOGIC_ANALYZER_PIN_INIT, 1);
# endif // ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT

        unsigned long key = uid[0];

        for (uint8_t i = 1; i < 4; i++) {
          key <<= 8;
          key  += uid[i];
        }
        unsigned long old_key = UserVar.getSensorTypeLong(event->TaskIndex);
        bool new_key          = false;

        if (old_key != key) {
          UserVar.setSensorTypeLong(event->TaskIndex, key);
          new_key = true;
        }

        tempcounter++;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("PN532: ");

          if (new_key) {
            log += F("New Tag: ");
          } else {
            log += F("Old Tag: ");
          }
          log += key;
          log += ' ';
          log += tempcounter;
          addLogMove(LOG_LEVEL_INFO, log);
        }

        if (new_key) { sendData(event); }
        uint32_t resetTimer = P017_REMOVAL_TIMEOUT;

        if (resetTimer < 250) { resetTimer = 250; }
        Scheduler.setPluginTaskTimer(resetTimer, event->TaskIndex, PN532_TIMER_TYPE_REMOVE_TAG);
      } else {
# ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT

        // DEBUG code using logic analyzer for timings
        // Mark we no longer see the card
        DIRECT_pinWrite(P017_DEBUG_LOGIC_ANALYZER_PIN_INIT, 0);
# endif // ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT
      }
      break;
    }
  }

  return success;
}

/*********************************************************************************************\
* PN532 init
\*********************************************************************************************/
boolean Plugin_017_Init(int8_t resetPin)
{
# ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT

  // DEBUG code using logic analyzer for timings
  DIRECT_pinWrite(P017_DEBUG_LOGIC_ANALYZER_PIN_INIT, 1);
# endif // ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT

  if (validGpio(resetPin))
  {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("PN532: Reset on pin: ");
      log += resetPin;
      addLogMove(LOG_LEVEL_INFO, log);
    }
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, LOW);
    delay(100);
    digitalWrite(resetPin, HIGH);
    pinMode(resetPin, INPUT_PULLUP);
    delay(10);
  }

  Wire.beginTransmission(PN532_I2C_ADDRESS);
  Wire.endTransmission();
  delay(5);

  uint32_t versiondata = getFirmwareVersion();

  if (versiondata) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("PN532: Found chip PN5");
      log += formatToHex_no_prefix((versiondata >> 24) & 0xFF, 2);
      log += F(" FW: ");
      log += formatToHex_no_prefix((versiondata >> 16) & 0xFF, 2);
      log += '.';
      log += formatToHex_no_prefix((versiondata >> 8) & 0xFF, 2);
      addLogMove(LOG_LEVEL_INFO, log);
    }
  }
  else {
# ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT

    // DEBUG code using logic analyzer for timings
    DIRECT_pinWrite(P017_DEBUG_LOGIC_ANALYZER_PIN_INIT, 0);
# endif // ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT

    return false;
  }

  uint8_t Plugin_017_pn532_packetbuffer[64] = { 0 };

  Plugin_017_pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
  Plugin_017_pn532_packetbuffer[1] = 0x01; // normal mode;
  Plugin_017_pn532_packetbuffer[2] = 0x2;  // timeout 50ms * 2 = 100 mS
  Plugin_017_pn532_packetbuffer[3] = 0x01; // use IRQ pin!

  if (Plugin_017_writeCommand(Plugin_017_pn532_packetbuffer, 4)) {
# ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT

    // DEBUG code using logic analyzer for timings
    DIRECT_pinWrite(P017_DEBUG_LOGIC_ANALYZER_PIN_INIT, 0);
# endif // ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT

    return false;
  }

  // to prevent nack on next read
  Wire.beginTransmission(PN532_I2C_ADDRESS);
  Wire.endTransmission();
  delay(1);
# ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT

  // DEBUG code using logic analyzer for timings
  DIRECT_pinWrite(P017_DEBUG_LOGIC_ANALYZER_PIN_INIT, 0);
# endif // ifdef P017_DEBUG_LOGIC_ANALYZER_PIN_INIT

  return true;
}

/*********************************************************************************************\
* PN532 get firmware version
\*********************************************************************************************/
uint32_t getFirmwareVersion(void)
{
  uint32_t response;
  uint8_t  Plugin_017_pn532_packetbuffer[64] = { 0 };

  Plugin_017_pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

  if (Plugin_017_writeCommand(Plugin_017_pn532_packetbuffer, 1)) {
    return 0;
  }

  // read data packet
  int16_t status = Plugin_017_readResponse(
    PN532_COMMAND_GETFIRMWAREVERSION, 
    Plugin_017_pn532_packetbuffer, 
    sizeof(Plugin_017_pn532_packetbuffer));

  if (0 > status) {
    return 0;
  }

  response   = Plugin_017_pn532_packetbuffer[0];
  response <<= 8;
  response  |= Plugin_017_pn532_packetbuffer[1];
  response <<= 8;
  response  |= Plugin_017_pn532_packetbuffer[2];
  response <<= 8;
  response  |= Plugin_017_pn532_packetbuffer[3];

  return response;
}

void Plugin_017_powerDown(void)
{
  uint8_t Plugin_017_pn532_packetbuffer[64] = { 0 };

  Plugin_017_pn532_packetbuffer[0] = PN532_COMMAND_POWERDOWN;
  Plugin_017_pn532_packetbuffer[1] = 1 << 7; // allowed wakeup source is i2c

  if (Plugin_017_writeCommand(Plugin_017_pn532_packetbuffer, 2)) {
    return;
  }

  // read and ignore response
  Plugin_017_readResponse(
    PN532_COMMAND_POWERDOWN,
    Plugin_017_pn532_packetbuffer, 
    sizeof(Plugin_017_pn532_packetbuffer));
}

/*********************************************************************************************\
* PN532 read tag
\*********************************************************************************************/
uint8_t Plugin_017_StartReadPassiveTargetID(uint8_t cardbaudrate)
{
  uint8_t Plugin_017_pn532_packetbuffer[64] = { 0 };

  Plugin_017_pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
  Plugin_017_pn532_packetbuffer[1] = 1; // max 1 cards at once
  Plugin_017_pn532_packetbuffer[2] = cardbaudrate;

  if (Plugin_017_writeCommand(Plugin_017_pn532_packetbuffer, 3)) {
    return 0x1; // command failed
  }
  return 0;
}

uint8_t Plugin_017_readPassiveTargetID(uint8_t *uid, uint8_t *uidLength)
{
  uint8_t Plugin_017_pn532_packetbuffer[64] = { 0 };

  // read data packet
  const int16_t read_code = Plugin_017_readResponse(
    PN532_COMMAND_INLISTPASSIVETARGET,
    Plugin_017_pn532_packetbuffer, 
    sizeof(Plugin_017_pn532_packetbuffer));

  if (read_code < 0) {
    // if no tag read, need to clear something ?
    // it seems that without this code, the next read fails, taking another read to work again...

    // to prevent nack on next read
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    Wire.endTransmission();

    return 0x2;
  }

  if (Plugin_017_pn532_packetbuffer[0] != 1) {
    return 0x3;
  }

  uint16_t sens_res = Plugin_017_pn532_packetbuffer[2];

  sens_res <<= 8;
  sens_res  |= Plugin_017_pn532_packetbuffer[3];

  /* Card appears to be Mifare Classic */
  *uidLength = Plugin_017_pn532_packetbuffer[5];

  for (uint8_t i = 0; i < Plugin_017_pn532_packetbuffer[5]; i++) {
    uid[i] = Plugin_017_pn532_packetbuffer[6 + i];
  }

  // Plugin_017_Init(-1);
  //  Plugin_017_powerDown();


  return 0;
}

/*********************************************************************************************\
* PN532 write command
\*********************************************************************************************/
int8_t Plugin_017_writeCommand(const uint8_t *header, uint8_t hlen)
{
  Wire.beginTransmission(PN532_I2C_ADDRESS);

  Wire.write(PN532_PREAMBLE);
  Wire.write(PN532_STARTCODE1);
  Wire.write(PN532_STARTCODE2);

  uint8_t length = hlen + 1;       // length of data field: TFI + DATA

  Wire.write(length);
  Wire.write(~length + 1);         // checksum of length

  Wire.write(PN532_HOSTTOPN532);
  uint8_t sum = PN532_HOSTTOPN532; // sum of TFI + DATA

  for (uint8_t i = 0; i < hlen; i++) {
    if (Wire.write(header[i])) {
      sum += header[i];
    } else {
      return PN532_INVALID_FRAME;
    }
  }

  uint8_t checksum = ~sum + 1; // checksum of TFI + DATA

  Wire.write(checksum);
  Wire.write(PN532_POSTAMBLE);
  uint8_t status = Wire.endTransmission();

  if (status != 0) {
    return PN532_INVALID_FRAME;
  }

  return Plugin_017_readAckFrame();
}

/*********************************************************************************************\
* PN532 read response
\*********************************************************************************************/
int16_t Plugin_017_readResponse(uint8_t command, uint8_t buf[], uint8_t len)
{
  delay(1); // Need to wait a little as the PN53x may not 'see' the request if we send it too quickly.

  if (!Wire.requestFrom(PN532_I2C_ADDRESS, len + 2)) {
    return -1;
  }


  if (!(Wire.read() & 1)) {
    return -1;
  }

  if ((0x00 != Wire.read()) || // PREAMBLE
      (0x00 != Wire.read()) || // STARTCODE1
      (0xFF != Wire.read())    // STARTCODE2
      ) {
    return PN532_INVALID_FRAME;
  }

  uint8_t length = Wire.read();

  if (0 != (uint8_t)(length + Wire.read())) { // checksum of length
    return PN532_INVALID_FRAME;
  }

  uint8_t cmd = command + 1; // response command

  if ((PN532_PN532TOHOST != Wire.read()) || ((cmd) != Wire.read())) {
    return PN532_INVALID_FRAME;
  }

  length -= 2;

  if (length > len) {
    return PN532_NO_SPACE; // not enough space
  }

  uint8_t sum = PN532_PN532TOHOST + cmd;

  for (uint8_t i = 0; i < length; i++) {
    buf[i] = Wire.read();
    sum   += buf[i];
  }

  uint8_t checksum = Wire.read();

  if (0 != (uint8_t)(sum + checksum)) {
    return PN532_INVALID_FRAME;
  }
  Wire.read(); // POSTAMBLE

  return length;
}

/*********************************************************************************************\
* PN532 read ack
\*********************************************************************************************/
int8_t Plugin_017_readAckFrame()
{
  delay(1); // Need to wait a little as the PN53x may not 'see' the request if we send it too quickly.

  const uint8_t PN532_ACK[] = { 0, 0, 0xFF, 0, 0xFF, 0 };
  uint8_t ackBuf[sizeof(PN532_ACK)];

  uint16_t time = 0;

  do {
    if (Wire.requestFrom(PN532_I2C_ADDRESS,  sizeof(PN532_ACK) + 1)) {
      if (Wire.read() & 1) { // check first uint8_t --- status
        break;               // PN532 is ready
      }
    }

    delay(1);
    time++;

    if (time > PN532_ACK_WAIT_TIME) {
      return PN532_TIMEOUT;
    }
  } while (1);


  for (uint8_t i = 0; i < sizeof(PN532_ACK); i++) {
    ackBuf[i] = Wire.read();
  }

  if (memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK))) {
    return PN532_INVALID_ACK;
  }

  return 0;
}

#endif // USES_P017
