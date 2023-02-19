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
      Device[deviceCount].PluginStats      = true;
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
      const bool oversample = bitRead(PLUGIN_053_DATA_PROCESSING_FLAGS, PLUGIN_053_OVERSAMPLING_BIT);

      switch (GET_PLUGIN_053_OUTPUT_SELECTOR) {
        case PMSx003_output_selection::Particles_ug_m3:
        {
          const uint8_t indices[] = {
            PMS_PM1_0_ug_m3_normal,
            PMS_PM2_5_ug_m3_normal,
            PMS_PM10_0_ug_m3_normal };
          P053_data_struct::setTaskValueNames(ExtraTaskSettings, indices, 3, oversample);
          break;
        }
        case PMSx003_output_selection::PM2_5_TempHum_Formaldehyde:
        {
          const uint8_t indices[] = {
            PMS_PM2_5_ug_m3_normal,
            PMS_Temp_C,
            PMS_Hum_pct,
            PMS_Formaldehyde_mg_m3 };
          P053_data_struct::setTaskValueNames(ExtraTaskSettings, indices, 4, oversample);
          break;
        }
        case PMSx003_output_selection::ParticlesCount_100ml_cnt0_3__cnt_2_5:
        {
          const uint8_t indices[] = {
            PMS_cnt0_3_100ml,
            PMS_cnt0_5_100ml,
            PMS_cnt1_0_100ml,
            PMS_cnt2_5_100ml };
          P053_data_struct::setTaskValueNames(ExtraTaskSettings, indices, 4, oversample);
          break;
        }
        case PMSx003_output_selection::ParticlesCount_100ml_cnt1_0_cnt2_5_cnt10:
        {
          const uint8_t indices[] = {
            PMS_cnt1_0_100ml,
            PMS_cnt2_5_100ml,
            PMS_cnt5_0_100ml,
            PMS_cnt10_0_100ml };
          P053_data_struct::setTaskValueNames(ExtraTaskSettings, indices, 4, oversample);
          break;
        }
      }
      success = true;
      break;
    }

    # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = GET_PLUGIN_053_OUTPUT_SELECTOR == PMSx003_output_selection::Particles_ug_m3 ? 3 : 4;
      success     = true;
      break;
    }
    # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

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

    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      string  = F("RST: ");
      string += formatGpioLabel(PLUGIN_053_RST_PIN, false);
      string += event->String1; // newline
      string += F("SET: ");
      string += formatGpioLabel(PLUGIN_053_PWR_PIN, false);
      success = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      PLUGIN_053_RST_PIN = -1;
      PLUGIN_053_PWR_PIN = -1;

      PLUGIN_053_SENSOR_MODEL_SELECTOR = static_cast<int>(PMSx003_type::PMS1003_5003_7003);
      PLUGIN_053_SEC_IGNORE_AFTER_WAKE = 0;

      bitSet(PLUGIN_053_DATA_PROCESSING_FLAGS, PLUGIN_053_OVERSAMPLING_BIT);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_output_optional(F("RST")), F("rstpin"), PLUGIN_053_RST_PIN);
      addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_output_optional(F("SET")), F("pwrpin"), PLUGIN_053_PWR_PIN);
      addFormNote(F("RST and SET pins on sensor are pulled up internal in the sensor"));
      # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      {
        addFormSubHeader(F("Device"));
        int unitModelCount                      = 5;
        const __FlashStringHelper *unitModels[] = {
          toString(PMSx003_type::PMS1003_5003_7003),
          toString(PMSx003_type::PMS2003_3003),
          toString(PMSx003_type::PMS5003_S),
          toString(PMSx003_type::PMS5003_T),
          toString(PMSx003_type::PMS5003_ST)
        };
        const int unitModelOptions[] = {
          static_cast<int>(PMSx003_type::PMS1003_5003_7003),
          static_cast<int>(PMSx003_type::PMS2003_3003),
          static_cast<int>(PMSx003_type::PMS5003_S),
          static_cast<int>(PMSx003_type::PMS5003_T),
          static_cast<int>(PMSx003_type::PMS5003_ST)
        };
        addFormSelector(F("Sensor model"), F("model"), unitModelCount, unitModels, unitModelOptions, PLUGIN_053_SENSOR_MODEL_SELECTOR);
      }

      addFormSubHeader(F("Output"));
      {
        const __FlashStringHelper *outputOptions[] = {
          toString(PMSx003_output_selection::Particles_ug_m3),
          toString(PMSx003_output_selection::PM2_5_TempHum_Formaldehyde),
          toString(PMSx003_output_selection::ParticlesCount_100ml_cnt0_3__cnt_2_5),
          toString(PMSx003_output_selection::ParticlesCount_100ml_cnt1_0_cnt2_5_cnt10) };
        const int outputOptionValues[] = {
          static_cast<int>(PMSx003_output_selection::Particles_ug_m3),
          static_cast<int>(PMSx003_output_selection::PM2_5_TempHum_Formaldehyde),
          static_cast<int>(PMSx003_output_selection::ParticlesCount_100ml_cnt0_3__cnt_2_5),
          static_cast<int>(PMSx003_output_selection::ParticlesCount_100ml_cnt1_0_cnt2_5_cnt10) };
        addFormSelector(F("Output values"), F("output"), 4, outputOptions, outputOptionValues, PLUGIN_053_OUTPUT_SELECTOR, true);
        addFormNote(F("Changing this reloads the page and updates task value names + nr decimals."));
      }
      {
        const __FlashStringHelper *eventOptions[] = {
          toString(PMSx003_event_datatype::Event_None),
          toString(PMSx003_event_datatype::Event_PMxx_TempHum_Formaldehyde),
          toString(PMSx003_event_datatype::Event_All_count_bins),
          toString(PMSx003_event_datatype::Event_All) };
        const int eventOptionValues[] = {
          static_cast<int>(PMSx003_event_datatype::Event_None),
          static_cast<int>(PMSx003_event_datatype::Event_PMxx_TempHum_Formaldehyde),
          static_cast<int>(PMSx003_event_datatype::Event_All_count_bins),
          static_cast<int>(PMSx003_event_datatype::Event_All) };
        addFormSelector(F("Events for non-output values"), F("events"), 4, eventOptions, eventOptionValues,
                        PLUGIN_053_EVENT_OUT_SELECTOR);
        addFormNote(F(
                      "Only generates the 'missing' events, (taskname#temp/humi/hcho, taskname#pm1.0/pm2.5/pm10, taskname#cnt0.3/cnt0.5/cnt1.0/cnt2.5/cnt5/cnt10)."));
      }
      # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

      addFormSubHeader(F("Data Processing"));
      {
        addFormNumericBox(F("Sensor init time after wake"), F("delay_wake"),
                          PLUGIN_053_SEC_IGNORE_AFTER_WAKE, 0, 30);
        addUnit(F("sec"));

        # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
        addFormCheckBox(F("Oversampling"), F("oversample"),
                        bitRead(PLUGIN_053_DATA_PROCESSING_FLAGS, PLUGIN_053_OVERSAMPLING_BIT));


        addFormCheckBox(F("Split count bins"), F("split_bins"),
                        bitRead(PLUGIN_053_DATA_PROCESSING_FLAGS, PLUGIN_053_SPLIT_CNT_BINS_BIT));
        addFormNote(F("Subtract next \"count/0.1L\" bin counts to get counts per bin, not a range of bins"));
        # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      int rstPin, pwrPin = -1;
      update_whenset_FormItemInt(F("rstpin"), rstPin);
      update_whenset_FormItemInt(F("pwrpin"), pwrPin);
      PLUGIN_053_RST_PIN = rstPin;
      PLUGIN_053_PWR_PIN = pwrPin;
      success            = true;

      # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      const int oldOutputSelector = PLUGIN_053_OUTPUT_SELECTOR;
      PLUGIN_053_SENSOR_MODEL_SELECTOR = getFormItemInt(F("model"));
      PLUGIN_053_OUTPUT_SELECTOR       = getFormItemInt(F("output"));
      PLUGIN_053_EVENT_OUT_SELECTOR    = getFormItemInt(F("events"));
      PLUGIN_053_SEC_IGNORE_AFTER_WAKE = getFormItemInt(F("delay_wake"));

      if (isFormItemChecked(F("oversample"))) {
        bitSet(PLUGIN_053_DATA_PROCESSING_FLAGS, PLUGIN_053_OVERSAMPLING_BIT);
      } else {
        bitClear(PLUGIN_053_DATA_PROCESSING_FLAGS, PLUGIN_053_OVERSAMPLING_BIT);
      }

      if (isFormItemChecked(F("split_bins"))) {
        bitSet(PLUGIN_053_DATA_PROCESSING_FLAGS, PLUGIN_053_SPLIT_CNT_BINS_BIT);
      } else {
        bitClear(PLUGIN_053_DATA_PROCESSING_FLAGS, PLUGIN_053_SPLIT_CNT_BINS_BIT);
      }

      switch (GET_PLUGIN_053_SENSOR_MODEL_SELECTOR) {
        case PMSx003_type::PMS1003_5003_7003:

          // Base models only support particle values, no use in setting other output values
          if (GET_PLUGIN_053_OUTPUT_SELECTOR == PMSx003_output_selection::PM2_5_TempHum_Formaldehyde) {
            PLUGIN_053_OUTPUT_SELECTOR = static_cast<int>(PMSx003_output_selection::Particles_ug_m3);
          }

          if (GET_PLUGIN_053_EVENT_OUT_SELECTOR == PMSx003_event_datatype::Event_PMxx_TempHum_Formaldehyde) {
            PLUGIN_053_EVENT_OUT_SELECTOR = static_cast<int>(PMSx003_event_datatype::Event_None);
          }
          break;
        case PMSx003_type::PMS2003_3003:
          PLUGIN_053_OUTPUT_SELECTOR    = static_cast<int>(PMSx003_output_selection::Particles_ug_m3);
          PLUGIN_053_EVENT_OUT_SELECTOR = static_cast<int>(PMSx003_event_datatype::Event_None);
          break;
        default:
          break;
      }

      if (oldOutputSelector != PLUGIN_053_OUTPUT_SELECTOR) {
        struct EventStruct TempEvent(event->TaskIndex);

        // Do not clear ExtraTaskSettings, leave formula fields in tact.
        // ExtraTaskSettings.clear();
        ExtraTaskSettings.TaskIndex = event->TaskIndex;
        String dummy;
        PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, dummy);
      }

      # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      break;
    }

    case PLUGIN_INIT:
    {
      const int8_t rxPin            = CONFIG_PIN1;
      const int8_t txPin            = CONFIG_PIN2;
      const  ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);

      PMSx003_type Plugin_053_sensortype = PMSx003_type::PMS1003_5003_7003;

      # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      Plugin_053_sensortype = GET_PLUGIN_053_SENSOR_MODEL_SELECTOR;
      # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

      initPluginTaskData(
        event->TaskIndex,
        new (std::nothrow) P053_data_struct(
          event->TaskIndex,
          rxPin, txPin, port,
          PLUGIN_053_RST_PIN,
          PLUGIN_053_PWR_PIN,
          Plugin_053_sensortype,
          PLUGIN_053_SEC_IGNORE_AFTER_WAKE * 1000u
          # ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
          , bitRead(PLUGIN_053_DATA_PROCESSING_FLAGS, PLUGIN_053_OVERSAMPLING_BIT)
          , bitRead(PLUGIN_053_DATA_PROCESSING_FLAGS, PLUGIN_053_SPLIT_CNT_BINS_BIT)
          # endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
          ));
      P053_data_struct *P053_data =
        static_cast<P053_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P053_data) {
        success = P053_data->init();
      }

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
          P053_data->clearPacket();
        }
      }
      break;
    }
    case PLUGIN_READ:
    {
      P053_data_struct *P053_data =
        static_cast<P053_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P053_data) && P053_data->initialized()) {
        // When new data is available, return true and send the events.
        success = P053_data->checkAndClearValuesReceived(event);
      }
      break;
    }
    case PLUGIN_WRITE:
    {
      P053_data_struct *P053_data =
        static_cast<P053_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P053_data) && P053_data->initialized()) {
        String command    = parseString(string, 1);
        String subcommand = parseString(string, 2);

        if (equals(command, F("pmsx003"))) {
          if (equals(subcommand, F("wake"))) {
            success = P053_data->wakeSensor();
          } else if (equals(subcommand, F("sleep"))) {
            success = P053_data->sleepSensor();
          } else if (equals(subcommand, F("reset"))) {
            success = P053_data->resetSensor();
          }
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P053
