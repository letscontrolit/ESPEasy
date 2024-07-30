#include "_Plugin_Helper.h"
#ifdef USES_P113

// #######################################################################################################
// ########################### Plugin 113 VL53L1X I2C Ranging LIDAR      #################################
// #######################################################################################################

/** Changelog:
 * 2024-07-29 tonhuisman: Add Region of Interest (ROI) settings for reducing the Field of View (FoV) of the sensor
 * 2024-04-25 tonhuisman: Add Direction value (1/0/-1), code improvements
 * 2023-08-11 tonhuisman: Fix issue not surfacing before, that the library right-shifts the I2C address when that is set...
 *                        Also use new/delete on sensor object (code improvement)
 *                        Limit the selection list of I2C addresses to 1 item, as changing the I2C address of the sensor does not work as
 *                        intended/expected
 * 2021-04-06 tonhuisman: Remove Interval optional attribute to avoid system overload, cleanup source
 * 2021-04-05 tonhuisman: Add VL53L1X Time of Flight sensor to main repo (similar to but not compatible with VL53L0X)
 */

// needs SparkFun_VL53L1X library from https://github.com/sparkfun/SparkFun_VL53L1X_Arduino_Library

# include "src/PluginStructs/P113_data_struct.h"

# define PLUGIN_113
# define PLUGIN_ID_113         113
# define PLUGIN_NAME_113       "Distance - VL53L1X (400cm)"
# define PLUGIN_VALUENAME1_113 "Distance"
# define PLUGIN_VALUENAME2_113 "Ambient"
# define PLUGIN_VALUENAME3_113 "Direction"


boolean Plugin_113(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_113;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_113);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_113));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_113));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_113));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      # define P113_ACTIVE_I2C_ADDRESSES 1 // Setting the address messes up the sensor, so disabled
      const uint8_t i2cAddressValues[] = { 0x29, 0x30 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c"), P113_ACTIVE_I2C_ADDRESSES, i2cAddressValues, P113_I2C_ADDRESS);
      } else {
        success = intArrayContains(P113_ACTIVE_I2C_ADDRESSES, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P113_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    # if P113_USE_ROI
    case PLUGIN_SET_DEFAULTS:
    {
      P113_ROI_X      = 16;
      P113_ROI_Y      = 16;
      P113_OPT_CENTER = 199; // Optical Center @ Center of sensor. See matrix in documentation
      break;
    }
    # endif // if P113_USE_ROI

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *optionsMode2[] = {
          F("100ms (Normal)"),
          F("20ms (Fastest)"),
          F("33ms (Fast)"),
          F("50ms"),
          F("200ms (Accurate)"),
          F("500ms"),
        };
        const int optionValuesMode2[] = { 100, 20, 33, 50, 200, 500 };
        addFormSelector(F("Timing"), F("timing"), 6, optionsMode2, optionValuesMode2, P113_TIMING);
      }

      {
        const __FlashStringHelper *optionsMode3[] = {
          F("Normal (~130cm)"),
          F("Long (~400cm)"),
        };
        const int optionValuesMode3[2] = { 0, 1 };
        addFormSelector(F("Range"), F("range"), 2, optionsMode3, optionValuesMode3, P113_RANGE);
      }
      addFormCheckBox(F("Send event when value unchanged"), F("notchanged"), P113_SEND_ALWAYS == 1);
      addFormNote(F("When checked, 'Trigger delta' setting is ignored!"));

      addFormNumericBox(F("Trigger delta"), F("delta"), P113_DELTA, 0, 100);
      addUnit(F("0-100mm"));
      # ifndef LIMIT_BUILD_SIZE
      addFormNote(F("Minimal change in Distance to trigger an event."));
      # endif // ifndef LIMIT_BUILD_SIZE

      # if P113_USE_ROI
      addFormSubHeader(F("Region Of Interest (ROI)"));

      addFormNumericBox(F("ROI 'x' SPADs"), F("roix"), P113_ROI_X, 4, 16);
      addUnit(F("4..16"));
      addFormNumericBox(F("ROI 'y' SPADs"), F("roiy"), P113_ROI_Y, 4, 16);
      addUnit(F("4..16"));
      addFormNumericBox(F("Optical Center index for ROI"), F("optcent"), P113_OPT_CENTER, 0, 255);
      addFormNote(F("Default: 199 = sensor-center, please check the documentation."));

      # endif // if P113_USE_ROI

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P113_I2C_ADDRESS = getFormItemInt(F("i2c"));
      P113_TIMING      = getFormItemInt(F("timing"));
      P113_RANGE       = getFormItemInt(F("range"));
      P113_SEND_ALWAYS = isFormItemChecked(F("notchanged")) ? 1 : 0;
      P113_DELTA       = getFormItemInt(F("delta"));
      # if P113_USE_ROI
      P113_ROI_X      = getFormItemInt(F("roix"));
      P113_ROI_Y      = getFormItemInt(F("roiy"));
      P113_OPT_CENTER = getFormItemInt(F("optcent"));
      # endif // if P113_USE_ROI

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P113_data_struct(P113_I2C_ADDRESS, P113_TIMING, P113_RANGE == 1));
      P113_data_struct *P113_data = static_cast<P113_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P113_data) && P113_data->begin(event); // Start the sensor
      break;
    }

    case PLUGIN_EXIT:
    {
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      P113_data_struct *P113_data = static_cast<P113_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P113_data) {
        const uint16_t dist      = P113_data->readDistance();
        const uint16_t ambient   = P113_data->readAmbient();
        const uint16_t p_dist    = UserVar.getFloat(event->TaskIndex, 0);
        const int16_t  direct    = dist == p_dist ? 0 : (dist < p_dist ? -1 : 1);
        const bool     triggered = (dist > p_dist + P113_DELTA) || (dist < p_dist - P113_DELTA);

        if (P113_data->isReadSuccessful() && (triggered || (P113_SEND_ALWAYS == 1)) && (dist != 0xFFFF)) {
          UserVar.setFloat(event->TaskIndex, 0, dist);
          UserVar.setFloat(event->TaskIndex, 1, ambient);
          UserVar.setFloat(event->TaskIndex, 2, direct);
          success = true;
        }
      }
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P113_data_struct *P113_data = static_cast<P113_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P113_data) {
        if (P113_data->startRead()) {
          if (P113_data->readAvailable() && (Settings.TaskDeviceTimer[event->TaskIndex] == 0)) { // Trigger as soon as there's a valid
                                                                                                 // measurement and the time-out is set to 0
            Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
          }
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P113
