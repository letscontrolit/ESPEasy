#include "_Plugin_Helper.h"
#ifdef USES_P027

// #######################################################################################################
// ######################### Plugin 027: INA219 DC Voltage/Current sensor ################################
// #######################################################################################################

/** Changelog:
 * 2024-07-16 tonhuisman: Set INA219 in Powerdown mode when not actually measuring, to reduce quiescent current
 * 2022-04-02 tonhuisman: Add all technically possible I2C addresses (16), instead of only the 4 most common
 *                        As requested in the forum: https://www.letscontrolit.com/forum/viewtopic.php?t=9079
 * (No previous changelog registered)
 *************************************************************************************************/

# include "src/PluginStructs/P027_data_struct.h"

# define PLUGIN_027
# define PLUGIN_ID_027         27
# define PLUGIN_NAME_027       "Energy (DC) - INA219"
# define PLUGIN_VALUENAME1_027 "Voltage"
# define PLUGIN_VALUENAME2_027 "Current"
# define PLUGIN_VALUENAME3_027 "Power"

# define P027_I2C_ADDR    (uint8_t)PCONFIG(1)

void P027_SetTaskDeviceValueNames(int16_t measureType) {
  ExtraTaskSettings.clearTaskDeviceValueName(1);
  ExtraTaskSettings.clearTaskDeviceValueName(2);

  switch (measureType) {
    case 0:
      ExtraTaskSettings.setTaskDeviceValueName(0, F(PLUGIN_VALUENAME1_027));
      break;
    case 1:
      ExtraTaskSettings.setTaskDeviceValueName(0, F(PLUGIN_VALUENAME2_027));
      break;
    case 2:
      ExtraTaskSettings.setTaskDeviceValueName(0, F(PLUGIN_VALUENAME3_027));
      break;
    case 3:
      ExtraTaskSettings.setTaskDeviceValueName(0, F(PLUGIN_VALUENAME1_027));
      ExtraTaskSettings.setTaskDeviceValueName(1, F(PLUGIN_VALUENAME2_027));
      ExtraTaskSettings.setTaskDeviceValueName(2, F(PLUGIN_VALUENAME3_027));
      break;
  }
}

boolean Plugin_027(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_027;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_TRIPLE;
      dev.FormulaOption  = true;
      dev.ValueCount     = 3;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_027);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      P027_SetTaskDeviceValueNames(PCONFIG(2));

      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = 3 == PCONFIG(2) ? 3 : 1;
      success     = true;
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      // Many boards, like Adafruit INA219: https://learn.adafruit.com/adafruit-ina219-current-sensor-breakout/assembly
      // A0 and A1 are default connected to GND with 10k pull-down resistor.
      // To select another address, bridge either A0 and/or A1 to set to VS+, SDA or SCL signale.
      //  (0x40) 1000000 (A0=GND, A1=GND)
      //  (0x41) 1000001 (A0=VS+, A1=GND)
      //  (0x44) 1000100 (A0=GND, A1=VS+)
      //  (0x45) 1000101 (A0=VS+, A1=VS+)
      //  (0x42) 1000010 (A0=SDA, A1=GND)
      //  (0x43) 1000011 (A0=SCL, A1=GND)
      //  (0x46) 1000110 (A0=SDA, A1=VS+)
      //  (0x47) 1000111 (A0=SCL, A1=VS+)
      //  (0x48) 1001000 (A0=GND, A1=SDA)
      //  (0x49) 1001001 (A0=VS+, A1=SDA)
      //  (0x4A) 1001010 (A0=SDA, A1=SDA)
      //  (0x4B) 1001011 (A0=SCL, A1=SDA)
      //  (0x4C) 1001100 (A0=GND, A1=SCL)
      //  (0x4D) 1001101 (A0=VS+, A1=SCL)
      //  (0x4E) 1001110 (A0=SDA, A1=SCL)
      //  (0x4F) 1001111 (A0=SCL, A1=SCL)

      const uint8_t i2cAddressValues[] = { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
                                           0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 16, i2cAddressValues, P027_I2C_ADDR);
      } else {
        success = intArrayContains(16, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P027_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *optionsMode[] = { F("32V, 2A"), F("32V, 1A"), F("16V, 0.4A"), F("26V, 8A") };
        addFormSelector(F("Measure range"), F("range"), 4, optionsMode, nullptr, PCONFIG(0));
      }
      {
        const __FlashStringHelper *options[] = { F("Voltage"), F("Current"), F("Power"), F("Voltage/Current/Power") };
        addFormSelector(F("Measurement Type"), F("measuretype"), 4, options, nullptr, PCONFIG(2));
      }
      # if P027_FEATURE_POWERDOWN
      addFormCheckBox(F("Use Powerdown mode"), F("pwrdwn"), PCONFIG(3) == 1);
      # endif // if P027_FEATURE_POWERDOWN

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      const int16_t lastType = PCONFIG(2);
      PCONFIG(0) = getFormItemInt(F("range"));
      PCONFIG(1) = getFormItemInt(F("i2c_addr"));
      PCONFIG(2) = getFormItemInt(F("measuretype"));
      # if P027_FEATURE_POWERDOWN
      PCONFIG(3) = isFormItemChecked(F("pwrdwn")) ? 1 : 0;
      # endif // if P027_FEATURE_POWERDOWN

      if (lastType != PCONFIG(2)) {
        P027_SetTaskDeviceValueNames(PCONFIG(2));
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // const uint8_t i2caddr =  P027_I2C_ADDR;

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P027_data_struct(P027_I2C_ADDR));
      P027_data_struct *P027_data =
        static_cast<P027_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P027_data) {
        const bool mustLog = loglevelActiveFor(LOG_LEVEL_INFO);
        String     log;

        if (mustLog) {
          log  = formatToHex(P027_I2C_ADDR, F("INA219 0x"), 2);
          log += F(" setting Range to: ");
        }

        switch (PCONFIG(0)) {
          case 0:
          {
            if (mustLog) {
              log += F("32V, 2A");
            }
            P027_data->setCalibration_32V_2A();
            break;
          }
          case 1:
          {
            if (mustLog) {
              log += F("32V, 1A");
            }
            P027_data->setCalibration_32V_1A();
            break;
          }
          case 2:
          {
            if (mustLog) {
              log += F("16V, 0.4A");
            }
            P027_data->setCalibration_16V_400mA();
            break;
          }
          case 3:
          {
            if (mustLog) {
              log += F("26V, 8A");
            }
            P027_data->setCalibration_26V_8A();
            break;
          }
        }

        # if P027_FEATURE_POWERDOWN

        if (1 == PCONFIG(3)) {
          P027_data->setPowerDown(); // Put sensor in powerdown mode
        }
        # endif // if P027_FEATURE_POWERDOWN

        if (mustLog) {
          addLogMove(LOG_LEVEL_INFO, log);
        }
        success = true;
      }
      break;
    }

    case PLUGIN_READ:
    {
      // shuntvoltage = getShuntVoltage_mV();
      // busvoltage = getBusVoltage_V();
      // current_mA = getCurrent_mA();
      // loadvoltage = getBusVoltage_V() + (getShuntVoltage_mV() / 1000);
      P027_data_struct *P027_data =
        static_cast<P027_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P027_data) {
        # if P027_FEATURE_POWERDOWN

        if (1 == PCONFIG(3)) {
          P027_data->setActiveMode(); // Get sensor out of powerdown mode
        }
        # endif // if P027_FEATURE_POWERDOWN
        const float voltage = P027_data->getBusVoltage_V() + (P027_data->getShuntVoltage_mV() / 1000.0f);
        const float current = P027_data->getCurrent_mA() / 1000.0f;
        const float power   = voltage * current;

        const bool mustLog = loglevelActiveFor(LOG_LEVEL_INFO);
        String     log;

        if (mustLog) {
          log = formatToHex(P027_I2C_ADDR, F("INA219 0x"), 2);
        }

        // for backward compatability we allow the user to select if only one measurement should be returned
        // or all 3 measurements at once
        // event->sensorType = Sensor_VType::SENSOR_TYPE_SINGLE;

        switch (PCONFIG(2)) {
          case 0:
          {
            UserVar.setFloat(event->TaskIndex, 0, voltage);

            if (mustLog) {
              log += F(" Voltage: ");
              log += voltage;
            }
            break;
          }
          case 1:
          {
            UserVar.setFloat(event->TaskIndex, 0, current);

            if (mustLog) {
              log += F(" Current: ");
              log += current;
            }
            break;
          }
          case 2:
          {
            UserVar.setFloat(event->TaskIndex, 0, power);

            if (mustLog) {
              log += F(" Power: ");
              log += power;
            }
            break;
          }
          case 3:
          {
            // event->sensorType = Sensor_VType::SENSOR_TYPE_TRIPLE;
            UserVar.setFloat(event->TaskIndex, 0, voltage);
            UserVar.setFloat(event->TaskIndex, 1, current);
            UserVar.setFloat(event->TaskIndex, 2, power);

            if (mustLog) {
              log += F(" Voltage: ");
              log += voltage;
              log += F(" Current: ");
              log += current;
              log += F(" Power: ");
              log += power;
            }
            break;
          }
        }

        # if P027_FEATURE_POWERDOWN

        if (1 == PCONFIG(3)) {
          P027_data->setPowerDown(); // Put sensor in powerdown mode
        }
        # endif // if P027_FEATURE_POWERDOWN

        if (mustLog) {
          addLogMove(LOG_LEVEL_INFO, log);
        }
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P027
