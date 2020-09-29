#ifdef USES_P100

// #######################################################################################################
// #################################### Plugin 100: Counter Dallas DS2423  ###############################
// #######################################################################################################

// Maxim Integrated (ex Dallas) DS2423 datasheet : https://datasheets.maximintegrated.com/en/ds/DS2423.pdf

#if defined(ESP32)
  # define ESP32noInterrupts() { portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; portENTER_CRITICAL(&mux)
  # define ESP32interrupts() portEXIT_CRITICAL(&mux); }

// https://github.com/espressif/arduino-esp32/issues/1335
uint8_t Plugin_100_DS_read_bit(int8_t Plugin_100_DallasPin) ICACHE_RAM_ATTR;
void    Plugin_100_DS_write_bit(uint8_t v, int8_t  Plugin_100_DallasPin) ICACHE_RAM_ATTR;

#endif // if defined(ESP32)

#include "_Plugin_Helper.h"


#define PLUGIN_100
#define PLUGIN_ID_100         100
#define PLUGIN_NAME_100       "Pulse Counter - DS2423 [TESTING]"
#define PLUGIN_VALUENAME1_100 "CountDelta"

boolean Plugin_100(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_100;
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
      string = F(PLUGIN_NAME_100);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_100));
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

      // Scan the onewire bus and fill dropdown list with devicecount on this GPIO.
      int8_t Plugin_100_DallasPin = CONFIG_PIN1;

      if (Plugin_100_DallasPin != -1) {
        // get currently saved address
        for (byte i = 0; i < 8; i++) {
          savedAddress[i] = ExtraTaskSettings.TaskDevicePluginConfigLong[i];
        }

        // find all suitable devices
        addRowLabel(F("Device Address"));
        addSelector_Head(F("p100_dev"));
        addSelector_Item("", -1, false, false, "");
        uint8_t tmpAddress[8];
        byte    count = 0;
        Plugin_100_DS_reset(Plugin_100_DallasPin);
        Plugin_100_DS_reset_search();

        while (Plugin_100_DS_search(tmpAddress, Plugin_100_DallasPin))
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

        // Counter select
        String resultsOptions[2]      = { F("A"), F("B") };
        int    resultsOptionValues[2] = { 0, 1 };
        addFormSelector(F("Counter"), F("p100_counter"), 2, resultsOptions, resultsOptionValues, PCONFIG(0));
        addFormNote(F("Counter value is incremental"));
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      uint8_t addr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

      // 1-wire GPIO
      int8_t Plugin_100_DallasPin = CONFIG_PIN1;

      // Counter choice
      PCONFIG(0) = getFormItemInt(F("p100_counter"));

      // 1-wire device address
      if (Plugin_100_DallasPin != -1) {
        Plugin_100_DS_scan(getFormItemInt(F("p100_dev")), addr, Plugin_100_DallasPin);

        for (byte x = 0; x < 8; x++) {
          ExtraTaskSettings.TaskDevicePluginConfigLong[x] = addr[x];
        }
      }

      success = true;
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
      UserVar[event->BaseVarIndex] = 0;
      UserVar[event->BaseVarIndex + 1] = 0;
      UserVar[event->BaseVarIndex + 2] = 0;

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      if (ExtraTaskSettings.TaskDevicePluginConfigLong[0] != 0) {
        uint8_t addr[8];

        LoadTaskSettings(event->TaskIndex);

        for (byte x = 0; x < 8; x++) {
          addr[x] = ExtraTaskSettings.TaskDevicePluginConfigLong[x];
        }

        if (CONFIG_PIN1 != -1) {
          float value = 0;
          String log = F("[P100]DS   : Counter ");
          log += PCONFIG(0) == 0 ? F("A") : F("B");
          log += F(": ");

          if (Plugin_100_DS_readCounter(addr, &value, CONFIG_PIN1, PCONFIG(0)))
          {
            UserVar[event->BaseVarIndex] = UserVar[event->BaseVarIndex + 2] != 0
              ? value - UserVar[event->BaseVarIndex + 1]
              : 0;
            UserVar[event->BaseVarIndex + 2] = 1;
            UserVar[event->BaseVarIndex + 1] = value;
            log += UserVar[event->BaseVarIndex];
            success = true;
          }
          else
          {
            UserVar[event->BaseVarIndex] = NAN;
            log += F("Error!");
          }

          log += F(" (");

          for (byte x = 0; x < 8; x++)
          {
            if (x != 0) {
              log += '-';
            }
            log += (addr[x] < 0x10 ? "0" : "") + String(addr[x], HEX);
          }

          log += ')';
          addLog(LOG_LEVEL_INFO, log);
        }
      }
      break;
    }
  }
  return success;
}

/*********************************************************************************************\
   Dallas Scan bus
\*********************************************************************************************/
byte Plugin_100_DS_scan(byte getDeviceROM, uint8_t *ROM, int8_t Plugin_100_DallasPin)
{
  byte tmpaddr[8];
  byte devCount = 0;

  Plugin_100_DS_reset(Plugin_100_DallasPin);

  Plugin_100_DS_reset_search();

  while (Plugin_100_DS_search(tmpaddr, Plugin_100_DallasPin))
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

/*********************************************************************************************\
   Dallas read DS2423 counter
   Taken from https://github.com/jbechter/arduino-onewire-DS2423
\*********************************************************************************************/
#define DS2423_READ_MEMORY_COMMAND 0xa5
#define DS2423_PAGE_ONE 0xc0
#define DS2423_PAGE_TWO 0xe0

bool Plugin_100_DS_readCounter(uint8_t ROM[8], float *value, int8_t Plugin_100_DallasPin, uint8_t counter)
{
  uint8_t data[45];

  data[0] = DS2423_READ_MEMORY_COMMAND;
  data[1] = (counter == 0 ? DS2423_PAGE_ONE : DS2423_PAGE_TWO);
  data[2] = 0x01;

  Plugin_100_DS_reset(Plugin_100_DallasPin);
  Plugin_100_DS_address_ROM(ROM, Plugin_100_DallasPin);

  Plugin_100_DS_write(data[0], Plugin_100_DallasPin);
  Plugin_100_DS_write(data[1], Plugin_100_DallasPin);
  Plugin_100_DS_write(data[2], Plugin_100_DallasPin);

  for (int j = 3; j < 45; j++) {
    data[j] = Plugin_100_DS_read(Plugin_100_DallasPin);
  }

  Plugin_100_DS_reset(Plugin_100_DallasPin);

  uint32_t count = (uint32_t)data[38];
  for (int j = 37; j >= 35; j--) {
    count = (count << 8) + (uint32_t)data[j];
  }

  uint16_t crc = Plugin_100_DS_crc16(data, 43, 0);
  uint8_t *crcBytes = (uint8_t *)&crc;
  uint8_t crcLo = ~data[43];
  uint8_t crcHi = ~data[44];
  boolean error = (crcLo != crcBytes[0]) || (crcHi != crcBytes[1]);

  if (!error)
  {
    *value = count;
    return true;
  }
  else
  {
    *value = 0;
    return false;
  }
}

/*********************************************************************************************\
*  Dallas Reset
\*********************************************************************************************/
uint8_t Plugin_100_DS_reset(int8_t Plugin_100_DallasPin)
{
  uint8_t r       = 0;
  uint8_t retries = 125;

    #if defined(ESP32)
  ESP32noInterrupts();
    #endif // if defined(ESP32)
  pinMode(Plugin_100_DallasPin, INPUT);
  bool success = true;

  do // wait until the wire is high... just in case
  {
    if (--retries == 0) {
      success = false;
    }
    delayMicroseconds(2);
  }
  while (!digitalRead(Plugin_100_DallasPin) && success);

  if (success) {
    digitalWrite(Plugin_100_DallasPin, LOW);
    pinMode(Plugin_100_DallasPin, OUTPUT);
    delayMicroseconds(500);
    pinMode(Plugin_100_DallasPin, INPUT); // Float

    for (uint8_t i = 0; i < 45; i++)      // 480us RX minimum
    {
      delayMicroseconds(15);

      if (!digitalRead(Plugin_100_DallasPin)) {
        r                     = 1;
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

unsigned char P100_ROM_NO[8];
uint8_t P100_LastDiscrepancy;
uint8_t P100_LastFamilyDiscrepancy;
uint8_t P100_LastDeviceFlag;

/*********************************************************************************************\
*  Dallas Reset Search
\*********************************************************************************************/
void Plugin_100_DS_reset_search()
{
  // reset the search state
  P100_LastDiscrepancy       = 0;
  P100_LastDeviceFlag        = FALSE;
  P100_LastFamilyDiscrepancy = 0;

  for (byte i = 0; i < 8; i++) {
    P100_ROM_NO[i] = 0;
  }
}

/*********************************************************************************************\
*  Dallas Search bus
\*********************************************************************************************/
uint8_t Plugin_100_DS_search(uint8_t *newAddr, int8_t Plugin_100_DallasPin)
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
  if (!P100_LastDeviceFlag)
  {
    // 1-Wire reset
    if (!Plugin_100_DS_reset(Plugin_100_DallasPin))
    {
      // reset the search
      P100_LastDiscrepancy       = 0;
      P100_LastDeviceFlag        = FALSE;
      P100_LastFamilyDiscrepancy = 0;
      return FALSE;
    }

    // issue the search command
    Plugin_100_DS_write(0xF0, Plugin_100_DallasPin);

    // loop to do the search
    do
    {
      // read a bit and its complement
      id_bit     = Plugin_100_DS_read_bit(Plugin_100_DallasPin);
      cmp_id_bit = Plugin_100_DS_read_bit(Plugin_100_DallasPin);

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
          if (id_bit_number < P100_LastDiscrepancy) {
            search_direction = ((P100_ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
          }
          else {
            // if equal to last pick 1, if not then pick 0
            search_direction = (id_bit_number == P100_LastDiscrepancy);
          }

          // if 0 was picked then record its position in LastZero
          if (search_direction == 0)
          {
            last_zero = id_bit_number;

            // check for Last discrepancy in family
            if (last_zero < 9) {
              P100_LastFamilyDiscrepancy = last_zero;
            }
          }
        }

        // set or clear the bit in the ROM byte rom_byte_number
        // with mask rom_byte_mask
        if (search_direction == 1) {
          P100_ROM_NO[rom_byte_number] |= rom_byte_mask;
        }
        else {
          P100_ROM_NO[rom_byte_number] &= ~rom_byte_mask;
        }

        // serial number search direction write bit
        Plugin_100_DS_write_bit(search_direction, Plugin_100_DallasPin);

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
      // search successful so set P100_LastDiscrepancy,P100_LastDeviceFlag,search_result
      P100_LastDiscrepancy = last_zero;

      // check for last device
      if (P100_LastDiscrepancy == 0) {
        P100_LastDeviceFlag = TRUE;
      }

      search_result = TRUE;
    }
  }

  // if no device found then reset counters so next 'search' will be like a first
  if (!search_result || !P100_ROM_NO[0])
  {
    P100_LastDiscrepancy       = 0;
    P100_LastDeviceFlag        = FALSE;
    P100_LastFamilyDiscrepancy = 0;
    search_result         = FALSE;
  }

  for (int i = 0; i < 8; i++) {
    newAddr[i] = P100_ROM_NO[i];
  }

  return search_result;
}

/*********************************************************************************************\
*  Dallas Read byte
\*********************************************************************************************/
uint8_t Plugin_100_DS_read(int8_t Plugin_100_DallasPin)
{
  uint8_t bitMask;
  uint8_t r = 0;

  for (bitMask = 0x01; bitMask; bitMask <<= 1) {
    if (Plugin_100_DS_read_bit(Plugin_100_DallasPin)) {
      r |= bitMask;
    }
  }

  return r;
}

/*********************************************************************************************\
*  Dallas Write byte
\*********************************************************************************************/
void Plugin_100_DS_write(uint8_t ByteToWrite, int8_t Plugin_100_DallasPin)
{
  uint8_t bitMask;

  for (bitMask = 0x01; bitMask; bitMask <<= 1) {
    Plugin_100_DS_write_bit((bitMask & ByteToWrite) ? 1 : 0, Plugin_100_DallasPin);
  }
}

/*********************************************************************************************\
*  Dallas Read bit
\*********************************************************************************************/
uint8_t Plugin_100_DS_read_bit(int8_t Plugin_100_DallasPin)
{
  if (Plugin_100_DallasPin == -1) { return 0; }
  uint8_t r;

    #if defined(ESP32)
  ESP32noInterrupts();
    #endif // if defined(ESP32)
  digitalWrite(Plugin_100_DallasPin, LOW);
  pinMode(Plugin_100_DallasPin, OUTPUT);
  delayMicroseconds(2);
  pinMode(Plugin_100_DallasPin, INPUT); // let pin float, pull up will raise
  delayMicroseconds(8);
  r = digitalRead(Plugin_100_DallasPin);
    #if defined(ESP32)
  ESP32interrupts();
    #endif // if defined(ESP32)
  delayMicroseconds(60);
  return r;
}

/*********************************************************************************************\
*  Dallas Write bit
\*********************************************************************************************/
void Plugin_100_DS_write_bit(uint8_t v, int8_t Plugin_100_DallasPin)
{
  if (Plugin_100_DallasPin == -1) { return; }

  if (v & 1)
  {
        #if defined(ESP32)
    ESP32noInterrupts();
        #endif // if defined(ESP32)
    digitalWrite(Plugin_100_DallasPin, LOW);
    pinMode(Plugin_100_DallasPin, OUTPUT);
    delayMicroseconds(2);
    digitalWrite(Plugin_100_DallasPin, HIGH);
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
    digitalWrite(Plugin_100_DallasPin, LOW);
    pinMode(Plugin_100_DallasPin, OUTPUT);
    delayMicroseconds(90);
    digitalWrite(Plugin_100_DallasPin, HIGH);
        #if defined(ESP32)
    ESP32interrupts();
        #endif // if defined(ESP32)
    delayMicroseconds(10);
  }
}

/*********************************************************************************************\
*  Standard function to initiate addressing a sensor.
\*********************************************************************************************/
void Plugin_100_DS_address_ROM(uint8_t ROM[8], int8_t Plugin_100_DallasPin)
{
  Plugin_100_DS_reset(Plugin_100_DallasPin);
  Plugin_100_DS_write(0x55, Plugin_100_DallasPin); // Choose ROM

  for (byte i = 0; i < 8; i++) {
    Plugin_100_DS_write(ROM[i], Plugin_100_DallasPin);
  }
}

/*********************************************************************************************\
*  Dallas Calculate CRC16
\*********************************************************************************************/
uint16_t Plugin_100_DS_crc16(const uint8_t* input, uint16_t len, uint16_t crc)
{
    static const uint8_t oddparity[16] =
        { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

    for (uint16_t i = 0 ; i < len ; i++) {
      // Even though we're just copying a byte from the input,
      // we'll be doing 16-bit computation with it.
      uint16_t cdata = input[i];
      cdata = (cdata ^ crc) & 0xff;
      crc >>= 8;

      if (oddparity[cdata & 0x0F] ^ oddparity[cdata >> 4])
          crc ^= 0xC001;

      cdata <<= 6;
      crc ^= cdata;
      cdata <<= 1;
      crc ^= cdata;
    }

    return crc;
}

#if defined(ESP32)
  # undef ESP32noInterrupts
  # undef ESP32interrupts
#endif // if defined(ESP32)

#endif // USES_P100