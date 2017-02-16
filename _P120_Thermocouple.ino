#ifdef PLUGIN_BUILD_DEV

//#######################################################################################################
//######################## Plugin 120: Thermocouple (MAX6675 / MAX31855) ################################
//#######################################################################################################

// Plugin Description
// This Plugin reads the data from Thermocouples. You have to use an Adapter Board with a
// MAX6675 or MAX31855 in order to read the values. Take a look at ebay to find such boards :-)
// You can only use ESP8266 boards which expose the SPI Interface. This Plugin uses only the Hardware
// SPI Interface - no software SPI at the moment.
// But neverless you need at least 3 Pins to use SPI. So using an very simple ESP-01 is no option - Sorry.
// The Wiring ist straight forward ...
//
// If you like to send suggestions feel free to send me an email : dominik@logview.info
// Have fun ... Dominik

// Wiring
// https://de.wikipedia.org/wiki/Serial_Peripheral_Interface
// You need an ESP8266 device with accessable SPI Pins. These are:
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

#define PLUGIN_120
#define PLUGIN_ID_120         120
#define PLUGIN_NAME_120       "Temperature Thermocouple [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_120 "Temperature C"
#define PLUGIN_VALUENAME2_120 "Temperature K"

uint8_t Plugin_120_SPI_CS_Pin = 15;  // D8
bool Plugin_120_SensorAttached = true;
double Plugin_120_Celsius = 0.0;

boolean Plugin_120(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_120;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_DUAL;                           // 2 Messwerte
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_120);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_120));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_120));
        break;
      }

    case PLUGIN_INIT:
      {
        // Get CS Pin
        // If no Pin is in Config we use 15 as default -> Hardware Chip Select on ESP8266
        if (Settings.TaskDevicePin1[event->TaskIndex] != 0)
        {
          // Konvert the GPIO Pin to a Dogotal Puin Number first ...
          Plugin_120_SPI_CS_Pin = Settings.TaskDevicePin1[event->TaskIndex];
        }

        // set the slaveSelectPin as an output:
        pinMode(Plugin_120_SPI_CS_Pin, OUTPUT);
        // initialize SPI:
        SPI.setHwCs(false);
        SPI.begin();

        addLog(LOG_LEVEL_INFO, (char*)"P120 : SPI Init");

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        string += F("<TR><TD>Info GPIO:<TD><b>1st GPIO</b> = CS (Usable GPIOs : 0, 2, 4, 5, 15)");

        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options[2];
        options[0] = F("MAX 6675");
        options[1] = F("MAX 31855");
        //options[2] = F("MAX 31865");
        int optionValues[2];
        optionValues[0] = 1;
        optionValues[1] = 2;
        //optionValues[2] = 3;
        string += F("<TR><TD>Adapter IC:<TD><select name='plugin_120_maxtype'>");
        for (byte x = 0; x < 2; x++)
        {
          string += F("<option value='");
          string += optionValues[x];
          string += "'";
          if (choice == optionValues[x])
            string += F(" selected");
          string += ">";
          string += options[x];
          string += F("</option>");
        }
        string += F("</select>");

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_120_maxtype");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // Get the MAX Type (6675 / 31855)
        // TBD ... Auswertung je nach Chip !!!
        byte MaxType = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

        // Get CS Pin
        // Konvert the GPIO Pin to a Dogotal Puin Number first ...
        Plugin_120_SPI_CS_Pin = Settings.TaskDevicePin1[event->TaskIndex];

        switch (MaxType) {
          case 1:       // MAX6675
            Plugin_120_Celsius = readMax6675();
            break;
          case 2:       // MAX31855
            Plugin_120_Celsius = readMax31855();
            break;
          case 3:       // MAX31865 (not implemented yet)
            //do something when var equals 2
            break;
        }

        if (Plugin_120_Celsius != NAN)
        {
          UserVar[event->BaseVarIndex] = Plugin_120_Celsius;
          UserVar[event->BaseVarIndex + 1] = CelsiusToFahrenheit(Plugin_120_Celsius);
          String log = F("P120 : C : ");
          log += UserVar[event->BaseVarIndex];
          log += F(" - F : ");
          log += UserVar[event->BaseVarIndex + 1];
          addLog(LOG_LEVEL_INFO, log);
          success = true;
        }
        else
        {
          UserVar[event->BaseVarIndex] = NAN;
          UserVar[event->BaseVarIndex + 1] = NAN;
          String log = F("P120 : No Sensor attached !");
          addLog(LOG_LEVEL_INFO, log);
          success = false;
        }

        break;
      }
  }
  return success;
}

double readMax6675()
{
  uint16_t rawvalue = 0;
  // take the SS pin low to select the chip:
  digitalWrite(Plugin_120_SPI_CS_Pin, LOW);
  // String log = F("P120 : CS Pin : ");
  // log += Plugin_120_SPI_CS_Pin;
  // addLog(LOG_LEVEL_INFO, log);
  // "transfer" 0x0 and read the Data from the Chip
  rawvalue = SPI.transfer16(0x0);
  // take the SS pin high to de-select the chip:
  digitalWrite(Plugin_120_SPI_CS_Pin, HIGH);

  String log = F("P120 : MAX6675 : RAW - BIN:");
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
  Plugin_120_SensorAttached = !(rawvalue & 0x0004);

  if (Plugin_120_SensorAttached)
  {
    // Shift RAW value 3 Bits to the right to get the data
    rawvalue >>= 3;

    // Calculate Celsius
    return rawvalue * 0.25;
  }
  else
  {
    return NAN;
  }
}

double readMax31855()
{
  uint32_t rawvalue = 0;
  // take the SS pin low to select the chip:
  digitalWrite(Plugin_120_SPI_CS_Pin, LOW);
  // "transfer" 0x0 and read the MSB Data from the Chip
  rawvalue = SPI.transfer16(0x0);
  // Shift MSB 16 Bits to the left
  rawvalue <<= 16;
  // "transfer" 0x0 and read the LSB Data from the Chip
  rawvalue |= SPI.transfer16(0x0);
  // take the SS pin high to de-select the chip:
  digitalWrite(Plugin_120_SPI_CS_Pin, HIGH);

  String log = F("P120 : MAX31855 : RAW - BIN:");
  log += String(rawvalue, BIN);
  log += " HEX:";
  log += String(rawvalue, HEX);
  log += " DEC:";
  log += String(rawvalue);
  addLog(LOG_LEVEL_DEBUG, log);

  // D16 - This bit reads at 1 when any of the SCV, SCG, or OC faults are active. Default value is 0.
  Plugin_120_SensorAttached = !(rawvalue & 0x00010000);

  if (Plugin_120_SensorAttached)
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
    if (rawvalue & 0x2000) // Bit 31=1 -> neg Values
    {
      // Negate all Bits
      rawvalue = ~rawvalue;
      // Add 1 and make negative
      rawvalue = (rawvalue + 1) * -1;
    }

    // Calculate Celsius
    return rawvalue * 0.25;
  }
  else
  {
    return NAN;
  }
}

// Convert Celsius to Fahrenheit
double CelsiusToFahrenheit(double celsius) {
  return celsius * 9.0 / 5.0 + 32;
}

#endif
