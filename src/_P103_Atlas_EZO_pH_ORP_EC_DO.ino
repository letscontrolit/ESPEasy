#include "_Plugin_Helper.h"

#ifdef USES_P103

// ###########################################################################
// ################## Plugin 103 : Atlas Scientific EZO pH ORP EC DO sensors #
// ###########################################################################

// datasheet at https://atlas-scientific.com/files/pH_EZO_Datasheet.pdf
// datasheet at https://atlas-scientific.com/files/ORP_EZO_Datasheet.pdf
// datasheet at https://atlas-scientific.com/files/EC_EZO_Datasheet.pdf
// datasheet at https://atlas-scientific.com/files/DO_EZO_Datasheet.pdf
// works only in i2c mode

/** Changelog:
 * 2023-01-07 tonhuisman: Refactored strings (a.o. shorter names for WEBFORM_LOAD and WEBFORM_SAVE events), separate javascript function
 *                        instead of repeated code, extract red/orange/green messages into functions
 *                        Uncrustify source and more optimizations
 *                        Reuse char arrays instead of instantiating a new one
 */

# include "src/Helpers/Rules_calculate.h"

# define PLUGIN_103
# define PLUGIN_ID_103          103
# define PLUGIN_NAME_103        "Environment - Atlas EZO pH ORP EC DO"
# define PLUGIN_VALUENAME1_103  "SensorData"
# define PLUGIN_VALUENAME2_103  "Voltage"

# define UNKNOWN  0 // FIXME Rename to avoid possible confusion
# define PH       1
# define ORP      2
# define EC       3
# define DO       4

# define P103_BOARD_TYPE                PCONFIG(0)
# define P103_I2C_ADDRESS               PCONFIG(1)
# define P103_STATUS_LED                PCONFIG(2)
# define P103_SENSOR_VERSION            PCONFIG_FLOAT(0)
# define P103_CALIBRATION_SINGLE        PCONFIG_FLOAT(1)
# define P103_CALIBRATION_LOW           PCONFIG_FLOAT(2)
# define P103_CALIBRATION_HIGH          PCONFIG_FLOAT(3)

# define ATLAS_EZO_RETURN_ARRAY_SIZE    33 // Max expected result 32 bytes + \0

# define P103_ATLASEZO_I2C_NB_OPTIONS   4  // was: 6 see comment below at 'const int i2cAddressValues'

# define FIXED_TEMP_VALUE               20 // Temperature correction for pH and EC sensor if no temperature is given from calculation

bool P103_send_I2C_command(uint8_t       I2Caddress,
                           const String& cmd,
                           char         *sensordata); // Forward declarations

void    P103_addDisabler();
void    P103_html_color_message(const __FlashStringHelper *color,
                                const String             & message);
void    P103_html_red(const String& message);
void    P103_html_orange(const String& message);
void    P103_html_green(const String& message);

boolean Plugin_103(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  uint8_t board_type = UNKNOWN;

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

        String  boardTypes = F("pH  ORP EC  D.O.");
        uint8_t boardIDs[] = { PH, ORP, EC, DO };
        int     bType      = boardTypes.indexOf(board);

        if (bType > -1) {
          board_type = boardIDs[bType / 4];
        }

        P103_BOARD_TYPE = board_type;

        if (board_type == UNKNOWN)
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

        if (board_type == UNKNOWN)
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
          case PH:
          {
            addUnit(F("pH"));
            break;
          }
          case ORP:
          {
            addUnit(F("mV"));
            break;
          }
          case EC:
          {
            addUnit(F("&micro;S"));
            break;
          }
          case DO:
          {
            addUnit(F("mg/L"));
            break;
          }
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
      if (board_type == EC)
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
        case PH:
        {
          addFormSubHeader(F("pH Calibration"));
          addFormNote(F(
                        "Calibration for pH-Probe could be 1 (single), 2 (single, low) or 3 point (single, low, high). The sequence is important."));
          const int nb_calibration_points = addCreate3PointCalibration(board_type, event, P103_I2C_ADDRESS, F("pH"), 0.0, 14.0, 2, 0.01);

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

        case ORP:
        {
          addFormSubHeader(F("ORP Calibration"));
          addCreateSinglePointCalibration(board_type, event, P103_I2C_ADDRESS, F("mV"), 0.0, 1500.0, 0, 1.0);
          break;
        }

        case EC:
        {
          addFormSubHeader(F("EC Calibration"));
          addCreateDryCalibration();
          addCreate3PointCalibration(board_type, event, P103_I2C_ADDRESS, F("&micro;S"), 0.0, 500000.0, 0, 1.0);
          break;
        }

        case DO:
        {
          addFormSubHeader(F("DO Calibration"));
          addDOCalibration(P103_I2C_ADDRESS);
          break;
        }
      }

      // Clear calibration
      addClearCalibration();

      // Temperature compensation
      if ((board_type == PH) || (board_type == EC) || (board_type == DO))
      {
        double value;
        char   strValue[6] = { 0 };

        addFormSubHeader(F("Temperature compensation"));
        char deviceTemperatureTemplate[40] = { 0 };
        LoadCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&deviceTemperatureTemplate), sizeof(deviceTemperatureTemplate));
        ZERO_TERMINATE(deviceTemperatureTemplate);
        addFormTextBox(F("Temperature "), F("_template"), deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));
        addFormNote(F("You can use a formula and idealy refer to a temp sensor (directly, via ESPEasyP2P or MQTT import),"
                      " e.g. '[Pool#Temperature]'. If you don't have a sensor, you could type a fixed value like '25' for '25.5'."));

        String deviceTemperatureTemplateString(deviceTemperatureTemplate);
        String pooltempString(parseTemplate(deviceTemperatureTemplateString, 40));

        if (Calculate(pooltempString, value) != CalculateReturnCode::OK)
        {
          addFormNote(F("It seems I can't parse your formula. Fixed value will be used!"));
          value = FIXED_TEMP_VALUE;
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
      board_type = P103_BOARD_TYPE;

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

      if ((board_type == EC) && isFormItemChecked(F("en_set_probe_type")))
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
        if (board_type == PH)
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

      if ((board_type == PH) || (board_type == EC) || (board_type == DO))
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
      success = true; // PLUGIN_INIT *must* return true on successful initialization, even if nothing is happening here
      break;
    }

    case PLUGIN_READ:
    {
      board_type = P103_BOARD_TYPE;

      String readCommand;

      if ((board_type == PH) || (board_type == EC) || (board_type == DO))
      {
        // first set the temperature of reading
        char deviceTemperatureTemplate[40] = { 0 };
        LoadCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&deviceTemperatureTemplate), sizeof(deviceTemperatureTemplate));
        ZERO_TERMINATE(deviceTemperatureTemplate);

        String deviceTemperatureTemplateString(deviceTemperatureTemplate);
        String pooltempString(parseTemplate(deviceTemperatureTemplateString, 40));

        readCommand = F("RT,");
        double temperatureReading;

        if (Calculate(pooltempString, temperatureReading) != CalculateReturnCode::OK)
        {
          temperatureReading = FIXED_TEMP_VALUE;
        }

        readCommand += temperatureReading;
      }
      else if (board_type == ORP)
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

// Call this function with two char arrays, one containing the command
// The other containing an allocatted char array for answer
// Returns true on success, false otherwise

bool P103_send_I2C_command(uint8_t I2Caddress, const String& cmd, char *sensordata)
{
  sensordata[0] = '\0';

  uint16_t sensor_bytes_received = 0;

  uint8_t error;
  uint8_t i2c_response_code = 0;
  uint8_t in_char           = 0;

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLogMove(LOG_LEVEL_DEBUG, concat(F("> cmd = "), cmd));
  }
  # endif // ifndef BUILD_NO_DEBUG
  Wire.beginTransmission(I2Caddress);
  Wire.write(cmd.c_str());
  error = Wire.endTransmission();

  if (error != 0)
  {
    addLog(LOG_LEVEL_ERROR, F("Wire.endTransmission() returns error: Check Atlas shield, pH, ORP, EC and DO are supported."));
    return false;
  }

  // don't read answer if we want to go to sleep
  if (cmd.substring(0, 5).equalsIgnoreCase(F("Sleep")))
  {
    return true;
  }

  i2c_response_code = 254;

  while (i2c_response_code == 254)
  {
    Wire.requestFrom(I2Caddress, (uint8_t)(ATLAS_EZO_RETURN_ARRAY_SIZE - 1)); // call the circuit and request ATLAS_EZO_RETURN_ARRAY_SIZE -
                                                                              // 1 = 32 bytes (this is more then we need).
    i2c_response_code = Wire.read();                                          // read response code

    while (Wire.available())
    {                                                                         // read response
      in_char = Wire.read();

      if (in_char == 0)
      {   // if we receive a null caracter, we're done
        while (Wire.available())
        { // purge the data line if needed
          Wire.read();
        }

        break; // exit the while loop.
      }
      else
      {
        if (sensor_bytes_received > ATLAS_EZO_RETURN_ARRAY_SIZE)
        {
          addLog(LOG_LEVEL_ERROR, F("< result array to short!"));
          return false;
        }
        sensordata[sensor_bytes_received] = in_char; // load this uint8_t into our array.
        sensor_bytes_received++;
      }
    }
    sensordata[sensor_bytes_received] = '\0';

    switch (i2c_response_code)
    {
      case 1:
      {
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLogMove(LOG_LEVEL_DEBUG, concat(F("< success, answer = "), String(sensordata)));
        }
        # endif // ifndef BUILD_NO_DEBUG
        break;
      }

      case 2:
        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("< command failed"));
        # endif // ifndef BUILD_NO_DEBUG
        return false;

      case 254:
        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG_MORE, F("< command pending"));
        # endif // ifndef BUILD_NO_DEBUG
        break;

      case 255:
        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("< no data"));
        # endif // ifndef BUILD_NO_DEBUG
        return false;
    }
  }

  return true;
}

int getCalibrationPoints(uint8_t i2cAddress)
{
  int  nb_calibration_points                   = -1;
  char sensordata[ATLAS_EZO_RETURN_ARRAY_SIZE] = { 0 };

  if (P103_send_I2C_command(i2cAddress, F("Cal,?"), sensordata))
  {
    if (strncmp(sensordata, "?Cal,", 5))
    {
      char tmp[2];
      tmp[0]                = sensordata[5];
      tmp[1]                = '\0',
      nb_calibration_points = str2int(tmp);
    }
  }

  return nb_calibration_points;
}

void P103_html_color_message(const __FlashStringHelper *color, const String& message) {
  addHtml(F("&nbsp;<span style='color:"));
  addHtml(color);
  addHtml(F(";'>"));
  addHtml(message);
  addHtml(F("</span>"));
}

void P103_html_red(const String& message) {
  P103_html_color_message(F("red"), message);
}

void P103_html_orange(const String& message) {
  P103_html_color_message(F("orange"), message);
}

void P103_html_green(const String& message) {
  P103_html_color_message(F("green"), message);
}

void P103_addDisabler() {
  // disabler(): argument(s) on 'true' will set that checkbox to false! non-boolean ignores argument
  // addHtml(F("\n<script type='text/javascript'>function disabler(clear,single,l,h,dry,nul,atm)"
  //           "{if (clear===true){document.getElementById('en_cal_clear').checked=false;}"
  //           "if (single===true){document.getElementById('en_cal_single').checked=false;}"
  //           "if(l===true){document.getElementById('en_cal_L').checked=false;}"
  //           "if(h===true){document.getElementById('en_cal_H').checked=false;}"
  //           "if(dry===true){document.getElementById('en_cal_dry').checked=false;}"
  //           "if(nul===true){document.getElementById('en_cal_0').checked=false;}"
  //           "if(atm===true){document.getElementById('en_cal_atm').checked=false;}};</script>"));
  // Minified:
  addHtml(F("\n<script type='text/javascript'>function disabler(e,l,c,a,n,d,t){!0===e&&(document.getElementById('en_cal_clear').checked=!1)"
            ",!0===l&&(document.getElementById('en_cal_single').checked=!1)"
            ",!0===c&&(document.getElementById('en_cal_L').checked=!1)"
            ",!0===a&&(document.getElementById('en_cal_H').checked=!1)"
            ",!0===n&&(document.getElementById('en_cal_dry').checked=!1)"
            ",!0===d&&(document.getElementById('en_cal_0').checked=!1)"
            ",!0===t&&(document.getElementById('en_cal_atm').checked=!1)};</script>"));
}

void addClearCalibration()
{
  addRowLabel(F("<strong>Clear calibration</strong>"));
  addFormCheckBox(F("Clear"), F("en_cal_clear"), false);
  addHtml(F("\n<script type='text/javascript'>document.getElementById('en_cal_clear').onclick=disabler(0,true,true,true,true,0,0);</script>"));
  addFormNote(F("Attention! This will reset all calibrated data. New calibration will be needed!!!"));
}

int addDOCalibration(uint8_t I2Cchoice)
{
  int nb_calibration_points = getCalibrationPoints(I2Cchoice);

  addRowLabel("Calibrated Points");
  addHtmlInt(nb_calibration_points);

  if (nb_calibration_points < 1)
  {
    P103_html_red(F("   Calibration needed"));
  }

  addRowLabel(F("<strong>Calibrate to atmospheric oxygen levels</strong>"));
  addFormCheckBox(F("Enable"), F("en_cal_atm"), false);
  addHtml(F("\n<script type='text/javascript'>document.getElementById('en_cal_atm').onclick=disabler(true,0,0,0,0,true,true);</script>"));

  addRowLabel(F("<strong>Calibrate device to 0 dissolved oxygen</strong>"));
  addFormCheckBox(F("Enable"), F("en_cal_0"), false);
  addHtml(F("\n<script type='text/javascript'>document.getElementById('en_cal_0').onclick=disabler(true,0,0,0,0,true,true);</script>"));

  if (nb_calibration_points > 0)
  {
    P103_html_green(F("OK"));
  }
  else
  {
    P103_html_red(F("Not yet calibrated"));
  }

  return nb_calibration_points;
}

void addCreateDryCalibration()
{
  addRowLabel(F("<strong>Dry calibration</strong>"));
  addFormCheckBox(F("Enable"), F("en_cal_dry"), false);
  addHtml(F("\n<script type='text/javascript'>document.getElementById('en_cal_dry').onclick=disabler(true,true,true,true,0,0,0);</script>"));
  addFormNote(F("Dry calibration must always be done first!"));
  addFormNote(F("Calibration for pH-Probe could be 1 (single) or 2 point (low, high)."));
}

int addCreateSinglePointCalibration(uint8_t             board_type,
                                    struct EventStruct *event,
                                    uint8_t             I2Cchoice,
                                    String              unit,
                                    float               min,
                                    float               max,
                                    uint8_t             nrDecimals,
                                    float               stepsize)
{
  int nb_calibration_points = getCalibrationPoints(I2Cchoice);

  addRowLabel("Calibrated Points");
  addHtmlInt(nb_calibration_points);

  if (nb_calibration_points < 1)
  {
    P103_html_red(F("   Calibration needed"));
  }

  addRowLabel(F("<strong>Single point calibration</strong>"));
  addFormFloatNumberBox(F("Ref single point"), F("ref_cal_single'"), P103_CALIBRATION_SINGLE, min, max, nrDecimals, stepsize);
  addUnit(unit);

  if (((board_type != EC) && (nb_calibration_points > 0)) || ((board_type == EC) && (nb_calibration_points == 1)))
  {
    P103_html_green(F("OK"));
  }
  else
  {
    if ((board_type == EC) && (nb_calibration_points > 1))
    {
      P103_html_green(F("Not calibrated, because two point calibration is active."));
    }
    else
    {
      P103_html_red(F("Not yet calibrated"));
    }
  }
  addFormCheckBox(F("Enable"), F("en_cal_single"), false);
  addHtml(F(
            "\n<script type='text/javascript'>document.getElementById('en_cal_single').onclick=disabler(true,0,true,true,true,0,0);</script>"));

  return nb_calibration_points;
}

int addCreate3PointCalibration(uint8_t             board_type,
                               struct EventStruct *event,
                               uint8_t             I2Cchoice,
                               String              unit,
                               float               min,
                               float               max,
                               uint8_t             nrDecimals,
                               float               stepsize)
{
  int nb_calibration_points = addCreateSinglePointCalibration(board_type, event, I2Cchoice, unit, min, max, nrDecimals, stepsize);

  addRowLabel(F("<strong>Low calibration</strong>"));
  addFormFloatNumberBox(F("Ref low point"), F("ref_cal_L"), P103_CALIBRATION_LOW, min, max, nrDecimals, stepsize);
  addUnit(unit);

  if (nb_calibration_points > 1)
  {
    P103_html_green(F("OK"));
  }
  else
  {
    P103_html_orange(F("Not yet calibrated"));
  }
  addFormCheckBox(F("Enable"), F("en_cal_L"), false);
  addHtml(F("\n<script type='text/javascript'>document.getElementById('en_cal_L').onclick=disabler(true,true,0,true,true,0,0);</script>"));

  addHtml(F("<TR><TD><strong>High calibration</strong></TD>"));
  addFormFloatNumberBox(F("Ref high point"), F("ref_cal_H"), P103_CALIBRATION_HIGH, min, max, nrDecimals, stepsize);
  addUnit(unit);

  // pH: low, high OK with 3 calibration points (single is the first one); EC: low high OK with 2 calibration points
  if ((nb_calibration_points > 2) || ((board_type == EC) && (nb_calibration_points > 1)))
  {
    P103_html_green(F("OK"));
  }
  else
  {
    P103_html_orange(F("Not yet calibrated"));
  }
  addFormCheckBox(F("Enable"), F("en_cal_H"), false);
  addHtml(F("\n<script type='text/javascript'>document.getElementById('en_cal_H').onclick=disabler(true,true,true,0,true,0,0);</script>"));

  return nb_calibration_points;
}

#endif // ifdef USES_P103
