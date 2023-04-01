#include "_Plugin_Helper.h"
#ifdef USES_P049

# include "src/PluginStructs/P049_data_struct.h"

/*

   This plug in is written by Dmitry (rel22 ___ inbox.ru)
   Plugin is based upon SenseAir plugin by Daniel Tedenljung info__AT__tedenljungconsulting.com
   Additional features based on https://geektimes.ru/post/285572/ by Gerben (infernix__AT__gmail.com)

   This plugin reads the CO2 value from MH-Z19 NDIR Sensor

   Pin-out:
   Hd o
   SR o   o PWM
   Tx o   o AOT
   Rx o   o GND
   Vo o   o Vin
   (bottom view)
   Skipping pin numbers due to inconsistancies in individual data sheet revisions.
   MHZ19:  Connection:
   VCC     5 V
   GND     GND
   Tx      ESP8266 1st GPIO specified in Device-settings
   Rx      ESP8266 2nd GPIO specified in Device-settings
 */

# define PLUGIN_049
# define PLUGIN_ID_049         49
# define PLUGIN_NAME_049       "Gases - CO2 MH-Z19"
# define PLUGIN_VALUENAME1_049 "PPM"
# define PLUGIN_VALUENAME2_049 "Temperature" // Temperature in C
# define PLUGIN_VALUENAME3_049 "U"           // Undocumented, minimum measurement per time period?


boolean Plugin_049(uint8_t function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_049;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_049);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_049));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_049));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_049));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_getGpioNames(event);
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
      {
        const __FlashStringHelper *options[2] = { F("Normal"), F("ABC disabled") };
        const int optionValues[2]             = { P049_ABC_enabled, P049_ABC_disabled };
        addFormSelector(F("Auto Base Calibration"), F("abcdisable"), 2, options, optionValues, PCONFIG(0));
      }
      {
        const __FlashStringHelper *filteroptions[5] =
        { F("Skip Unstable"), F("Use Unstable"), F("Fast Response"), F("Medium Response"), F("Slow Response") };
        const int filteroptionValues[5] = {
          PLUGIN_049_FILTER_OFF,
          PLUGIN_049_FILTER_OFF_ALLSAMPLES,
          PLUGIN_049_FILTER_FAST,
          PLUGIN_049_FILTER_MEDIUM,
          PLUGIN_049_FILTER_SLOW };
        addFormSelector(F("Filter"), F("filter"), 5, filteroptions, filteroptionValues, PCONFIG(1));
      }
      P049_html_show_stats(event);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("abcdisable"));

      P049_data_struct *P049_data = static_cast<P049_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P049_data) {
        P049_data->setABCmode(PCONFIG(0));
      }
      PCONFIG(1) = getFormItemInt(F("filter"));
      success    = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P049_data_struct());
      success = P049_perform_init(event);

      break;
    }

    case PLUGIN_EXIT: {
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      P049_data_struct *P049_data = static_cast<P049_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P049_data) {
        success = P049_data->plugin_write(event, string);
      }

      break;
    }

    case PLUGIN_READ:
    {
      P049_data_struct *P049_data = static_cast<P049_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P049_data) {
        return success;
      }
      bool expectReset  = false;
      unsigned int ppm  = 0;
      signed int   temp = 0;
      unsigned int s    = 0;
      float u           = 0;

      if (P049_data->read_ppm(ppm, temp, s, u)) {
        const bool mustLog = loglevelActiveFor(LOG_LEVEL_INFO);
        String     log;

        if (mustLog) {
          log = F("MHZ19: ");
        }

        // During (and only ever at) sensor boot, 'u' is reported as 15000
        // We log but don't process readings during that time
        if (approximatelyEqual(u, 15000)) {
          if (mustLog) {
            log += F("Bootup detected! ");
          }

          if (P049_data->ABC_Disable) {
            // After bootup of the sensor the ABC will be enabled.
            // Thus only actively disable after bootup.
            P049_data->ABC_MustApply = true;

            if (mustLog) {
              log += F("Will disable ABC when bootup complete. ");
            }
          }
          success = false;

          // Finally, stable readings are used for variables
        } else {
          const int filterValue = PCONFIG(1);

          if (Plugin_049_Check_and_ApplyFilter(UserVar[event->BaseVarIndex], ppm, s, filterValue, log)) {
            UserVar[event->BaseVarIndex]     = ppm;
            UserVar[event->BaseVarIndex + 1] = temp;
            UserVar[event->BaseVarIndex + 2] = u;
            success                          = true;
          } else {
            success = false;
          }
        }

        if ((s == 0) || (s == 64)) {
          // Reading is stable.
          if (P049_data->ABC_MustApply) {
            // Send ABC enable/disable command based on the desired state.
            String log = F("MHZ19: Sent sensor ABC ");

            if (P049_data->ABC_Disable) {
              P049_data->send_mhzCmd(mhzCmdABCDisable);
              log += F("Disable!");
            } else {
              P049_data->send_mhzCmd(mhzCmdABCEnable);
              log += F("Enable!");
            }
            addLog(LOG_LEVEL_INFO, log);
            P049_data->ABC_MustApply = false;
          }
        }

        if (mustLog) {
          // Log values in all cases
          log += F("PPM value: ");
          log += ppm;
          log += F(" Temp/S/U values: ");
          log += temp;
          log += '/';
          log += s;
          log += '/';
          log += u;
          addLogMove(LOG_LEVEL_INFO, log);
        }
        break;

        // #ifdef ENABLE_DETECTION_RANGE_COMMANDS
        // Sensor responds with 0x99 whenever we send it a measurement range adjustment
      } else if (P049_data->receivedCommandAcknowledgement(expectReset))  {
        addLog(LOG_LEVEL_INFO, F("MHZ19: Received command acknowledgment! "));

        if (expectReset) {
          addLog(LOG_LEVEL_INFO, F("Expecting sensor reset..."));
        }
        success = false;
        break;

        // #endif

        // log verbosely anything else that the sensor reports
      } else {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("MHZ19: Unknown response:");
          log += P049_data->getBufferHexDump();
          addLogMove(LOG_LEVEL_INFO, log);
        }

        // Check for stable reads and allow unstable reads the first 3 minutes after reset.
        if ((P049_data->nrUnknownResponses > 10) && P049_data->initTimePassed) {
          P049_perform_init(event);
        }
        success = false;
        break;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P049
