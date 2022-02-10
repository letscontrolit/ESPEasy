#include "_Plugin_Helper.h"
#ifdef USES_P007

// #######################################################################################################
// #################################### Plugin 007: ExtWiredAnalog #######################################
// #######################################################################################################


# define PLUGIN_007
# define PLUGIN_ID_007         7
# define PLUGIN_NAME_007       "Analog input - PCF8591"
# define PLUGIN_VALUENAME1_007 "Analog"

# define P007_SENSOR_TYPE_INDEX  2
# define P007_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P007_SENSOR_TYPE_INDEX)))

String Plugin_007_valuename(uint8_t value_nr, bool displayString) {
  String name = F(PLUGIN_VALUENAME1_007);

  if (value_nr != 0) {
    name += String(value_nr + 1);
  }

  if (!displayString) {
    name.toLowerCase();
  }
  return name;
}

boolean Plugin_007(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_007;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].OutputDataType     = Output_Data_type_t::Simple;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_007);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P007_NR_OUTPUT_VALUES) {
          safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            Plugin_007_valuename(i, true),
            sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
          ExtraTaskSettings.TaskDeviceValueDecimals[i] = 2;
        } else {
          ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
        }
      }
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      if (PCONFIG(P007_SENSOR_TYPE_INDEX) == 0) {
        PCONFIG(P007_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_SINGLE);
      }
      event->Par1 = P004_NR_OUTPUT_VALUES;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(P007_SENSOR_TYPE_INDEX));
      event->idx        = P007_SENSOR_TYPE_INDEX;
      success           = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG(P007_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_SINGLE);

      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        ExtraTaskSettings.TaskDeviceValueDecimals[i] = 2;
      }

      success = true;
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        String  portNames[4];
        int     portValues[4];
        uint8_t unit    = (CONFIG_PORT - 1) / 4;
        uint8_t port    = CONFIG_PORT - (unit * 4);
        uint8_t address = 0x48 + unit;

        for (uint8_t x = 0; x < 4; x++) {
          portValues[x] = x + 1;
          portNames[x]  = 'A';
          portNames[x] += x;
        }
        addFormSelectorI2C(F("plugin_007_i2c"), 8, i2cAddressValues, address);
        addFormSelector(F("Port"), F("plugin_007_port"), 4, portNames, portValues, port);
        addFormNote(F("Selected Port value will be stored in first 'Values' field and consecutively for 'Number Output Values' &gt; Single."));
      } else {
        success = intArrayContains(8, i2cAddressValues, event->Par1);
      }

      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      if (PCONFIG(P007_SENSOR_TYPE_INDEX) == 0) {
        PCONFIG(P007_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_SINGLE);
      }
      uint8_t i2c  = getFormItemInt(F("plugin_007_i2c"));
      uint8_t port = getFormItemInt(F("plugin_007_port"));
      CONFIG_PORT = (((i2c - 0x48) << 2) + port);

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      uint8_t unit    = (CONFIG_PORT - 1) / 4;
      uint8_t port    = CONFIG_PORT - (unit * 4);
      uint8_t address = 0x48 + unit;

      uint8_t var = 0;

      for (; var < P007_NR_OUTPUT_VALUES; ++port, ++var) {
        if (port <= 4) { // Only read available ports, hardwired limited to 4
          Wire.beginTransmission(address);

          // get the current pin value
          Wire.write(port - 1);
          Wire.endTransmission();

          Wire.requestFrom(address, (uint8_t)0x2);

          if (Wire.available())
          {
            Wire.read();                                      // Read older value first (stored in chip)
            UserVar[event->BaseVarIndex + var] = Wire.read(); // now read actual value and store into Value var

            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              String log;
              if (log.reserve(40)) {
                log += F("PCF  : Analog port: A");
                log += port - 1;
                log += F(" value ");
                log += var + 1;
                log += ':';
                log += ' ';
                log += formatUserVarNoCheck(event->TaskIndex, var);
                addLogMove(LOG_LEVEL_INFO, log);
              }
            }
            success = true;
          }
        } else {
          UserVar[event->BaseVarIndex + var] = 0;
        }
      }

      for (; var < VARS_PER_TASK; ++var) {
        UserVar[event->BaseVarIndex + var] = 0;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P007
