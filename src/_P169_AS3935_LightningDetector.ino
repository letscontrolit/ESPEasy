#include "_Plugin_Helper.h"
#ifdef USES_P169

// #######################################################################################################
// ########################   Plugin 169 AS3935 Lightning Detector I2C  ##################################
// #######################################################################################################

# include "./src/PluginStructs/P169_data_struct.h"

# define PLUGIN_169
# define PLUGIN_ID_169     169
# define PLUGIN_NAME_169   "Environment - AS3935 Lightning Detector"
# define PLUGIN_VALUENAME1_169 "DistanceNear"
# define PLUGIN_VALUENAME2_169 "DistanceFar"
# define PLUGIN_VALUENAME3_169 "Lightning"
# define PLUGIN_VALUENAME4_169 "Total"


boolean Plugin_169(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number           = PLUGIN_ID_169;
      dev.Type             = DEVICE_TYPE_I2C;
      dev.VType            = Sensor_VType::SENSOR_TYPE_TRIPLE;
      dev.FormulaOption    = true;
      dev.ValueCount       = 4;
      dev.SendDataOption   = true;
      dev.TimerOption      = true;
      dev.I2CNoDeviceCheck = true; // Sensor may sometimes not respond immediately
      dev.PluginStats      = true;
      break;
    }


    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_169);
      break;
    }


    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_169));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_169));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_169));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_169));
      break;
    }


    case PLUGIN_SET_DEFAULTS:
    {
      // Set a default config here, which will be called when a plugin is assigned to a task.
      P169_I2C_ADDRESS         = P169_I2C_ADDRESS_DFLT;
      P169_LIGHTNING_THRESHOLD = AS3935MI::AS3935_MNL_1;
      P169_AFE_GAIN_LOW        = AS3935MI::AS3935_OUTDOORS;
      P169_AFE_GAIN_HIGH       = AS3935MI::AS3935_OUTDOORS;
      P169_SET_MASK_DISTURBANCE(false);
      P169_SET_SEND_ONLY_ON_LIGHTNING(true);
      P169_SET_TOLERANT_CALIBRATION_RANGE(true);

      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 1; // Distance Near
      ExtraTaskSettings.TaskDeviceValueDecimals[1] = 1; // Distance Far
      ExtraTaskSettings.TaskDeviceValueDecimals[2] = 0; // Lightning count since last PLUGIN_READ
      ExtraTaskSettings.TaskDeviceValueDecimals[3] = 0; // Total lightning count
      success                                      = true;
      break;
    }


    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P169_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS


    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      constexpr uint8_t i2cAddressValues[] = { 0x01, 0x02, 0x03 };
      constexpr size_t  i2cOptionCount     = NR_ELEMENTS(i2cAddressValues);

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS)
      {
        // addFormSelectorI2C(P169_I2C_ADDRESS_LABEL, 3, i2cAddressValues, P169_I2C_ADDRESS);
        addFormSelectorI2C(F("i2c_addr"), i2cOptionCount, i2cAddressValues, P169_I2C_ADDRESS);
        addFormNote(F("Addr: 0-0-0-0-0-A1-A0. Both A0 & A1 low is not valid."));
      }
      else
      {
        success = intArrayContains(i2cOptionCount, i2cAddressValues, event->Par1);
      }
      break;
    }

    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      string  = concat(F("IRQ: "),  formatGpioLabel(P169_IRQ_PIN, false));
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormPinSelect(
        PinSelectPurpose::Generic_input,
        formatGpioName_input(F("IRQ")),
        F(P169_IRQ_PIN_LABEL),
        P169_IRQ_PIN);

      {
        const __FlashStringHelper *options[] = { F("1"), F("5"), F("9"), F("16") };
        const int optionValues[]             = {
          AS3935MI::AS3935_MNL_1,
          AS3935MI::AS3935_MNL_5,
          AS3935MI::AS3935_MNL_9,
          AS3935MI::AS3935_MNL_16 };
        constexpr size_t optionCount = NR_ELEMENTS(optionValues);
        addFormSelector(F("Lightning Threshold"),
                        P169_LIGHTNING_THRESHOLD_LABEL,
                        optionCount,
                        options,
                        optionValues,
                        P169_LIGHTNING_THRESHOLD);
        addFormNote(F("Minimum number of lightning strikes in the last 15 minutes"));
      }
      {
        const __FlashStringHelper *options[] = {
          F("0.30x"),
          F("0.40x"),
          F("0.55x"),
          F("0.74x"),
          F("1.00x (Outdoor)"),
          F("1.35x"),
          F("1.83x"),
          F("2.47x"),
          F("3.34x (Indoor)")
        };
        const int optionValues[] = {
          10,
          11,
          12,
          13,
          14,
          15,
          16,
          17,
          18
        };
        constexpr size_t optionCount = NR_ELEMENTS(optionValues);
        addFormSelector(F("AFE Gain Min"), P169_AFE_GAIN_LOW_LABEL,  optionCount, options, optionValues, P169_AFE_GAIN_LOW);
        addFormSelector(F("AFE Gain Max"), P169_AFE_GAIN_HIGH_LABEL, optionCount, options, optionValues, P169_AFE_GAIN_HIGH);
        addFormNote(F("Lower and upper limit for the Analog Frond-End auto gain to use."));
      }

      addFormCheckBox(F("Ignore Disturbance"),                F(P169_MASK_DISTURBANCE_LABEL),           P169_GET_MASK_DISTURBANCE);
      addFormCheckBox(F("Tolerate out-of-range calibration"), F(P169_TOLERANT_CALIBRATION_RANGE_LABEL), P169_GET_TOLERANT_CALIBRATION_RANGE);
      addFormNote(F("When checked, allow for more than 3.5% deviation for the 500 kHz LCO resonance frequency"));
      addFormCheckBox(F("Slow LCO Calibration"), F(P169_SLOW_LCO_CALIBRATION_LABEL), P169_GET_SLOW_LCO_CALIBRATION);
      addFormNote(F("Slow Calibration may improve accuracy of measured resonance frequency"));
      addFormCheckBox(F("Send Only On Lightning"), F(P169_SEND_ONLY_ON_LIGHTNING_LABEL), P169_GET_SEND_ONLY_ON_LIGHTNING);
      addFormNote(F("Only send to controller when lightning detected since last taskrun"));

      P169_data_struct *P169_data =
        static_cast<P169_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P169_data != nullptr)  {
        P169_data->html_show_sensor_info(event);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P169_I2C_ADDRESS = getFormItemInt(F("i2c_addr"));

      /*
            P169_NOISE               = getFormItemInt(P169_NOISE_LABEL);
            P169_WATCHDOG            = getFormItemInt(P169_WATCHDOG_LABEL);
            P169_SPIKE_REJECTION     = getFormItemInt(P169_SPIKE_REJECTION_LABEL);
       */
      P169_LIGHTNING_THRESHOLD = getFormItemInt(P169_LIGHTNING_THRESHOLD_LABEL);
      const int gain_low  = getFormItemInt(P169_AFE_GAIN_LOW_LABEL);
      const int gain_high = getFormItemInt(P169_AFE_GAIN_HIGH_LABEL);
      P169_AFE_GAIN_LOW  = gain_low;
      P169_AFE_GAIN_HIGH = gain_high;

      if (gain_low > gain_high) {
        P169_AFE_GAIN_LOW  = gain_high;
        P169_AFE_GAIN_HIGH = gain_low;
      }
      P169_SET_MASK_DISTURBANCE(isFormItemChecked(F(P169_MASK_DISTURBANCE_LABEL)));
      P169_SET_SEND_ONLY_ON_LIGHTNING(isFormItemChecked(F(P169_SEND_ONLY_ON_LIGHTNING_LABEL)));
      P169_SET_TOLERANT_CALIBRATION_RANGE(isFormItemChecked(F(P169_TOLERANT_CALIBRATION_RANGE_LABEL)));
      P169_SET_SLOW_LCO_CALIBRATION(isFormItemChecked(F(P169_SLOW_LCO_CALIBRATION_LABEL)));
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P169_data_struct(event));
      P169_data_struct *P169_data = static_cast<P169_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P169_data) {
        success = P169_data->plugin_init(event);
      }

      break;
    }

    case PLUGIN_READ:
    {
      P169_data_struct *P169_data = static_cast<P169_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P169_data) {
        if (P169_data->getAndClearLightningCount() > 0) {
          success = true;
        } else {
          UserVar.setFloat(event->TaskIndex, 0, -1.0f);
          UserVar.setFloat(event->TaskIndex, 1, -1.0f);
          UserVar.setFloat(event->TaskIndex, 2, 0.0f);
          P169_data->clearStatistics();

          if (!P169_GET_SEND_ONLY_ON_LIGHTNING) {
            success = true;
          }
        }
      }

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P169_data_struct *P169_data = static_cast<P169_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P169_data) {
        if (P169_data->loop(event)) {}
        success = true;
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P169_data_struct *P169_data = static_cast<P169_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P169_data) {
        success = P169_data->plugin_write(event, string);
      }

      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P169_data_struct *P169_data = static_cast<P169_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P169_data) {
        success = P169_data->plugin_get_config_value(event, string);
      }

      break;
    }
  }

  return success;
} // function

#endif  // USES_P169
