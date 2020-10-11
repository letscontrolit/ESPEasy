#ifdef USES_P090

// #######################################################################################################
// ########################### Plugin 90: CCS811 Air Quality TVOC/eCO2 Sensor ###########################
// #######################################################################################################

/*
   Plugin written by Alexander Schwantes
   Includes sparkfun library https://github.com/sparkfun/SparkFun_CCS811_Arduino_Library

   There are various modes for setting up sensor:
 * Interrupt: Requires interrupt pin to signal that a new reading is available. Can read ever 1/10/60 seconds.
 * Wake: Requires a wake pin to wake device for reading when required.
 * Continuous: Takes a reading every 1/10/60 seconds.

   This plugin currently implements just the last continuous method as it requires the least number of connected pins.
   The library has provisions for the other modes.
 */

#define PLUGIN_090
#define PLUGIN_ID_090         90
#define PLUGIN_NAME_090       "Gases - CCS811 TVOC/eCO2 [TESTING]"
#define PLUGIN_VALUENAME1_090 "TVOC"
#define PLUGIN_VALUENAME2_090 "eCO2"

// int Plugin_090_WAKE_Pin;
// int Plugin_090_INT_Pin;

// #define Plugin_090_nWAKE           2
// #define Plugin_090_nINT            14

#define Plugin_090_D_AWAKE 20  // microseconds to wait before waking waking (deassert) sensor. min 20 microseconds
#define Plugin_090_T_AWAKE 100 // microseconds to wait after waking sensor. min 50 microseconds

/******************************************************************************
   CCS811 Arduino library
   Marshall Taylor @ SparkFun Electronics
   Nathan Seidle @ SparkFun Electronics
   April 4, 2017
   https://github.com/sparkfun/CCS811_Air_Quality_Breakout
   https://github.com/sparkfun/SparkFun_CCS811_Arduino_Library
   Resources:
   Uses Wire.h for i2c operation
   Development environment specifics:
   Arduino IDE 1.8.1
   This code is released under the [MIT License](http://opensource.org/licenses/MIT).
   Please review the LICENSE.md file included with this example. If you have any questions
   or concerns with licensing, please contact techsupport@sparkfun.com.
   Distributed as-is; no warranty is given.
******************************************************************************/

// **************************************************************************/
// CCS811 Library
// **************************************************************************/

#include "_Plugin_Helper.h"

#include "src/PluginStructs/P090_data_struct.h"


#define P090_I2C_ADDR                 Settings.TaskDevicePluginConfig[event->TaskIndex][0]
#define P090_COMPENSATE_ENABLE        Settings.TaskDevicePluginConfig[event->TaskIndex][1]
#define P090_TEMPERATURE_TASK_INDEX   Settings.TaskDevicePluginConfig[event->TaskIndex][2]
#define P090_TEMPERATURE_TASK_VALUE   Settings.TaskDevicePluginConfig[event->TaskIndex][3]
#define P090_HUMIDITY_TASK_INDEX      Settings.TaskDevicePluginConfig[event->TaskIndex][4]
#define P090_HUMIDITY_TASK_VALUE      Settings.TaskDevicePluginConfig[event->TaskIndex][5]
#define P090_TEMPERATURE_SCALE        Settings.TaskDevicePluginConfig[event->TaskIndex][6] // deg C/F
#define P090_READ_INTERVAL            Settings.TaskDevicePluginConfigLong[event->TaskIndex][0]


boolean Plugin_090(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_090;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_DUAL;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_090);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_090));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_090));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      // I2C address choice
      String options[2]      = { F("0x5A (ADDR pin is LOW)"), F("0x5B (ADDR pin is HIGH)") };
      int    optionValues[2] = { 0x5A, 0x5B };
      addFormSelector(F("I2C Address"), F("p090_i2c_address"), 2, options, optionValues, P090_I2C_ADDR);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        // read frequency
        int frequencyChoice        = (int)P090_READ_INTERVAL;
        String frequencyOptions[3] = { F("1 second"), F("10 seconds"), F("60 seconds") };
        int    frequencyValues[3]  = { 1, 2, 3 };
        addFormSelector(F("Take reading every"), F("p090_read_frequency"), 3, frequencyOptions, frequencyValues, frequencyChoice);
      }

      addFormSeparator(2);

      {
        // mode
        addFormCheckBox(F("Enable temp/humid compensation"), F("p090_enable_compensation"), P090_COMPENSATE_ENABLE);
        addFormNote(F("If this is enabled, the Temperature and Humidity values below need to be configured."));

        // temperature
        addRowLabel(F("Temperature"));
        addTaskSelect(F("p090_temperature_task"), P090_TEMPERATURE_TASK_INDEX);
        LoadTaskSettings(P090_TEMPERATURE_TASK_INDEX); // we need to load the values from another task for selection!
        addRowLabel(F("Temperature Value:"));
        addTaskValueSelect(F("p090_temperature_value"), P090_TEMPERATURE_TASK_VALUE, P090_TEMPERATURE_TASK_INDEX);

        // temperature scale
        int temperatureScale = P090_TEMPERATURE_SCALE;
        addRowLabel(F("Temperature Scale")); // checked
        addHtml(F("<input type='radio' id='p090_temperature_c' name='p090_temperature_scale' value='0'"));
        addHtml((temperatureScale == 0) ? F(" checked>") : F(">"));
        addHtml(F("<label for='p090_temperature_c'> &deg;C</label> &nbsp; "));
        addHtml(F("<input type='radio' id='p090_temperature_f' name='p090_temperature_scale' value='1'"));
        addHtml((temperatureScale == 1) ? F(" checked>") : F(">"));
        addHtml(F("<label for='p090_temperature_f'> &deg;F</label><br>"));

        // humidity
        addRowLabel(F("Humidity"));
        addTaskSelect(F("p090_humidity_task"), P090_HUMIDITY_TASK_INDEX);
        LoadTaskSettings(P090_HUMIDITY_TASK_INDEX); // we need to load the values from another task for selection!
        addRowLabel(F("Humidity Value"));
        addTaskValueSelect(F("p090_humidity_value"), P090_HUMIDITY_TASK_VALUE, P090_HUMIDITY_TASK_INDEX);
      }

      LoadTaskSettings(event->TaskIndex); // we need to restore our original taskvalues!
      //            addFormSeparator(string);
      addFormSeparator(2);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P090_I2C_ADDR               = getFormItemInt(F("p090_i2c_address"));
      P090_COMPENSATE_ENABLE      = isFormItemChecked(F("p090_enable_compensation"));
      P090_TEMPERATURE_TASK_INDEX = getFormItemInt(F("p090_temperature_task"));
      P090_TEMPERATURE_TASK_VALUE = getFormItemInt(F("p090_temperature_value"));
      P090_HUMIDITY_TASK_INDEX    = getFormItemInt(F("p090_humidity_task"));
      P090_HUMIDITY_TASK_VALUE    = getFormItemInt(F("p090_humidity_value"));
      P090_TEMPERATURE_SCALE      = getFormItemInt(F("p090_temperature_scale"));
      P090_READ_INTERVAL          = getFormItemInt(F("p090_read_frequency"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P090_data_struct(P090_I2C_ADDR));
      P090_data_struct *P090_data =
        static_cast<P090_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P090_data) {
        break;
      }

      // Plugin_090_WAKE_Pin = Settings.TaskDevicePin1[event->TaskIndex];
      CCS811Core::status returnCode;
      returnCode = P090_data->myCCS811.begin();

      #ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("CCS811 : Begin exited with: ");
        log += P090_data->myCCS811.getDriverError(returnCode);
        addLog(LOG_LEVEL_DEBUG, log);
      }
      #endif // ifndef BUILD_NO_DEBUG
      UserVar[event->BaseVarIndex]     = NAN;
      UserVar[event->BaseVarIndex + 1] = NAN;

      // This sets the mode to 1 second reads, and prints returned error status.
      // Mode 0 = Idle (not used)
      // Mode 1 = read every 1s
      // Mode 2 = every 10s
      // Mode 3 = every 60s
      // Mode 4 = RAW mode (not used)
      returnCode = P090_data->myCCS811.setDriveMode(P090_READ_INTERVAL);

      if (returnCode != CCS811Core::SENSOR_SUCCESS) {
      #ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("CCS811 : Mode request exited with: ");
          log += P090_data->myCCS811.getDriverError(returnCode);
          addLog(LOG_LEVEL_DEBUG, log);
        }
      #endif // ifndef BUILD_NO_DEBUG
      } else {
        success = true;
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P090_data_struct *P090_data =
        static_cast<P090_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P090_data) {
        break;
      }

      if (P090_data->myCCS811.dataAvailable())
      {
        // Calling readAlgorithmResults() function updates the global tVOC and CO2 variables
        CCS811Core::status readstatus = P090_data->myCCS811.readAlgorithmResults();

        if (readstatus == 0)
        {
          success = true;

          if (P090_data->compensation_set) {
            // Temp compensation was set, so we have to dump the first reading.
            P090_data->compensation_set = false;
          } else {
            UserVar[event->BaseVarIndex]     = P090_data->myCCS811.getTVOC();
            UserVar[event->BaseVarIndex + 1] = P090_data->myCCS811.getCO2();
            P090_data->newReadingAvailable   = true;

            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              String log = F("CCS811 : tVOC: ");
              log += P090_data->myCCS811.getTVOC();
              log += F(", eCO2: ");
              log += P090_data->myCCS811.getCO2();
              addLog(LOG_LEVEL_INFO, log);
            }
          }
        }
      }

      break;
    }

    case PLUGIN_READ:
    {
      P090_data_struct *P090_data =
        static_cast<P090_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P090_data) {
        break;
      }

      // if CCS811 is compensated with temperature and humidity
      if (P090_COMPENSATE_ENABLE)
      {
        // we're checking a var from another task, so calculate that basevar
        byte  TaskIndex    = P090_TEMPERATURE_TASK_INDEX;
        byte  BaseVarIndex = TaskIndex * VARS_PER_TASK + P090_TEMPERATURE_TASK_VALUE;
        float temperature  = UserVar[BaseVarIndex]; // in degrees C
        // convert to celsius if required
        int temperature_in_fahrenheit = P090_TEMPERATURE_SCALE;
        String temp                   = F("C");

        if (temperature_in_fahrenheit)
        {
          temperature = ((temperature - 32) * 5.0f) / 9.0f;
          temp        =  F("F");
        }

        byte  TaskIndex2    = P090_HUMIDITY_TASK_INDEX;
        byte  BaseVarIndex2 = TaskIndex2 * VARS_PER_TASK + P090_HUMIDITY_TASK_VALUE;
        float humidity      = UserVar[BaseVarIndex2]; // in % relative

      #ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("CCS811 : Compensating for Temperature: ");
          log += String(temperature) + temp + F(" & Humidity: ") + String(humidity) + F("%");
          addLog(LOG_LEVEL_DEBUG, log);
        }
      #endif // ifndef BUILD_NO_DEBUG

        P090_data->myCCS811.setEnvironmentalData(humidity, temperature);

        P090_data->compensation_set = true;
      }

      if (P090_data->newReadingAvailable) {
        P090_data->newReadingAvailable = false;
        success                        = true;
      }
      else if (P090_data->myCCS811.checkForStatusError())
      {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          // If the CCS811 found an internal error, print it.
          String log = F("CCS811 : Error: ");
          log += P090_data->myCCS811.getSensorError();
          addLog(LOG_LEVEL_ERROR, log);
        }
      }

      /*
         else
         {
         addLog(LOG_LEVEL_ERROR, F("CCS811 : No values found."));
         }
       */

      break;
    }
  } // switch

  return success;
}   // Plugin_090

#endif // ifdef USES_P090
