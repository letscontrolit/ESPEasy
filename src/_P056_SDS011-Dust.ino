#include "_Plugin_Helper.h"
#ifdef USES_P056
//#######################################################################################################
//#################################### Plugin 056: Dust Sensor SDS011 / SDS018 ##########################
//#######################################################################################################
/*
  Plugin is based upon SDS011 dust sensor PM2.5 and PM10 lib
  This plugin and lib was written by Jochen Krapf (jk@nerd2nerd.org)

  This plugin reads the particle concentration from SDS011 Sensor
  DevicePin1 - RX on ESP, TX on SDS
*/
//#ifdef ESP8266  // Needed for precompile issues.

#define PLUGIN_056
#define PLUGIN_ID_056         56
#define PLUGIN_NAME_056       "Dust - SDS011/018/198"
#define PLUGIN_VALUENAME1_056 "PM2.5"   // Dust <2.5µm in µg/m³   SDS198:<100µm in µg/m³
#define PLUGIN_VALUENAME2_056 "PM10"    // Dust <10µm in µg/m³

#include <jkSDS011.h>
#include "ESPEasy-Globals.h"


CjkSDS011 *Plugin_056_SDS = NULL;


boolean Plugin_056(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_056;
        Device[deviceCount].Type = DEVICE_TYPE_SERIAL;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_DUAL;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = false;
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
        serialHelper_getGpioNames(event, false, true); // TX optional
        break;
      }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
      {
        string += serialHelper_getSerialTypeLabel(event);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {

        // FIXME TD-er:  Whether TX pin is connected should be set somewhere
        if (Plugin_056_hasTxPin(event)) {
          addFormNumericBox(F("Sleep time"), F("p056_sleeptime"),
                            PCONFIG(0),
                            0, 30);
          addUnit(F("Minutes"));
          addFormNote(F("0 = continous, 1..30 = Work 30 seconds and sleep n*60-30 seconds"));
        }
        break;
      }
      case PLUGIN_WEBFORM_SAVE:
        {

          if (Plugin_056_hasTxPin(event)) {
            // Communications to device should work.
            const int newsleeptime = getFormItemInt(F("p056_sleeptime"));
            if (PCONFIG(0) != newsleeptime) {
              PCONFIG(0) = getFormItemInt(F("p056_sleeptime"));
              Plugin_056_setWorkingPeriod(newsleeptime);
            }
          }
          success = true;
          break;
        }

    case PLUGIN_INIT:
      {
        if (Plugin_056_SDS)
          delete Plugin_056_SDS;
        const int16_t serial_rx = CONFIG_PIN1;
        const int16_t serial_tx = CONFIG_PIN2;
        const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
        Plugin_056_SDS = new CjkSDS011(port, serial_rx, serial_tx);
        String log = F("SDS  : Init OK  ESP GPIO-pin RX:");
        log += serial_rx;
        log += F(" TX:");
        log += serial_tx;
        addLog(LOG_LEVEL_INFO, log);

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
          const float pm2_5 = Plugin_056_SDS->GetPM2_5();
          const float pm10 = Plugin_056_SDS->GetPM10_();
          String log = F("SDS  : act ");
          log += pm2_5;
          log += ' ';
          log += pm10;
          addLog(LOG_LEVEL_DEBUG, log);

          if (Settings.TaskDeviceTimer[event->TaskIndex] == 0)
          {
            UserVar[event->BaseVarIndex + 0] = pm2_5;
            UserVar[event->BaseVarIndex + 1] = pm10;
            event->sensorType = Sensor_VType::SENSOR_TYPE_DUAL;
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
        if (Plugin_056_SDS->ReadAverage(pm25, pm10)) {
          UserVar[event->BaseVarIndex + 0] = pm25;
          UserVar[event->BaseVarIndex + 1] = pm10;
          success = true;
        }
        break;
      }
  }

  return success;
}

boolean Plugin_056_hasTxPin(struct EventStruct *event) {
  const int16_t serial_tx = CONFIG_PIN2;
  return serial_tx >= 0;
}

String Plugin_056_ErrorToString(int error) {
  String log;
  if (error < 0) {
    log =  F("comm error: ");
    log += error;
  }
  return log;
}

String Plugin_056_WorkingPeriodToString(int workingPeriod) {
  if (workingPeriod < 0) {
    return Plugin_056_ErrorToString(workingPeriod);
  }
  String log;
  if (workingPeriod > 0) {
    log += workingPeriod;
    log += F(" minutes");
  } else {
    log += F(" continuous");
  }
  return log;
}

void Plugin_056_setWorkingPeriod(int minutes) {
  if (!Plugin_056_SDS)
    return;
  Plugin_056_SDS->SetWorkingPeriod(minutes);
  String log = F("SDS  : Working Period set to: ");
  log += Plugin_056_WorkingPeriodToString(minutes);
  addLog(LOG_LEVEL_INFO, log);
}

//#endif
#endif // USES_P056
