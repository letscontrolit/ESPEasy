#include "_Plugin_Helper.h"
#ifdef USES_P067

// #######################################################################################################
// ####################################### Plugin 067: HX711 Load Cell ###################################
// #######################################################################################################

// ESPEasy Plugin to scan a 24 bit AD value from a load cell chip HX711
// written by Jochen Krapf (jk@nerd2nerd.org)
//
// Modified by chunter to support dual channel measurements.
// When both channels are enabled, sample-rate drops to approx. 1 sample/s for each channel.

// Electronics:
// Connect SCL to 1st GPIO and DOUT to 2nd GPIO. Use 3.3 volt for VCC.

// Datasheet: https://cdn.sparkfun.com/datasheets/Sensors/ForceFlex/hx711_english.pdf

/** Changelog:
 * 2023-02-23 tonhuisman: Ignore first PLUGIN_READ after startup, as no samples have been read yet so no measurement data is available
 * 2023-01-01 tonhuisman: Minor string reductions
 * 2022-12-30 tonhuisman: Fix no longer generating events, use DIRECT_pinRead() and DIRECT_pinWrite() to ensure proper working on ESP32,
 *                        include Task number when logging values, reset any previous values on init, change Pin names to match board text
 * 2022-12-28 tonhuisman: Refactor using PluginTaskData struct to eliminate the use of static and global variables
 * 2022-12-28 tonhuisman: Add changelog, older log not registered
 */


# define PLUGIN_067
# define PLUGIN_ID_067           67
# define PLUGIN_NAME_067         "Weight - HX711 Load Cell"
# define PLUGIN_VALUENAME1_067   "WeightChanA"
# define PLUGIN_VALUENAME2_067   "WeightChanB"

# include "./src/PluginStructs/P067_data_struct.h"

boolean Plugin_067(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_067;
      dev.Type           = DEVICE_TYPE_DUAL;
      dev.VType          = Sensor_VType::SENSOR_TYPE_DUAL;
      dev.FormulaOption  = true;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
      dev.setPin1Direction(gpio_direction::gpio_output);
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_067);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_067));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_067));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("SCK"));
      event->String2 = formatGpioName_input(F("DT"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      float valFloat;

      // A ------------
      addFormSubHeader(F("Measurement Channel A"));

      addFormCheckBox(F("Oversampling"), F("osChA"), P067_GET_CHANNEL_A_OS);

      {
        const __FlashStringHelper *optionsModeChanA[] = { F("Off"), F("Gain 64"), F("Gain 128") };
        addFormSelector(F("Mode"), F("modeChA"), 3, optionsModeChanA, nullptr, P067_GET_CHANNEL_A_MODE);
      }

      P067_int2float(P067_OFFSET_CHANNEL_A_1, P067_OFFSET_CHANNEL_A_2, &valFloat);
      addFormTextBox(F("Offset"), F("offsChA"), toString(valFloat, 3), 25);
      addHtml(F("&nbsp; &nbsp;&#8617; Tare: "));
      addCheckBox(F("tareChA"), 0); // always off

      // B ------------
      addFormSubHeader(F("Measurement Channel B"));

      addFormCheckBox(F("Oversampling"), F("osChB"), P067_GET_CHANNEL_B_OS);

      {
        const __FlashStringHelper *optionsModeChanB[] = { F("Off"), F("Gain 32") };
        addFormSelector(F("Mode"), F("modeChB"), 2, optionsModeChanB, nullptr, P067_GET_CHANNEL_B_MODE);
      }

      P067_int2float(P067_OFFSET_CHANNEL_B_1, P067_OFFSET_CHANNEL_B_2, &valFloat);
      addFormTextBox(F("Offset"), F("offsChB"), toString(valFloat, 3), 25);
      addHtml(F("&nbsp; &nbsp;&#8617; Tare: "));
      addCheckBox(F("tareChB"), 0); // always off

      // A ------------
      addFormSubHeader(F("Two Point Calibration Channel A"));
      addFormCheckBox(F("Calibration Enabled"), F("calChA"), P067_GET_CHANNEL_A_CALIB);

      addFormNumericBox(F("Point 1"), F("adc1ChA"), P067_CONFIG_CHANNEL_A_ADC1);
      html_add_estimate_symbol();
      addTextBox(F("out1ChA"), toString(P067_CONFIG_CHANNEL_A_OUT1, 3), 10);

      addFormNumericBox(F("Point 2"), F("adc2ChA"), P067_CONFIG_CHANNEL_A_ADC2);
      html_add_estimate_symbol();
      addTextBox(F("out2ChA"), toString(P067_CONFIG_CHANNEL_A_OUT2, 3), 10);

      // B ------------
      addFormSubHeader(F("Two Point Calibration Channel B"));

      addFormCheckBox(F("Calibration Enabled"), F("calChB"), P067_GET_CHANNEL_B_CALIB);

      addFormNumericBox(F("Point 1"), F("adc1ChB"), P067_CONFIG_CHANNEL_B_ADC1);
      html_add_estimate_symbol();
      addTextBox(F("out1ChB"), toString(P067_CONFIG_CHANNEL_B_OUT1, 3), 10);

      addFormNumericBox(F("Point 2"), F("adc2ChB"), P067_CONFIG_CHANNEL_B_ADC2);
      html_add_estimate_symbol();
      addTextBox(F("out2ChB"), toString(P067_CONFIG_CHANNEL_B_OUT2, 3), 10);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      float valFloat;

      P067_CONFIG_FLAGS = 0;

      P067_SET_CHANNEL_A_OS(isFormItemChecked(F("osChA")));
      P067_SET_CHANNEL_B_OS(isFormItemChecked(F("osChB")));

      uint32_t tmp = P067_CONFIG_FLAGS;
      set2BitToUL(tmp, P067_CONFIG_CHANNEL_A_MODE, getFormItemInt(F("modeChA")));
      P067_CONFIG_FLAGS = tmp;

      P067_SET_CHANNEL_B_MODE(getFormItemInt(F("modeChB")));

      P067_SET_CHANNEL_A_CALIB(isFormItemChecked(F("calChA")));
      P067_SET_CHANNEL_B_CALIB(isFormItemChecked(F("calChB")));

      if (isFormItemChecked(F("tareChA"))) {
        valFloat = -UserVar[event->BaseVarIndex + 2];
      } else {
        valFloat = getFormItemFloat(F("offsChA"));
      }
      P067_float2int(valFloat, &P067_OFFSET_CHANNEL_A_1, &P067_OFFSET_CHANNEL_A_2);

      if (isFormItemChecked(F("tareChB"))) {
        valFloat = -UserVar[event->BaseVarIndex + 3];
      } else {
        valFloat = getFormItemFloat(F("offsChB"));
      }
      P067_float2int(valFloat, &P067_OFFSET_CHANNEL_B_1, &P067_OFFSET_CHANNEL_B_2);

      P067_CONFIG_CHANNEL_A_ADC1 = getFormItemInt(F("adc1ChA"));
      P067_CONFIG_CHANNEL_A_OUT1 = getFormItemFloat(F("out1ChA"));

      P067_CONFIG_CHANNEL_A_ADC2 = getFormItemInt(F("adc2ChA"));
      P067_CONFIG_CHANNEL_A_OUT2 = getFormItemFloat(F("out2ChA"));

      P067_CONFIG_CHANNEL_B_ADC1 = getFormItemInt(F("adc1ChB"));
      P067_CONFIG_CHANNEL_B_OUT1 = getFormItemFloat(F("out1ChB"));

      P067_CONFIG_CHANNEL_B_ADC2 = getFormItemInt(F("adc2ChB"));
      P067_CONFIG_CHANNEL_B_OUT2 = getFormItemFloat(F("out2ChB"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P067_data_struct(event,
                                                                               CONFIG_PIN1,
                                                                               CONFIG_PIN2));
      P067_data_struct *P067_data = static_cast<P067_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P067_data) {
        success = P067_data->init(event);
      }

      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P067_data_struct *P067_data = static_cast<P067_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P067_data) {
        success = P067_data->plugin_fifty_per_second(event);
      }

      break;
    }

    case PLUGIN_READ:
    {
      P067_data_struct *P067_data = static_cast<P067_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P067_data) {
        success = P067_data->plugin_read(event);
      }

      break;
    }

    case PLUGIN_WRITE:
    {
      P067_data_struct *P067_data = static_cast<P067_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P067_data) {
        success = P067_data->plugin_write(event, string);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P067
