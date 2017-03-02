//#######################################################################################################
//#################################### Plugin 004: TempSensor Dallas DS18B20  ###########################
//#######################################################################################################

#define PLUGIN_004
#define PLUGIN_ID_004         4
#define PLUGIN_NAME_004       "Temperature - DS18b20"
#define PLUGIN_VALUENAME1_004 "Temperature"

uint8_t Plugin_004_DallasPin;

boolean Plugin_004(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_004;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
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

    case PLUGIN_WEBFORM_LOAD:
      {
        uint8_t addr[8];

        // Scan the onewire bus and fill dropdown list with devicecount on this GPIO.
        Plugin_004_DallasPin = Settings.TaskDevicePin1[event->TaskIndex];

        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        byte devCount = Plugin_004_DS_scan(choice, addr);
        string += F("<TR><TD>Device Nr:<TD><select name='plugin_004_dev'>");
        for (byte x = 0; x < devCount; x++)
        {
          string += F("<option value='");
          string += x;
          string += "'";
          if (choice == x)
            string += F(" selected");
          string += ">";
          string += x + 1;
          string += F("</option>");
        }
        string += F("</select> ROM: ");
        if (devCount)
        {
          for (byte  i = 0; i < 8; i++)
          {
            string += String(addr[i], HEX);
            if (i < 7) string += "-";
          }
        }
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        uint8_t addr[8];
        String plugin1 = WebServer.arg(F("plugin_004_dev"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();

        // find the address for selected device and store into extra tasksettings
        Plugin_004_DallasPin = Settings.TaskDevicePin1[event->TaskIndex];
        byte devCount = Plugin_004_DS_scan(Settings.TaskDevicePluginConfig[event->TaskIndex][0], addr);
        for (byte x = 0; x < 8; x++)
          ExtraTaskSettings.TaskDevicePluginConfigLong[x] = addr[x];
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
      {
        for (byte x = 0; x < 8; x++)
        {
          if (x != 0)
            string += "-";
          string += String(ExtraTaskSettings.TaskDevicePluginConfigLong[x], HEX);
        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        uint8_t addr[8];
        // Load ROM address from tasksettings
        LoadTaskSettings(event->TaskIndex);
        for (byte x = 0; x < 8; x++)
          addr[x] = ExtraTaskSettings.TaskDevicePluginConfigLong[x];

        Plugin_004_DallasPin = Settings.TaskDevicePin1[event->TaskIndex];
        float value = 0;
        String log = F("DS   : Temperature: ");
        if (Plugin_004_DS_readTemp(addr, &value))
        {
          UserVar[event->BaseVarIndex] = value;
          log += UserVar[event->BaseVarIndex];
          success = true;
        }
        else
        {
          UserVar[event->BaseVarIndex] = NAN;
          log += F("Error!");
        }
        log += (" (");
        for (byte x = 0; x < 8; x++)
        {
          if (x != 0)
            log += "-";
          log += String(ExtraTaskSettings.TaskDevicePluginConfigLong[x], HEX);
        }
        log += ')';
        addLog(LOG_LEVEL_INFO, log);
        break;
      }

  }
  return success;
}


/*********************************************************************************************\
   Dallas Scan bus
  \*********************************************************************************************/
byte Plugin_004_DS_scan(byte getDeviceROM, uint8_t* ROM)
{
  byte tmpaddr[8];
  byte devCount = 0;
  Plugin_004_DS_reset();

  Plugin_004_DS_reset_search();
  while (Plugin_004_DS_search(tmpaddr))
  {
    if (getDeviceROM == devCount)
      for (byte  i = 0; i < 8; i++)
        ROM[i] = tmpaddr[i];
    devCount++;
  }
  return devCount;
}


/*********************************************************************************************\
   Dallas Read temperature
  \*********************************************************************************************/
boolean Plugin_004_DS_readTemp(uint8_t ROM[8], float *value)
{
  int16_t DSTemp;
  byte ScratchPad[12];

  Plugin_004_DS_reset();
  Plugin_004_DS_write(0x55);           // Choose ROM
  for (byte i = 0; i < 8; i++)
    Plugin_004_DS_write(ROM[i]);
  Plugin_004_DS_write(0x44);

  delay(800);

  Plugin_004_DS_reset();
  Plugin_004_DS_write(0x55);           // Choose ROM
  for (byte i = 0; i < 8; i++)
    Plugin_004_DS_write(ROM[i]);
  Plugin_004_DS_write(0xBE); // Read scratchpad

  for (byte i = 0; i < 9; i++)            // copy 8 bytes
    ScratchPad[i] = Plugin_004_DS_read();

  if (Plugin_004_DS_crc8(ScratchPad, 8) != ScratchPad[8])
  {
    *value = 0;
    return false;
  }

  if ((ROM[0] == 0x28 ) || (ROM[0] == 0x3b)) //DS18B20 or DS1825
  {
    DSTemp = (ScratchPad[1] << 8) + ScratchPad[0];
    *value = (float(DSTemp) * 0.0625);
  }
  else if (ROM[0] == 0x10 ) //DS1820 DS18S20
  {
    DSTemp = (ScratchPad[1] << 11) | ScratchPad[0] << 3;
    DSTemp = ((DSTemp & 0xfff0) << 3) - 16 +
             (
               ((ScratchPad[7] - ScratchPad[6]) << 7) /
               ScratchPad[7]
             );
    *value = float(DSTemp) * 0.0078125;
  }
  return true;
}


/*********************************************************************************************\
   Dallas Reset
  \*********************************************************************************************/
uint8_t Plugin_004_DS_reset()
{
  uint8_t r;
  uint8_t retries = 125;
  //noInterrupts();
  pinMode(Plugin_004_DallasPin, INPUT);
  do  {  // wait until the wire is high... just in case
    if (--retries == 0) return 0;
    delayMicroseconds(2);
  } while ( !digitalRead(Plugin_004_DallasPin));

  pinMode(Plugin_004_DallasPin, OUTPUT); digitalWrite(Plugin_004_DallasPin, LOW);
  delayMicroseconds(492); // Dallas spec. = Min. 480uSec. Arduino 500uSec.
  pinMode(Plugin_004_DallasPin, INPUT); //Float
  delayMicroseconds(40);
  r = !digitalRead(Plugin_004_DallasPin);
  delayMicroseconds(420);
  //interrupts();
  return r;
}


#define FALSE 0
#define TRUE  1

unsigned char ROM_NO[8];
uint8_t LastDiscrepancy;
uint8_t LastFamilyDiscrepancy;
uint8_t LastDeviceFlag;


/*********************************************************************************************\
   Dallas Reset Search
  \*********************************************************************************************/
void Plugin_004_DS_reset_search()
{
  // reset the search state
  LastDiscrepancy = 0;
  LastDeviceFlag = FALSE;
  LastFamilyDiscrepancy = 0;
  for (byte i = 0; i < 8; i++)
    ROM_NO[i] = 0;
}


/*********************************************************************************************\
   Dallas Search bus
  \*********************************************************************************************/
uint8_t Plugin_004_DS_search(uint8_t *newAddr)
{
  uint8_t id_bit_number;
  uint8_t last_zero, rom_byte_number, search_result;
  uint8_t id_bit, cmp_id_bit;
  unsigned char rom_byte_mask, search_direction;

  // initialize for search
  id_bit_number = 1;
  last_zero = 0;
  rom_byte_number = 0;
  rom_byte_mask = 1;
  search_result = 0;

  // if the last call was not the last one
  if (!LastDeviceFlag)
  {
    // 1-Wire reset
    if (!Plugin_004_DS_reset())
    {
      // reset the search
      LastDiscrepancy = 0;
      LastDeviceFlag = FALSE;
      LastFamilyDiscrepancy = 0;
      return FALSE;
    }

    // issue the search command
    Plugin_004_DS_write(0xF0);

    // loop to do the search
    do
    {
      // read a bit and its complement
      id_bit = Plugin_004_DS_read_bit();
      cmp_id_bit = Plugin_004_DS_read_bit();

      // check for no devices on 1-wire
      if ((id_bit == 1) && (cmp_id_bit == 1))
        break;
      else
      {
        // all devices coupled have 0 or 1
        if (id_bit != cmp_id_bit)
          search_direction = id_bit;  // bit write value for search
        else
        {
          // if this discrepancy if before the Last Discrepancy
          // on a previous next then pick the same as last time
          if (id_bit_number < LastDiscrepancy)
            search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
          else
            // if equal to last pick 1, if not then pick 0
            search_direction = (id_bit_number == LastDiscrepancy);

          // if 0 was picked then record its position in LastZero
          if (search_direction == 0)
          {
            last_zero = id_bit_number;

            // check for Last discrepancy in family
            if (last_zero < 9)
              LastFamilyDiscrepancy = last_zero;
          }
        }

        // set or clear the bit in the ROM byte rom_byte_number
        // with mask rom_byte_mask
        if (search_direction == 1)
          ROM_NO[rom_byte_number] |= rom_byte_mask;
        else
          ROM_NO[rom_byte_number] &= ~rom_byte_mask;

        // serial number search direction write bit
        Plugin_004_DS_write_bit(search_direction);

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
      if (LastDiscrepancy == 0)
        LastDeviceFlag = TRUE;

      search_result = TRUE;
    }
  }

  // if no device found then reset counters so next 'search' will be like a first
  if (!search_result || !ROM_NO[0])
  {
    LastDiscrepancy = 0;
    LastDeviceFlag = FALSE;
    LastFamilyDiscrepancy = 0;
    search_result = FALSE;
  }
  for (int i = 0; i < 8; i++) newAddr[i] = ROM_NO[i];
  return search_result;
}

/*********************************************************************************************\
   Dallas Read byte
  \*********************************************************************************************/
uint8_t Plugin_004_DS_read(void)
{
  uint8_t bitMask;
  uint8_t r = 0;

  for (bitMask = 0x01; bitMask; bitMask <<= 1)
    if (Plugin_004_DS_read_bit())
      r |= bitMask;

  return r;
}


/*********************************************************************************************\
   Dallas Write byte
  \*********************************************************************************************/
void Plugin_004_DS_write(uint8_t ByteToWrite)
{
  uint8_t bitMask;
  for (bitMask = 0x01; bitMask; bitMask <<= 1)
    Plugin_004_DS_write_bit( (bitMask & ByteToWrite) ? 1 : 0);
}


/*********************************************************************************************\
   Dallas Read bit
  \*********************************************************************************************/
uint8_t Plugin_004_DS_read_bit(void)
{
  uint8_t r;

  //noInterrupts();
  pinMode(Plugin_004_DallasPin, OUTPUT);
  digitalWrite(Plugin_004_DallasPin, LOW);
  delayMicroseconds(3);
  pinMode(Plugin_004_DallasPin, INPUT); // let pin float, pull up will raise
  delayMicroseconds(10);
  r = digitalRead(Plugin_004_DallasPin);
  //interrupts();
  delayMicroseconds(53);
  return r;
}


/*********************************************************************************************\
   Dallas Write bit
  \*********************************************************************************************/
void Plugin_004_DS_write_bit(uint8_t v)
{
  if (v & 1) {
    //noInterrupts();
    digitalWrite(Plugin_004_DallasPin, LOW);
    pinMode(Plugin_004_DallasPin, OUTPUT);
    delayMicroseconds(10);
    digitalWrite(Plugin_004_DallasPin, HIGH);
    //interrupts();
    delayMicroseconds(55);
  } else {
    //noInterrupts();
    digitalWrite(Plugin_004_DallasPin, LOW);
    pinMode(Plugin_004_DallasPin, OUTPUT);
    delayMicroseconds(65);
    digitalWrite(Plugin_004_DallasPin, HIGH);
    //interrupts();
    delayMicroseconds(5);
  }
}

uint8_t Plugin_004_DS_crc8( uint8_t *addr, uint8_t len)
{
  uint8_t crc = 0;

  while (len--) {
    uint8_t inbyte = *addr++;
    for (uint8_t i = 8; i; i--) {
      uint8_t mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix) crc ^= 0x8C;
      inbyte >>= 1;
    }
  }
  return crc;
}
