//#######################################################################################################
//######################## Plugin 074 TSL2591 I2C Lux Sensor ############################################
//#######################################################################################################
//
// by: https://github.com/krikk
// this plugin is based on the sparkfun library
// written based on version 1.0.2 from https://github.com/adafruit/Adafruit_TSL2591_Library

#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_074
#define PLUGIN_ID_074        74
#define PLUGIN_NAME_074       "Light/Lux - TSL2591 [TESTING]"
#define PLUGIN_VALUENAME1_074 "Lux"


#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif

Adafruit_TSL2591 tsl;

boolean Plugin_074(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_074;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = false;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_074);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_074));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        int optionValues[1] = { TSL2591_ADDR };
        addFormSelectorI2C(string, F("plugin_074_i2c_addr"), 1, optionValues, TSL2591_ADDR);   //Only for display I2C address


//        tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
        // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
        // tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
        // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
        // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
        // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

        String optionsMode[6] = { F("100ms"), F("200ms"), F("300ms"), F("400ms"), F("500ms"), F("600ms") };
        addFormSelector(string, F("Integration Time"), F("plugin_074_itime"), 6, optionsMode, NULL, CONFIG(1));


//        TSL2591_GAIN_LOW                  = 0x00,    // low gain (1x)
//        TSL2591_GAIN_MED                  = 0x10,    // medium gain (25x)
//        TSL2591_GAIN_HIGH                 = 0x20,    // medium gain (428x)
//        TSL2591_GAIN_MAX                  = 0x30,    // max gain (9876x)

        String optionsGain[4] = {
          F("low gain (1x)"),
          F("medium gain (25x)"),
          F("medium gain (428x)"),
          F("max gain (9876x)") };
        addFormSelector(string, F("Value Mapping"), F("plugin_074_gain"), 4, optionsGain, NULL, CONFIG(2));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        //CONFIG(0) = getFormItemInt(F("plugin_074_i2c_addr"));
        CONFIG(1) = getFormItemInt(F("plugin_074_itime"));
        CONFIG(2) = getFormItemInt(F("plugin_074_gain"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
      	tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)

        Serial.println(F("Starting Adafruit TSL2591 Test!"));

        if (tsl.begin())
        {
          Serial.println(F("Found a TSL2591 sensor"));
        }
        else
        {
          Serial.println(F("No sensor found ... check your wiring?"));
          while (1);
        }

        String log = F("TSL2591: Address: 0x");
        log += String(TSL2591_ADDR,HEX);


        // Changing the integration time gives you a longer time over which to sense light
        // longer timelines are slower, but are good in very low light situtations!
        switch (CONFIG(1))
        {
          default:
          case 0:
          {
          	tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);
            break;
          }
          case 1:
          {
          	tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
            break;
          }
          case 2:
          {
          	tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
            break;
          }
          case 3:
          {
          	tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
            break;
          }
          case 4:
          {
          	tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
            break;
          }
          case 5:
          {
          	tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);
            break;
          }
        }

        log += F(": Integration Time: ");
        log += String((tsl.getTiming() + 1) * 100, DEC);
        addLog(LOG_LEVEL_INFO,log);

				// You can change the gain on the fly, to adapt to brighter/dimmer light situations
				switch (CONFIG(2))
				{
					default:
					case 0:
					{
						tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
						break;
					}
					case 1:
					{
						tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
						break;
					}
					case 2:
					{
						tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
						break;
					}
					case 3:
					{
						tsl.setGain(TSL2591_GAIN_MAX);   // 9876x gain
						break;
					}
				}



        /* Display the gain and integration time for reference sake */
        Serial.println(F("------------------------------------"));
        Serial.print  (F("Gain:         "));
        tsl2591Gain_t gain = tsl.getGain();
        switch(gain)
        {
          case TSL2591_GAIN_LOW:
            Serial.println(F("1x (Low)"));
            break;
          case TSL2591_GAIN_MED:
            Serial.println(F("25x (Medium)"));
            break;
          case TSL2591_GAIN_HIGH:
            Serial.println(F("428x (High)"));
            break;
          case TSL2591_GAIN_MAX:
            Serial.println(F("9876x (Max)"));
            break;
        }
        Serial.print  (F("Timing:       "));
        Serial.print((tsl.getTiming() + 1) * 100, DEC);
        Serial.println(F(" ms"));
        Serial.println(F("------------------------------------"));
        Serial.println(F(""));

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // Simple data read example. Just read the infrared, fullspecrtrum diode
        // or 'visible' (difference between the two) channels.
        // This can take 100-600 milliseconds! Uncomment whichever of the following you want to read
      	UserVar[event->BaseVarIndex + 0] = tsl.getLuminosity(TSL2591_VISIBLE);
        //uint16_t x = tsl.getLuminosity(TSL2591_FULLSPECTRUM);
        //uint16_t x = tsl.getLuminosity(TSL2591_INFRARED);

        Serial.print(F("[ ")); Serial.print(millis()); Serial.print(F(" ms ] "));
        Serial.print(F("Luminosity: "));
        Serial.println(UserVar[event->BaseVarIndex + 0], DEC);
      }

  }
  return success;
}
#endif
