#include "Dallas1WireHelper.h"

#include "../../_Plugin_Helper.h"
#include "../ESPEasyCore/ESPEasy_Log.h"


#if defined(ESP32)
  # define ESP32noInterrupts() { portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; portENTER_CRITICAL(&mux)
  # define ESP32interrupts() portEXIT_CRITICAL(&mux); }
#endif // if defined(ESP32)

// See: http://owfs.sourceforge.net/simple_family.html
String Dallas_getModel(uint8_t family) {
  String model;

  switch (family) {
    case 0x28: model = F("DS18B20"); break;
    case 0x3b: model = F("DS1825");  break;
    case 0x22: model = F("DS1822");  break;
    case 0x10: model = F("DS1820 / DS18S20");  break;
  }
  return model;
}

String Dallas_format_address(const uint8_t addr[]) {
  String result;

  result.reserve(40);

  for (byte j = 0; j < 8; j++)
  {
    result += String(addr[j], HEX);

    if (j < 7) { result += '-'; }
  }
  result += F(" [");
  result += Dallas_getModel(addr[0]);
  result += ']';

  return result;
}

/*********************************************************************************************\
   Dallas Scan bus
\*********************************************************************************************/
byte Dallas_scan(byte getDeviceROM, uint8_t *ROM, int8_t gpio_pin)
{
  byte tmpaddr[8];
  byte devCount = 0;

  Dallas_reset(gpio_pin);

  Dallas_reset_search();

  while (Dallas_search(tmpaddr, gpio_pin))
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
bool Dallas_is_parasite(const uint8_t ROM[8], int8_t gpio_pin)
{
  Dallas_address_ROM(ROM, gpio_pin);
  Dallas_write(0xB4, gpio_pin); // read power supply
  return !Dallas_read_bit(gpio_pin);
}

void Dallas_startConversion(const uint8_t ROM[8], int8_t gpio_pin)
{
  Dallas_reset(gpio_pin);
  Dallas_write(0x55, gpio_pin); // Choose ROM

  for (byte i = 0; i < 8; i++) {
    Dallas_write(ROM[i], gpio_pin);
  }
  Dallas_write(0x44, gpio_pin);
}

/*********************************************************************************************\
*  Dallas Read temperature from scratchpad
\*********************************************************************************************/
bool Dallas_readTemp(const uint8_t ROM[8], float *value, int8_t gpio_pin)
{
  int16_t DSTemp;
  byte    ScratchPad[12];

  Dallas_address_ROM(ROM, gpio_pin);
  Dallas_write(0xBE, gpio_pin);  // Read scratchpad

  for (byte i = 0; i < 9; i++) { // read 9 bytes
    ScratchPad[i] = Dallas_read(gpio_pin);
  }

  bool crc_ok = Dallas_crc8(ScratchPad);

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

    if (Dallas_is_parasite(ROM, gpio_pin)) {
      log += F(",P");
    }
    log += ',';
    log += String(Dallas_reset_time, DEC);
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

bool Dallas_readiButton(const byte addr[8], int8_t gpio_pin)
{
  // maybe this is needed to trigger the reading
  //    byte ScratchPad[12];

  Dallas_reset(gpio_pin);
  Dallas_write(0x55, gpio_pin); // Choose ROM

  for (byte i = 0; i < 8; i++) {
    Dallas_write(addr[i], gpio_pin);
  }

  Dallas_write(0xBE, gpio_pin); // Read scratchpad

  //    for (byte i = 0; i < 9; i++) // read 9 bytes
  //        ScratchPad[i] = Dallas_read();
  // end maybe this is needed to trigger the reading

  byte tmpaddr[8];
  bool found = false;

  Dallas_reset(gpio_pin);
  String log = F("DS   : iButton searching for address: ");

  for (byte j = 0; j < 8; j++)
  {
    log += String(addr[j], HEX);

    if (j < 7) { log += '-'; }
  }
  log += F(" found: ");
  Dallas_reset_search();

  while (Dallas_search(tmpaddr, gpio_pin))
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
   Dallas read DS2423 counter
   Taken from https://github.com/jbechter/arduino-onewire-DS2423
\*********************************************************************************************/
#define DS2423_READ_MEMORY_COMMAND 0xa5
#define DS2423_PAGE_ONE 0xc0
#define DS2423_PAGE_TWO 0xe0

bool Dallas_readCounter(const uint8_t ROM[8], float *value, int8_t gpio_pin, uint8_t counter)
{
  uint8_t data[45];

  data[0] = DS2423_READ_MEMORY_COMMAND;
  data[1] = (counter == 0 ? DS2423_PAGE_ONE : DS2423_PAGE_TWO);
  data[2] = 0x01;

  Dallas_reset(gpio_pin);
  Dallas_address_ROM(ROM, gpio_pin);

  Dallas_write(data[0], gpio_pin);
  Dallas_write(data[1], gpio_pin);
  Dallas_write(data[2], gpio_pin);

  for (int j = 3; j < 45; j++) {
    data[j] = Dallas_read(gpio_pin);
  }

  Dallas_reset(gpio_pin);

  uint32_t count = (uint32_t)data[38];

  for (int j = 37; j >= 35; j--) {
    count = (count << 8) + (uint32_t)data[j];
  }

  uint16_t crc      = Dallas_crc16(data, 43, 0);
  uint8_t *crcBytes = (uint8_t *)&crc;
  uint8_t  crcLo    = ~data[43];
  uint8_t  crcHi    = ~data[44];
  bool     error    = (crcLo != crcBytes[0]) || (crcHi != crcBytes[1]);

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
* Dallas Get Resolution
\*********************************************************************************************/
byte Dallas_getResolution(const uint8_t ROM[8], int8_t gpio_pin)
{
  // DS1820 and DS18S20 have no resolution configuration register
  if (ROM[0] == 0x10) { return 12; }

  byte ScratchPad[12];

  Dallas_address_ROM(ROM, gpio_pin);
  Dallas_write(0xBE, gpio_pin);  // Read scratchpad

  for (byte i = 0; i < 9; i++) { // read 9 bytes
    ScratchPad[i] = Dallas_read(gpio_pin);
  }

  if (!Dallas_crc8(ScratchPad)) {
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
bool Dallas_setResolution(const uint8_t ROM[8], byte res, int8_t gpio_pin)
{
  // DS1820 and DS18S20 have no resolution configuration register
  if (ROM[0] == 0x10) { return true; }

  byte ScratchPad[12];

  Dallas_address_ROM(ROM, gpio_pin);
  Dallas_write(0xBE, gpio_pin);  // Read scratchpad

  for (byte i = 0; i < 9; i++) { // read 9 bytes
    ScratchPad[i] = Dallas_read(gpio_pin);
  }

  if (!Dallas_crc8(ScratchPad)) {
    addLog(LOG_LEVEL_ERROR, F("DS   : Cannot set resolution"));
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

    Dallas_address_ROM(ROM, gpio_pin);
    Dallas_write(0x4E,          gpio_pin); // Write to EEPROM
    Dallas_write(ScratchPad[2], gpio_pin); // high alarm temp
    Dallas_write(ScratchPad[3], gpio_pin); // low alarm temp
    Dallas_write(ScratchPad[4], gpio_pin); // configuration register

    Dallas_address_ROM(ROM, gpio_pin);

    // save the newly written values to eeprom
    Dallas_write(0x48, gpio_pin);
    delay(100);  // <--- added 20ms delay to allow 10ms long EEPROM write operation (as specified by datasheet)
    Dallas_reset(gpio_pin);

    return true; // new value set
  }
}

/*********************************************************************************************\
*  Dallas Reset
\*********************************************************************************************/
uint8_t Dallas_reset(int8_t gpio_pin)
{
  uint8_t r       = 0;
  uint8_t retries = 125;

    #if defined(ESP32)
  ESP32noInterrupts();
    #endif // if defined(ESP32)
  pinMode(gpio_pin, INPUT);
  bool success = true;

  do // wait until the wire is high... just in case
  {
    if (--retries == 0) {
      success = false;
    }
    delayMicroseconds(2);
  }
  while (!digitalRead(gpio_pin) && success);

  if (success) {
    digitalWrite(gpio_pin, LOW);
    pinMode(gpio_pin, OUTPUT);
    delayMicroseconds(500);
    pinMode(gpio_pin, INPUT);        // Float

    for (uint8_t i = 0; i < 45; i++) // 480us RX minimum
    {
      delayMicroseconds(15);

      if (!digitalRead(gpio_pin)) {
        r                 = 1;
        Dallas_reset_time = i;
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
uint8_t Dallas_reset_time = 0;


/*********************************************************************************************\
*  Dallas Reset Search
\*********************************************************************************************/
void Dallas_reset_search()
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
uint8_t Dallas_search(uint8_t *newAddr, int8_t gpio_pin)
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
    if (!Dallas_reset(gpio_pin))
    {
      // reset the search
      LastDiscrepancy       = 0;
      LastDeviceFlag        = FALSE;
      LastFamilyDiscrepancy = 0;
      return FALSE;
    }

    // issue the search command
    Dallas_write(0xF0, gpio_pin);

    // loop to do the search
    do
    {
      // read a bit and its complement
      id_bit     = Dallas_read_bit(gpio_pin);
      cmp_id_bit = Dallas_read_bit(gpio_pin);

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
        Dallas_write_bit(search_direction, gpio_pin);

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
uint8_t Dallas_read(int8_t gpio_pin)
{
  uint8_t bitMask;
  uint8_t r = 0;

  for (bitMask = 0x01; bitMask; bitMask <<= 1) {
    if (Dallas_read_bit(gpio_pin)) {
      r |= bitMask;
    }
  }

  return r;
}

/*********************************************************************************************\
*  Dallas Write byte
\*********************************************************************************************/
void Dallas_write(uint8_t ByteToWrite, int8_t gpio_pin)
{
  uint8_t bitMask;

  for (bitMask = 0x01; bitMask; bitMask <<= 1) {
    Dallas_write_bit((bitMask & ByteToWrite) ? 1 : 0, gpio_pin);
  }
}

/*********************************************************************************************\
*  Dallas Read bit
\*********************************************************************************************/
uint8_t Dallas_read_bit(int8_t gpio_pin)
{
  if (gpio_pin == -1) { return 0; }
  uint8_t r;

    #if defined(ESP32)
  ESP32noInterrupts();
    #endif // if defined(ESP32)
  digitalWrite(gpio_pin, LOW);
  pinMode(gpio_pin, OUTPUT);
  delayMicroseconds(2);
  pinMode(gpio_pin, INPUT); // let pin float, pull up will raise
  delayMicroseconds(8);
  r = digitalRead(gpio_pin);
    #if defined(ESP32)
  ESP32interrupts();
    #endif // if defined(ESP32)
  delayMicroseconds(60);
  return r;
}

/*********************************************************************************************\
*  Dallas Write bit
\*********************************************************************************************/
void Dallas_write_bit(uint8_t v, int8_t gpio_pin)
{
  if (gpio_pin == -1) { return; }

  if (v & 1)
  {
        #if defined(ESP32)
    ESP32noInterrupts();
        #endif // if defined(ESP32)
    digitalWrite(gpio_pin, LOW);
    pinMode(gpio_pin, OUTPUT);
    delayMicroseconds(2);
    digitalWrite(gpio_pin, HIGH);
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
    digitalWrite(gpio_pin, LOW);
    pinMode(gpio_pin, OUTPUT);
    delayMicroseconds(90);
    digitalWrite(gpio_pin, HIGH);
        #if defined(ESP32)
    ESP32interrupts();
        #endif // if defined(ESP32)
    delayMicroseconds(10);
  }
}

/*********************************************************************************************\
*  Standard function to initiate addressing a sensor.
\*********************************************************************************************/
void Dallas_address_ROM(const uint8_t ROM[8], int8_t gpio_pin)
{
  Dallas_reset(gpio_pin);
  Dallas_write(0x55, gpio_pin); // Choose ROM

  for (byte i = 0; i < 8; i++) {
    Dallas_write(ROM[i], gpio_pin);
  }
}

/*********************************************************************************************\
*  Dallas Calculate CRC8 and compare it of addr[0-7] and compares it to addr[8]
\*********************************************************************************************/
bool Dallas_crc8(const uint8_t *addr)
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

/*********************************************************************************************\
*  Dallas Calculate CRC16
\*********************************************************************************************/
uint16_t Dallas_crc16(const uint8_t *input, uint16_t len, uint16_t crc)
{
  static const uint8_t oddparity[16] =
  { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

  for (uint16_t i = 0; i < len; i++) {
    // Even though we're just copying a byte from the input,
    // we'll be doing 16-bit computation with it.
    uint16_t cdata = input[i];
    cdata = (cdata ^ crc) & 0xff;
    crc >>= 8;

    if (oddparity[cdata & 0x0F] ^ oddparity[cdata >> 4]) {
      crc ^= 0xC001;
    }

    cdata <<= 6;
    crc    ^= cdata;
    cdata <<= 1;
    crc    ^= cdata;
  }

  return crc;
}

#if defined(ESP32)
  # undef ESP32noInterrupts
  # undef ESP32interrupts
#endif // if defined(ESP32)
