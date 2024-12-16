#include "_Plugin_Helper.h"

#ifdef USES_P103

// ########################################################################################
// ################## Plugin 103 : Atlas Scientific EZO pH ORP EC DO HUM RTD FLOW sensors #
// ########################################################################################

// datasheet at https://atlas-scientific.com/files/pH_EZO_Datasheet.pdf (0x63, pH level)
// datasheet at https://atlas-scientific.com/files/ORP_EZO_Datasheet.pdf (0x62, Oxidation Reduction Potential)
// datasheet at https://atlas-scientific.com/files/EC_EZO_Datasheet.pdf (0x64, electric conductivity)
// datasheet at https://atlas-scientific.com/files/DO_EZO_Datasheet.pdf (0x61, dissolved oxigen)
// datasheet at https://files.atlas-scientific.com/EZO-HUM-C-Datasheet.pdf (0x6F, humidity)
// datasheet at https://files.atlas-scientific.com/EZO_RTD_Datasheet.pdf (0x66, thermosensors)
// datasheet at https://files.atlas-scientific.com/flow_EZO_Datasheet.pdf (0x68, flow meter)
// only i2c mode is supported

/** Changelog:
 * 2024-10-19 tonhuisman: Fix javascript errors, some code improvements
 * 2023-10-23 tonhuisman: Handle EZO-HUM firmware issue of including 'Dew,' in the result values
 * // TODO Rewrite plugin using PluginDataStruct so it will allow proper async handling of commands requiring 300 msec delay before reading
 *         responses
 * 2023-10-22 tonhuisman: Fix more irregularities, read configured EZO-HUM output options, and add options to enable/disable
 *                        Temperature and Dew point values
 * 2023-10-22 tonhuisman: Fix some irregularities, add logging for status read (UI) and value(s) read (INFO log)
 * 2023-10-17 tonhuisman: Add support for EZO HUM, RTD and FLOW sensor modules (I2C only!) (RTD, FLOW disabled, default to UART mode)
 * 2023-01-08 tonhuisman: Replace ambiguous #define UNKNOWN, move support functions to plugin_struct source
 * 2023-01-07 tonhuisman: Refactored strings (a.o. shorter names for WEBFORM_LOAD and WEBFORM_SAVE events), separate javascript function
 *                        instead of repeated code, extract red/orange/green messages into functions
 *                        Uncrustify source and more optimizations
 *                        Reuse char arrays instead of instantiating a new one
 */

# include "src/PluginStructs/P103_data_struct.h"

# define PLUGIN_103
# define PLUGIN_ID_103          103
# define PLUGIN_NAME_103        "Environment - Atlas EZO pH ORP EC DO HUM"
# if P103_USE_RTD
" RTD"
# endif // if P103_USE_RTD
# if P103_USE_FLOW
" FLOW"
# endif // if P103_USE_FLOW
# define PLUGIN_VALUENAME1_103  "SensorData"
# define PLUGIN_VALUENAME2_103  "Voltage"
# define PLUGIN_VALUENAME3_103  "Temperature" // TODO Only used for HUM TODO: Fix for EZO-FLOW extra reading
# define PLUGIN_VALUENAME4_103  "Dewpoint"    // Only used for HUM

boolean Plugin_103(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  AtlasEZO_Sensors_e board_type    = AtlasEZO_Sensors_e::UNKNOWN;
  const uint8_t i2cAddressValues[] = { 0x63, 0x62, 0x64, 0x61, 0x6F
                                       # if P103_USE_RTD
                                       ,     0x66
                                       # endif // if P103_USE_RTD
                                       # if P103_USE_FLOW
                                       ,     0x68
                                       # endif // if P103_USE_FLOW
  };
  constexpr int i2c_nr_elements = NR_ELEMENTS(i2cAddressValues);

  char boarddata[ATLAS_EZO_RETURN_ARRAY_SIZE]{};

  bool _HUMhasHum  = true; // EZO-HUM options (& defaults)
  bool _HUMhasTemp = false;
  bool _HUMhasDew  = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number       = PLUGIN_ID_103;
      Device[deviceCount].Type           = DEVICE_TYPE_I2C;
      Device[deviceCount].VType          = Sensor_VType::SENSOR_TYPE_DUAL;
      Device[deviceCount].Ports          = 0;
      Device[deviceCount].FormulaOption  = true;
      Device[deviceCount].ValueCount     = 2;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption    = true;
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
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_103)); // Only used for HUM
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_103)); // Only used for HUM
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P103_NR_OUTPUT_VALUES = 2;

      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P103_NR_OUTPUT_VALUES; // Depends on sensor

      success = true;

      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      // Disabled unsupported devices as discussed
      // here: https://github.com/letscontrolit/ESPEasy/pull/3733 (review comment by TD-er)

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c"), i2c_nr_elements, i2cAddressValues, P103_I2C_ADDRESS);
        addFormNote(F("pH: 0x63, ORP: 0x62, EC: 0x64, DO: 0x61, HUM: 0x6F"
                      # if P103_USE_RTD
                      ", RTD: 0x66"
                      # endif // if P103_USE_RTD
                      # if P103_USE_FLOW
                      ", FLOW: 0x68"
                      # endif // if P103_USE_FLOW
                      ". The plugin can recognize the type of device."));
      } else {
        success = intArrayContains(i2c_nr_elements, i2cAddressValues, event->Par1);
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

      addFormCheckBox(F("Setup without sensor"), F("uncon"), P103_UNCONNECTED_SETUP == 1);

      if (P103_send_I2C_command(P103_I2C_ADDRESS, F("i"), boarddata) || P103_UNCONNECTED_SETUP) {
        const String boardInfo(boarddata);
        addRowLabel(F("Board type"));

        String board         = parseStringKeepCase(boardInfo, 2);
        const String version = parseStringKeepCase(boardInfo, 3);

        const String boardTypes             = F("pH  ORP EC  D.O.HUM RTD FLO"); // Unsupported boards are still ignored
        const AtlasEZO_Sensors_e boardIDs[] = {
          AtlasEZO_Sensors_e::PH,
          AtlasEZO_Sensors_e::ORP,
          AtlasEZO_Sensors_e::EC,
          AtlasEZO_Sensors_e::DO,
          AtlasEZO_Sensors_e::HUM,
          # if P103_USE_RTD
          AtlasEZO_Sensors_e::RTD,
          # endif // if P103_USE_RTD
          # if P103_USE_FLOW
          AtlasEZO_Sensors_e::FLOW,
          # endif // if P103_USE_FLOW
        };
        int bType = boardTypes.indexOf(board);

        if ((board.isEmpty() || (bType == -1)) && P103_UNCONNECTED_SETUP) {
          // Not recognized, lets assume I2C address is correct, so we can setup the options
          for (uint8_t i = 0; i < i2c_nr_elements; ++i) {
            if (i2cAddressValues[i] == P103_I2C_ADDRESS) {
              bType = i * 4; // Divided in the next check
              break;
            }
          }
        }

        if ((bType > -1) && ((size_t)(bType / 4) < NR_ELEMENTS(boardIDs))) {
          board_type = boardIDs[bType / 4];
          board      = toString(board_type);
        }

        addHtml(board);

        P103_BOARD_TYPE = static_cast<uint8_t>(board_type);
        const int output_values[] = { 2, 2, 2, 2, 2, 4
                                      # if P103_USE_RTD
                                      ,  2
                                      # endif // if P103_USE_RTD
                                      # if P103_USE_FLOW
                                      ,  3
                                      # endif // if P103_USE_FLOW
        };
        P103_NR_OUTPUT_VALUES = output_values[P103_BOARD_TYPE];


        if (board_type == AtlasEZO_Sensors_e::UNKNOWN) {
          P103_html_red(F("  WARNING : Board type should be 'pH', 'ORP', 'EC', 'DO', 'HUM'"
                          # if P103_USE_RTD
                          ", 'RTD'"
                          # endif // if P103_USE_RTD
                          # if P103_USE_FLOW
                          ", 'FLOW'"
                          # endif // if P103_USE_FLOW
                          ", check your i2c address?"));
        }
        addRowLabel(F("Board version"));
        addHtml(version);

        addHtml(F("<input type='hidden' name='sensorVersion' value='"));
        addHtml(version);
        addHtml('\'', '>');
      } else {
        P103_html_red(F("Unable to send command to device"));

        if (board_type == AtlasEZO_Sensors_e::UNKNOWN) {
          P103_html_red(F("  WARNING : Board type should be 'pH', 'ORP', 'EC', 'DO', 'HUM'"
                          # if P103_USE_RTD
                          ", 'RTD'"
                          # endif // if P103_USE_RTD
                          # if P103_USE_FLOW
                          ", 'FLOW'"
                          # endif // if P103_USE_FLOW
                          ", check your i2c address?"));
        }
        success = false;
        break;
      }

      memset(boarddata, 0, ATLAS_EZO_RETURN_ARRAY_SIZE); // Cleanup

      if (P103_send_I2C_command(P103_I2C_ADDRESS, F("Status"), boarddata) || P103_UNCONNECTED_SETUP) {
        const String boardStatus(boarddata);

        addRowLabel(F("Board status"));
        addHtml(boardStatus);

        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, concat(F("Board status: "), boardStatus));
        # endif // ifndef BUILD_NO_DEBUG

        addRowLabel(F("Board restart code"));

        const String stat = parseStringKeepCase(boardStatus, 2);

        if (!stat.isEmpty()) {
          addHtml(P103_statusToString(stat[0]));
        }

        addRowLabel(F("Board voltage"));
        addHtml(parseString(boardStatus, 3));
        addUnit('V');

        addRowLabel(F("Sensor Data"));
        addHtmlFloat(UserVar.getFloat(event->TaskIndex, 0));

        switch (board_type) {
          case AtlasEZO_Sensors_e::PH:
            addUnit(F("pH"));
            break;
          case AtlasEZO_Sensors_e::ORP:
            addUnit(F("mV"));
            break;
          case AtlasEZO_Sensors_e::EC:
            addUnit(F("&micro;S"));
            break;
          case AtlasEZO_Sensors_e::DO:
            addUnit(F("mg/L"));
            break;
          case AtlasEZO_Sensors_e::HUM:
            addUnit(F("%RH"));
            memset(boarddata, 0, ATLAS_EZO_RETURN_ARRAY_SIZE); // Cleanup

            if (P103_getHUMOutputOptions(event,
                                         _HUMhasHum,
                                         _HUMhasTemp,
                                         _HUMhasDew)) {
              if (_HUMhasTemp) {
                addRowLabel(F("Temperature"));
                addHtmlFloat(UserVar.getFloat(event->TaskIndex, 2));
                addUnit(F("&deg;C"));
              }

              if (_HUMhasDew) {
                addRowLabel(F("Dew point"));
                addHtmlFloat(UserVar.getFloat(event->TaskIndex, 3));
                addUnit(F("&deg;C"));
              }
            }
            break;
          # if P103_USE_RTD
          case AtlasEZO_Sensors_e::RTD:
            addUnit(F("&deg;C")); // TODO Read current scale (C/F/K) from device, show flow
            break;
          # endif // if P103_USE_RTD
          # if P103_USE_FLOW
          case AtlasEZO_Sensors_e::FLOW:
            addUnit(F("mL/min"));
            break;
          # endif // if P103_USE_FLOW
          case AtlasEZO_Sensors_e::UNKNOWN:
            break;
        }
      } else {
        P103_html_red(F("Unable to send Status command to device"));
        success = false;
        break;
      }

      // Ability to turn status LED of board on or off
      addFormCheckBox(F("Status LED"), F("status_led"), P103_STATUS_LED);

      // Ability to see and change EC Probe Type (e.g., 0.1, 1.0, 10)
      if (board_type == AtlasEZO_Sensors_e::EC) {
        memset(boarddata, 0, ATLAS_EZO_RETURN_ARRAY_SIZE); // Cleanup

        if (P103_send_I2C_command(P103_I2C_ADDRESS, F("K,?"), boarddata)) {
          const String ecProbeType(boarddata);

          addFormTextBox(F("EC Probe Type"), F("ec_probe_type"), parseStringKeepCase(ecProbeType, 2), 32);
          addFormCheckBox(F("Set Probe Type"), F("en_set_probe_type"), false);
        }
      }

      // calibrate
      switch (board_type) {
        case AtlasEZO_Sensors_e::PH:
        {
          addFormSubHeader(F("pH Calibration"));
          addFormNote(F("Calibration for pH-Probe could be 1 (single), 2 (single, low) or 3 point (single, low, high)."
                        " The sequence is important."));
          const int nb_calibration_points = P103_addCreate3PointCalibration(board_type, event, P103_I2C_ADDRESS, F("pH"), 0.0, 14.0, 2, 0.01);

          if (nb_calibration_points > 1) {
            memset(boarddata, 0, ATLAS_EZO_RETURN_ARRAY_SIZE); // Cleanup

            if (P103_send_I2C_command(P103_I2C_ADDRESS, F("Slope,?"), boarddata)) {
              addFormNote(concat(F("Answer to 'Slope' command : "), String(boarddata)));
            }
          }
          break;
        }

        case AtlasEZO_Sensors_e::ORP:
          addFormSubHeader(F("ORP Calibration"));
          P103_addCreateSinglePointCalibration(board_type, event, P103_I2C_ADDRESS, F("mV"), 0.0, 1500.0, 0, 1.0);
          break;

        case AtlasEZO_Sensors_e::EC:
          addFormSubHeader(F("EC Calibration"));
          P103_addCreateDryCalibration();
          P103_addCreate3PointCalibration(board_type, event, P103_I2C_ADDRESS, F("&micro;S"), 0.0, 500000.0, 0, 1.0);
          break;

        case AtlasEZO_Sensors_e::DO:
          addFormSubHeader(F("DO Calibration"));
          P103_addDOCalibration(P103_I2C_ADDRESS);
          break;

        case AtlasEZO_Sensors_e::HUM:  // No calibration
        # if P103_USE_RTD
        case AtlasEZO_Sensors_e::RTD:  // TODO Decide what calibration data to retrieve/store
        # endif // if P103_USE_RTD
        # if P103_USE_FLOW
        case AtlasEZO_Sensors_e::FLOW: // TODO Size/type of flow meter, default: 1/2", Flow rate, Conversion factor, Output values: total,
                                       // flow rate
        # endif // if P103_USE_FLOW
        case AtlasEZO_Sensors_e::UNKNOWN:
          break;
      }

      if ((AtlasEZO_Sensors_e::PH == board_type) ||
          (AtlasEZO_Sensors_e::ORP == board_type) ||
          (AtlasEZO_Sensors_e::EC == board_type)) {
        // Clear calibration option, only when using calibration
        P103_addClearCalibration();

        // }

        // Temperature compensation
        // if ((AtlasEZO_Sensors_e::PH == board_type) ||
        //     (AtlasEZO_Sensors_e::ORP == board_type) ||
        //     (AtlasEZO_Sensors_e::EC == board_type)) {
        ESPEASY_RULES_FLOAT_TYPE value{};

        addFormSubHeader(F("Temperature compensation"));
        char deviceTemperatureTemplate[40]{};
        LoadCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&deviceTemperatureTemplate), sizeof(deviceTemperatureTemplate));
        ZERO_TERMINATE(deviceTemperatureTemplate);
        addFormTextBox(F("Temperature "), F("_template"), deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));
        addFormNote(F("You can use a formula and ideally refer to a temp sensor"
                      # ifndef LIMIT_BUILD_SIZE
                      " (directly, via ESPEasyP2P or MQTT import),"
                      " e.g. '[Pool#Temperature]'. If you don't have a sensor, you could"
                      # else // ifndef LIMIT_BUILD_SIZE
                      " or"
                      # endif // ifndef LIMIT_BUILD_SIZE
                      " type a fixed value like '25' or '25.5'."
                      ));

        String deviceTemperatureTemplateString(deviceTemperatureTemplate);
        const String pooltempString(parseTemplate(deviceTemperatureTemplateString));

        if (Calculate(pooltempString, value) != CalculateReturnCode::OK) {
          addFormNote(F("Formula parse error. Using fixed value!"));
          value = P103_FIXED_TEMP_VALUE;
        }

        addFormNote(strformat(F("Actual value: %.2f"), value));
      }

      if (AtlasEZO_Sensors_e::HUM == board_type) {
        addFormSubHeader(F("HUM Options"));
        addFormCheckBox(F("Enable Temperature reading"), F("hum_temp"), _HUMhasTemp);
        addFormCheckBox(F("Enable Dew point reading"),   F("hum_dew"),  _HUMhasTemp);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      board_type = static_cast<AtlasEZO_Sensors_e>(P103_BOARD_TYPE);

      P103_I2C_ADDRESS       = getFormItemInt(F("i2c"));
      P103_UNCONNECTED_SETUP = isFormItemChecked(F("uncon")) ? 1 : 0;

      P103_SENSOR_VERSION = getFormItemFloat(F("sensorVersion"));

      P103_STATUS_LED = isFormItemChecked(F("status_led")) ? 1 : 0;

      P103_send_I2C_command(P103_I2C_ADDRESS, concat(F("L,"), P103_STATUS_LED), boarddata);

      if ((board_type == AtlasEZO_Sensors_e::EC) && isFormItemChecked(F("en_set_probe_type"))) {
        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("isFormItemChecked"));
        # endif // ifndef BUILD_NO_DEBUG
        const String probeType = concat(F("K,"), webArg(F("ec_probe_type")));
        memset(boarddata, 0, ATLAS_EZO_RETURN_ARRAY_SIZE); // Cleanup
        P103_send_I2C_command(P103_I2C_ADDRESS, probeType, boarddata);
      }

      String cmd(F("Cal,"));
      bool   triggerCalibrate = false;

      P103_CALIBRATION_SINGLE = getFormItemFloat(F("ref_cal_single"));
      P103_CALIBRATION_LOW    = getFormItemFloat(F("ref_cal_L"));
      P103_CALIBRATION_HIGH   = getFormItemFloat(F("ref_cal_H"));

      if (isFormItemChecked(F("en_cal_clear"))) {
        cmd             += F("clear");
        triggerCalibrate = true;
      } else if (isFormItemChecked(F("en_cal_dry"))) {
        cmd             += F("dry");
        triggerCalibrate = true;
      } else if (isFormItemChecked(F("en_cal_single"))) {
        if (board_type == AtlasEZO_Sensors_e::PH) {
          cmd += F("mid,");
        }
        cmd             += P103_CALIBRATION_SINGLE;
        triggerCalibrate = true;
      } else if (isFormItemChecked(F("en_cal_L"))) {
        cmd             += F("low,");
        cmd             += P103_CALIBRATION_LOW;
        triggerCalibrate = true;
      } else if (isFormItemChecked(F("en_cal_H"))) {
        cmd             += F("high,");
        cmd             += P103_CALIBRATION_HIGH;
        triggerCalibrate = true;
      }

      if (isFormItemChecked(F("en_cal_atm"))) {
        triggerCalibrate = true;
      }

      if (isFormItemChecked(F("en_cal_0"))) {
        cmd             += '0';
        triggerCalibrate = true;
      }


      if (triggerCalibrate &&
          ((AtlasEZO_Sensors_e::PH == board_type) ||
           (AtlasEZO_Sensors_e::EC == board_type) ||
           (AtlasEZO_Sensors_e::DO == board_type))
          ) {
        memset(boarddata, 0, ATLAS_EZO_RETURN_ARRAY_SIZE); // Cleanup
        P103_send_I2C_command(P103_I2C_ADDRESS, cmd, boarddata);
      }

      if ((AtlasEZO_Sensors_e::PH == board_type) ||
          (AtlasEZO_Sensors_e::EC == board_type) ||
          (AtlasEZO_Sensors_e::DO == board_type)) {
        char   deviceTemperatureTemplate[40]{};
        String tmpString = webArg(F("_template"));
        safe_strncpy(deviceTemperatureTemplate, tmpString.c_str(), sizeof(deviceTemperatureTemplate) - 1);
        ZERO_TERMINATE(deviceTemperatureTemplate); // be sure that our string ends with a \0

        addHtmlError(SaveCustomTaskSettings(event->TaskIndex, reinterpret_cast<const uint8_t *>(&deviceTemperatureTemplate),
                                            sizeof(deviceTemperatureTemplate)));
      }

      if ((AtlasEZO_Sensors_e::HUM == board_type) && P103_getHUMOutputOptions(event,
                                                                              _HUMhasHum,
                                                                              _HUMhasTemp,
                                                                              _HUMhasDew)) {
        if (!_HUMhasHum) { // If humidity not enabled, then enable it
          P103_send_I2C_command(P103_I2C_ADDRESS, F("O,Hum,1"), boarddata);
        }
        bool _humOpt = isFormItemChecked(F("hum_temp"));

        if (_humOpt != _HUMhasTemp) {
          P103_send_I2C_command(P103_I2C_ADDRESS, concat(F("O,T,"), _humOpt ? 1 : 0), boarddata);
        }
        _humOpt = isFormItemChecked(F("hum_dew"));

        if (_humOpt != _HUMhasDew) {
          P103_send_I2C_command(P103_I2C_ADDRESS, concat(F("O,Dew,"), _humOpt ? 1 : 0), boarddata);
        }
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

      if ((AtlasEZO_Sensors_e::PH == board_type) ||
          (AtlasEZO_Sensors_e::EC == board_type) ||
          (AtlasEZO_Sensors_e::DO == board_type))
      {
        // first set the temperature of reading
        char deviceTemperatureTemplate[40]{};
        LoadCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&deviceTemperatureTemplate), sizeof(deviceTemperatureTemplate));
        ZERO_TERMINATE(deviceTemperatureTemplate);

        String deviceTemperatureTemplateString(deviceTemperatureTemplate);
        const String temperatureString(parseTemplate(deviceTemperatureTemplateString));

        readCommand = F("RT,");
        ESPEASY_RULES_FLOAT_TYPE temperatureReading{};

        if (Calculate(temperatureString, temperatureReading) != CalculateReturnCode::OK) {
          temperatureReading = P103_FIXED_TEMP_VALUE;
        }

        readCommand += temperatureReading;
      }
      else if ((AtlasEZO_Sensors_e::ORP == board_type) ||
               (AtlasEZO_Sensors_e::HUM == board_type)
               # if P103_USE_RTD
               || (AtlasEZO_Sensors_e::RTD == board_type)
               # endif // if P103_USE_RTD
               # if P103_USE_FLOW
               || (AtlasEZO_Sensors_e::FLOW == board_type)
               # endif // if P103_USE_FLOW
               ) {
        readCommand = F("R,");
      }

      // ok, now we can read the sensor data
      memset(boarddata, 0, ATLAS_EZO_RETURN_ARRAY_SIZE); // Cleanup
      UserVar.setFloat(event->TaskIndex, 0, -1);

      if (P103_send_I2C_command(P103_I2C_ADDRESS, readCommand, boarddata)) {
        String sensorString(boarddata);
        addLog(LOG_LEVEL_INFO, concat(F("P103: READ result: "), sensorString));

        float sensor_f{};

        if (string2float(parseString(sensorString, 1), sensor_f)) {
          UserVar.setFloat(event->TaskIndex, 0, sensor_f);
        }

        if (board_type == AtlasEZO_Sensors_e::HUM) { // TODO Fix reading Dew point without Temperature enabled
          if (string2float(parseString(sensorString, 2), sensor_f)) {
            UserVar.setFloat(event->TaskIndex, 2, sensor_f);
          }
          String dewVal = parseString(sensorString, 3);

          if (equals(dewVal, F("dew"))) { // Handle EZO-HUM firmware bug including 'Dew,' in the result string
            dewVal = parseString(sensorString, 4);
          }

          if (string2float(dewVal, sensor_f)) {
            UserVar.setFloat(event->TaskIndex, 3, sensor_f);
          }
        }

        # if P103_USE_FLOW

        if ((board_type == AtlasEZO_Sensors_e::FLOW) &&
            string2float(parseString(sensorString, 2), sensor_f)) {
          UserVar.setFloat(event->TaskIndex, 2, sensor_f);
        }
        # endif // if P103_USE_FLOW
        string2float(sensorString, sensor_f);
        UserVar.setFloat(event->TaskIndex, 0, sensor_f);
      }

      // we read the voltagedata
      memset(boarddata, 0, ATLAS_EZO_RETURN_ARRAY_SIZE); // Cleanup
      UserVar.setFloat(event->TaskIndex, 1, -1);

      if (P103_send_I2C_command(P103_I2C_ADDRESS, F("Status"), boarddata)) {
        String voltage(boarddata);
        float  volt_f{};
        string2float(voltage.substring(voltage.lastIndexOf(',') + 1), volt_f);
        UserVar.setFloat(event->TaskIndex, 1, volt_f);
      }

      success = true;
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P103
