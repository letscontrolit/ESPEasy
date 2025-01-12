#include "_Plugin_Helper.h"

#ifdef USES_P010

// #######################################################################################################
// #################################### Plugin-010: LuxRead   ############################################
// #######################################################################################################


# include <AS_BH1750.h>

# define PLUGIN_010
# define PLUGIN_ID_010         10
# define PLUGIN_NAME_010       "Light/Lux - BH1750"
# define PLUGIN_VALUENAME1_010 "Lux"


boolean Plugin_010(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_010;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.FormulaOption  = true;
      dev.ValueCount     = 1;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_010);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_010));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { BH1750_DEFAULT_I2CADDR, BH1750_SECOND_I2CADDR };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 2, i2cAddressValues, PCONFIG(0));
        addFormNote(F("ADDR Low=0x23, High=0x5c"));
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = PCONFIG(0);
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      const __FlashStringHelper *optionsMode[] = {
        F("RESOLUTION_LOW"),
        F("RESOLUTION_NORMAL"),
        F("RESOLUTION_HIGH"),
        F("RESOLUTION_AUTO_HIGH"),
      };
      const int optionValuesMode[] = {
        RESOLUTION_LOW,
        RESOLUTION_NORMAL,
        RESOLUTION_HIGH,
        RESOLUTION_AUTO_HIGH,
      };
      constexpr size_t optionCount = NR_ELEMENTS(optionValuesMode);
      addFormSelector(F("Measurement mode"), F("pmode"), optionCount, optionsMode, optionValuesMode, PCONFIG(1));

      addFormCheckBox(F("Send sensor to sleep"), F("psleep"), PCONFIG(2));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));
      PCONFIG(1) = getFormItemInt(F("pmode"));
      PCONFIG(2) = isFormItemChecked(F("psleep"));
      success    = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      AS_BH1750 sensor = AS_BH1750(PCONFIG(0));

      // replaced the 8 lines below to optimize code
      sensors_resolution_t mode = static_cast<sensors_resolution_t>(PCONFIG(1));

      // if (PCONFIG(1)==RESOLUTION_LOW)
      //        mode = RESOLUTION_LOW;
      // if (PCONFIG(1)==RESOLUTION_NORMAL)
      //        mode = RESOLUTION_NORMAL;
      // if (PCONFIG(1)==RESOLUTION_HIGH)
      //        mode = RESOLUTION_HIGH;
      // if (PCONFIG(1)==RESOLUTION_AUTO_HIGH)
      //        mode = RESOLUTION_AUTO_HIGH;

      sensor.begin(mode, PCONFIG(2) == 1);

      const float lux = sensor.readLightLevel();

      if (lux != -1.0f) {
        UserVar.setFloat(event->TaskIndex, 0, lux);

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO,
                 strformat(F("BH1750 Address: 0x%02x Mode: 0x%02x : Light intensity: %s"),
                           PCONFIG(0),  PCONFIG(1),
                           formatUserVarNoCheck(event, 0).c_str()));
        }
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P010
