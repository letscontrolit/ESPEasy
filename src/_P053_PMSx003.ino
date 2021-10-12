#include "_Plugin_Helper.h"
#ifdef USES_P053

// #######################################################################################################
// #################################### Plugin 053: Plantower PMSx003 ####################################
// #######################################################################################################
//
// http://www.aqmd.gov/docs/default-source/aq-spec/resources-page/plantower-pms5003-manual_v2-3.pdf?sfvrsn=2
//
// The PMSx003 are particle sensors. Particles are measured by blowing air through the enclosure and,
// together with a laser, count the amount of particles. These sensors have an integrated microcontroller
// that counts particles and transmits measurement data over the serial connection.

# include "src/PluginStructs/P053_data_struct.h"

# define PLUGIN_053
# define PLUGIN_ID_053 53
# define PLUGIN_NAME_053 "Dust - PMSx003"
# ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
#  ifdef PLUGIN_NAME_053
#   undef PLUGIN_NAME_053
#  endif // ifdef PLUGIN_NAME_053
#  define PLUGIN_NAME_053 "Dust - PMSx003 / PMSx003ST" // 'upgrade' plugin-name
# endif  // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
# define PLUGIN_VALUENAME1_053 "pm1.0"
# define PLUGIN_VALUENAME2_053 "pm2.5"
# define PLUGIN_VALUENAME3_053 "pm10"
# define PLUGIN_VALUENAME4_053 "HCHO" // Is not set into the Values on purpose.


boolean Plugin_053(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_053;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
        # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      Device[deviceCount].FormulaOption = true;
      Device[deviceCount].ValueCount    = 4;
        # else // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      Device[deviceCount].FormulaOption = false;
      Device[deviceCount].ValueCount    = 3;
        # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      Device[deviceCount].SendDataOption   = true;
      Device[deviceCount].TimerOption      = true;
      Device[deviceCount].GlobalSyncOption = true;
      success                              = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string  = F(PLUGIN_NAME_053);
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_053));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_053));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_053));
        # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

      for (uint8_t i = 0; i < 3; i++) {
        ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0; // Set to former default
      }

      // 4th ValueName and decimals not (re)set on purpose
        # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      success = true;
      break;
    }

    # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = GET_PLUGIN_053_OUTPUT_SELECTOR == PMSx003_output_selection::PLUGIN_053_OUTPUT_PART ? 3 : 4;
      success     = true;
      break;
    }
    # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_getGpioNames(event);
      event->String3 = formatGpioName_output(F("Reset"));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_output_optional(F("Reset")),   F("rstpin"), PLUGIN_053_RST_PIN);
      addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_output_optional(F("PWR set")), F("pwrpin"), PLUGIN_053_PWR_PIN);
      # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      {
        addFormSubHeader(F("Device"));
        #  ifdef PLUGIN_053_ENABLE_S_AND_T
        int unitModelCount = 5;
        #  else // ifdef PLUGIN_053_ENABLE_S_AND_T
        int unitModelCount = 3;
        #  endif // ifdef PLUGIN_053_ENABLE_S_AND_T
        const __FlashStringHelper *unitModels[] = {
          toString(PMSx003_type::PMS1003_5003_7003),
          toString(PMSx003_type::PMS2003_3003),
          #  ifdef PLUGIN_053_ENABLE_S_AND_T
          toString(PMSx003_type::PMS5003_S),
          toString(PMSx003_type::PMS5003_T),
          #  endif // ifdef PLUGIN_053_ENABLE_S_AND_T
          toString(PMSx003_type::PMS5003_ST)
        };
        const int unitModelOptions[] = {
          static_cast<int>(PMSx003_type::PMS1003_5003_7003),
          static_cast<int>(PMSx003_type::PMS2003_3003),
          #  ifdef PLUGIN_053_ENABLE_S_AND_T
          static_cast<int>(PMSx003_type::PMS5003_S),
          static_cast<int>(PMSx003_type::PMS5003_T),
          #  endif // ifdef PLUGIN_053_ENABLE_S_AND_T
          static_cast<int>(PMSx003_type::PMS5003_ST)
        };
        addFormSelector(F("Sensor model"), F("p053_model"), unitModelCount, unitModels, unitModelOptions, PLUGIN_053_SENSOR_MODEL_SELECTOR);
      }
      {
        addFormSubHeader(F("Output"));
        const __FlashStringHelper *outputOptions[] = {
          toString(PMSx003_output_selection::PLUGIN_053_OUTPUT_PART),
          toString(PMSx003_output_selection::PLUGIN_053_OUTPUT_THC),
          toString(PMSx003_output_selection::PLUGIN_053_OUTPUT_CNT) };
        int outputOptionValues[] = {
          static_cast<int>(PMSx003_output_selection::PLUGIN_053_OUTPUT_PART),
          static_cast<int>(PMSx003_output_selection::PLUGIN_053_OUTPUT_THC),
          static_cast<int>(PMSx003_output_selection::PLUGIN_053_OUTPUT_CNT) };
        addFormSelector(F("Output values"), F("p053_output"), 3, outputOptions, outputOptionValues, PLUGIN_053_OUTPUT_SELECTOR, true);
        addFormNote(F("Manually change 'Values' names and decimals accordingly! Changing this reloads the page."));
      }
      {
        const __FlashStringHelper *eventOptions[] = {
          toString(PMSx003_event_datatype::PLUGIN_053_EVENT_NONE),
          toString(PMSx003_event_datatype::PLUGIN_053_EVENT_PARTICLES),
          toString(PMSx003_event_datatype::PLUGIN_053_EVENT_PARTCOUNT) };
        int eventOptionValues[] = {
          static_cast<int>(PMSx003_event_datatype::PLUGIN_053_EVENT_NONE),
          static_cast<int>(PMSx003_event_datatype::PLUGIN_053_EVENT_PARTICLES),
          static_cast<int>(PMSx003_event_datatype::PLUGIN_053_EVENT_PARTCOUNT) };
        addFormSelector(F("Events for non-output values"), F("p053_events"), 3, eventOptions, eventOptionValues,
                        PLUGIN_053_EVENT_OUT_SELECTOR);
        addFormNote(F(
                      "Only generates the 'missing' events, (taskname#temp/humi/hcho, taskname#pm1.0/pm10, taskname#cnt1.0/cnt2.5/cnt5/cnt10)."));
      }
      # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      int rstPin, pwrPin = -1;
      update_whenset_FormItemInt(F("rstpin"), rstPin);
      update_whenset_FormItemInt(F("pwrpin"), pwrPin);
      PLUGIN_053_RST_PIN = rstPin;
      PLUGIN_053_PWR_PIN = pwrPin;

      # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      PLUGIN_053_SENSOR_MODEL_SELECTOR = getFormItemInt(F("p053_model"));
      PLUGIN_053_OUTPUT_SELECTOR       = getFormItemInt(F("p053_output"));

      switch (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR) {
        case PMSx003_type::PMS1003_5003_7003:

          // Base models only support particle values, no use in setting other output values
          if (GET_PLUGIN_053_OUTPUT_SELECTOR == PMSx003_output_selection::PLUGIN_053_OUTPUT_THC) {
            PLUGIN_053_OUTPUT_SELECTOR = static_cast<int>(PMSx003_output_selection::PLUGIN_053_OUTPUT_PART);
          }
          break;
        case PMSx003_type::PMS2003_3003:
          PLUGIN_053_OUTPUT_SELECTOR = static_cast<int>(PMSx003_output_selection::PLUGIN_053_OUTPUT_PART);
          break;
        default:
          break;
      }

      PLUGIN_053_EVENT_OUT_SELECTOR = getFormItemInt(F("P053_events"));
      # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      int8_t rxPin                  = CONFIG_PIN1;
      int8_t txPin                  = CONFIG_PIN2;
      const  ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
      int8_t resetPin               = CONFIG_PIN3;

      PMSx003_type Plugin_053_sensortype = PMSx003_type::PMS1003_5003_7003;

        # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      Plugin_053_sensortype = GET_PLUGIN_053_SENSOR_MODEL_SELECTOR;
        # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P053_data_struct(rxPin, txPin, port, resetPin, Plugin_053_sensortype));
      P053_data_struct *P053_data =
        static_cast<P053_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P053_data) {
        return success;
      }
      success = P053_data->initialized();

      break;
    }

    // The update rate from the module is 200ms .. multiple seconds. Practise
    // shows that we need to read the buffer many times per seconds to stay in
    // sync.
    case PLUGIN_TEN_PER_SECOND:
    {
      P053_data_struct *P053_data =
        static_cast<P053_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P053_data) && P053_data->initialized()) {
        if (P053_data->packetAvailable()) {
          // Check if a complete packet is available in the UART FIFO.
          # ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG_MORE, F("PMSx003 : Packet available"));
          # endif // ifndef BUILD_NO_DEBUG
          success = P053_data->processData(event);
        }
      }
      break;
    }
    case PLUGIN_READ:
    {
      P053_data_struct *P053_data =
        static_cast<P053_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P053_data) && P053_data->initialized()) {
        // When new data is available, return true
        success = P053_data->checkAndClearValuesReceived();
        break;
      }
    }
  }
  return success;
}

#endif // USES_P053
