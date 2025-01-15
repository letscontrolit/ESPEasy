#include "_Plugin_Helper.h"

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

# define PLUGIN_090
# define PLUGIN_ID_090         90
# define PLUGIN_NAME_090       "Gases - CCS811 TVOC/eCO2"
# define PLUGIN_VALUENAME1_090 "TVOC"
# define PLUGIN_VALUENAME2_090 "eCO2"

// int Plugin_090_WAKE_Pin;
// int Plugin_090_INT_Pin;

// #define Plugin_090_nWAKE           2
// #define Plugin_090_nINT            14

# define Plugin_090_D_AWAKE 20  // microseconds to wait before waking waking (deassert) sensor. min 20 microseconds
# define Plugin_090_T_AWAKE 100 // microseconds to wait after waking sensor. min 50 microseconds

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


# include "src/PluginStructs/P090_data_struct.h"


# define P090_I2C_ADDR                 PCONFIG(0)
# define P090_COMPENSATE_ENABLE        PCONFIG(1)
# define P090_TEMPERATURE_TASK_INDEX   PCONFIG(2)
# define P090_TEMPERATURE_TASK_VALUE   PCONFIG(3)
# define P090_HUMIDITY_TASK_INDEX      PCONFIG(4)
# define P090_HUMIDITY_TASK_VALUE      PCONFIG(5)
# define P090_TEMPERATURE_SCALE        PCONFIG(6) // deg C/F
# define P090_READ_INTERVAL            PCONFIG_LONG(0)


boolean Plugin_090(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_090;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_DUAL;
      dev.FormulaOption  = true;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
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

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const int i2cAddressValues[] = { 0x5A, 0x5B };

      // I2C address choice
      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        const __FlashStringHelper *options[] = { F("0x5A (ADDR pin is LOW)"), F("0x5B (ADDR pin is HIGH)") };
        constexpr size_t optionCount         = NR_ELEMENTS(options);
        addFormSelector(F("I2C Address"), F("i2c_addr"), optionCount, options, i2cAddressValues, P090_I2C_ADDR);
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P090_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        // read frequency
        const int frequencyChoice                     = P090_READ_INTERVAL;
        const __FlashStringHelper *frequencyOptions[] = { F("1 second"), F("10 seconds"), F("60 seconds") };
        const int frequencyValues[]                   = { 1, 2, 3 };
        constexpr size_t optionCount                  = NR_ELEMENTS(frequencyValues);
        addFormSelector(F("Take reading every"), F("temp_freq"), optionCount, frequencyOptions, frequencyValues, frequencyChoice);
      }

      addFormSeparator(2);

      {
        // mode
        addFormCheckBox(F("Enable temp/humid compensation"), F("en_comp"), P090_COMPENSATE_ENABLE);
        # ifndef BUILD_NO_DEBUG
        addFormNote(F("If this is enabled, the Temperature and Humidity values below need to be configured."));
        # endif // ifndef BUILD_NO_DEBUG

        // temperature
        addRowLabel(F("Temperature"));
        addTaskSelect(F("temp_task"), P090_TEMPERATURE_TASK_INDEX);

        if (validTaskIndex(P090_TEMPERATURE_TASK_INDEX)) {
          addRowLabel(F("Temperature Value:"));
          addTaskValueSelect(F("temp_val"), P090_TEMPERATURE_TASK_VALUE, P090_TEMPERATURE_TASK_INDEX);

          // temperature scale
          int temperatureScale = P090_TEMPERATURE_SCALE;
          addRowLabel(F("Temperature Scale")); // checked
          addHtml(F("<input type='radio' id='p090_temperature_c' name='temp_scale' value='0'"));
          addHtml((temperatureScale == 0) ? F(" checked>") : F(">"));
          addHtml(F("<label for='p090_temperature_c'> &deg;C</label> &nbsp; "));
          addHtml(F("<input type='radio' id='p090_temperature_f' name='temp_scale' value='1'"));
          addHtml((temperatureScale == 1) ? F(" checked>") : F(">"));
          addHtml(F("<label for='p090_temperature_f'> &deg;F</label><br>"));

          // humidity
          addRowLabel(F("Humidity"));
          addTaskSelect(F("hum_task"), P090_HUMIDITY_TASK_INDEX);

          if (validTaskIndex(P090_HUMIDITY_TASK_INDEX)) {
            addRowLabel(F("Humidity Value"));
            addTaskValueSelect(F("hum_val"), P090_HUMIDITY_TASK_VALUE, P090_HUMIDITY_TASK_INDEX);
          }
        }
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P090_I2C_ADDR               = getFormItemInt(F("i2c_addr"));
      P090_COMPENSATE_ENABLE      = isFormItemChecked(F("en_comp"));
      P090_TEMPERATURE_TASK_INDEX = getFormItemInt(F("temp_task"));
      P090_TEMPERATURE_TASK_VALUE = getFormItemInt(F("temp_val"));
      P090_HUMIDITY_TASK_INDEX    = getFormItemInt(F("hum_task"));
      P090_HUMIDITY_TASK_VALUE    = getFormItemInt(F("hum_val"));
      P090_TEMPERATURE_SCALE      = getFormItemInt(F("temp_scale"));
      P090_READ_INTERVAL          = getFormItemInt(F("temp_freq"));

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

      // Plugin_090_WAKE_Pin = CONFIG_PIN1;
      CCS811Core::status returnCode;
      returnCode = P090_data->myCCS811.begin();

      # ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        addLogMove(LOG_LEVEL_DEBUG, concat(F("CCS811 : Begin exited with: "), P090_data->myCCS811.getDriverError(returnCode)));
      }
      # endif // ifndef BUILD_NO_DEBUG
      UserVar.setFloat(event->TaskIndex, 0, NAN);
      UserVar.setFloat(event->TaskIndex, 1, NAN);

      // This sets the mode to 1 second reads, and prints returned error status.
      // Mode 0 = Idle (not used)
      // Mode 1 = read every 1s
      // Mode 2 = every 10s
      // Mode 3 = every 60s
      // Mode 4 = RAW mode (not used)
      returnCode = P090_data->myCCS811.setDriveMode(P090_READ_INTERVAL);

      if (returnCode != CCS811Core::SENSOR_SUCCESS) {
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLogMove(LOG_LEVEL_DEBUG, concat(F("CCS811 : Mode request exited with: "), P090_data->myCCS811.getDriverError(returnCode)));
        }
        # endif // ifndef BUILD_NO_DEBUG
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
            UserVar.setFloat(event->TaskIndex, 0, P090_data->myCCS811.getTVOC());
            UserVar.setFloat(event->TaskIndex, 1, P090_data->myCCS811.getCO2());
            P090_data->newReadingAvailable = true;

            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              addLogMove(LOG_LEVEL_INFO,
                         strformat(F("CCS811 : tVOC: %d, eCO2: %d"), P090_data->myCCS811.getTVOC(), P090_data->myCCS811.getCO2()));
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
        const taskIndex_t TaskIndex = P090_TEMPERATURE_TASK_INDEX;

        if (validTaskIndex(TaskIndex)) {
          float temperature = UserVar.getFloat(TaskIndex, P090_TEMPERATURE_TASK_VALUE); // in degrees C
          // convert to celsius if required
          # ifndef BUILD_NO_DEBUG
          char temp('C');
          # endif // ifndef BUILD_NO_DEBUG

          if (P090_TEMPERATURE_SCALE)
          {
            temperature = CelsiusToFahrenheit(temperature);
            # ifndef BUILD_NO_DEBUG
            temp = 'F';
            # endif // ifndef BUILD_NO_DEBUG
          }

          const taskIndex_t TaskIndex2 = P090_HUMIDITY_TASK_INDEX;

          if (validTaskIndex(TaskIndex2)) {
            const float humidity = UserVar.getFloat(TaskIndex2, P090_HUMIDITY_TASK_VALUE); // in % relative

            # ifndef BUILD_NO_DEBUG

            if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
              addLogMove(LOG_LEVEL_DEBUG, strformat(F("CCS811 : Compensating for Temperature: %s%c & Humidity: %s%%"),
                                                    toString(temperature).c_str(), temp, toString(humidity).c_str()));
            }
            # endif // ifndef BUILD_NO_DEBUG

            P090_data->myCCS811.setEnvironmentalData(humidity, temperature);
          }
        }
        P090_data->compensation_set = true;
      }

      if (P090_data->newReadingAvailable) {
        P090_data->newReadingAvailable = false;
        success                        = true;
      }
      else if (P090_data->myCCS811.checkForStatusError())
      {
        // get error and also clear it
        const String errorMsg = P090_data->myCCS811.getSensorError();

        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          // If the CCS811 found an internal error, print it.
          addLogMove(LOG_LEVEL_ERROR, concat(F("CCS811 : Error: "), errorMsg));
        }
      }

      break;
    }
  } // switch

  return success;
}   // Plugin_090

#endif // ifdef USES_P090
