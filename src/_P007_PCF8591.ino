#include "_Plugin_Helper.h"
#ifdef USES_P007

// #######################################################################################################
// #################################### Plugin 007: ExtWiredAnalog #######################################
// #######################################################################################################

/** Changelog:
 * 2022-05-08 tonhuisman: Use ESPEasy core I2C functions where possible
 *                        Add support for use of the Analog output pin and 'analogout,<value>' command
 *                        Add configuration of all possible analog input modes
 * 2022-05-08 tonhuisman: Started changelog, older changes not recorded
 ********************************************************************************************************/

// commands:
// analogout,<value>    : If the Analog output is enabled, the value range is 0..255, and linear to Vref

# define PLUGIN_007
# define PLUGIN_ID_007         7
# define PLUGIN_NAME_007       "Analog input - PCF8591"
# define PLUGIN_VALUENAME1_007 "Analog"

# define P007_SENSOR_TYPE_INDEX  2
# define P007_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P007_SENSOR_TYPE_INDEX)))
# define P007_INPUT_MODE         PCONFIG_LONG(0)
# define P007_OUTPUT_MODE        PCONFIG_LONG(1)
# define P007_OUTPUT_ENABLED     (0b01000000)


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
      ExtraTaskSettings.populateDeviceValueNamesSeq(F(PLUGIN_VALUENAME1_007), P007_NR_OUTPUT_VALUES, 2, true);
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      if (PCONFIG(P007_SENSOR_TYPE_INDEX) == 0) {
        PCONFIG(P007_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_SINGLE);
      }
      event->Par1 = P007_NR_OUTPUT_VALUES;
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

      success = true;
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        String portNames[4];
        int    portValues[4];
        const uint8_t unit    = (CONFIG_PORT - 1) / 4;
        const uint8_t port    = CONFIG_PORT - (unit * 4);
        const uint8_t address = 0x48 + unit;

        for (uint8_t x = 0; x < 4; x++) {
          portValues[x] = x + 1;
          portNames[x]  = 'A';
          portNames[x] += x;
        }
        addFormSelectorI2C(F("pi2c"), 8, i2cAddressValues, address);
        addFormSelector(F("Port"), F("pport"), 4, portNames, portValues, port);
        addFormNote(F("Selected Port value will be stored in first 'Values' field and consecutively for 'Number Output Values' &gt; Single."));
      } else {
        success = intArrayContains(8, i2cAddressValues, event->Par1);
      }

      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      const uint8_t unit = (CONFIG_PORT - 1) / 4;
      event->Par1 = 0x48 + unit;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Hardware configuration"));

      const __FlashStringHelper *inputModeOptions[] = {
        F("4 single-ended inputs"),
        F("3 differential inputs, A0/A1/A2 differential with AIN3"),
        F("2 single-ended, A0, A1, AIN2/AIN3 differential -&gt; A2"),
        F("AIN0/AIN1 differential -&gt; A0, AIN2/AIN3 differential -&gt; A1"),
      };
      const int inputModeValues[] = {
        0b00000000,
        0b00010000,
        0b00100000,
        0b00110000,
      };
      addFormSelector(F("Input mode"), F("input_mode"), 4, inputModeOptions, inputModeValues, P007_INPUT_MODE);

      addFormCheckBox(F("Enable Analog output (AOUT)"), F("output_mode"), P007_OUTPUT_MODE == P007_OUTPUT_ENABLED);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      if (PCONFIG(P007_SENSOR_TYPE_INDEX) == 0) {
        PCONFIG(P007_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_SINGLE);
      }
      uint8_t i2c  = getFormItemInt(F("pi2c"));
      uint8_t port = getFormItemInt(F("pport"));
      CONFIG_PORT = (((i2c - 0x48) << 2) + port);

      P007_INPUT_MODE  = getFormItemInt(F("input_mode"));
      P007_OUTPUT_MODE = isFormItemChecked(F("output_mode")) ? P007_OUTPUT_ENABLED : 0;

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
      const uint8_t unit    = (CONFIG_PORT - 1) / 4;
      uint8_t port          = CONFIG_PORT - (unit * 4);
      const uint8_t address = 0x48 + unit;

      uint8_t var = 0;

      for (; var < P007_NR_OUTPUT_VALUES; ++port, ++var) {
        if (port <= 4) { // Only read available ports, hardwired limited to 4
          // Setup all required bits to the config register
          uint8_t configRegister = port - 1;
          configRegister |= P007_INPUT_MODE;
          configRegister |= P007_OUTPUT_MODE;

          // get the current pin value
          I2C_write8(address, configRegister);

          Wire.requestFrom(address, (uint8_t)0x2); // No fitting I2C standard function available

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

    case PLUGIN_WRITE:
    {
      String command = parseString(string, 1);

      if ((P007_OUTPUT_MODE == P007_OUTPUT_ENABLED) &&
          equals(command, F("analogout")) &&
          (event->Par1 >= 0) && (event->Par1 <= 255)) {
        uint8_t unit    = (CONFIG_PORT - 1) / 4;
        uint8_t address = 0x48 + unit;

        // Setup all required bits to the config register
        uint8_t configRegister = 0;
        configRegister |= P007_INPUT_MODE;
        configRegister |= P007_OUTPUT_MODE;

        I2C_write8_reg(address, configRegister, static_cast<uint8_t>(event->Par1));

        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P007
