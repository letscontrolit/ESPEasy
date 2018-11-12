#ifdef USES_P074
//#######################################################################################################
//######################## Plugin 074 TSL2591 I2C Lux/IR Sensor #########################################
//#######################################################################################################
//
// by: https://github.com/krikk
// this plugin is based on the adafruit library
// written based on version 1.0.2 from https://github.com/adafruit/Adafruit_TSL2591_Library
// does need Adafruit Sensors Library
// added lux calculation improvement https://github.com/adafruit/Adafruit_TSL2591_Library/issues/14
// added fix for issue https://github.com/adafruit/Adafruit_TSL2591_Library/issues/17


#define PLUGIN_074
#define PLUGIN_ID_074        74
#define PLUGIN_NAME_074       "Light/Lux - TSL2591 [TESTING]"
#define PLUGIN_VALUENAME1_074 "Lux"
#define PLUGIN_VALUENAME2_074 "Full"
#define PLUGIN_VALUENAME3_074 "Visible"
#define PLUGIN_VALUENAME4_074 "IR"


#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif

Adafruit_TSL2591 tsl;
boolean TSL2591_initialized = false;

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
        Device[deviceCount].VType = SENSOR_TYPE_QUAD;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 4;
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
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_074));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_074));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_074));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        int optionValues[1] = { TSL2591_ADDR };
        addFormSelectorI2C(F("p074_i2c_addr"), 1, optionValues, TSL2591_ADDR);   //Only for display I2C address


//        tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
        // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
        // tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
        // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
        // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
        // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

        String optionsMode[6] = { F("100ms"), F("200ms"), F("300ms"), F("400ms"), F("500ms"), F("600ms") };
        addFormSelector(F("Integration Time"), F("p074_itime"), 6, optionsMode, NULL, CONFIG(1));


//        TSL2591_GAIN_LOW                  = 0x00,    // low gain (1x)
//        TSL2591_GAIN_MED                  = 0x10,    // medium gain (25x)
//        TSL2591_GAIN_HIGH                 = 0x20,    // medium gain (428x)
//        TSL2591_GAIN_MAX                  = 0x30,    // max gain (9876x)

        String optionsGain[4] = {
          F("low gain (1x)"),
          F("medium gain (25x)"),
          F("medium gain (428x)"),
          F("max gain (9876x)") };
        addFormSelector(F("Value Mapping"), F("p074_gain"), 4, optionsGain, NULL, CONFIG(2));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        //CONFIG(0) = getFormItemInt(F("p074_i2c_addr"));
        CONFIG(1) = getFormItemInt(F("p074_itime"));
        CONFIG(2) = getFormItemInt(F("p074_gain"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
      	tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)

        if (tsl.begin())
        {
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
          log += F(" ms");


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
  				log += (F(" Gain: "));
          tsl2591Gain_t gain = tsl.getGain();
          switch(gain)
          {
            case TSL2591_GAIN_LOW:
            	log += F("1x (Low)");
              break;
            case TSL2591_GAIN_MED:
            	log += F("25x (Medium)");
              break;
            case TSL2591_GAIN_HIGH:
            	log += F("428x (High)");
              break;
            case TSL2591_GAIN_MAX:
            	log += F("9876x (Max)");
              break;
          }

          TSL2591_initialized = true;
          addLog(LOG_LEVEL_INFO,log);

        }
        else
        {
        	TSL2591_initialized = false;
        	addLog(LOG_LEVEL_ERROR,F("TSL2591: No sensor found ... check your wiring?"));
        }


        success = true;
        break;
      }

    case PLUGIN_READ:
      {
      	if (TSL2591_initialized)
      	{
					// Simple data read example. Just read the infrared, fullspecrtrum diode
					// or 'visible' (difference between the two) channels.
					// This can take 100-600 milliseconds! Uncomment whichever of the following you want to read
					float lux, full, visible, ir;
					visible = tsl.getLuminosity(TSL2591_VISIBLE);
					ir = tsl.getLuminosity(TSL2591_INFRARED);
					full = tsl.getLuminosity(TSL2591_FULLSPECTRUM);
					lux = tsl.calculateLuxf(full, ir); // get LUX

					UserVar[event->BaseVarIndex + 0] = lux;
					UserVar[event->BaseVarIndex + 1] = full;
					UserVar[event->BaseVarIndex + 2] = visible;
					UserVar[event->BaseVarIndex + 3] = ir;

					String log = F("TSL2591: Lux: ");
					log += String(lux);
					log += F(" Full: ");
					log += String(full);
					log += F(" Visible: ");
					log += String(visible);
					log += F(" IR: ");
					log += String(ir);
					addLog(LOG_LEVEL_INFO,log);
          success = true;
      	}
      	else {
      		addLog(LOG_LEVEL_ERROR,F("TSL2591: Sensor not initialized!?"));
      	}
        break;
      }

  }
  return success;
}
#endif // USES_P074
