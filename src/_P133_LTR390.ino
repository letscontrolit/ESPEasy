#include "_Plugin_Helper.h"
#ifdef USES_P133

// #######################################################################################################
// ############################## Plugin 133 LTR390 I2C UV and Ambient Sensor ############################
// #######################################################################################################

// Changelog:
//
// 2022-03-26 tonhuisman: Initial plugin creation

# define PLUGIN_133
# define PLUGIN_ID_133         133
# define PLUGIN_NAME_133       "UV - LTR390"
# define PLUGIN_VALUENAME1_133 "UV"
# define PLUGIN_VALUENAME2_133 "UVIndex"
# define PLUGIN_VALUENAME3_133 "Ambient"
# define PLUGIN_VALUENAME4_133 "Lux"

# include "./src/PluginStructs/P133_data_struct.h"

boolean Plugin_133(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_133;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_133);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_133));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_133));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_133));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_133));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = event->Par1 == 0x53;
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = 0x53;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P133_SELECT_MODE   = static_cast<int>(P133_selectMode_e::DualMode);
      P133_UVGAIN        = LTR390_GAIN_3;
      P133_UVRESOLUTION  = LTR390_RESOLUTION_18BIT;
      P133_ALSGAIN       = LTR390_GAIN_3;
      P133_ALSRESOLUTION = LTR390_RESOLUTION_18BIT;
      P133_INITRESET     = 1;

      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0; // UVS and ALS values are integers
      ExtraTaskSettings.TaskDeviceValueDecimals[2] = 0;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *selectModeOptions[] = {
          F("Dual mode, read alternating UV/Ambient"),
          F("UV reading only"),
          F("Ambient reading only")
        };
        const int selectModeValues[] = {
          static_cast<int>(P133_selectMode_e::DualMode),
          static_cast<int>(P133_selectMode_e::UVMode),
          static_cast<int>(P133_selectMode_e::ALSMode)
        };
        addFormSelector(F("Read mode"), F("mode"), 3, selectModeOptions, selectModeValues, P133_SELECT_MODE, true);
      }

      const __FlashStringHelper *gainOptions[] = { F("1x"), F("3x"), F("6x"), F("9x"), F("18x") };
      const int gainValues[] = {
        LTR390_GAIN_1,
        LTR390_GAIN_3,
        LTR390_GAIN_6,
        LTR390_GAIN_9,
        LTR390_GAIN_18
      };

      const __FlashStringHelper *resolutionOptions[] = {
        F("20 bit"),
        F("19 bit"),
        F("18 bit"),
        F("17 bit"),
        F("16 bit"),
        F("13 bit")
      };
      const int resolutionValues[] = {
        LTR390_RESOLUTION_20BIT,
        LTR390_RESOLUTION_19BIT,
        LTR390_RESOLUTION_18BIT,
        LTR390_RESOLUTION_17BIT,
        LTR390_RESOLUTION_16BIT,
        LTR390_RESOLUTION_13BIT,
      };

      if (static_cast<P133_selectMode_e>(P133_SELECT_MODE) != P133_selectMode_e::ALSMode) {
        addFormSelector(F("UV Gain"),       F("uvgain"), 5, gainOptions,       gainValues,       P133_UVGAIN);
        addFormSelector(F("UV Resolution"), F("uvres"),  6, resolutionOptions, resolutionValues, P133_UVRESOLUTION);
      }

      if (static_cast<P133_selectMode_e>(P133_SELECT_MODE) != P133_selectMode_e::UVMode) {
        addFormSelector(F("Ambient Gain"),       F("alsgain"), 5, gainOptions,       gainValues,       P133_ALSGAIN);
        addFormSelector(F("Ambient Resolution"), F("alsres"),  6, resolutionOptions, resolutionValues, P133_ALSRESOLUTION);
      }

      addFormCheckBox(F("Reset sensor on init"), F("initreset"), P133_INITRESET == 1);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      if (static_cast<P133_selectMode_e>(P133_SELECT_MODE) != P133_selectMode_e::ALSMode) {
        P133_UVGAIN       = getFormItemInt(F("uvgain"));
        P133_UVRESOLUTION = getFormItemInt(F("uvres"));
      }

      if (static_cast<P133_selectMode_e>(P133_SELECT_MODE) != P133_selectMode_e::UVMode) {
        P133_ALSGAIN       = getFormItemInt(F("alsgain"));
        P133_ALSRESOLUTION = getFormItemInt(F("alsres"));
      }

      P133_SELECT_MODE = getFormItemInt(F("mode"));
      P133_INITRESET   = isFormItemChecked(F("initreset")) ? 1 : 0;

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P133_data_struct(static_cast<P133_selectMode_e>(P133_SELECT_MODE),
                                                                               static_cast<ltr390_gain_t>(P133_UVGAIN),
                                                                               static_cast<ltr390_resolution_t>(P133_UVRESOLUTION),
                                                                               static_cast<ltr390_gain_t>(P133_ALSGAIN),
                                                                               static_cast<ltr390_resolution_t>(P133_ALSRESOLUTION),
                                                                               P133_INITRESET == 1));
      P133_data_struct *P133_data = static_cast<P133_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P133_data) && P133_data->plugin_init(event);
      break;
    }

    case PLUGIN_READ:
    {
      P133_data_struct *P133_data = static_cast<P133_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P133_data) {
        success = P133_data->plugin_read(event);
      }

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P133_data_struct *P133_data = static_cast<P133_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P133_data) {
        success = P133_data->plugin_ten_per_second(event);
      }

      break;
    }
  }
  return success;
}

#endif // ifdef USES_P133
