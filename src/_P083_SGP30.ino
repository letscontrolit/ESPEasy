#ifdef USES_P083

/*********************************************************************************************\
* SGP30 - Gas (TVOC - Total Volatile Organic Compounds) and Air Quality (eCO2)
*
*
* I2C Address: 0x58
\*********************************************************************************************/
#include "Adafruit_SGP30.h"
#include "_Plugin_Helper.h"


#define PLUGIN_083
#define PLUGIN_ID_083        83
#define PLUGIN_NAME_083       "Gasses - SGP30 [TESTING]"
#define PLUGIN_VALUENAME1_083 "TVOC"
#define PLUGIN_VALUENAME2_083 "eCO2"


#define P083_TVOC (event->BaseVarIndex + 0)
#define P083_ECO2 (event->BaseVarIndex + 1)
#define P083_TVOC_BASELINE (event->BaseVarIndex + 2)
#define P083_ECO2_BASELINE (event->BaseVarIndex + 3)


Adafruit_SGP30 sgp;
unsigned long  Plugin_083_init_time = 0;
bool Plugin_083_init                = false;
bool Plugin_083_newValues           = false;

boolean Plugin_083(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_083;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = SENSOR_TYPE_DUAL;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_083);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_083));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_083));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Sensor"));
      addRowLabel(F("Sensor State"));
      addHtml(Plugin_083_init ? F("Initialized") : F("-"));
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      Plugin_083_init      = sgp.begin();
      Plugin_083_init_time = millis();

      if (!Plugin_083_init) {
        addLog(LOG_LEVEL_ERROR, F("SGP30: Sensor not found"))
      } else {
        // Look at the stored base line values to see if we can restore them.
        uint16_t eco2_base = UserVar[P083_TVOC_BASELINE];
        uint16_t tvoc_base = UserVar[P083_ECO2_BASELINE];

        if ((eco2_base != 0) && (tvoc_base != 0)) {
          addLog(LOG_LEVEL_INFO, F("SGP30: Restore last known baseline values"));
          sgp.setIAQBaseline(eco2_base, tvoc_base);
        }
      }

      success = Plugin_083_init;
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      if (Plugin_083_init)
      {
        if (sgp.IAQmeasure())
        {
          UserVar[P083_TVOC] = sgp.TVOC;
          UserVar[P083_ECO2] = sgp.eCO2;
          success            = true;

          // For the first 15s after the sgp30_iaq_init command the sensor is
          // in an initialization phase during which a sgp30_measure_iaq command
          // returns fixed values of 400 ppm CO2eq and 0 ppb TVOC.
          if ((timePassedSince(Plugin_083_init_time) > 15000) || ((sgp.TVOC != 0) && (sgp.eCO2 != 400))) {
            Plugin_083_newValues = true;
          }
        }
      }
      break;
    }

    case PLUGIN_READ:
    {
      if (Plugin_083_init)
      {
        if (Plugin_083_newValues)
        {
          Plugin_083_newValues = false;

          // Keep track of the base line and store it in the unused uservar values.
          // When starting we will restore them so a crash or reboot will not result in strange values.
          uint16_t eco2_base, tvoc_base;

          if (sgp.getIAQBaseline(&eco2_base, &tvoc_base)) {
            UserVar[P083_TVOC_BASELINE] = eco2_base;
            UserVar[P083_ECO2_BASELINE] = tvoc_base;
          }


          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("SGP30: TVOC: ");
            log += UserVar[P083_TVOC];
            addLog(LOG_LEVEL_INFO, log);
            log  = F("SGP30: eCO2: ");
            log += UserVar[P083_ECO2];
            addLog(LOG_LEVEL_INFO, log);
          }
          success = true;
          break;
        } else {
          addLog(LOG_LEVEL_ERROR, F("SGP30: No new measured values"))
          break;
        }
      } else {
        addLog(LOG_LEVEL_ERROR, F("SGP30: Sensor not found"))
        break;
      }
    }
  }
  return success;
}

#endif // USES_P083
