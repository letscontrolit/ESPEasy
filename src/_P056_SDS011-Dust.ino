//#######################################################################################################
//#################################### Plugin 056: Dust Sensor SDS011 / SDS018 ##########################
//#######################################################################################################
/*
  Plugin is based upon SDS011 dust sensor PM2.5 and PM10 lib
  This plugin and lib was written by Jochen Krapf (jk@nerd2nerd.org)

  This plugin reads the particle concentration from SDS011 Sensor
  DevicePin1 - RX on ESP, TX on SDS
*/

#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_056
#define PLUGIN_ID_056         56
#define PLUGIN_NAME_056       "Dust - SDS011/018/198 [TESTING]"
#define PLUGIN_VALUENAME1_056 "PM2.5"   // Dust <2.5µm in µg/m³   SDS198:<100µm in µg/m³
#define PLUGIN_VALUENAME2_056 "PM10"    // Dust <10µm in µg/m³

#include <jkSDS011.h>


CjkSDS011 *Plugin_056_SDS = NULL;


boolean Plugin_056(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_056;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_DUAL;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_056);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_056));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_056));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = F("GPIO &larr; TX");
        //event->String2 = F("GPIO &#8674; RX (optional)");
        break;
      }

    case PLUGIN_INIT:
      {
        if (Plugin_056_SDS)
          delete Plugin_056_SDS;
        Plugin_056_SDS = new CjkSDS011(Settings.TaskDevicePin1[event->TaskIndex], -1);
        addLog(LOG_LEVEL_INFO, F("SDS  : Init OK "));

        success = true;
        break;
      }

    case PLUGIN_EXIT:
      {
        // //FIXME: if this plugin is used more than once at the same time, things go horribly wrong :)
        //
        // if (Plugin_056_SDS)
        //   delete Plugin_056_SDS;
        // addLog(LOG_LEVEL_INFO, F("SDS  : Exit"));
        shouldReboot=true;
        break;
      }

    case PLUGIN_FIFTY_PER_SECOND:
      {
        if (!Plugin_056_SDS)
          break;

        Plugin_056_SDS->Process();

        if (Plugin_056_SDS->available())
        {
          String log = F("SDS  : act ");
          log += Plugin_056_SDS->GetPM2_5();
          log += F(" ");
          log += Plugin_056_SDS->GetPM10_();
          addLog(LOG_LEVEL_DEBUG, log);

          if (Settings.TaskDeviceTimer[event->TaskIndex] == 0)
          {
            UserVar[event->BaseVarIndex + 0] = Plugin_056_SDS->GetPM2_5();
            UserVar[event->BaseVarIndex + 1] = Plugin_056_SDS->GetPM10_();
            event->sensorType = SENSOR_TYPE_DUAL;
            sendData(event);
          }
        }

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        if (!Plugin_056_SDS)
          break;

        float pm25, pm10;
        Plugin_056_SDS->ReadAverage(pm25, pm10);

        UserVar[event->BaseVarIndex + 0] = pm25;
        UserVar[event->BaseVarIndex + 1] = pm10;
        success = true;
        break;
      }
  }

  return success;
}

#endif   //PLUGIN_BUILD_TESTING
