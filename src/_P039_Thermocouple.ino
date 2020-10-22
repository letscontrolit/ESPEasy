#include "_Plugin_Helper.h"
#ifdef USES_P039
//#######################################################################################################
//######################## Plugin 039: Thermocouple (MAX6675 / MAX31855) ################################
//#######################################################################################################

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

#include <SPI.h>


#define PLUGIN_039
#define PLUGIN_ID_039         39
#define PLUGIN_NAME_039       "Environment - Thermocouple"
#define PLUGIN_VALUENAME1_039 "Temperature"

uint8_t Plugin_039_SPI_CS_Pin = 15;  // D8
bool Plugin_039_SensorAttached = true;
uint32_t Plugin_039_Sensor_fault = 0;
float Plugin_039_Celsius = 0.0f;

boolean Plugin_039(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_039;
        Device[deviceCount].Type = DEVICE_TYPE_SPI;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SINGLE;
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
        // Get CS Pin
        // If no Pin is in Config we use 15 as default -> Hardware Chip Select on ESP8266
        if (CONFIG_PIN1 != 0)
        {
          // Konvert the GPIO Pin to a Dogotal Puin Number first ...
          Plugin_039_SPI_CS_Pin = CONFIG_PIN1;
        }

        // set the slaveSelectPin as an output:
        pinMode(Plugin_039_SPI_CS_Pin, OUTPUT);
        // initialize SPI:
        SPI.setHwCs(false);
        SPI.begin();

        addLog(LOG_LEVEL_INFO, F("P039 : SPI Init"));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        // FIXME TD-er: Why is this list needed? GPIO selector should provide this info.
        #ifdef ESP8266
        addFormNote(F("<b>1st GPIO</b> = CS (Usable GPIOs : 0, 2, 4, 5, 15)"));
        #endif
        #ifdef ESP32
        addFormNote(F("<b>1st GPIO</b> = CS (Usable GPIOs : 0, 2, 4, 5, 15..19, 21..23, 25..27, 32, 33)"));
        #endif
        //addHtml(F("<TR><TD>Info GPIO:<TD><b>1st GPIO</b> = CS (Usable GPIOs : 0, 2, 4, 5, 15)"));

        byte choice = PCONFIG(0);
        String options[2];
        options[0] = F("MAX 6675");
        options[1] = F("MAX 31855");
        //options[2] = F("MAX 31865");
        int optionValues[2] = { 1, 2 };
        addFormSelector(F("Adapter IC"), F("p039_maxtype"), 2, options, optionValues, choice);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = getFormItemInt(F("p039_maxtype"));
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // Get the MAX Type (6675 / 31855)
        // TBD ... Auswertung je nach Chip !!!
        byte MaxType = PCONFIG(0);

        // Get CS Pin
        // Konvert the GPIO Pin to a Dogotal Puin Number first ...
        Plugin_039_SPI_CS_Pin = CONFIG_PIN1;

        switch (MaxType) {
          case 1:       // MAX6675
            Plugin_039_Celsius = readMax6675();
            break;
          case 2:       // MAX31855
            Plugin_039_Celsius = readMax31855();
            break;
          case 3:       // MAX31865 (not implemented yet)
            //do something when var equals 2
            break;
        }

        if (Plugin_039_Celsius != NAN)
        {
          UserVar[event->BaseVarIndex] = Plugin_039_Celsius;
          String log = F("P039 : Temperature ");
          log += UserVar[event->BaseVarIndex];
          addLog(LOG_LEVEL_INFO, log);
          success = true;
        }
        else
        {
          UserVar[event->BaseVarIndex] = NAN;
          UserVar[event->BaseVarIndex + 1] = NAN;
          addLog(LOG_LEVEL_INFO, F("P039 : No Sensor attached !"));
          success = false;
        }

        break;
      }
  }
  return success;
}

float readMax6675()
{
  uint16_t rawvalue = 0;
  // take the SS pin low to select the chip:
  digitalWrite(Plugin_039_SPI_CS_Pin, LOW);
  // String log = F("P039 : CS Pin : ");
  // log += Plugin_039_SPI_CS_Pin;
  // addLog(LOG_LEVEL_INFO, log);
  // "transfer" 0x0 and read the Data from the Chip
  rawvalue = SPI.transfer16(0x0);
  // take the SS pin high to de-select the chip:
  digitalWrite(Plugin_039_SPI_CS_Pin, HIGH);

  String log = F("P039 : MAX6675 : RAW - BIN:");
  log += String(rawvalue, BIN);
  log += " HEX:";
  log += String(rawvalue, HEX);
  log += " DEC:";
  log += String(rawvalue);
  addLog(LOG_LEVEL_DEBUG, log);

  // Open Thermocouple
  // Bit D2 is normally low and goes high if the thermocouple input is open. In order to allow the operation of the
  // open  thermocouple  detector,  T-  must  be  grounded. Make  the  ground  connection  as  close  to  the  GND  pin
  // as possible.
  Plugin_039_SensorAttached = !(rawvalue & 0x0004);

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

float readMax31855()
{
  uint32_t rawvalue = 0;
  // take the SS pin low to select the chip:
  digitalWrite(Plugin_039_SPI_CS_Pin, LOW);
  // "transfer" 0x0 and read the MSB Data from the Chip
  rawvalue = SPI.transfer16(0x0);
  // Shift MSB 16 Bits to the left
  rawvalue <<= 16;
  // "transfer" 0x0 and read the LSB Data from the Chip
  rawvalue |= SPI.transfer16(0x0);
  // take the SS pin high to de-select the chip:
  digitalWrite(Plugin_039_SPI_CS_Pin, HIGH);

  String log = F("P039 : MAX31855 : RAW - BIN:");
  log += String(rawvalue, BIN);
  log += " HEX:";
  log += String(rawvalue, HEX);
  log += " DEC:";
  log += String(rawvalue);
  addLog(LOG_LEVEL_DEBUG, log);

  if (Plugin_039_Sensor_fault != (rawvalue & 0x7)) {
    // Fault code changed, log them
    Plugin_039_Sensor_fault = (rawvalue & 0x7);
    log = F("P039 : MAX31855");
    if (Plugin_039_Sensor_fault == 0) {
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
    addLog(LOG_LEVEL_DEBUG, log);
  }
  // D16 - This bit reads at 1 when any of the SCV, SCG, or OC faults are active. Default value is 0.
  Plugin_039_SensorAttached = !(rawvalue & 0x00010000);
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
#endif // USES_P039
