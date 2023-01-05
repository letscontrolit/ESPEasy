//#######################################################################################################
//#################################### Plugin 145: MQ-xx ################################################
//#################################### by flashmark      ################################################
//#######################################################################################################
// Analog gas sensors based upon electro-chemical resistance changes like the cheap MQ-xxx series
//
// References:
//   https://github.com/GeorgK/MQ135 (I borrowed code from here)
//   https://github.com/miguel5612/MQSensorsLib (I borrowed code from here)
//   https://jayconsystems.com/blog/understanding-a-gas-sensor
//   http://davidegironi.blogspot.com/2014/01/cheap-co2-meter-using-mq135-sensor-with.html
//   ttps://hackaday.io/project/3475-sniffing-trinket/log/12363-mq135-arduino-library
//
//#################################### Change log        ###############################################
// 2020-05-21 Refactored and extended by flashmark
// 2022-07-11 Refactored, first attempt for calibration
//#################################### Description       ################################################
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
//       ESP32 provides the standard ESPeasy serial configuration
//
// Conversion algorithm:
// Sensor resistance is logaritmic to the concentration of a gas. Sensors are not specific to a single
// gas. Key is the ratio between the measured Rs and a reference resistor Rzero (Rs/Rzero).
// For each specific gas a somewhat linear relation between gas concentration and the ratio (Rs/Rzero) is
// show on a log-log chart. Various algorithms are described in above mentioned literature. 
// This plugin supports 3, almost similar, algorithms to make it easier to copy a parameter set from the
// internet for a specific sensor/gas combination.
// 
// Calibration algorithm:
// Each measurement calculate Rcal assuming the concentration is at the reference level
// Remember the lowest value of Rcal assuming it belongs to measuring the lowest concentration 
///################################### Configuration data ###############################################
// This plugin uses the following predefined static data storage 
// analog input reading which is now always A0.
// P145_PCONFIG_RLOAD  PCONFIG_FLOAT(0)  RLOAD   [Ohm] 
// P145_PCONFIG_RZERO  PCONFIG_FLOAT(1)  RZERO   [Ohm]
// P145_PCONFIG_REF    PCONFIG_FLOAT(2)  REF. level [ppm]
// P145_PCONFIG_FLAGS        PCONFIG(0)        Enable compensation & Enable Calibration
// P145_PCONFIG_TEMP_TASK    PCONFIG(1)        Temperature compensation task
// P145_PCONFIG_TEMP_VAL     PCONFIG(2)        Temperature compensation value
// P145_PCONFIG_HUM_TASK     PCONFIG(3)        Humidity compensation task
// P145_PCONFIG_HUM_VAL      PCONFIG(4)        Humidity compensation value
// P145_PCONFIG_SENSORT      PCONFIG(5)        Sensor type
//#######################################################################################################

#include "_Plugin_Helper.h"
#ifdef USES_P145
#include "src/PluginStructs/P145_data_struct.h"  // MQ-xxx sensor specific data

#define PLUGIN_145
#define PLUGIN_ID_145     145           // plugin id
#define PLUGIN_NAME_145   "Gases - MQxxx (MQ135 CO2, MQ3 Alcohol) [TESTING]" // "Plugin Name" is what will be dislpayed in the selection list
#define PLUGIN_VALUENAME1_145 "level"   // variable output of the plugin. The label is in quotation marks
#define PLUGIN_145_DEBUG  false         // set to true for extra log info in the debug

// Static, per plugin instance, data stored in the generic ESPeasy structures see usage defined above
// PCONFIG(n)    : stores an integer (8)
// PCONFIG_FLOAT : stors a float (4)
// PCONFIG_LONG  : stores a long (4, shared with PCONFIG_ULONG)
// PCONFIG_ULONG : stores an unsigned long (4, shared with PCONFIG_LONG)
#define P145_PCONFIG_RLOAD       PCONFIG_FLOAT(0)
#define P145_PCONFIG_RZERO       PCONFIG_FLOAT(1)
#define P145_PCONFIG_REF         PCONFIG_FLOAT(2)
#define P145_PCONFIG_FLAGS       PCONFIG(0)
#define P145_PCONFIG_TEMP_TASK   PCONFIG(1)
#define P145_PCONFIG_TEMP_VAL    PCONFIG(2)
#define P145_PCONFIG_HUM_TASK    PCONFIG(3)
#define P145_PCONFIG_HUM_VAL     PCONFIG(4)
#define P145_PCONFIG_SENSORT     PCONFIG(5)

// PIN/port configuration is stored in the following:
#define CONFIG_PIN_AIN      CONFIG_PIN1

// A plugin has to implement the following function
// ------------------------------------------------
boolean Plugin_145(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number = PLUGIN_ID_145;
      Device[deviceCount].Type = DEVICE_TYPE_ANALOG;
      Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports = 0;
      Device[deviceCount].PullUpOption = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption = true;
      Device[deviceCount].ValueCount = 1;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption = true;
      Device[deviceCount].GlobalSyncOption = true;
      break;
    }
    
    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_145);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_145));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // FormSelector with all predefined "Sensor - Gas" options
      String options[P145_MAXTYPES] = {};
      int optionValues[P145_MAXTYPES] = {};
      int x = P145_data_struct::getNbrOfTypes();
      if (x > P145_MAXTYPES) 
      {
        x = P145_MAXTYPES;
      }
      for (int i=0; i<x; i++)
      {
        options[i] = concat(P145_data_struct::getTypeName(i), F(" - ")) + (String)P145_data_struct::getGasName(i);
        optionValues[i] = i; 
      }
      addFormSelector(F("Sensor type"), F("plugin_145_sensor"), x, options, optionValues, P145_PCONFIG_SENSORT);

# ifdef ESP32
      // Analog input selection
      addRowLabel(F("Analog Pin"));
      addADC_PinSelect(AdcPinSelectPurpose::ADC_Touch_HallEffect, F("taskdevicepin1"), CONFIG_PIN1);
# endif // ifdef ESP32

      addFormFloatNumberBox(F("Load Resistance"), F("plugin_145_RLOAD"), P145_PCONFIG_RLOAD, 0.0, 10e6, 2U);
      addUnit(F("Ohm"));
      addFormFloatNumberBox(F("R Zero"), F("plugin_145_RZERO"), P145_PCONFIG_RZERO, 0.0, 10e6, 2U);
      addUnit(F("Ohm"));
       addFormFloatNumberBox(F("Reference Level"), F("plugin_145_REFLEVEL"), P145_PCONFIG_REF, 0.0, 10e6, 2U);
      addUnit(F("ppm"));
      P145_data_struct *P145_data = static_cast<P145_data_struct *>(getPluginTaskData(event->TaskIndex));
      if (P145_data != nullptr)
      {
        float calVal = P145_data->getCalibrationValue();
        if (calVal > 0.0) 
        {
          addFormNote((String)F("Current measurement suggests Rzero= ") + (String)calVal);
        }
      }

      addFormSeparator(2);
      // Auto calibration and Temp/hum compensation flags are bitfields in P145_PCONFIG_FLAGS
      bool compensate = P145_PCONFIG_FLAGS & 0x0001;
      bool calibrate = (P145_PCONFIG_FLAGS >> 1) & 0x0001;
      addFormCheckBox(F("Enable automatic calibration"), F("plugin_145_enable_calibrarion"), calibrate);
      addFormCheckBox(F("Enable temp/humid compensation"), F("plugin_145_enable_compensation"), compensate);
      addFormNote(F("If this is enabled, the Temperature and Humidity values below need to be configured."));
      // temperature
      addHtml(F("<TR><TD>Temperature:<TD>"));
      addTaskSelect(F("plugin_145_temperature_task"), P145_PCONFIG_TEMP_TASK);
      LoadTaskSettings(P145_PCONFIG_TEMP_TASK); // we need to load the values from another task for selection!
      addHtml(F("<TR><TD>Temperature Value:<TD>"));
      addTaskValueSelect(F("plugin_145_temperature_value"), P145_PCONFIG_TEMP_VAL, P145_PCONFIG_TEMP_TASK);
      // humidity
      addHtml(F("<TR><TD>Humidity:<TD>"));
      addTaskSelect(F("plugin_145_humidity_task"), P145_PCONFIG_HUM_TASK);
      LoadTaskSettings(P145_PCONFIG_HUM_TASK); // we need to load the values from another task for selection!
      addHtml(F("<TR><TD>Humidity Value:<TD>"));
      addTaskValueSelect(F("plugin_145_humidity_value"), P145_PCONFIG_HUM_VAL, P145_PCONFIG_HUM_TASK);
      LoadTaskSettings(event->TaskIndex); // we need to restore our original taskvalues!

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P145_PCONFIG_SENSORT   = getFormItemInt(F("plugin_145_sensor"));
      P145_PCONFIG_RLOAD     = getFormItemFloat(F("plugin_145_RLOAD"));
      P145_PCONFIG_RZERO     = getFormItemFloat(F("plugin_145_RZERO"));
      P145_PCONFIG_REF       = getFormItemFloat(F("plugin_145_REFLEVEL"));
      bool compensate        = isFormItemChecked(F("plugin_145_enable_compensation") );
      bool calibrate         = isFormItemChecked(F("plugin_145_enable_calibrarion") );
      P145_PCONFIG_FLAGS     = compensate + (calibrate << 1);
      P145_PCONFIG_TEMP_TASK = getFormItemInt(F("plugin_145_temperature_task"));
      P145_PCONFIG_TEMP_VAL  = getFormItemInt(F("plugin_145_temperature_value"));
      P145_PCONFIG_HUM_TASK  = getFormItemInt(F("plugin_145_humidity_task"));
      P145_PCONFIG_HUM_VAL   = getFormItemInt(F("plugin_145_humidity_value"));

      P145_data_struct *P145_data = static_cast<P145_data_struct *>(getPluginTaskData(event->TaskIndex));
      if (P145_data != nullptr)
      {
        P145_data->setSensorData(P145_PCONFIG_SENSORT, P145_PCONFIG_FLAGS & 0x0001, (P145_PCONFIG_FLAGS >> 1) & 0x0001, P145_PCONFIG_RLOAD, P145_PCONFIG_RZERO, P145_PCONFIG_REF);
        P145_data->dump();
      }
      success = true;
      break;
    }

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
          P145_data->setSensorData(P145_PCONFIG_SENSORT, P145_PCONFIG_FLAGS & 0x0001, (P145_PCONFIG_FLAGS >> 1) & 0x0001, P145_PCONFIG_RLOAD, P145_PCONFIG_RZERO, P145_PCONFIG_REF);
          P145_data->dump();
        }
      }
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P145_data_struct *P145_data = static_cast<P145_data_struct *>(getPluginTaskData(event->TaskIndex));
      if (P145_data != nullptr)
      {
        success = P145_data->plugin_ten_per_second();
      }
      break;
    }

    case PLUGIN_READ:
    {
      P145_data_struct *P145_data = static_cast<P145_data_struct *>(getPluginTaskData(event->TaskIndex));
      if (P145_data != nullptr)
      {
        // we're checking a var from another task, so calculate that basevar
        float temperature = UserVar[P145_PCONFIG_TEMP_TASK * VARS_PER_TASK + P145_PCONFIG_TEMP_VAL]; // in degrees C
        float humidity = UserVar[P145_PCONFIG_HUM_TASK * VARS_PER_TASK + P145_PCONFIG_HUM_VAL];    // in % relative
        UserVar[event->BaseVarIndex] = P145_data->readValue(temperature, humidity);
        success = true;
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      // Update Rzero in case of autocalibration
      // TODO is there an event to signal the plugin code that the value has been updated to prevent polling?
      if ((P145_PCONFIG_FLAGS >> 1) & 0x0001)
      {
        P145_data_struct *P145_data = static_cast<P145_data_struct *>(getPluginTaskData(event->TaskIndex));
        if (P145_data != nullptr)
        {
          P145_PCONFIG_RZERO = P145_data->getAutoCalibrationValue();
        }
      }
      break;
    }
  }
  return success;
}  // function Plugin_145()
#endif  // USES_P145