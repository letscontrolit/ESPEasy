#include "_Plugin_Helper.h"
#ifdef USES_P056

// #######################################################################################################
// #################################### Plugin 056: Dust Sensor SDS011 / SDS018 ##########################
// #######################################################################################################

/*
   Plugin is based upon SDS011 dust sensor PM2.5 and PM10 lib
   This plugin and lib was written by Jochen Krapf (jk@nerd2nerd.org)

   This plugin reads the particle concentration from SDS011 Sensor
   DevicePin1 - RX on ESP, TX on SDS
   DevicePin2 - TX on ESP, RX on SDS, optional, for setting the sleep time
 */

/** Changelog:
 * 2025-08-05 tonhuisman: Introduce multi-instance use
 * 2025-01-12 tonhuisman: Add support for MQTT AutoDiscovery
 */

# define PLUGIN_056
# define PLUGIN_ID_056         56
# define PLUGIN_NAME_056       "Dust - SDS011/018/198"
# define PLUGIN_VALUENAME1_056 "PM2.5" // Dust <2.5µm in µg/m³   SDS198:<100µm in µg/m³
# define PLUGIN_VALUENAME2_056 "PM10"  // Dust <10µm in µg/m³

# include "src/PluginStructs/P056_data_struct.h"


boolean Plugin_056(uint8_t function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_056;
      dev.Type           = DEVICE_TYPE_SERIAL;
      dev.VType          = Sensor_VType::SENSOR_TYPE_DUAL;
      dev.FormulaOption  = true;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;

      // Keep alive for setting the sleep time on save, only when changed
      dev.ExitTaskBeforeSave = false;
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

    # if FEATURE_MQTT_DISCOVER
    case PLUGIN_GET_DISCOVERY_VTYPES:
    {
      event->Par1 = static_cast<int>(Sensor_VType::SENSOR_TYPE_DUSTPM2_5_ONLY);
      event->Par2 = static_cast<int>(Sensor_VType::SENSOR_TYPE_DUSTPM10_ONLY);
      success     = true;
      break;
    }
    # endif // if FEATURE_MQTT_DISCOVER

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
      if (validGpio(CONFIG_PIN2)) {
        addFormNumericBox(F("Sleep time"), F("sleeptime"),
                          PCONFIG(0),
                          0, 30);
        addUnit(F("Minutes"));
        addFormNote(F("0 = continous, 1..30 = Work 30 seconds and sleep n*60-30 seconds"));
      }
      break;
    }
    case PLUGIN_WEBFORM_SAVE:
    {
      if (validGpio(CONFIG_PIN2)) {
        // Communications to device should work.
        const int newsleeptime = getFormItemInt(F("sleeptime"));

        if (PCONFIG(0) != newsleeptime) {
          PCONFIG(0) = newsleeptime;
          Plugin_056_setWorkingPeriod(event, newsleeptime);
        }
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P056_data_struct(event));
      P056_data_struct *P056_data =
        static_cast<P056_data_struct *>(getPluginTaskData(event->TaskIndex));
      success = (nullptr != P056_data && P056_data->isInitialized());

      break;
    }

    case PLUGIN_EXIT:
    {
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P056_data_struct *P056_data =
        static_cast<P056_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr == P056_data) || !P056_data->isInitialized()) {
        break;
      }

      P056_data->Process();

      if (P056_data->available())
      {
        const float pm2_5 = P056_data->GetPM2_5();
        const float pm10  = P056_data->GetPM10_();
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLog(LOG_LEVEL_DEBUG, strformat(F("SDS  : act %.2f %.2f"), pm2_5, pm10));
        }
        # endif // ifndef BUILD_NO_DEBUG

        if (Settings.TaskDeviceTimer[event->TaskIndex] == 0)
        {
          UserVar.setFloat(event->TaskIndex, 0, pm2_5);
          UserVar.setFloat(event->TaskIndex, 1, pm10);
          event->sensorType = Sensor_VType::SENSOR_TYPE_DUAL;
          sendData(event);
        }
      }

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      P056_data_struct *P056_data =
        static_cast<P056_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr == P056_data) || !P056_data->isInitialized()) {
        break;
      }

      float pm25{};
      float pm10{};

      if (P056_data->ReadAverage(pm25, pm10)) {
        UserVar.setFloat(event->TaskIndex, 0, pm25);
        UserVar.setFloat(event->TaskIndex, 1, pm10);
        success = true;
      }
      break;
    }
  }

  return success;
}

String Plugin_056_ErrorToString(int error) {
  String log;

  if (error < 0) {
    log =  concat(F("comm error: "), error);
  }
  return log;
}

String Plugin_056_WorkingPeriodToString(int workingPeriod) {
  if (workingPeriod < 0) {
    return Plugin_056_ErrorToString(workingPeriod);
  }
  String log;

  if (workingPeriod > 0) {
    log = strformat(F("%d minutes"), workingPeriod);
  } else {
    log = F(" continuous");
  }
  return log;
}

void Plugin_056_setWorkingPeriod(EventStruct *event, int minutes) {
  P056_data_struct *P056_data =
    static_cast<P056_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P056_data) || !P056_data->isInitialized()) {
    return;
  }
  P056_data->SetWorkingPeriod(minutes);

#ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(F("SDS  : Working Period set to: "), Plugin_056_WorkingPeriodToString(minutes)));
  }
#endif
}

// #endif
#endif // USES_P056
