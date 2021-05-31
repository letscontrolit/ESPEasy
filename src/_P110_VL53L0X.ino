#ifdef USES_P110
//#######################################################################################################
//########################### Plugin 110 VL53L0X I2C Ranging LIDAR      #################################
//#######################################################################################################
//###################################### stefan@clumsy.ch      ##########################################
//#######################################################################################################

// Changelog:
// 2021-04-05, tonhuisman: Removed check for VL53L1X as that is not compatible with this driver (Got its own plugin P113)
// 2021-02-06, tonhuisman: Refactored to use PluginStruct to enable multiple-instance use with an I2C Multiplexer
// 2021-01-07, tonhuisman: Moved from PluginPlayground (P133) to main repo (P110), fixed some issues

// needs VL53L0X library from pololu https://github.com/pololu/vl53l0x-arduino

#include "src/PluginStructs/P110_data_struct.h"

#define PLUGIN_110
#define PLUGIN_ID_110         110
#define PLUGIN_NAME_110       "Distance - VL53L0X (200cm) [TESTING]"
#define PLUGIN_VALUENAME1_110 "Distance"


////////////////////////////
// VL53L0X Command Codes //
////////////////////////////

boolean Plugin_110_init[3] = {false, false, false};

boolean Plugin_110(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_110;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
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
        string = F(PLUGIN_NAME_110);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_110));
        break;
      }
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
      {
        byte choice = PCONFIG(0);
        int optionValues[2] = { 0x29, 0x30 };
        addFormSelectorI2C(F("plugin_110_vl53l0x_i2c"), 2, optionValues, choice);
        addFormNote(F("SDO Low=0x29, High=0x30"));
        break;
      }
    case PLUGIN_WEBFORM_LOAD:
      {
        unsigned int choiceMode2 = PCONFIG(1);
        {
          const __FlashStringHelper * optionsMode2[3];
          optionsMode2[0] = F("Normal");
          optionsMode2[1] = F("Fast");
          optionsMode2[2] = F("Accurate");
          int optionValuesMode2[3] = { 80, 20, 320 };
          addFormSelector(F("Timing"), F("plugin_110_vl53l0x_timing"), 3, optionsMode2, optionValuesMode2, choiceMode2);
        }

        int choiceMode3 = PCONFIG(2);
        {
          const __FlashStringHelper * optionsMode3[2];
          optionsMode3[0] = F("Normal");
          optionsMode3[1] = F("Long");
          int optionValuesMode3[2] = {0, 1 };
          addFormSelector(F("Range"), F("plugin_110_vl53l0x_range"), 2, optionsMode3, optionValuesMode3, choiceMode3);
        }

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = getFormItemInt(F("plugin_110_vl53l0x_i2c"));
        PCONFIG(1) = getFormItemInt(F("plugin_110_vl53l0x_timing"));
        PCONFIG(2) = getFormItemInt(F("plugin_110_vl53l0x_range"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        initPluginTaskData(event->TaskIndex, new (std::nothrow) P110_data_struct(PCONFIG(0), PCONFIG(1), PCONFIG(2) == 1));
        P110_data_struct *P110_data = static_cast<P110_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P110_data) {
          P110_data->begin(); // Start the sensor
          success = true;
        }
        break;
      }
    case PLUGIN_READ:
      {
        P110_data_struct *P110_data = static_cast<P110_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P110_data) {
          long dist = P110_data->readDistance();
          if (P110_data->isReadSuccessful()) {
            UserVar[event->BaseVarIndex] = dist;  // Value is classified as invalid when > 8190, so no conversion or 'split' needed
            success = true;
          }
        }
        break;
      }

  }
  return success;
}


#endif
