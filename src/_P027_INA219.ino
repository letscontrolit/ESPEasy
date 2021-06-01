#include "_Plugin_Helper.h"
#ifdef USES_P027

// #######################################################################################################
// ######################### Plugin 027: INA219 DC Voltage/Current sensor ################################
// #######################################################################################################


#include "src/PluginStructs/P027_data_struct.h"

#define PLUGIN_027
#define PLUGIN_ID_027         27
#define PLUGIN_NAME_027       "Energy (DC) - INA219"
#define PLUGIN_VALUENAME1_027 "Voltage"
#define PLUGIN_VALUENAME2_027 "Current"
#define PLUGIN_VALUENAME3_027 "Power"

#define P027_I2C_ADDR    (uint8_t)PCONFIG(1)

boolean Plugin_027(byte function, struct EventStruct *event, String& string)
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

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      int Plugin_27_i2c_addresses[4] = { INA219_ADDRESS, INA219_ADDRESS2, INA219_ADDRESS3, INA219_ADDRESS4 };
      addFormSelectorI2C(F("i2c_addr"), 4, Plugin_27_i2c_addresses, P027_I2C_ADDR);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        byte choiceMode = PCONFIG(0);
        const __FlashStringHelper * optionsMode[3];
        optionsMode[0] = F("32V, 2A");
        optionsMode[1] = F("32V, 1A");
        optionsMode[2] = F("16V, 0.4A");
        int optionValuesMode[3];
        optionValuesMode[0] = 0;
        optionValuesMode[1] = 1;
        optionValuesMode[2] = 2;
        addFormSelector(F("Measure range"), F("p027_range"), 3, optionsMode, optionValuesMode, choiceMode);
      }
      {
        byte   choiceMeasureType = PCONFIG(2);
        const __FlashStringHelper * options[4]        = { F("Voltage"), F("Current"), F("Power"), F("Voltage/Current/Power") };
        addFormSelector(F("Measurement Type"), F("p027_measuretype"), 4, options, NULL, choiceMeasureType);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("p027_range"));
      PCONFIG(1) = getFormItemInt(F("i2c_addr"));
      PCONFIG(2) = getFormItemInt(F("p027_measuretype"));
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
        String log;
        if (mustLog) {
          log = F("INA219 0x");
          log += String(i2caddr, HEX);
          log += F(" setting Range to: ");
        }

        switch (PCONFIG(0))
        {
          case 0:
          {
            if (mustLog)
              log += F("32V, 2A");
            P027_data->setCalibration_32V_2A();
            break;
          }
          case 1:
          {
            if (mustLog)
              log += F("32V, 1A");
            P027_data->setCalibration_32V_1A();
            break;
          }
          case 2:
          {
            if (mustLog)
              log += F("16V, 400mA");
            P027_data->setCalibration_16V_400mA();
            break;
          }
        }
        if (mustLog)
          addLog(LOG_LEVEL_INFO, log);
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
        String log;
        if (mustLog) {
          log = F("INA219 0x");
          log += String(P027_I2C_ADDR, HEX);
        }

        // for backward compability we allow the user to select if only one measurement should be returned
        // or all 3 measurement at once
        switch (PCONFIG(2))
        {
          case 0:
          {
            event->sensorType            = Sensor_VType::SENSOR_TYPE_SINGLE;
            UserVar[event->BaseVarIndex] = voltage;
            if (mustLog) {
              log                         += F(": Voltage: ");
              log                         += voltage;
            }
            break;
          }
          case 1:
          {
            event->sensorType            = Sensor_VType::SENSOR_TYPE_SINGLE;
            UserVar[event->BaseVarIndex] = current;
            if (mustLog) {
              log                         += F(" Current: ");
              log                         += current;
            }
            break;
          }
          case 2:
          {
            event->sensorType            = Sensor_VType::SENSOR_TYPE_SINGLE;
            UserVar[event->BaseVarIndex] = power;
            if (mustLog) {
              log                         += F(" Power: ");
              log                         += power;
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
              log                             += F(": Voltage: ");
              log                             += voltage;
              log                             += F(" Current: ");
              log                             += current;
              log                             += F(" Power: ");
              log                             += power;
            }
            break;
          }
        }
        if (mustLog)
          addLog(LOG_LEVEL_INFO, log);
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P027
