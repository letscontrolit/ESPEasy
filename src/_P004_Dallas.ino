#ifdef USES_P004

// #######################################################################################################
// #################################### Plugin 004: TempSensor Dallas DS18B20  ###########################
// #######################################################################################################

// Maxim Integrated (ex Dallas) DS18B20 datasheet : https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf

#if defined(ESP32)
  # define ESP32noInterrupts() { portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; portENTER_CRITICAL(&mux)
  # define ESP32interrupts() portEXIT_CRITICAL(&mux); }

// https://github.com/espressif/arduino-esp32/issues/1335
uint8_t Plugin_004_DS_read_bit(int8_t Plugin_004_DallasPin) ICACHE_RAM_ATTR;
void    Plugin_004_DS_write_bit(uint8_t v, int8_t  Plugin_004_DallasPin) ICACHE_RAM_ATTR;

#endif // if defined(ESP32)

#include "_Plugin_Helper.h"


#define PLUGIN_004
#define PLUGIN_ID_004         4
#define PLUGIN_NAME_004       "Environment - DS18b20"
#define PLUGIN_VALUENAME1_004 "Temperature"

#define P004_ERROR_NAN        0
#define P004_ERROR_MIN_RANGE  1
#define P004_ERROR_ZERO       2
#define P004_ERROR_MAX_RANGE  3
#define P004_ERROR_IGNORE     4

uint8_t Plugin_004_reset_time = 0;

static boolean Plugin_004_newValue[TASKS_MAX];
static unsigned long Plugin_004_timeoutGPIO[MAX_GPIO];

boolean Plugin_004(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_004;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_004);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_004));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_bidirectional(F("1-Wire"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNote(F("External pull up resistor is needed, see docs!"));

      uint8_t savedAddress[8];
      byte    resolutionChoice = 0;

      // Scan the onewire bus and fill dropdown list with devicecount on this GPIO.
      int8_t Plugin_004_DallasPin = CONFIG_PIN1;

      if (Plugin_004_DallasPin != -1) {
        // get currently saved address
        for (byte i = 0; i < 8; i++) {
          savedAddress[i] = ExtraTaskSettings.TaskDevicePluginConfigLong[i];
        }

        // find all suitable devices
        addRowLabel(F("Device Address"));
        addSelector_Head(F("p004_dev"));
        addSelector_Item("", -1, false, false, "");
        uint8_t tmpAddress[8];
        byte    count = 0;
        Plugin_004_DS_reset(Plugin_004_DallasPin);
        Plugin_004_DS_reset_search();

        while (Plugin_004_DS_search(tmpAddress, Plugin_004_DallasPin))
        {
          String option = "";

          for (byte j = 0; j < 8; j++)
          {
            option += String(tmpAddress[j], HEX);

            if (j < 7) { option += '-'; }
          }
          bool selected = (memcmp(tmpAddress, savedAddress, 8) == 0) ? true : false;
          addSelector_Item(option, count, selected, false, "");
          count++;
        }
        addSelector_Foot();

        {
          // Device Resolution select
          if (ExtraTaskSettings.TaskDevicePluginConfigLong[0] != 0) {
            resolutionChoice = Plugin_004_DS_getResolution(savedAddress, Plugin_004_DallasPin);
          }
          else {
            resolutionChoice = 9;
          }
          String resultsOptions[4]      = { F("9"), F("10"), F("11"), F("12") };
          int    resultsOptionValues[4] = { 9, 10, 11, 12 };
          addFormSelector(F("Device Resolution"), F("p004_res"), 4, resultsOptions, resultsOptionValues, resolutionChoice);
          addHtml(F(" Bit"));
        }

        {
          // Value in case of Error
          String resultsOptions[5]      = { F("NaN"), F("-127"), F("0"), F("125"), F("Ignore") };
          int    resultsOptionValues[5] = { P004_ERROR_NAN, P004_ERROR_MIN_RANGE, P004_ERROR_ZERO, P004_ERROR_MAX_RANGE, P004_ERROR_IGNORE };
          addFormSelector(F("Error State Value"), F("p004_err"), 5, resultsOptions, resultsOptionValues, PCONFIG(0));
        }
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      uint8_t addr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

      // save the address for selected device and store into extra tasksettings
      int8_t Plugin_004_DallasPin = CONFIG_PIN1;

      if (Plugin_004_DallasPin != -1) {
        Plugin_004_DS_scan(getFormItemInt(F("p004_dev")), addr, Plugin_004_DallasPin);

        for (byte x = 0; x < 8; x++) {
          ExtraTaskSettings.TaskDevicePluginConfigLong[x] = addr[x];
        }

        Plugin_004_DS_setResolution(addr, getFormItemInt(F("p004_res")), Plugin_004_DallasPin);
        Plugin_004_DS_startConversion(addr, Plugin_004_DallasPin);
      }
      PCONFIG(0) = getFormItemInt(F("p004_err"));
      success    = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      for (byte x = 0; x < 8; x++)
      {
        if (x != 0) {
          string += '-';
        }
        string += String(ExtraTaskSettings.TaskDevicePluginConfigLong[x], HEX);
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      int8_t Plugin_004_DallasPin = CONFIG_PIN1;

      if (Plugin_004_DallasPin != -1) {
        Plugin_004_timeoutGPIO[Plugin_004_DallasPin] = millis();
        Plugin_004_newValue[event->TaskIndex]        = false;
      }
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      if (ExtraTaskSettings.TaskDevicePluginConfigLong[0] != 0) {
        uint8_t addr[8];
        Plugin_004_get_addr(addr, event->TaskIndex);

        int8_t Plugin_004_DallasPin = CONFIG_PIN1;

        if (Plugin_004_DallasPin != -1) {
          if (!timeOutReached(Plugin_004_timeoutGPIO[Plugin_004_DallasPin])){
              Scheduler.schedule_task_device_timer(event->TaskIndex, Plugin_004_timeoutGPIO[Plugin_004_DallasPin]);
          } else {
            if (!Plugin_004_newValue[event->TaskIndex]) {
              Plugin_004_DS_startConversion(addr, Plugin_004_DallasPin);
              Scheduler.schedule_task_device_timer(event->TaskIndex, Plugin_004_timeoutGPIO[Plugin_004_DallasPin]);
              Plugin_004_newValue[event->TaskIndex] = true;
            } else {
              Plugin_004_newValue[event->TaskIndex] = false;
              float  value = 0;
              String log   = F("DS   : Temperature: ");

              if (Plugin_004_DS_readTemp(addr, &value, Plugin_004_DallasPin))
              {
                UserVar[event->BaseVarIndex] = value;
                log                         += UserVar[event->BaseVarIndex];
                success                      = true;
              }
              else
              {
                if (PCONFIG(0) != P004_ERROR_IGNORE) {
                  float errorValue = NAN;

                  switch (PCONFIG(0)) {
                    case P004_ERROR_MIN_RANGE: errorValue = -127; break;
                    case P004_ERROR_ZERO:      errorValue = 0; break;
                    case P004_ERROR_MAX_RANGE: errorValue = 125; break;
                    default:
                      break;
                  }
                  UserVar[event->BaseVarIndex] = errorValue;
                }
                log += F("Error!");
              }

              log += F(" (");

              for (byte x = 0; x < 8; x++)
              {
                if (x != 0) {
                  log += '-';
                }
                log += String(ExtraTaskSettings.TaskDevicePluginConfigLong[x], HEX);
              }

              log += ')';
              addLog(LOG_LEVEL_INFO, log);
            }
          }
        }
      }
      break;
    }
  }
  return success;
}

void Plugin_004_get_addr(uint8_t addr[], taskIndex_t TaskIndex)
{
  // Load ROM address from tasksettings
  LoadTaskSettings(TaskIndex);

  for (byte x = 0; x < 8; x++) {
    addr[x] = ExtraTaskSettings.TaskDevicePluginConfigLong[x];
  }
}

void Plugin_004_set_timeout(int res, int8_t Plugin_004_DallasPin)
{
  if ((res < 9) || (res > 12)) { res = 12; }
  Plugin_004_timeoutGPIO[Plugin_004_DallasPin] = millis() + (800 / (1 << (12 - res)));
}

/*********************************************************************************************\
   Dallas Scan bus
\*********************************************************************************************/
byte Plugin_004_DS_scan(byte getDeviceROM, uint8_t *ROM, int8_t Plugin_004_DallasPin)
{
  byte tmpaddr[8];
  byte devCount = 0;

  Plugin_004_DS_reset(Plugin_004_DallasPin);

  Plugin_004_DS_reset_search();

  while (Plugin_004_DS_search(tmpaddr, Plugin_004_DallasPin))
  {
    if (getDeviceROM == devCount) {
      for (byte  i = 0; i < 8; i++) {
        ROM[i] = tmpaddr[i];
      }
    }
    devCount++;
  }
  return devCount;
}

// read power supply
bool Plugin_004_DS_is_parasite(uint8_t ROM[8], int8_t Plugin_004_DallasPin)
{
  Plugin_004_DS_address_ROM(ROM, Plugin_004_DallasPin);
  Plugin_004_DS_write(0xB4, Plugin_004_DallasPin); // read power supply
  return !Plugin_004_DS_read_bit(Plugin_004_DallasPin);
}

/*********************************************************************************************\
*  Dallas Start Temperature Conversion, expected max duration:
*    9 bits resolution ->  93.75 ms
*   10 bits resolution -> 187.5 ms
*   11 bits resolution -> 375 ms
*   12 bits resolution -> 750 ms
\*********************************************************************************************/
void Plugin_004_DS_startConversion(uint8_t ROM[8], int8_t Plugin_004_DallasPin)
{
  Plugin_004_set_timeout(Plugin_004_DS_getResolution(ROM, Plugin_004_DallasPin), Plugin_004_DallasPin);
  Plugin_004_DS_address_ROM(ROM, Plugin_004_DallasPin);
  Plugin_004_DS_write(0x44, Plugin_004_DallasPin); // Take temperature mesurement
}

/*********************************************************************************************\
*  Dallas Read temperature from scratchpad
\*********************************************************************************************/
boolean Plugin_004_DS_readTemp(uint8_t ROM[8], float *value, int8_t Plugin_004_DallasPin)
{
  int16_t DSTemp;
  byte    ScratchPad[12];

  Plugin_004_DS_address_ROM(ROM, Plugin_004_DallasPin);
  Plugin_004_DS_write(0xBE, Plugin_004_DallasPin); // Read scratchpad

  for (byte i = 0; i < 9; i++) {                   // read 9 bytes
    ScratchPad[i] = Plugin_004_DS_read(Plugin_004_DallasPin);
  }

  bool crc_ok = Plugin_004_DS_crc8(ScratchPad);

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("DS: SP: ");

    for (byte x = 0; x < 9; x++)
    {
      if (x != 0) {
        log += ',';
      }
      log += String(ScratchPad[x], HEX);
    }

    if (crc_ok) {
      log += F(",OK");
    }

    if (Plugin_004_DS_is_parasite(ROM, Plugin_004_DallasPin)) {
      log += F(",P");
    }
    log += ',';
    log += String(Plugin_004_reset_time, DEC);
    addLog(LOG_LEVEL_DEBUG, log);
  }

  if (!crc_ok)
  {
    *value = 0;
    return false;
  }

  if ((ROM[0] == 0x28) || (ROM[0] == 0x3b) || (ROM[0] == 0x22)) // DS18B20 or DS1825 or DS1822
  {
    DSTemp = (ScratchPad[1] << 8) + ScratchPad[0];

    if (DSTemp == 0x550) { // power-on reset value
      return false;
    }
    *value = (float(DSTemp) * 0.0625);
  }
  else if (ROM[0] == 0x10)       // DS1820 DS18S20
  {
    if (ScratchPad[0] == 0xaa) { // power-on reset value
      return false;
    }
    DSTemp = (ScratchPad[1] << 11) | ScratchPad[0] << 3;
    DSTemp = ((DSTemp & 0xfff0) << 3) - 16 +
             (((ScratchPad[7] - ScratchPad[6]) << 7) / ScratchPad[7]);
    *value = float(DSTemp) * 0.0078125;
  }
  return true;
}

/*********************************************************************************************\
* Dallas Get Resolution
\*********************************************************************************************/
byte Plugin_004_DS_getResolution(uint8_t ROM[8], int8_t Plugin_004_DallasPin)
{
  // DS1820 and DS18S20 have no resolution configuration register
  if (ROM[0] == 0x10) { return 12; }

  byte ScratchPad[12];
  Plugin_004_DS_address_ROM(ROM, Plugin_004_DallasPin);
  Plugin_004_DS_write(0xBE, Plugin_004_DallasPin); // Read scratchpad

  for (byte i = 0; i < 9; i++) {                   // read 9 bytes
    ScratchPad[i] = Plugin_004_DS_read(Plugin_004_DallasPin);
  }

  if (!Plugin_004_DS_crc8(ScratchPad)) {
    return 0;
  }
  else
  {
    switch (ScratchPad[4])
    {
      case 0x7F: // 12 bit
        return 12;

      case 0x5F: // 11 bit
        return 11;

      case 0x3F: // 10 bit
        return 10;

      case 0x1F: //  9 bit
      default:
        return 9;
    }
  }
  return 0;
}

/*********************************************************************************************\
* Dallas Get Resolution
\*********************************************************************************************/
boolean Plugin_004_DS_setResolution(uint8_t ROM[8], byte res, int8_t Plugin_004_DallasPin)
{
  // DS1820 and DS18S20 have no resolution configuration register
  if (ROM[0] == 0x10) { return true; }

  byte ScratchPad[12];
  Plugin_004_DS_address_ROM(ROM, Plugin_004_DallasPin);
  Plugin_004_DS_write(0xBE, Plugin_004_DallasPin); // Read scratchpad

  for (byte i = 0; i < 9; i++) {                   // read 9 bytes
    ScratchPad[i] = Plugin_004_DS_read(Plugin_004_DallasPin);
  }

  if (!Plugin_004_DS_crc8(ScratchPad)) {
    return false;
  }
  else
  {
    byte old_configuration = ScratchPad[4];

    switch (res)
    {
      case 12:
        ScratchPad[4] = 0x7F; // 12 bits
        break;
      case 11:
        ScratchPad[4] = 0x5F; // 11 bits
        break;
      case 10:
        ScratchPad[4] = 0x3F; // 10 bits
        break;
      case 9:
      default:
        ScratchPad[4] = 0x1F; //  9 bits
        break;
    }

    if (ScratchPad[4] == old_configuration) {
      return true;
    }

    Plugin_004_DS_address_ROM(ROM, Plugin_004_DallasPin);
    Plugin_004_DS_write(0x4E,          Plugin_004_DallasPin); // Write to EEPROM
    Plugin_004_DS_write(ScratchPad[2], Plugin_004_DallasPin); // high alarm temp
    Plugin_004_DS_write(ScratchPad[3], Plugin_004_DallasPin); // low alarm temp
    Plugin_004_DS_write(ScratchPad[4], Plugin_004_DallasPin); // configuration register

    Plugin_004_DS_address_ROM(ROM, Plugin_004_DallasPin);

    // save the newly written values to eeprom
    Plugin_004_DS_write(0x48, Plugin_004_DallasPin);
    delay(100);  // <--- added 20ms delay to allow 10ms long EEPROM write operation (as specified by datasheet)
    Plugin_004_DS_reset(Plugin_004_DallasPin);

    return true; // new value set
  }
}

/*********************************************************************************************\
*  Dallas Reset
\*********************************************************************************************/
uint8_t Plugin_004_DS_reset(int8_t Plugin_004_DallasPin)
{
  uint8_t r       = 0;
  uint8_t retries = 125;

    #if defined(ESP32)
  ESP32noInterrupts();
    #endif // if defined(ESP32)
  pinMode(Plugin_004_DallasPin, INPUT);
  bool success = true;

  do // wait until the wire is high... just in case
  {
    if (--retries == 0) {
      success = false;
    }
    delayMicroseconds(2);
  }
  while (!digitalRead(Plugin_004_DallasPin) && success);

  if (success) {
    digitalWrite(Plugin_004_DallasPin, LOW);
    pinMode(Plugin_004_DallasPin, OUTPUT);
    delayMicroseconds(500);
    pinMode(Plugin_004_DallasPin, INPUT); // Float

    for (uint8_t i = 0; i < 45; i++)      // 480us RX minimum
    {
      delayMicroseconds(15);

      if (!digitalRead(Plugin_004_DallasPin)) {
        r                     = 1;
        Plugin_004_reset_time = i;
      }
    }
  }
    #if defined(ESP32)
  ESP32interrupts();
    #endif // if defined(ESP32)
  return r;
}

#define FALSE 0
#define TRUE  1

unsigned char ROM_NO[8];
uint8_t LastDiscrepancy;
uint8_t LastFamilyDiscrepancy;
uint8_t LastDeviceFlag;


/*********************************************************************************************\
*  Dallas Reset Search
\*********************************************************************************************/
void Plugin_004_DS_reset_search()
{
  // reset the search state
  LastDiscrepancy       = 0;
  LastDeviceFlag        = FALSE;
  LastFamilyDiscrepancy = 0;

  for (byte i = 0; i < 8; i++) {
    ROM_NO[i] = 0;
  }
}

/*********************************************************************************************\
*  Dallas Search bus
\*********************************************************************************************/
uint8_t Plugin_004_DS_search(uint8_t *newAddr, int8_t Plugin_004_DallasPin)
{
  uint8_t id_bit_number;
  uint8_t last_zero, rom_byte_number, search_result;
  uint8_t id_bit, cmp_id_bit;
  unsigned char rom_byte_mask, search_direction;

  // initialize for search
  id_bit_number   = 1;
  last_zero       = 0;
  rom_byte_number = 0;
  rom_byte_mask   = 1;
  search_result   = 0;

  // if the last call was not the last one
  if (!LastDeviceFlag)
  {
    // 1-Wire reset
    if (!Plugin_004_DS_reset(Plugin_004_DallasPin))
    {
      // reset the search
      LastDiscrepancy       = 0;
      LastDeviceFlag        = FALSE;
      LastFamilyDiscrepancy = 0;
      return FALSE;
    }

    // issue the search command
    Plugin_004_DS_write(0xF0, Plugin_004_DallasPin);

    // loop to do the search
    do
    {
      // read a bit and its complement
      id_bit     = Plugin_004_DS_read_bit(Plugin_004_DallasPin);
      cmp_id_bit = Plugin_004_DS_read_bit(Plugin_004_DallasPin);

      // check for no devices on 1-wire
      if ((id_bit == 1) && (cmp_id_bit == 1)) {
        break;
      }
      else
      {
        // all devices coupled have 0 or 1
        if (id_bit != cmp_id_bit) {
          search_direction = id_bit; // bit write value for search
        }
        else
        {
          // if this discrepancy if before the Last Discrepancy
          // on a previous next then pick the same as last time
          if (id_bit_number < LastDiscrepancy) {
            search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
          }
          else {
            // if equal to last pick 1, if not then pick 0
            search_direction = (id_bit_number == LastDiscrepancy);
          }

          // if 0 was picked then record its position in LastZero
          if (search_direction == 0)
          {
            last_zero = id_bit_number;

            // check for Last discrepancy in family
            if (last_zero < 9) {
              LastFamilyDiscrepancy = last_zero;
            }
          }
        }

        // set or clear the bit in the ROM byte rom_byte_number
        // with mask rom_byte_mask
        if (search_direction == 1) {
          ROM_NO[rom_byte_number] |= rom_byte_mask;
        }
        else {
          ROM_NO[rom_byte_number] &= ~rom_byte_mask;
        }

        // serial number search direction write bit
        Plugin_004_DS_write_bit(search_direction, Plugin_004_DallasPin);

        // increment the byte counter id_bit_number
        // and shift the mask rom_byte_mask
        id_bit_number++;
        rom_byte_mask <<= 1;

        // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
        if (rom_byte_mask == 0)
        {
          rom_byte_number++;
          rom_byte_mask = 1;
        }
      }
    }
    while (rom_byte_number < 8); // loop until through all ROM bytes 0-7

    // if the search was successful then
    if (!(id_bit_number < 65))
    {
      // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
      LastDiscrepancy = last_zero;

      // check for last device
      if (LastDiscrepancy == 0) {
        LastDeviceFlag = TRUE;
      }

      search_result = TRUE;
    }
  }

  // if no device found then reset counters so next 'search' will be like a first
  if (!search_result || !ROM_NO[0])
  {
    LastDiscrepancy       = 0;
    LastDeviceFlag        = FALSE;
    LastFamilyDiscrepancy = 0;
    search_result         = FALSE;
  }

  for (int i = 0; i < 8; i++) {
    newAddr[i] = ROM_NO[i];
  }

  return search_result;
}

/*********************************************************************************************\
*  Dallas Read byte
\*********************************************************************************************/
uint8_t Plugin_004_DS_read(int8_t Plugin_004_DallasPin)
{
  uint8_t bitMask;
  uint8_t r = 0;

  for (bitMask = 0x01; bitMask; bitMask <<= 1) {
    if (Plugin_004_DS_read_bit(Plugin_004_DallasPin)) {
      r |= bitMask;
    }
  }

  return r;
}

/*********************************************************************************************\
*  Dallas Write byte
\*********************************************************************************************/
void Plugin_004_DS_write(uint8_t ByteToWrite, int8_t Plugin_004_DallasPin)
{
  uint8_t bitMask;

  for (bitMask = 0x01; bitMask; bitMask <<= 1) {
    Plugin_004_DS_write_bit((bitMask & ByteToWrite) ? 1 : 0, Plugin_004_DallasPin);
  }
}

/*********************************************************************************************\
*  Dallas Read bit
\*********************************************************************************************/
uint8_t Plugin_004_DS_read_bit(int8_t Plugin_004_DallasPin)
{
  if (Plugin_004_DallasPin == -1) { return 0; }
  uint8_t r;

    #if defined(ESP32)
  ESP32noInterrupts();
    #endif // if defined(ESP32)
  digitalWrite(Plugin_004_DallasPin, LOW);
  pinMode(Plugin_004_DallasPin, OUTPUT);
  delayMicroseconds(2);
  pinMode(Plugin_004_DallasPin, INPUT); // let pin float, pull up will raise
  delayMicroseconds(8);
  r = digitalRead(Plugin_004_DallasPin);
    #if defined(ESP32)
  ESP32interrupts();
    #endif // if defined(ESP32)
  delayMicroseconds(60);
  return r;
}

/*********************************************************************************************\
*  Dallas Write bit
\*********************************************************************************************/
void Plugin_004_DS_write_bit(uint8_t v, int8_t Plugin_004_DallasPin)
{
  if (Plugin_004_DallasPin == -1) { return; }

  if (v & 1)
  {
        #if defined(ESP32)
    ESP32noInterrupts();
        #endif // if defined(ESP32)
    digitalWrite(Plugin_004_DallasPin, LOW);
    pinMode(Plugin_004_DallasPin, OUTPUT);
    delayMicroseconds(2);
    digitalWrite(Plugin_004_DallasPin, HIGH);
        #if defined(ESP32)
    ESP32interrupts();
        #endif // if defined(ESP32)
    delayMicroseconds(70);
  }
  else
  {
        #if defined(ESP32)
    ESP32noInterrupts();
        #endif // if defined(ESP32)
    digitalWrite(Plugin_004_DallasPin, LOW);
    pinMode(Plugin_004_DallasPin, OUTPUT);
    delayMicroseconds(90);
    digitalWrite(Plugin_004_DallasPin, HIGH);
        #if defined(ESP32)
    ESP32interrupts();
        #endif // if defined(ESP32)
    delayMicroseconds(10);
  }
}

/*********************************************************************************************\
*  Standard function to initiate addressing a sensor.
\*********************************************************************************************/
void Plugin_004_DS_address_ROM(uint8_t ROM[8], int8_t Plugin_004_DallasPin)
{
  Plugin_004_DS_reset(Plugin_004_DallasPin);
  Plugin_004_DS_write(0x55, Plugin_004_DallasPin); // Choose ROM

  for (byte i = 0; i < 8; i++) {
    Plugin_004_DS_write(ROM[i], Plugin_004_DallasPin);
  }
}

/*********************************************************************************************\
*  Dallas Calculate CRC8 and compare it of addr[0-7] and compares it to addr[8]
\*********************************************************************************************/
boolean Plugin_004_DS_crc8(uint8_t *addr)
{
  uint8_t crc = 0;
  uint8_t len = 8;

  while (len--)
  {
    uint8_t inbyte = *addr++; // from 0 to 7

    for (uint8_t i = 8; i; i--)
    {
      uint8_t mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;

      if (mix) { crc ^= 0x8C; }
      inbyte >>= 1;
    }
  }
  return crc == *addr; // addr 8
}

#if defined(ESP32)
  # undef ESP32noInterrupts
  # undef ESP32interrupts
#endif // if defined(ESP32)

#endif // USES_P004
