#include "_Plugin_Helper.h"

#ifdef USES_P103

// ###########################################################################
// ################## Plugin 103 : Atlas Scientific EZO pH ORP EC DO sensors #
// ###########################################################################

// datasheet at https://atlas-scientific.com/files/pH_EZO_Datasheet.pdf
// datasheet at https://atlas-scientific.com/files/ORP_EZO_Datasheet.pdf
// datasheet at https://atlas-scientific.com/files/EC_EZO_Datasheet.pdf
// datasheet at https://atlas-scientific.com/files/DO_EZO_Datasheet.pdf
// only i2c mode is supported

/** Changelog:
 * 2023-01-08 tonhuisman: Replace ambiguous #define UNKNOWN, move support functions to plugin_struct source
 * 2023-01-07 tonhuisman: Refactored strings (a.o. shorter names for WEBFORM_LOAD and WEBFORM_SAVE events), separate javascript function
 *                        instead of repeated code, extract red/orange/green messages into functions
 *                        Uncrustify source and more optimizations
 *                        Reuse char arrays instead of instantiating a new one
 */

# define PLUGIN_103
# define PLUGIN_ID_103          103
# define PLUGIN_NAME_103        "Environment - Atlas EZO pH ORP EC DO"
# define PLUGIN_VALUENAME1_103  "SensorData"
# define PLUGIN_VALUENAME2_103  "Voltage"

# include "src/PluginStructs/P103_data_struct.h"

boolean Plugin_103(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  AtlasEZO_Sensors_e board_type = AtlasEZO_Sensors_e::UNKNOWN;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_103;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_DUAL;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_103);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_103));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_103));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x61, 0x62, 0x63, 0x64 }; // , 0x65, 0x66, 0x67}; // Disabled unsupported devices as discussed
                                                                     // here: https://github.com/letscontrolit/ESPEasy/pull/3733 (review
                                                                     // comment by TD-er)

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c"), P103_ATLASEZO_I2C_NB_OPTIONS, i2cAddressValues, P103_I2C_ADDRESS);
        addFormNote(F("pH: 0x63, ORP: 0x62, EC: 0x64, DO: 0x61. The plugin is able to detect the type of device automatically."));
      } else {
        success = intArrayContains(P103_ATLASEZO_I2C_NB_OPTIONS, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P103_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Board"));

      P103_addDisabler(); // JS function disabler(clear,single,l,h,dry,nul,atm)

      char boarddata[ATLAS_EZO_RETURN_ARRAY_SIZE] = { 0 };

      if (P103_send_I2C_command(P103_I2C_ADDRESS, F("i"), boarddata))
      {
        String boardInfo(boarddata);
        addRowLabel(F("Board type"));

        String board   = boardInfo.substring(boardInfo.indexOf(',') + 1, boardInfo.lastIndexOf(','));
        String version = boardInfo.substring(boardInfo.lastIndexOf(',') + 1);
        addHtml(board);

        String boardTypes             = F("pH  ORP EC  D.O.");
        AtlasEZO_Sensors_e boardIDs[] = {
          AtlasEZO_Sensors_e::PH,
          AtlasEZO_Sensors_e::ORP,
          AtlasEZO_Sensors_e::EC,
          AtlasEZO_Sensors_e::DO,
        };
        int bType = boardTypes.indexOf(board);

        if (bType > -1) {
          board_type = boardIDs[bType / 4];
        }

        P103_BOARD_TYPE = static_cast<uint8_t>(board_type);

        if (board_type == AtlasEZO_Sensors_e::UNKNOWN)
        {
          P103_html_red(F("  WARNING : Board type should be 'pH', 'ORP', 'EC' or 'DO', check your i2c address? "));
        }
        addRowLabel(F("Board version"));
        addHtml(version);

        addHtml(F("<input type='hidden' name='sensorVersion' value='"));
        addHtml(version);
        addHtml('\'', '>');
      }
      else
      {
        P103_html_red(F("Unable to send command to device"));

        if (board_type == AtlasEZO_Sensors_e::UNKNOWN)
        {
          P103_html_red(F("  WARNING : Board type should be 'pH', 'ORP', 'EC' or 'DO', check your i2c address? "));
        }
        success = false;
        break;
      }

      memset(boarddata, 0, ATLAS_EZO_RETURN_ARRAY_SIZE); // Cleanup

      if (P103_send_I2C_command(P103_I2C_ADDRESS, F("Status"), boarddata))
      {
        String boardStatus(boarddata);

        addRowLabel(F("Board restart code"));

      # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, boardStatus);
      # endif // ifndef BUILD_NO_DEBUG

        char *statuschar = strchr(boarddata, ',');

        if (statuschar > 0)
        {
          switch (boarddata[statuschar - boarddata + 1])
          {
            case 'P':
            {
              addHtml(F("powered off"));
              break;
            }
            case 'S':
            {
              addHtml(F("software reset"));
              break;
            }
            case 'B':
            {
              addHtml(F("brown out"));
              break;
            }
            case 'W':
            {
              addHtml(F("watch dog"));
              break;
            }
            case 'U':
            default:
            {
              addHtml(F("unknown"));
              break;
            }
          }
        }

        addRowLabel(F("Board voltage"));
        addHtml(boardStatus.substring(boardStatus.lastIndexOf(',') + 1));
        addUnit('V');

        addRowLabel(F("Sensor Data"));
        addHtmlFloat(UserVar[event->BaseVarIndex]);

        switch (board_type)
        {
          case AtlasEZO_Sensors_e::PH:
          {
            addUnit(F("pH"));
            break;
          }
          case AtlasEZO_Sensors_e::ORP:
          {
            addUnit(F("mV"));
            break;
          }
          case AtlasEZO_Sensors_e::EC:
          {
            addUnit(F("&micro;S"));
            break;
          }
          case AtlasEZO_Sensors_e::DO:
          {
            addUnit(F("mg/L"));
            break;
          }
          case AtlasEZO_Sensors_e::UNKNOWN:
            break;
        }
      }
      else
      {
        P103_html_red(F("Unable to send status command to device"));
        success = false;
        break;
      }

      // Ability to turn status LED of board on or off
      addFormCheckBox(F("Status LED"), F("status_led"), P103_STATUS_LED);

      // Ability to see and change EC Probe Type (e.g., 0.1, 1.0, 10)
      if (board_type == AtlasEZO_Sensors_e::EC)
      {
        memset(boarddata, 0, ATLAS_EZO_RETURN_ARRAY_SIZE); // Cleanup

        if (P103_send_I2C_command(P103_I2C_ADDRESS, F("K,?"), boarddata))
        {
          String ecProbeType(boarddata);

          addFormTextBox(F("EC Probe Type"), F("ec_probe_type"), ecProbeType.substring(ecProbeType.lastIndexOf(',') + 1), 32);
          addFormCheckBox(F("Set Probe Type"), F("en_set_probe_type"), false);
        }
      }

      // calibrate
      switch (board_type)
      {
        case AtlasEZO_Sensors_e::PH:
        {
          addFormSubHeader(F("pH Calibration"));
          addFormNote(F(
                        "Calibration for pH-Probe could be 1 (single), 2 (single, low) or 3 point (single, low, high). The sequence is important."));
          const int nb_calibration_points = P103_addCreate3PointCalibration(board_type, event, P103_I2C_ADDRESS, F("pH"), 0.0, 14.0, 2, 0.01);

          if (nb_calibration_points > 1)
          {
            memset(boarddata, 0, ATLAS_EZO_RETURN_ARRAY_SIZE); // Cleanup

            if (P103_send_I2C_command(P103_I2C_ADDRESS, F("Slope,?"), boarddata))
            {
              addFormNote(concat(F("Answer to 'Slope' command : "), String(boarddata)));
            }
          }
          break;
        }

        case AtlasEZO_Sensors_e::ORP:
        {
          addFormSubHeader(F("ORP Calibration"));
          P103_addCreateSinglePointCalibration(board_type, event, P103_I2C_ADDRESS, F("mV"), 0.0, 1500.0, 0, 1.0);
          break;
        }

        case AtlasEZO_Sensors_e::EC:
        {
          addFormSubHeader(F("EC Calibration"));
          P103_addCreateDryCalibration();
          P103_addCreate3PointCalibration(board_type, event, P103_I2C_ADDRESS, F("&micro;S"), 0.0, 500000.0, 0, 1.0);
          break;
        }

        case AtlasEZO_Sensors_e::DO:
        {
          addFormSubHeader(F("DO Calibration"));
          P103_addDOCalibration(P103_I2C_ADDRESS);
          break;
        }
        case AtlasEZO_Sensors_e::UNKNOWN:
          break;
      }

      // Clear calibration
      P103_addClearCalibration();

      // Temperature compensation
      if ((board_type == AtlasEZO_Sensors_e::PH) ||
          (board_type == AtlasEZO_Sensors_e::EC) ||
          (board_type == AtlasEZO_Sensors_e::DO))
      {
        double value;
        char   strValue[6] = { 0 };

        addFormSubHeader(F("Temperature compensation"));
        char deviceTemperatureTemplate[40] = { 0 };
        LoadCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&deviceTemperatureTemplate), sizeof(deviceTemperatureTemplate));
        ZERO_TERMINATE(deviceTemperatureTemplate);
        addFormTextBox(F("Temperature "), F("_template"), deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));
        addFormNote(F("You can use a formula and idealy refer to a temp sensor (directly, via ESPEasyP2P or MQTT import),"
                      " e.g. '[Pool#Temperature]'. If you don't have a sensor, you could type a fixed value like '25' or '25.5'."));

        String deviceTemperatureTemplateString(deviceTemperatureTemplate);
        String pooltempString(parseTemplate(deviceTemperatureTemplateString, 40));

        if (Calculate(pooltempString, value) != CalculateReturnCode::OK)
        {
          addFormNote(F("It seems I can't parse your formula. Fixed value will be used!"));
          value = P103_FIXED_TEMP_VALUE;
        }

        dtostrf(value, 5, 2, strValue);
        ZERO_TERMINATE(strValue);
        addFormNote(concat(F("Actual value: "), String(strValue)));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      board_type = static_cast<AtlasEZO_Sensors_e>(P103_BOARD_TYPE);

      P103_I2C_ADDRESS = getFormItemInt(F("i2c"));

      P103_SENSOR_VERSION = getFormItemFloat(F("sensorVersion"));

      char boarddata[ATLAS_EZO_RETURN_ARRAY_SIZE] = { 0 };

      if (isFormItemChecked(F("status_led")))
      {
        P103_send_I2C_command(P103_I2C_ADDRESS, F("L,1"), boarddata);
      }
      else
      {
        P103_send_I2C_command(P103_I2C_ADDRESS, F("L,0"), boarddata);
      }
      P103_STATUS_LED = isFormItemChecked(F("status_led"));

      if ((board_type == AtlasEZO_Sensors_e::EC) && isFormItemChecked(F("en_set_probe_type")))
      {
        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("isFormItemChecked"));
        # endif // ifndef BUILD_NO_DEBUG
        String probeType(F("K,"));
        probeType += webArg(F("ec_probe_type"));
        memset(boarddata, 0, ATLAS_EZO_RETURN_ARRAY_SIZE); // Cleanup
        P103_send_I2C_command(P103_I2C_ADDRESS, probeType, boarddata);
      }

      String cmd(F("Cal,"));
      bool   triggerCalibrate = false;

      P103_CALIBRATION_SINGLE = getFormItemFloat(F("ref_cal_single"));
      P103_CALIBRATION_LOW    = getFormItemFloat(F("ref_cal_L"));
      P103_CALIBRATION_HIGH   = getFormItemFloat(F("ref_cal_H"));

      if (isFormItemChecked(F("en_cal_clear")))
      {
        cmd             += F("clear");
        triggerCalibrate = true;
      }
      else if (isFormItemChecked(F("en_cal_dry")))
      {
        cmd             += F("dry");
        triggerCalibrate = true;
      }
      else if (isFormItemChecked(F("en_cal_single")))
      {
        if (board_type == AtlasEZO_Sensors_e::PH)
        {
          cmd += F("mid,");
        }
        cmd             += P103_CALIBRATION_SINGLE;
        triggerCalibrate = true;
      }
      else if (isFormItemChecked(F("en_cal_L")))
      {
        cmd             += F("low,");
        cmd             += P103_CALIBRATION_LOW;
        triggerCalibrate = true;
      }
      else if (isFormItemChecked(F("en_cal_H")))
      {
        cmd             += F("high,");
        cmd             += P103_CALIBRATION_HIGH;
        triggerCalibrate = true;
      }
      else if (isFormItemChecked(F("en_cal_atm")))
      {
        triggerCalibrate = true;
      }
      else if (isFormItemChecked(F("en_cal_0")))
      {
        cmd             += '0';
        triggerCalibrate = true;
      }


      if (triggerCalibrate)
      {
        memset(boarddata, 0, ATLAS_EZO_RETURN_ARRAY_SIZE); // Cleanup
        P103_send_I2C_command(P103_I2C_ADDRESS, cmd, boarddata);
      }

      if ((board_type == AtlasEZO_Sensors_e::PH) ||
          (board_type == AtlasEZO_Sensors_e::EC) ||
          (board_type == AtlasEZO_Sensors_e::DO))
      {
        char   deviceTemperatureTemplate[40] = { 0 };
        String tmpString                     = webArg(F("_template"));
        safe_strncpy(deviceTemperatureTemplate, tmpString.c_str(), sizeof(deviceTemperatureTemplate) - 1);
        ZERO_TERMINATE(deviceTemperatureTemplate); // be sure that our string ends with a \0

        addHtmlError(SaveCustomTaskSettings(event->TaskIndex, reinterpret_cast<const uint8_t *>(&deviceTemperatureTemplate),
                                            sizeof(deviceTemperatureTemplate)));
      }

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
      board_type = static_cast<AtlasEZO_Sensors_e>(P103_BOARD_TYPE);

      String readCommand;

      if ((board_type == AtlasEZO_Sensors_e::PH) ||
          (board_type == AtlasEZO_Sensors_e::EC) ||
          (board_type == AtlasEZO_Sensors_e::DO))
      {
        // first set the temperature of reading
        char deviceTemperatureTemplate[40] = { 0 };
        LoadCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&deviceTemperatureTemplate), sizeof(deviceTemperatureTemplate));
        ZERO_TERMINATE(deviceTemperatureTemplate);

        String deviceTemperatureTemplateString(deviceTemperatureTemplate);
        String temperatureString(parseTemplate(deviceTemperatureTemplateString, 40));

        readCommand = F("RT,");
        double temperatureReading;

        if (Calculate(temperatureString, temperatureReading) != CalculateReturnCode::OK)
        {
          temperatureReading = P103_FIXED_TEMP_VALUE;
        }

        readCommand += temperatureReading;
      }
      else if (board_type == AtlasEZO_Sensors_e::ORP)
      {
        readCommand = F("R,");
      }

      // ok, now we can read the sensor data
      char boarddata[ATLAS_EZO_RETURN_ARRAY_SIZE] = { 0 };
      UserVar[event->BaseVarIndex] = -1;

      if (P103_send_I2C_command(P103_I2C_ADDRESS, readCommand, boarddata))
      {
        String sensorString(boarddata);
        string2float(sensorString, UserVar[event->BaseVarIndex]);
      }

      // we read the voltagedata
      memset(boarddata, 0, ATLAS_EZO_RETURN_ARRAY_SIZE); // Cleanup
      UserVar[event->BaseVarIndex + 1] = -1;

      if (P103_send_I2C_command(P103_I2C_ADDRESS, F("Status"), boarddata))
      {
        String voltage(boarddata);
        string2float(voltage.substring(voltage.lastIndexOf(',') + 1), UserVar[event->BaseVarIndex + 1]);
      }

      success = true;
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P103
