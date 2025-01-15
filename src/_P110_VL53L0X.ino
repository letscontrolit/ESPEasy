#ifdef USES_P110

// #######################################################################################################
// ########################### Plugin 110 VL53L0X I2C Ranging LIDAR      #################################
// #######################################################################################################
// ###################################### stefan@clumsy.ch      ##########################################
// #######################################################################################################

/** Changelog:
 * 2024-04-27 tonhuisman: Read sensor asynchronously to enable (the new default) trigger on changed value
 * 2024-04-26 tonhuisman: Migrate 'Send event when value unchanged' and 'Trigger delta' settings from P113 (at last...)
 *                        Add Direction value, -1 = closer, 0 = unchanged, 1 = further away
 * 2022-06-22 tonhuisman: Remove delay() call from begin(), handle delay via PLUGIN_FIFTY_PER_SECOND
 *                        Reformat source (uncrustify)
 * 2021-04-05 tonhuisman: Removed check for VL53L1X as that is not compatible with this driver (Got its own plugin P113)
 * 2021-02-06 tonhuisman: Refactored to use PluginStruct to enable multiple-instance use with an I2C Multiplexer
 * 2021-01-07 tonhuisman: Moved from PluginPlayground (P133) to main repo (P110), fixed some issues
 */

// needs VL53L0X library from pololu https://github.com/pololu/vl53l0x-arduino

#include "src/PluginStructs/P110_data_struct.h"

#define PLUGIN_110
#define PLUGIN_ID_110         110
#define PLUGIN_NAME_110       "Distance - VL53L0X (200cm)"
#define PLUGIN_VALUENAME1_110 "Distance"
#define PLUGIN_VALUENAME2_110 "Direction"


///////////////////////////
// VL53L0X Command Codes //
///////////////////////////

boolean Plugin_110(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_110;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.FormulaOption  = true;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.TimerOptional  = true;
      dev.PluginStats    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_110);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_110));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_110));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x29, 0x30 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2cAddr"), 2, i2cAddressValues, P110_I2C_ADDRESS);
        #ifndef BUILD_NO_DEBUG
        addFormNote(F("SDO Low=0x29, High=0x30"));
        #endif // ifndef BUILD_NO_DEBUG
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    #if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P110_I2C_ADDRESS;
      success     = true;
      break;
    }
    #endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *optionsMode2[] = {
          F("Normal"),
          F("Fast"),
          F("Accurate") };
        const int optionValuesMode2[] = { 80, 20, 320 };
        constexpr size_t optionCount  = NR_ELEMENTS(optionValuesMode2);
        addFormSelector(F("Timing"), F("ptiming"), optionCount, optionsMode2, optionValuesMode2, P110_TIMING);
      }

      {
        const __FlashStringHelper *optionsMode3[] = {
          F("Normal"),
          F("Long") };
        constexpr size_t optionCount = NR_ELEMENTS(optionsMode3);
        addFormSelector(F("Range"), F("prange"), optionCount, optionsMode3, nullptr, P110_RANGE);
      }
      addFormCheckBox(F("Send event when value unchanged"), F("notchanged"), P110_SEND_ALWAYS == 1);
      addFormNote(F("When checked, 'Trigger delta' setting is ignored!"));

      addFormNumericBox(F("Trigger delta"), F("delta"), P110_DELTA, 0, 100);
      addUnit(F("0-100mm"));
      #ifndef LIMIT_BUILD_SIZE
      addFormNote(F("Minimal change in Distance to trigger an event."));
      #endif // ifndef LIMIT_BUILD_SIZE

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P110_I2C_ADDRESS = getFormItemInt(F("i2cAddr"));
      P110_TIMING      = getFormItemInt(F("ptiming"));
      P110_RANGE       = getFormItemInt(F("prange"));
      P110_SEND_ALWAYS = isFormItemChecked(F("notchanged")) ? 1 : 0;
      P110_DELTA       = getFormItemInt(F("delta"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P110_data_struct(P110_I2C_ADDRESS, P110_TIMING, P110_RANGE == 1));
      P110_data_struct *P110_data = static_cast<P110_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P110_data) {
        const uint32_t interval_ms = Settings.TaskDeviceTimer[event->TaskIndex] * 1000;

        // Clear the "previous" distance so there will be a new result when starting the task
        UserVar.setFloat(event->TaskIndex, 3, -1);
        success = P110_data->begin(interval_ms); // Start the sensor
      }
      break;
    }
    case PLUGIN_READ:
    {
      P110_data_struct *P110_data = static_cast<P110_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P110_data) {
        success = P110_data->plugin_read(event);
      }
      break;
    }
    case PLUGIN_TEN_PER_SECOND: // Handle startup delay and sensor reading
    {
      P110_data_struct *P110_data = static_cast<P110_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P110_data) {
        success = P110_data->check_reading_ready(event);
      }
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P110
