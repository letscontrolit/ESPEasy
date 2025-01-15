// #######################################################################################################
// #################################### Plugin 145: MQ-xx ################################################
// #################################### by flashmark      ################################################
// #######################################################################################################
// Analog gas sensors based upon electro-chemical resistance changes like the cheap MQ-xxx series
//
// References:
//   https://github.com/GeorgK/MQ135 (I borrowed code from here)
//   https://github.com/miguel5612/MQSensorsLib (I borrowed code from here)
//   https://jayconsystems.com/blog/understanding-a-gas-sensor
//   http://davidegironi.blogspot.com/2014/01/cheap-co2-meter-using-mq135-sensor-with.html
//   https://hackaday.io/project/3475-sniffing-trinket/log/12363-mq135-arduino-library
//
// #################################### Change log        ###############################################

/** Changelog:
 * 2025-01-06 tonhuisman: Formatted source uing Uncrustify and small cleanups
 * 2023-01-06 Reworked after review
 * 2022-07-11 Refactored, first attempt for calibration
 * 2020-05-21 Refactored and extended by flashmark
 */

// #################################### Description       ################################################
// This plugin supports some gas sensors for which the resistance depends on a gas concentration (MQ-xxx)
// Conversion depends on the sensor type. Main property is the logarithmic Rsensor/Rzero curve
// It expects a measurement circuit measuring the voltage over a load resistance Rload using an analog in
//
//             ---------             -------
// (VCC) -----| Rsensor |-----+-----| Rload |----- (GND)
//             ---------      |      -------
//         (measured voltage) +---Analog pin---> [Analog to Digital Converter]--> (raw digital input value)
//
// VCC:      Supply voltage on the sensor. (#define VCC)
// GND:      Reference voltage, ground
// Rsensor:  Sensor resistor value
// Rload:    Load resistor (configuration P145_PCONFIG_RLOAD)
// Rzero:    Calibrated reference resistance used as input for Rsensor to concentration value conversion
//           (configuration P145_PCONFIG_RZERO)
// Reference:Reference value expected when calibrating Rzero (configuration P145_PCONFIG_REF)
// Rcal:     Computed Rzero as if the current measurement value is at Reference level
//
// Temperature/humidity  compensation is supported for some sensors.
// In these cases the value will be read from another task.
// temperature: Read from task/value configured in P145_PCONFIG_TEMP_TASK/P145_PCONFIG_TEMP_VAL
// humidity:    Read from task/value configured in P145_PCONFIG_HUM_TASK/P145_PCONFIG_HUM_VAL
//
// Analog input is sampled 10/second and averaged with the same oversampling algoritm of P002_ADC
// Note: ESP8266 uses hard coded A0 as analog input.
//       ESP32 provides the standard ESPeasy Analog input GPIO configuration
//
// Conversion algorithm:
// Sensor resistance is logaritmic to the concentration of a gas. Sensors are not specific to a single
// gas. Key is the ratio between the measured Rs and a reference resistor Rzero (Rs/Rzero).
// For each specific gas a somewhat linear relation between gas concentration and the ratio (Rs/Rzero) is
// shown on a log-log chart. Various algorithms are described in above mentioned literature.
// This plugin supports 3, almost similar, algorithms to make it easier to copy a parameter set from the
// internet for a specific sensor/gas combination.
//
// Calibration algorithm:
// Each measurement calculate Rcal assuming the concentration is at the reference level
// Remember the lowest value of Rcal assuming it belongs to measuring the lowest concentration
// ################################### Configuration data ###############################################
// This plugin uses the following predefined static data storage
// P145_PCONFIG_RLOAD  PCONFIG_FLOAT(0)  RLOAD   [Ohm]
// P145_PCONFIG_RZERO  PCONFIG_FLOAT(1)  RZERO   [Ohm]
// P145_PCONFIG_REF    PCONFIG_FLOAT(2)  REF. level [ppm]
// P145_PCONFIG_FLAGS        PCONFIG(0)  Enable compensation & Enable Calibration & use low Vcc
// P145_PCONFIG_TEMP_TASK    PCONFIG(1)  Temperature compensation task
// P145_PCONFIG_TEMP_VAL     PCONFIG(2)  Temperature compensation value
// P145_PCONFIG_HUM_TASK     PCONFIG(3)  Humidity compensation task
// P145_PCONFIG_HUM_VAL      PCONFIG(4)  Humidity compensation value
// P145_PCONFIG_SENSORT      PCONFIG(5)  Sensor type
// #######################################################################################################

#include "_Plugin_Helper.h"
#ifdef USES_P145
# include "src/PluginStructs/P145_data_struct.h"                    // MQ-xxx sensor specific data

# define PLUGIN_145
# define PLUGIN_ID_145     145                                      // plugin id
# define PLUGIN_NAME_145   "Gases - MQxxx (MQ135 CO2, MQ3 Alcohol)" // "Plugin Name" is what will be displayed in the selection list
# define PLUGIN_VALUENAME1_145 "level"                              // variable output of the plugin. The label is in quotation marks
# define PLUGIN_145_DEBUG  false                                    // set to true for extra log info in the debug

// Static, per plugin instance, data stored in the generic ESPeasy structures see usage defined above
// PCONFIG(n)    : stores an integer (8)
// PCONFIG_FLOAT : stors a float (4)
// PCONFIG_LONG  : stores a long (4, shared with PCONFIG_ULONG)
// PCONFIG_ULONG : stores an unsigned long (4, shared with PCONFIG_LONG)
# define P145_PCONFIG_RLOAD       PCONFIG_FLOAT(0)
# define P145_PCONFIG_RZERO       PCONFIG_FLOAT(1)
# define P145_PCONFIG_REF         PCONFIG_FLOAT(2)
# define P145_PCONFIG_FLAGS       PCONFIG(0)
# define P145_PCONFIG_TEMP_TASK   PCONFIG(1)
# define P145_PCONFIG_TEMP_VAL    PCONFIG(2)
# define P145_PCONFIG_HUM_TASK    PCONFIG(3)
# define P145_PCONFIG_HUM_VAL     PCONFIG(4)
# define P145_PCONFIG_SENSORT     PCONFIG(5)

// PIN/port configuration is stored in the following:
# define P145_CONFIG_PIN_AIN      CONFIG_PIN1
# define P145_CONFIG_PIN_HEATER   CONFIG_PIN2

// Form IDs used on the device setup page. Should be a short unique string.
# define P145_GUID_TYPE       "f01"
# define P145_GUID_RLOAD      "f02"
# define P145_GUID_RZERO      "f03"
# define P145_GUID_RREFLEVEL  "f04"
# define P145_GUID_CAL        "f05"
# define P145_GUID_COMP       "f06"
# define P145_GUID_LOWVCC     "f07"
# define P145_GUID_TEMP_T     "f08"
# define P145_GUID_TEMP_V     "f09"
# define P145_GUID_HUM_T      "f10"
# define P145_GUID_HUM_V      "f11"
# define P145_GUID_AINPIN     "f12"
# define P145_GUID_HEATPIN    "f13"

// A plugin has to implement the following function
// ------------------------------------------------
boolean Plugin_145(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    // Make the plugin known with its options
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_145;
      dev.Type           = DEVICE_TYPE_CUSTOM0;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.FormulaOption  = true;
      dev.ValueCount     = 1;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      break;
    }

    // Return the DEVICENAME for this plugin
    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_145);
      break;
    }

    // Setup the value names for the task
    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_145));
      break;
    }

    // Add custom GPIO description on device overview page
    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      string = F("AIN: ");
# ifdef ESP32
      string += formatGpioLabel(P145_CONFIG_PIN_AIN, false);
# else // ifdef ESP32
      string += F("ADC (TOUT)");
# endif // ifdef ESP32

      if (validGpio(P145_CONFIG_PIN_HEATER))
      {
        string += F("<BR>HEAT: ");
        string += formatGpioLabel(P145_CONFIG_PIN_HEATER, false);
      }
      success = true;
      break;
    }

    // Setup the web form for the task
    case PLUGIN_WEBFORM_LOAD:
    {
      const bool compensate = P145_PCONFIG_FLAGS & 0x0001;        // Compensation enable flag
      const bool calibrate  = (P145_PCONFIG_FLAGS >> 1) & 0x0001; // Calibration enable flag
      const bool lowvcc     = (P145_PCONFIG_FLAGS >> 2) & 0x0001; // Low voltage power supply indicator

      // FormSelector with all predefined "Sensor - Gas" options
      String options[P145_MAXTYPES] = {};
      int    x                      = P145_data_struct::getNbrOfTypes();

      if (x > P145_MAXTYPES)
      {
        x = P145_MAXTYPES; // Clip to prevent array boundary out of range access
      }

      for (int i = 0; i < x; ++i)
      {
        options[i] = concat(concat(P145_data_struct::getTypeName(i), F(" - ")), P145_data_struct::getGasName(i));
      }
      addFormSelector(F("Sensor type"), F(P145_GUID_TYPE), x, options, nullptr, P145_PCONFIG_SENSORT);

# ifdef ESP32

      // Analog input selection
      addRowLabel(formatGpioName_input(F("Analog Pin ")));
#  if HAS_HALL_EFFECT_SENSOR
      addADC_PinSelect(AdcPinSelectPurpose::ADC_Touch_HallEffect, F(P145_GUID_AINPIN), P145_CONFIG_PIN_AIN);
#  else // if HAS_HALL_EFFECT_SENSOR
      addADC_PinSelect(AdcPinSelectPurpose::ADC_Touch,            F(P145_GUID_AINPIN), P145_CONFIG_PIN_AIN);
#  endif // if HAS_HALL_EFFECT_SENSOR
# endif // ifdef ESP32
      addFormPinSelect(PinSelectPurpose::Generic_output,
                       formatGpioName_output_optional(F("Heater Pin ")),
                       F(P145_GUID_HEATPIN),
                       P145_CONFIG_PIN_HEATER);

      addFormFloatNumberBox(F("Load Resistance"), F(P145_GUID_RLOAD), P145_PCONFIG_RLOAD, 0.0f, 10e6f, 2U);
      addUnit(F("Ohm"));
      addFormFloatNumberBox(F("R Zero"), F(P145_GUID_RZERO), P145_PCONFIG_RZERO, 0.0f, 10e6f, 2U);
      addUnit(F("Ohm"));
      addFormFloatNumberBox(F("Reference Level"), F(P145_GUID_RREFLEVEL), P145_PCONFIG_REF, 0.0f, 10e6f, 2U);
      addUnit(F("ppm"));
      P145_data_struct *P145_data = static_cast<P145_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P145_data != nullptr)
      {
        float calVal = P145_data->getCalibrationValue();

        if (definitelyGreaterThan(calVal, 0.0f))
        {
          addFormNote(concat(F("Current measurement suggests Rzero= "), calVal));
        }
      }
      addFormCheckBox(F("Low sensor supply voltage"), F(P145_GUID_LOWVCC), lowvcc);

      addFormSeparator(2);

      // Auto calibration and Temp/hum compensation flags are bitfields in P145_PCONFIG_FLAGS
      addFormCheckBox(F("Enable automatic calibration"), F(P145_GUID_CAL), calibrate);
      addFormSelector_YesNo(F("Enable temp/humid compensation"), F(P145_GUID_COMP), compensate, true);

      // Above selector will fore reloading the page and thus updating the compensate flag
      // Show the compensation details only when compensation is enabled
      if (compensate)
      {
        addFormNote(F("If compensation is enabled, the Temperature and Humidity values below need to be configured."));

        // temperature
        addRowLabel(F("Temperature"));
        addTaskSelect(F(P145_GUID_TEMP_T), P145_PCONFIG_TEMP_TASK);

        if (validTaskIndex(P145_PCONFIG_TEMP_TASK))
        {
          LoadTaskSettings(P145_PCONFIG_TEMP_TASK); // we need to load the values from another task for selection!
          addRowLabel(F("Temperature Value"));
          addTaskValueSelect(F(P145_GUID_TEMP_V), P145_PCONFIG_TEMP_VAL, P145_PCONFIG_TEMP_TASK);
        }

        // humidity
        addRowLabel(F("Humidity"));
        addTaskSelect(F(P145_GUID_HUM_T), P145_PCONFIG_HUM_TASK);

        if (validTaskIndex(P145_PCONFIG_HUM_TASK))
        {
          LoadTaskSettings(P145_PCONFIG_HUM_TASK); // we need to load the values from another task for selection!
          addRowLabel(F("Humidity Value"));
          addTaskValueSelect(F(P145_GUID_HUM_V), P145_PCONFIG_HUM_VAL, P145_PCONFIG_HUM_TASK);
        }
        LoadTaskSettings(event->TaskIndex); // we need to restore our original taskvalues!
      }

      success = true;
      break;
    }

    // Save setting from web form for the task
    case PLUGIN_WEBFORM_SAVE:
    {
      P145_PCONFIG_SENSORT = getFormItemInt(F(P145_GUID_TYPE));
      P145_PCONFIG_RLOAD   = getFormItemFloat(F(P145_GUID_RLOAD));
      P145_PCONFIG_RZERO   = getFormItemFloat(F(P145_GUID_RZERO));
      P145_PCONFIG_REF     = getFormItemFloat(F(P145_GUID_RREFLEVEL));
      const bool compensate = (getFormItemInt(F(P145_GUID_COMP)) == 1);
      const bool calibrate  = isFormItemChecked(F(P145_GUID_CAL));
      const bool lowvcc     = isFormItemChecked(F(P145_GUID_LOWVCC));
      P145_PCONFIG_FLAGS     = compensate + (calibrate << 1) + (lowvcc << 2);
      P145_PCONFIG_TEMP_TASK = getFormItemInt(F(P145_GUID_TEMP_T));
      P145_PCONFIG_TEMP_VAL  = getFormItemInt(F(P145_GUID_TEMP_V));
      P145_PCONFIG_HUM_TASK  = getFormItemInt(F(P145_GUID_HUM_T));
      P145_PCONFIG_HUM_VAL   = getFormItemInt(F(P145_GUID_HUM_V));
# ifdef ESP32
      P145_CONFIG_PIN_AIN = getFormItemInt(F(P145_GUID_AINPIN));
# endif // ifdef ESP32
      P145_CONFIG_PIN_HEATER = getFormItemInt(F(P145_GUID_HEATPIN));

      P145_data_struct *P145_data = static_cast<P145_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P145_data != nullptr)
      {
        P145_data->setSensorData(P145_PCONFIG_SENSORT, compensate, calibrate, lowvcc, P145_PCONFIG_RLOAD, P145_PCONFIG_RZERO,
                                 P145_PCONFIG_REF);
        P145_data->setSensorPins(P145_CONFIG_PIN_AIN, P145_CONFIG_PIN_HEATER);
        P145_data->dump();
      }
      success = true;
      break;
    }

    // Initialize the task
    case PLUGIN_INIT:
    {
      P145_data_struct *P145_data = static_cast<P145_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P145_data == nullptr)
      {
        P145_data = new (std::nothrow) P145_data_struct();
        initPluginTaskData(event->TaskIndex, P145_data);

        if (P145_data != nullptr)
        {
          P145_data->plugin_init();
          P145_data->setSensorData(P145_PCONFIG_SENSORT, P145_PCONFIG_FLAGS & 0x0001, (P145_PCONFIG_FLAGS >> 1) & 0x0001,
                                   (P145_PCONFIG_FLAGS >> 2) & 0x0001, P145_PCONFIG_RLOAD, P145_PCONFIG_RZERO, P145_PCONFIG_REF);
          P145_data->setSensorPins(P145_CONFIG_PIN_AIN, P145_CONFIG_PIN_HEATER);
          P145_data->dump();
        }
      }
      success = true;
      break;
    }

    // Execute at 10Hz
    case PLUGIN_TEN_PER_SECOND:
    {
      P145_data_struct *P145_data = static_cast<P145_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P145_data != nullptr)
      {
        success = P145_data->plugin_ten_per_second();
      }
      break;
    }

    // Read fresh data from the task
    case PLUGIN_READ:
    {
      P145_data_struct *P145_data = static_cast<P145_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P145_data != nullptr)
      {
        float temperature     = 20.0f; // A reasonable value in case temperature source task is invalid
        float humidity        = 60.0f; // A reasonable value in case humidity source task is invalid
        const bool compensate = P145_PCONFIG_FLAGS & 0x0001;

        if (compensate && validTaskIndex(P145_PCONFIG_TEMP_TASK) && validTaskIndex(P145_PCONFIG_HUM_TASK))
        {
          // we're checking a var from another task, so calculate that basevar
          temperature = UserVar.getFloat(P145_PCONFIG_TEMP_TASK, P145_PCONFIG_TEMP_VAL); // in degrees C
          humidity    = UserVar.getFloat(P145_PCONFIG_HUM_TASK, P145_PCONFIG_HUM_VAL);   // in % relative
        }
        UserVar.setFloat(event->TaskIndex, 0, P145_data->readValue(temperature, humidity));
        success = true;
      }
      break;
    }

    // Execute each second
    case PLUGIN_ONCE_A_SECOND:
    {
      P145_data_struct *P145_data = static_cast<P145_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P145_data != nullptr)
      {
        if ((P145_PCONFIG_FLAGS >> 1) & 0x0001) // Calibration flag
        {
          // Update Rzero in case of autocalibration
          // TODO is there an event to signal the plugin code that the value has been updated to prevent polling?
          P145_PCONFIG_RZERO = P145_data->getAutoCalibrationValue(); // Store autocalibration value
        }
        P145_data->heaterControl();                                  // Execute heater control algorithm
      }
      break;
    }
  }
  return success;
} // function Plugin_145()

#endif  // USES_P145
