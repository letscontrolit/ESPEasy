#ifdef USES_P080

// #######################################################################################################
// #################################### Plugin 080: iButton Sensor  DS1990A    ###########################
// #######################################################################################################

// Maxim Integrated

#if defined(ESP32)
  # define ESP32noInterrupts() { portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; portENTER_CRITICAL(&mux)
  # define ESP32interrupts() portEXIT_CRITICAL(&mux); }
#endif // if defined(ESP32)

#define PLUGIN_080
#define PLUGIN_ID_080         80
#define PLUGIN_NAME_080       "Input - iButton [TESTING]"
#define PLUGIN_VALUENAME1_080 "iButton"

#include "_Plugin_Helper.h"

int8_t Plugin_080_DallasPin;

boolean Plugin_080(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_080;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_LONG;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_080);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_080));
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
      Plugin_080_DallasPin = CONFIG_PIN1;

      if (Plugin_080_DallasPin != -1) {
        // get currently saved address
        for (byte i = 0; i < 8; i++) {
          savedAddress[i] = ExtraTaskSettings.TaskDevicePluginConfigLong[i];
        }

        // find all suitable devices
        addRowLabel(F("Device Address"));
        addSelector_Head(F("p080_dev"));
        addSelector_Item("", -1, false, false, "");
        uint8_t tmpAddress[8];
        byte    count = 0;
        Plugin_080_DS_reset();
        Plugin_080_DS_reset_search();

        while (Plugin_080_DS_search(tmpAddress))
        {
          String option = "";

          for (byte j = 0; j < 8; j++)
          {
            option += String(tmpAddress[j], HEX);

            if (j < 7) { option += '-'; }
          }
          bool selected = (memcmp(tmpAddress, savedAddress, 8) == 0) ? true : false;

          // check for DS1990A
          if (tmpAddress[0] == 0x01) {
            addSelector_Item(option, count, selected, false, "");
          }


          count++;
        }
        addSelector_Foot();
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      uint8_t addr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

      // save the address for selected device and store into extra tasksettings
      Plugin_080_DallasPin = CONFIG_PIN1;

      // byte devCount =
      if (Plugin_080_DallasPin != -1) {
        Plugin_080_DS_scan(getFormItemInt(F("p080_dev")), addr);

        for (byte x = 0; x < 8; x++) {
          ExtraTaskSettings.TaskDevicePluginConfigLong[x] = addr[x];
        }

        Plugin_080_DS_startConvertion(addr);
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

        // string += String(ExtraTaskSettings.TaskDevicePluginConfigLong[x], HEX);
      }
      success = true;
      break;
    }
    case PLUGIN_INIT:
    {
      Plugin_080_DallasPin = CONFIG_PIN1;

      if (Plugin_080_DallasPin != -1) {
        uint8_t addr[8];
        Plugin_080_get_addr(addr, event->TaskIndex);
        Plugin_080_DS_startConvertion(addr);
        delay(800); // give it time to do intial conversion
      }
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND: // PLUGIN_READ:
    {
      if (ExtraTaskSettings.TaskDevicePluginConfigLong[0] != 0) {
        uint8_t addr[8];
        Plugin_080_get_addr(addr, event->TaskIndex);

        Plugin_080_DallasPin = CONFIG_PIN1;
        String log = F("DS   : iButton: ");

        if (Plugin_080_DS_readiButton(addr))
        {
          UserVar[event->BaseVarIndex] = 1;
          log                         += UserVar[event->BaseVarIndex];
          success                      = true;
        }
        else
        {
          UserVar[event->BaseVarIndex] = 0;
          log                         += F("Not Present!");
        }
        Plugin_080_DS_startConvertion(addr);
        addLog(LOG_LEVEL_DEBUG, log);
      }
      break;
    }
  }
  return success;
}

void Plugin_080_get_addr(uint8_t addr[], taskIndex_t TaskIndex)
{
  // Load ROM address from tasksettings
  LoadTaskSettings(TaskIndex);

  for (byte x = 0; x < 8; x++) {
    addr[x] = ExtraTaskSettings.TaskDevicePluginConfigLong[x];
  }
}

/*********************************************************************************************\
   Dallas Scan bus
\*********************************************************************************************/
byte Plugin_080_DS_scan(byte getDeviceROM, uint8_t *ROM)
{
  byte tmpaddr[8];
  byte devCount = 0;

  Plugin_080_DS_reset();

  Plugin_080_DS_reset_search();

  while (Plugin_080_DS_search(tmpaddr))
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
*  Dallas Start Temperature Conversion, expected duration:
*    9 bits resolution ->  93.75 ms
*   10 bits resolution -> 187.5 ms
*   11 bits resolution -> 375 ms
*   12 bits resolution -> 750 ms
\*********************************************************************************************/
void Plugin_080_DS_startConvertion(uint8_t ROM[8])
{
  Plugin_080_DS_reset();
  Plugin_080_DS_write(0x55); // Choose ROM

  for (byte i = 0; i < 8; i++) {
    Plugin_080_DS_write(ROM[i]);
  }
  Plugin_080_DS_write(0x44); // Take temperature mesurement
}

/*********************************************************************************************\
*  Dallas Reset
\*********************************************************************************************/
uint8_t Plugin_080_DS_reset()
{
  uint8_t r       = 0;
  uint8_t retries = 125;
  bool    success = true;

    #if defined(ESP32)
  ESP32noInterrupts();
    #endif // if defined(ESP32)
  pinMode(Plugin_080_DallasPin, INPUT);

  do // wait until the wire is high... just in case
  {
    if (--retries == 0) {
      success = false;
    }
    delayMicroseconds(2);
  }
  while (!digitalRead(Plugin_080_DallasPin) && success);

  if (success) {
    pinMode(Plugin_080_DallasPin, OUTPUT); digitalWrite(Plugin_080_DallasPin, LOW);
    delayMicroseconds(492);               // Dallas spec. = Min. 480uSec. Arduino 500uSec.
    pinMode(Plugin_080_DallasPin, INPUT); // Float
    delayMicroseconds(40);
    r = !digitalRead(Plugin_080_DallasPin);
    delayMicroseconds(420);
  }
#if defined(ESP32)
  ESP32interrupts();
#endif // if defined(ESP32)
  return r;
}

#define FALSE 0
#define TRUE  1

unsigned char ROM_NBR[8];
uint8_t LastDiscrep;
uint8_t LastFamilyDiscrep;
uint8_t LastDeviceFlg;


/*********************************************************************************************\
*  Dallas Reset Search
\*********************************************************************************************/
void Plugin_080_DS_reset_search()
{
  // reset the search state
  LastDiscrep       = 0;
  LastDeviceFlg     = FALSE;
  LastFamilyDiscrep = 0;

  for (byte i = 0; i < 8; i++) {
    ROM_NBR[i] = 0;
  }
}

/*********************************************************************************************\
*  Dallas Search bus
\*********************************************************************************************/
uint8_t Plugin_080_DS_search(uint8_t *newAddr)
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
  if (!LastDeviceFlg)
  {
    // 1-Wire reset
    if (!Plugin_080_DS_reset())
    {
      // reset the search
      LastDiscrep       = 0;
      LastDeviceFlg     = FALSE;
      LastFamilyDiscrep = 0;
      return FALSE;
    }

    // issue the search command
    Plugin_080_DS_write(0xF0);

    // loop to do the search
    do
    {
      // read a bit and its complement
      id_bit     = Plugin_080_DS_read_bit();
      cmp_id_bit = Plugin_080_DS_read_bit();

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
          if (id_bit_number < LastDiscrep) {
            search_direction = ((ROM_NBR[rom_byte_number] & rom_byte_mask) > 0);
          }
          else {
            // if equal to last pick 1, if not then pick 0
            search_direction = (id_bit_number == LastDiscrep);
          }

          // if 0 was picked then record its position in LastZero
          if (search_direction == 0)
          {
            last_zero = id_bit_number;

            // check for Last discrepancy in family
            if (last_zero < 9) {
              LastFamilyDiscrep = last_zero;
            }
          }
        }

        // set or clear the bit in the ROM byte rom_byte_number
        // with mask rom_byte_mask
        if (search_direction == 1) {
          ROM_NBR[rom_byte_number] |= rom_byte_mask;
        }
        else {
          ROM_NBR[rom_byte_number] &= ~rom_byte_mask;
        }

        // serial number search direction write bit
        Plugin_080_DS_write_bit(search_direction);

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
      // search successful so set LastDiscrep,LastDeviceFlg,search_result
      LastDiscrep = last_zero;

      // check for last device
      if (LastDiscrep == 0) {
        LastDeviceFlg = TRUE;
      }

      search_result = TRUE;
    }
  }

  // if no device found then reset counters so next 'search' will be like a first
  if (!search_result || !ROM_NBR[0])
  {
    LastDiscrep       = 0;
    LastDeviceFlg     = FALSE;
    LastFamilyDiscrep = 0;
    search_result     = FALSE;
  }

  for (int i = 0; i < 8; i++) {
    newAddr[i] = ROM_NBR[i];
  }

  return search_result;
}

/*********************************************************************************************\
*  Dallas Read byte
\*********************************************************************************************/
uint8_t Plugin_080_DS_read(void)
{
  uint8_t bitMask;
  uint8_t r = 0;

  for (bitMask = 0x01; bitMask; bitMask <<= 1) {
    if (Plugin_080_DS_read_bit()) {
      r |= bitMask;
    }
  }

  return r;
}

/*********************************************************************************************\
*  Dallas Write byte
\*********************************************************************************************/
void Plugin_080_DS_write(uint8_t ByteToWrite)
{
  uint8_t bitMask;

  for (bitMask = 0x01; bitMask; bitMask <<= 1) {
    Plugin_080_DS_write_bit((bitMask & ByteToWrite) ? 1 : 0);
  }
}

/*********************************************************************************************\
*  Dallas Read bit
\*********************************************************************************************/
uint8_t Plugin_080_DS_read_bit(void)
{
  uint8_t r;

    #if defined(ESP32)
  ESP32noInterrupts();
    #endif // if defined(ESP32)
  pinMode(Plugin_080_DallasPin, OUTPUT);
  digitalWrite(Plugin_080_DallasPin, LOW);
  delayMicroseconds(3);
  pinMode(Plugin_080_DallasPin, INPUT); // let pin float, pull up will raise
  delayMicroseconds(10);
  r = digitalRead(Plugin_080_DallasPin);
    #if defined(ESP32)
  ESP32interrupts();
    #endif // if defined(ESP32)
  delayMicroseconds(53);
  return r;
}

boolean Plugin_080_DS_readiButton(byte addr[8])
{
  // maybe this is needed to trigger the reading
  //    byte ScratchPad[12];

  Plugin_080_DS_reset();
  Plugin_080_DS_write(0x55); // Choose ROM

  for (byte i = 0; i < 8; i++) {
    Plugin_080_DS_write(addr[i]);
  }

  Plugin_080_DS_write(0xBE); // Read scratchpad

  //    for (byte i = 0; i < 9; i++) // read 9 bytes
  //        ScratchPad[i] = Plugin_080_DS_read();
  // end maybe this is needed to trigger the reading

  byte tmpaddr[8];
  bool found = false;
  Plugin_080_DS_reset();
  String log = F("DS   : iButton searching for address: ");

  for (byte j = 0; j < 8; j++)
  {
    log += String(addr[j], HEX);

    if (j < 7) { log += '-'; }
  }
  log += F(" found: ");
  Plugin_080_DS_reset_search();

  while (Plugin_080_DS_search(tmpaddr))
  {
    for (byte j = 0; j < 8; j++)
    {
      log += String(tmpaddr[j], HEX);

      if (j < 7) { log += '-'; }
    }
    log += ',';

    if (memcmp(addr, tmpaddr, 8) == 0)
    {
      log  += F("Success. Button was found");
      found = true;
    }
  }
  addLog(LOG_LEVEL_INFO, log);
  return found;
}

/*********************************************************************************************\
*  Dallas Write bit
\*********************************************************************************************/
void Plugin_080_DS_write_bit(uint8_t v)
{
  int timeLow  = (v & 1) ? 10 : 65;
  int timeHigh = (v & 1) ? 55 : 5;

    #if defined(ESP32)
  ESP32noInterrupts();
    #endif // if defined(ESP32)
  digitalWrite(Plugin_080_DallasPin, LOW);
  pinMode(Plugin_080_DallasPin, OUTPUT);
  delayMicroseconds(timeLow);
  digitalWrite(Plugin_080_DallasPin, HIGH);
    #if defined(ESP32)
  ESP32interrupts();
    #endif // if defined(ESP32)
  delayMicroseconds(timeHigh);
}

/*********************************************************************************************\
*  Dallas Calculate CRC8 and compare it of addr[0-7] and compares it to addr[8]
\*********************************************************************************************/
boolean Plugin_080_DS_crc8(uint8_t *addr)
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


#endif // USES_P080
