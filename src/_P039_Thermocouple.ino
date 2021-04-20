#include "_Plugin_Helper.h"
#ifdef USES_P039

// #######################################################################################################
// ######################## Plugin 039: Thermocouple (MAX6675 / MAX31855) ################################
// #######################################################################################################

// Original work by Dominik

// Plugin Description
// This Plugin reads the data from Thermocouples. You have to use an Adapter Board with a
// MAX6675 or MAX31855 in order to read the values. Take a look at ebay to find such boards :-)
// You can only use ESP8266 boards which expose the SPI Interface. This Plugin uses only the Hardware
// SPI Interface - no software SPI at the moment.
// But nevertheless you need at least 3 Pins to use SPI. So using an very simple ESP-01 is no option - Sorry.
// The Wiring is straight forward ...
//
// If you like to send suggestions feel free to send me an email : dominik@logview.info
// Have fun ... Dominik

// Wiring
// https://de.wikipedia.org/wiki/Serial_Peripheral_Interface
// You need an ESP8266 device with accessible SPI Pins. These are:
// Name   Description     GPIO      NodeMCU   Notes
// MOSI   Master Output   GPIO13    D7        Not used (No Data sending to MAX)
// MISO   Master Input    GPIO12    D6        Hardware SPI
// SCK    Clock Output    GPIO14    D5        Hardware SPI
// CS     Chip Select     GPIO15    D8        Hardware SPI (CS is configurable through the web interface)

// Thermocouple Infos
// http://www.bristolwatch.com/ele2/therc.htm

// Chips
// MAX6675  - Cold-Junction-Compensated K-Thermocouple-to-Digital Converter (   0°C to +1024°C)
//            https://cdn-shop.adafruit.com/datasheets/MAX6675.pdf (only
// MAX31855 - Cold-Junction Compensated Thermocouple-to-Digital Converter   (-270°C to +1800°C)
//            https://cdn-shop.adafruit.com/datasheets/MAX31855.pdf
// MAX31856 - Precision Thermocouple to Digital Converter with Linearization   (-210°C to +1800°C)
//            https://datasheets.maximintegrated.com/en/ds/MAX31856.pdf

# include <SPI.h>


# define PLUGIN_039
# define PLUGIN_ID_039         39
# define PLUGIN_NAME_039       "Environment - Thermocouple"
# define PLUGIN_VALUENAME1_039 "Temperature"

# define P039_MAX_TYPE         PCONFIG(0)
# define P039_TC_TYPE          PCONFIG(1)

# define P039_MAX_6675         1
# define P039_MAX_31855        2
# define P039_MAX_31856        3


boolean Plugin_039(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_039;
      Device[deviceCount].Type               = DEVICE_TYPE_SPI;
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
      string = F(PLUGIN_NAME_039);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_039));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("CS"));
      break;
    }

    case PLUGIN_INIT:
    {
      // set the slaveSelectPin as an output:
      pinMode(Plugin_039_Get_SPI_CS_Pin(event), OUTPUT);

      // initialize SPI:
      SPI.setHwCs(false);
      SPI.begin();

      if (P039_MAX_TYPE == P039_MAX_31856) {
        // FIXME TD-er: Must really look into those enormous long delays
        digitalWrite(Plugin_039_Get_SPI_CS_Pin(event), LOW);
        delay(650);
        SPI.transfer(0x80);
        SPI.transfer(0x01);         // noisefilter 50Hz (set this to 0x00 if You live in a 60Hz country)
        SPI.transfer(P039_TC_TYPE); // thermocouple type
        SPI.transfer(0xFF);
        SPI.transfer(0x7F);
        SPI.transfer(0xC0);
        SPI.transfer(0x7F);
        SPI.transfer(0xFF);
        SPI.transfer(0x80);
        SPI.transfer(0x00);
        SPI.transfer(0x00);

        digitalWrite(Plugin_039_Get_SPI_CS_Pin(event), HIGH);
        delay(50);
      }

      addLog(LOG_LEVEL_INFO, F("P039 : SPI Init"));
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // FIXME TD-er: Why is this list needed? GPIO selector should provide this info.
        # ifdef ESP8266
      addFormNote(F("<b>1st GPIO</b> = CS (Usable GPIOs : 0, 2, 4, 5, 15)"));
        # endif // ifdef ESP8266
        # ifdef ESP32
      addFormNote(F("<b>1st GPIO</b> = CS (Usable GPIOs : 0, 2, 4, 5, 15..19, 21..23, 25..27, 32, 33)"));
        # endif // ifdef ESP32

      // addHtml(F("<TR><TD>Info GPIO:<TD><b>1st GPIO</b> = CS (Usable GPIOs : 0, 2, 4, 5, 15)"));

      const byte choice = P039_MAX_TYPE;
      {
        const String options[3]      = {   F("MAX 6675"), F("MAX 31855"), F("MAX 31856") };
        const int    optionValues[3] = { P039_MAX_6675, P039_MAX_31855, P039_MAX_31856 };
        addFormSelector(F("Adapter IC"), F("p039_maxtype"), 3, options, optionValues, choice);
      }

      if (choice == P039_MAX_31856) {
        addFormNote(F("Set Thermocouple type for MAX31856"));
        const String Toptions[8]      = { F("B"), F("E"), F("J"), F("K"), F("N"), F("R"), F("S"), F("T") };
        const int    ToptionValues[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
        addFormSelector(F("Thermocouple type"), F("p039_tctype"), 8, Toptions, ToptionValues, P039_TC_TYPE);
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P039_MAX_TYPE = getFormItemInt(F("p039_maxtype"));
      P039_TC_TYPE  = getFormItemInt(F("p039_tctype"));
      success       = true;
      break;
    }

    case PLUGIN_READ:
    {
      // Get the MAX Type (6675 / 31855 / 31856)
      byte MaxType = P039_MAX_TYPE;

      float Plugin_039_Celsius = NAN;

      switch (MaxType) {
        case P039_MAX_6675:
          Plugin_039_Celsius = readMax6675(event);
          break;
        case P039_MAX_31855:
          Plugin_039_Celsius = readMax31855(event);
          break;
        case P039_MAX_31856:
          Plugin_039_Celsius = readMax31856(event);
          break;
      }

      if (Plugin_039_Celsius != NAN)
      {
        UserVar[event->BaseVarIndex] = Plugin_039_Celsius;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("P039 : Temperature ");
          log += formatUserVarNoCheck(event->TaskIndex, 0);
          addLog(LOG_LEVEL_INFO, log);
        }
        success = true;
      }
      else
      {
        UserVar[event->BaseVarIndex]     = NAN;
        UserVar[event->BaseVarIndex + 1] = NAN;
        addLog(LOG_LEVEL_INFO, F("P039 : No Sensor attached !"));
        success = false;
      }

      break;
    }
  }
  return success;
}

float readMax6675(struct EventStruct *event)
{
  // take the SS pin low to select the chip:
  digitalWrite(Plugin_039_Get_SPI_CS_Pin(event), LOW);

  // String log = F("P039 : CS Pin : ");
  // log += Plugin_039_Get_SPI_CS_Pin(event);
  // addLog(LOG_LEVEL_INFO, log);
  // "transfer" 0x0 and read the Data from the Chip
  uint16_t rawvalue = SPI.transfer16(0x0);

  // take the SS pin high to de-select the chip:
  digitalWrite(Plugin_039_Get_SPI_CS_Pin(event), HIGH);

# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("P039 : MAX6675 : RAW - BIN:");

    log += String(rawvalue, BIN);
    log += " HEX:";
    log += String(rawvalue, HEX);
    log += " DEC:";
    log += String(rawvalue);
    addLog(LOG_LEVEL_DEBUG, log);
  }
  # endif // ifndef BUILD_NO_DEBUG

  // Open Thermocouple
  // Bit D2 is normally low and goes high if the thermocouple input is open. In order to allow the operation of the
  // open  thermocouple  detector,  T-  must  be  grounded. Make  the  ground  connection  as  close  to  the  GND  pin
  // as possible.
  const bool Plugin_039_SensorAttached = !(rawvalue & 0x0004);

  if (Plugin_039_SensorAttached)
  {
    // Shift RAW value 3 Bits to the right to get the data
    rawvalue >>= 3;

    // Calculate Celsius
    return rawvalue * 0.25f;
  }
  else
  {
    return NAN;
  }
}

float readMax31855(struct EventStruct *event)
{
  // take the SS pin low to select the chip:
  digitalWrite(Plugin_039_Get_SPI_CS_Pin(event), LOW);

  // "transfer" 0x0 and read the MSB Data from the Chip
  uint32_t rawvalue = SPI.transfer16(0x0);

  // Shift MSB 16 Bits to the left
  rawvalue <<= 16;

  // "transfer" 0x0 and read the LSB Data from the Chip
  rawvalue |= SPI.transfer16(0x0);

  // take the SS pin high to de-select the chip:
  digitalWrite(Plugin_039_Get_SPI_CS_Pin(event), HIGH);

# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("P039 : MAX31855 : RAW - BIN:");

    log += String(rawvalue, BIN);
    log += " HEX:";
    log += String(rawvalue, HEX);
    log += " DEC:";
    log += String(rawvalue);
    addLog(LOG_LEVEL_DEBUG, log);
  }
# endif // ifndef BUILD_NO_DEBUG


  // FIXME TD-er: This static flag is shared among all instances of this plugin
  static bool sensorFault = false;

  if (sensorFault != ((rawvalue & 0x7) == 0)) {
    // Fault code changed, log them
    sensorFault = ((rawvalue & 0x7) == 0);

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("P039 : MAX31855");

      if (!sensorFault) {
        log += F("Fault resolved");
      } else {
        log += F("Fault code:");

        if (rawvalue & 0x01) {
          log += F(" Open (no connection)");
        }

        if (rawvalue & 0x02) {
          log += F(" Short-circuit to GND");
        }

        if (rawvalue & 0x04) {
          log += F(" Short-circuit to Vcc");
        }
      }
      addLog(LOG_LEVEL_INFO, log);
    }
  }

  // D16 - This bit reads at 1 when any of the SCV, SCG, or OC faults are active. Default value is 0.
  const bool Plugin_039_SensorAttached = !(rawvalue & 0x00010000);

  if (Plugin_039_SensorAttached)
  {
    // Data is D[31:18]
    // Shift RAW value 18 Bits to the right to get the data
    rawvalue >>= 18;

    // Check for negative Values
    //  +25.00    0000 0001 1001 00
    //    0.00    0000 0000 0000 00
    //   -0.25    1111 1111 1111 11
    //   -1.00    1111 1111 1111 00
    // -250.00    1111 0000 0110 00
    // We're left with (32 - 18 =) 14 bits
    int temperature = Plugin_039_convert_two_complement(rawvalue, 14);

    // Calculate Celsius
    return temperature * 0.25f;
  }
  else
  {
    // Fault state, thus output no value.
    return NAN;
  }
}

float readMax31856(struct EventStruct *event)
{
  digitalWrite(Plugin_039_Get_SPI_CS_Pin(event), LOW);


  # define P039_RAWVALUE 0
  # define P039_CR0      1
  # define P039_CR1      2
  # define P039_MASK     3
  # define P039_CJHF     4
  # define P039_CJLF     5
  # define P039_LTHFTH   6
  # define P039_LTHFTL   7
  # define P039_LTLFTH   8
  # define P039_LTLFTL   9
  # define P039_CJTO    10
  # define P039_CJTH    11
  # define P039_CJTL    12
  # define P039_LTCBH   13
  # define P039_LTCBM   14
  # define P039_LTCBL   15
  # define P039_SR      16

  uint32_t registers[17] = { 0 };

  for (int i = 0; i < 17; ++i) {
    registers[i] = SPI.transfer(0x0);
  }

  // take the SS pin high to de-select the chip:
  digitalWrite(Plugin_039_Get_SPI_CS_Pin(event), HIGH);


  uint32_t rawvalue = registers[P039_LTCBH];
  rawvalue = (rawvalue << 8) | registers[P039_LTCBM];
  rawvalue = (rawvalue << 8) | registers[P039_LTCBL];

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    log.reserve(66);
    log = F("P039 : MAX31856 :");

    for (int i = 1; i < 17; ++i) {
      log += ' ';
      log += String(registers[i], HEX);
    }

    addLog(LOG_LEVEL_DEBUG, log);
  }
  # endif // ifndef BUILD_NO_DEBUG

  const uint32_t sr = registers[P039_SR];

  // FIXME TD-er: This static flag is shared among all instances of this plugin
  static bool sensorFault = false;

  const bool faultResolved = sensorFault && (sr == 0);
  sensorFault = (sr != 0); // Set new state

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    if (sensorFault || faultResolved) {
      String log = F("P039 : MAX31856");

      if (sensorFault == 0) {
        log += F("Fault resolved");
      } else {
        log += F("Fault :");

        if (sr & 0x01) {
          log += F(" Open (no connection)");
        }

        if (sr & 0x02) {
          log += F(" Over/Under Voltage");
        }

        if (sr & 0x04) {
          log += F(" TC Low");
        }

        if (sr & 0x08) {
          log += F(" TC High");
        }

        if (sr & 0x10) {
          log += F(" CJ Low");
        }

        if (sr & 0x20) {
          log += F(" CJ High");
        }

        if (sr & 0x40) {
          log += F(" TC Range");
        }

        if (sr & 0x80) {
          log += F(" CJ Range");
        }
      }
      addLog(LOG_LEVEL_INFO, log);
    }
  }
  const bool Plugin_039_SensorAttached = (sr == 0);

  if (Plugin_039_SensorAttached)
  {
    registers[P039_RAWVALUE] >>= 5; // bottom 5 bits are unused
    // We're left with (24 - 5 =) 19 bits
    float temperature = Plugin_039_convert_two_complement(registers[P039_RAWVALUE], 19);

    // Calculate Celsius
    return temperature / 128.0f;
  }
  else
  {
    // Fault state, thus output no value.
    return NAN;
  }
}

int Plugin_039_convert_two_complement(uint32_t value, int nr_bits) {
  const bool negative = (value & (1 << (nr_bits - 1))) != 0;
  int nativeInt;

  if (negative) {
    // Add zeroes to the left to create the proper negative native-sized integer.
    nativeInt = value | ~((1 << nr_bits) - 1);
  } else {
    nativeInt = value;
  }
  return nativeInt;
}

int Plugin_039_Get_SPI_CS_Pin(struct EventStruct *event) {
  // If no Pin is in Config we use 15 as default -> Hardware Chip Select on ESP8266
  if (CONFIG_PIN1 != 0) {
    return CONFIG_PIN1;
  }
  return 15; // D8
}

#endif // USES_P039
