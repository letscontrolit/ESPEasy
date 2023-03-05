#include "_Plugin_Helper.h"
#ifdef USES_P027

// #######################################################################################################
// ######################### Plugin 027: INA219 DC Voltage/Current sensor ################################
// #######################################################################################################

/** Changelog:
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

boolean Plugin_027(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_027;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_027);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_027));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_027));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_027));
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
        const __FlashStringHelper *optionsMode[] = { F("32V, 2A"), F("32V, 1A"), F("16V, 0.4A") };
        const int optionValuesMode[]             = { 0, 1, 2 };
        addFormSelector(F("Measure range"), F("range"), 3, optionsMode, optionValuesMode, PCONFIG(0));
      }
      {
        const __FlashStringHelper *options[] = { F("Voltage"), F("Current"), F("Power"), F("Voltage/Current/Power") };
        addFormSelector(F("Measurement Type"), F("measuretype"), 4, options, nullptr, PCONFIG(2));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("range"));
      PCONFIG(1) = getFormItemInt(F("i2c_addr"));
      PCONFIG(2) = getFormItemInt(F("measuretype"));
      success    = true;
      break;
    }

    case PLUGIN_INIT:
    {
      const uint8_t i2caddr =  P027_I2C_ADDR;

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P027_data_struct(i2caddr));
      P027_data_struct *P027_data =
        static_cast<P027_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P027_data) {
        const bool mustLog = loglevelActiveFor(LOG_LEVEL_INFO);
        String     log;

        if (mustLog) {
          log  = formatToHex(i2caddr, F("INA219 0x"), 2);
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
              log += F("16V, 400mA");
            }
            P027_data->setCalibration_16V_400mA();
            break;
          }
        }

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
        float voltage = P027_data->getBusVoltage_V() + (P027_data->getShuntVoltage_mV() / 1000);
        float current = P027_data->getCurrent_mA() / 1000;
        float power   = voltage * current;

        UserVar[event->BaseVarIndex]     = voltage;
        UserVar[event->BaseVarIndex + 1] = current;
        UserVar[event->BaseVarIndex + 2] = power;

        const bool mustLog = loglevelActiveFor(LOG_LEVEL_INFO);
        String     log;

        if (mustLog) {
          log = formatToHex(P027_I2C_ADDR, F("INA219 0x"), 2);
        }

        // for backward compability we allow the user to select if only one measurement should be returned
        // or all 3 measurements at once
        switch (PCONFIG(2)) {
          case 0:
          {
            event->sensorType            = Sensor_VType::SENSOR_TYPE_SINGLE;
            UserVar[event->BaseVarIndex] = voltage;

            if (mustLog) {
              log += F(": Voltage: ");
              log += voltage;
            }
            break;
          }
          case 1:
          {
            event->sensorType            = Sensor_VType::SENSOR_TYPE_SINGLE;
            UserVar[event->BaseVarIndex] = current;

            if (mustLog) {
              log += F(" Current: ");
              log += current;
            }
            break;
          }
          case 2:
          {
            event->sensorType            = Sensor_VType::SENSOR_TYPE_SINGLE;
            UserVar[event->BaseVarIndex] = power;

            if (mustLog) {
              log += F(" Power: ");
              log += power;
            }
            break;
          }
          case 3:
          {
            event->sensorType                = Sensor_VType::SENSOR_TYPE_TRIPLE;
            UserVar[event->BaseVarIndex]     = voltage;
            UserVar[event->BaseVarIndex + 1] = current;
            UserVar[event->BaseVarIndex + 2] = power;

            if (mustLog) {
              log += F(": Voltage: ");
              log += voltage;
              log += F(" Current: ");
              log += current;
              log += F(" Power: ");
              log += power;
            }
            break;
          }
        }

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
