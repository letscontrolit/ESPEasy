#ifdef USES_P050
//#######################################################################################################
//#################### Plugin 050 I2C TCS34725 RGB Color Sensor with IR filter and White LED ############
//#######################################################################################################
//
// RGB Color Sensor with IR filter and White LED
// like this one: https://www.adafruit.com/products/1334
// based on this library: https://github.com/adafruit/Adafruit_TCS34725
// this code is based on 20170331 date version of the above library
// this code is UNTESTED, because my TCS34725 sensor is still not shipped :(
//

#include "Adafruit_TCS34725.h"

#define PLUGIN_050
#define PLUGIN_ID_050        50
#define PLUGIN_NAME_050       "Color - TCS34725  [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_050 "Red"
#define PLUGIN_VALUENAME2_050 "Green"
#define PLUGIN_VALUENAME3_050 "Blue"
#define PLUGIN_VALUENAME4_050 "Color Temperature"




boolean Plugin_050(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_050;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_QUAD;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 4;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_050);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_050));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_050));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_050));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_050));
        break;
      }


    case PLUGIN_WEBFORM_LOAD:
      {
        byte choiceMode = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String optionsMode[6];
        optionsMode[0] = F("TCS34725_INTEGRATIONTIME_2_4MS");
        optionsMode[1] = F("TCS34725_INTEGRATIONTIME_24MS");
        optionsMode[2] = F("TCS34725_INTEGRATIONTIME_50MS");
        optionsMode[3] = F("TCS34725_INTEGRATIONTIME_101MS");
        optionsMode[4] = F("TCS34725_INTEGRATIONTIME_154MS");
        optionsMode[5] = F("TCS34725_INTEGRATIONTIME_700MS");
        int optionValuesMode[6];
        optionValuesMode[0] = TCS34725_INTEGRATIONTIME_2_4MS;
        optionValuesMode[1] = TCS34725_INTEGRATIONTIME_24MS;
        optionValuesMode[2] = TCS34725_INTEGRATIONTIME_50MS;
        optionValuesMode[3] = TCS34725_INTEGRATIONTIME_101MS;
        optionValuesMode[4] = TCS34725_INTEGRATIONTIME_154MS;
        optionValuesMode[5] = TCS34725_INTEGRATIONTIME_700MS;
        addFormSelector(F("Integration Time"), F("plugin_050_integrationTime"), 6, optionsMode, optionValuesMode, choiceMode);

        byte choiceMode2 = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        String optionsMode2[4];
        optionsMode2[0] = F("TCS34725_GAIN_1X");
        optionsMode2[1] = F("TCS34725_GAIN_4X");
        optionsMode2[2] = F("TCS34725_GAIN_16X");
        optionsMode2[3] = F("TCS34725_GAIN_60X");
        int optionValuesMode2[4];
        optionValuesMode2[0] = TCS34725_GAIN_1X;
        optionValuesMode2[1] = TCS34725_GAIN_4X;
        optionValuesMode2[2] = TCS34725_GAIN_16X;
        optionValuesMode2[3] = TCS34725_GAIN_60X;
        addFormSelector(F("Gain"), F("plugin_050_gain"), 4, optionsMode2, optionValuesMode2, choiceMode2);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg(F("plugin_050_integrationTime"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        String plugin2 = WebServer.arg(F("plugin_050_gain"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin2.toInt();

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
      	tcs34725IntegrationTime_t integrationTime;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0]==TCS34725_INTEGRATIONTIME_2_4MS)
        	integrationTime = TCS34725_INTEGRATIONTIME_2_4MS;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0]==TCS34725_INTEGRATIONTIME_24MS)
        	integrationTime = TCS34725_INTEGRATIONTIME_24MS;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0]==TCS34725_INTEGRATIONTIME_50MS)
        	integrationTime = TCS34725_INTEGRATIONTIME_50MS;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0]==TCS34725_INTEGRATIONTIME_101MS)
        	integrationTime = TCS34725_INTEGRATIONTIME_101MS;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0]==TCS34725_INTEGRATIONTIME_154MS)
        	integrationTime = TCS34725_INTEGRATIONTIME_154MS;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0]==TCS34725_INTEGRATIONTIME_700MS)
        	integrationTime = TCS34725_INTEGRATIONTIME_700MS;

        tcs34725Gain_t gain;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1]==TCS34725_GAIN_1X)
        	gain = TCS34725_GAIN_1X;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1]==TCS34725_GAIN_4X)
        	gain = TCS34725_GAIN_4X;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1]==TCS34725_GAIN_16X)
        	gain = TCS34725_GAIN_16X;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1]==TCS34725_GAIN_60X)
        	gain = TCS34725_GAIN_60X;

      	/* Initialise with specific int time and gain values */
      	Adafruit_TCS34725 tcs = Adafruit_TCS34725(integrationTime, gain);
        if (tcs.begin()) {

        	addLog(LOG_LEVEL_DEBUG, F("Found TCS34725 sensor"));

          uint16_t r, g, b, c;

          tcs.getRawData(&r, &g, &b, &c);
          tcs.calculateColorTemperature(r, g, b);
          tcs.calculateLux(r, g, b);

          UserVar[event->BaseVarIndex] = r;
          UserVar[event->BaseVarIndex + 1] = g;
          UserVar[event->BaseVarIndex + 2] = b;
          UserVar[event->BaseVarIndex + 3] = tcs.calculateColorTemperature(r, g, b);

          String log = F("TCS34725: Color Temp (K): ");
          log += String(UserVar[event->BaseVarIndex + 3], DEC);
          log += F(" R: ");
          log += String(UserVar[event->BaseVarIndex], DEC);
          log += F(" G: ");
          log += String(UserVar[event->BaseVarIndex + 1], DEC);
          log += F(" B: ");
          log += String(UserVar[event->BaseVarIndex + 2], DEC);
          addLog(LOG_LEVEL_INFO, log);
          success = true;

        } else {
        	addLog(LOG_LEVEL_DEBUG, F("No TCS34725 found"));
        	success = false;
        }

        break;
      }

  }
  return success;
}


#endif // USES_P050
