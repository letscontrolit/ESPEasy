#include "_Plugin_Helper.h"

#ifdef USES_P103

// ########################################################################
// ################## Plugin 103 : Atlas Scientific EZO pH ORP EC sensors #
// ########################################################################

// datasheet at https://atlas-scientific.com/files/pH_EZO_Datasheet.pdf
// datasheet at https://atlas-scientific.com/files/ORP_EZO_Datasheet.pdf
// datasheet at https://atlas-scientific.com/files/EC_EZO_Datasheet.pdf
// works only in i2c mode

# include "src/Helpers/Rules_calculate.h"

# define PLUGIN_103
# define PLUGIN_ID_103           103
# define PLUGIN_NAME_103         "Environment - Atlas EZO pH ORP EC"
# define PLUGIN_VALUENAME1_103   "SensorData"
# define PLUGIN_VALUENAME2_103   "Voltage"
# define UNKNOWN                 0
# define PH                      1
# define ORP                     2
# define EC                      3

# define FIXED_TEMP_VALUE        20   // Temperature correction for pH and EC sensor if no temperature is given from calculation

boolean Plugin_103(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  byte board_type;
  byte I2Cchoice;

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

    case PLUGIN_WEBFORM_LOAD:
    {
      I2Cchoice  = PCONFIG(1);
      addHtml(F("<TR><TD>Default I2C Address:</TD><TD>pH: 0x63, ORP: 0x62, EC: 0x64. The plugin is able to detect the type of device automatically.</TD></TR>"));
      # define _P103_ATLASEZO_I2C_NB_OPTIONS 6
      int optionValues[_P103_ATLASEZO_I2C_NB_OPTIONS] = { 0x62, 0x63, 0x64, 0x65, 0x66, 0x67 };
      addFormSelectorI2C(F("plugin_103_i2c"), _P103_ATLASEZO_I2C_NB_OPTIONS, optionValues, I2Cchoice);

      addFormSubHeader(F("General"));

      char sensordata[32];
      bool info;
      info = _P103_send_I2C_command(I2Cchoice, F("i"), sensordata);

      if (info) {
        String boardInfo(sensordata);

        addHtml(F("<TR><TD>Board type : </TD><TD>"));
        String board = boardInfo.substring(boardInfo.indexOf(',') + 1, boardInfo.lastIndexOf(','));
        String version = boardInfo.substring(boardInfo.lastIndexOf(',') + 1);
        addHtml(board);

        if (board == F("pH"))
        {
            board_type = PH;
        }
        else if (board == F("ORP"))
        {
            board_type = ORP;
        }
        else if (board == F("EC"))
        {
            board_type = EC;
        }

        PCONFIG(0) = board_type;

        if (board_type == UNKNOWN) {
          addHtml(F("<span style='color:red'>  WARNING : Board type should be 'pH' or 'ORP' or 'EC', check your i2c Address ? </span>"));
        }
        addHtml(F("</TD></TR><TR><TD>Board version :</TD><TD>"));
        addHtml(version);
        addHtml(F("</TD></TR>"));

        addHtml(F("<input type='hidden' name='plugin_103_sensorVersion' value='"));
        addHtml(version);
        addHtml(F("'>"));
      } else {
        addHtml(F("<span style='color:red;'>Unable to send command to device</span>"));
        if (board_type == UNKNOWN) {
          addHtml(F("<span style='color:red'>  WARNING : Board type should be 'pH' or 'ORP' or 'EC', check your i2c Address ? </span>"));
        }
        success = false;
        break;
      }

      addFormCheckBox(F("Status LED"), F("Plugin_103_status_led"), PCONFIG(2));

      char statussensordata[32];
      bool status;
      status = _P103_send_I2C_command(I2Cchoice, F("Status"), statussensordata);

      if (status) {
        String boardStatus(statussensordata);

        addHtml(F("<TR><TD>Board restart code: </TD><TD>"));
        int pos1 = boardStatus.indexOf(',');
        int pos2 = boardStatus.lastIndexOf(',');

        switch ((char)boardStatus.substring(pos1 + 1, pos2)[0])
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

        addHtml(F("</TD></TR><TR><TD>Board voltage :</TD><TD>"));
        addHtml(boardStatus.substring(pos2 + 1));
        addHtml(F(" V</TD></TR>"));

        addHtml(F("<input type='hidden' name='plugin_103_sensorVoltage' value='"));
        addHtml(boardStatus.substring(pos2 + 1));
        addHtml(F("'>"));

        addHtml(F("</TD></TR><TR><TD>Sensor Data :</TD><TD>"));
        addHtml(String(UserVar[event->BaseVarIndex]));
        addHtml(F("</TD></TR>"));

        addHtml(F("<input type='hidden' name='plugin_103_sensorVoltage' value='"));
        addHtml(String(UserVar[event->BaseVarIndex]));
        addHtml(F("'>"));
      } else {
        addHtml(F("<span style='color:red;'>Unable to send status command to device</span>"));
        success = false;
        break;
      }

      // calibrate
      addFormSubHeader(F("Calibration"));

      switch (board_type)
      {
          case PH:
          {
            int nb_calibration_points = -1;
            status = _P103_send_I2C_command(I2Cchoice, F("Cal,?"), sensordata);

            if (status) {
                if (strncmp(sensordata, "?Cal,", 5)) {
                char tmp[2];
                tmp[0]                = sensordata[5];
                tmp[1]                = '\0',
                nb_calibration_points = atoi(tmp);
                }
            }

            addRowLabel(F("<strong>Middle</strong>"));
            addFormFloatNumberBox(F("Ref Ph"),
                                    F("Plugin_103_ref_cal_M'"),
                                    PCONFIG_FLOAT(1),
                                    0,
                                    14,
                                    2,
                                    0.01);

            if (nb_calibration_points > 0) {
                addHtml(F("&nbsp;<span style='color:green;'>OK</span>"));
            } else {
                addHtml(F("&nbsp;<span style='color:red;'>Not yet calibrated</span>"));
            }
            addFormCheckBox(F("Enable"), F("Plugin_103_enable_cal_M"), false);
            addHtml(F(
                        "\n<script type='text/javascript'>document.getElementById(\"Plugin_103_enable_cal_M\").onclick = function(){document.getElementById(\"Plugin_103_enable_cal_L\").checked = false;document.getElementById(\"Plugin_103_enable_cal_H\").checked = false;};</script>\n"));

            addRowLabel(F("<strong>Low</strong>"));
            addFormFloatNumberBox(F("Ref Ph"),
                                    F("Plugin_103_ref_cal_L"),
                                    PCONFIG_FLOAT(2),
                                    0,
                                    14,
                                    2,
                                    0.01);

            if (nb_calibration_points > 1) {
                addHtml(F("&nbsp;<span style='color:green;'>OK</span>"));
            } else {
                addHtml(F("&nbsp;<span style='color:red;'>Not yet calibrated</span>"));
            }
            addFormCheckBox(F("Enable"), F("Plugin_103_enable_cal_L"), false);
            addHtml(F(
                        "\n<script type='text/javascript'>document.getElementById(\"Plugin_103_enable_cal_L\").onclick = function(){document.getElementById(\"Plugin_103_enable_cal_M\").checked = false;document.getElementById(\"Plugin_103_enable_cal_H\").checked = false;};</script>\n"));

            addHtml(F("<TR><TD><strong>High</strong></TD>"));
            addFormFloatNumberBox(F("Ref Ph"),
                                    F("Plugin_103_ref_cal_H"),
                                    PCONFIG_FLOAT(3),
                                    0,
                                    14,
                                    2,
                                    0.01);

            if (nb_calibration_points > 2) {
                addHtml(F("&nbsp;<span style='color:green;'>OK</span>"));
            } else {
                addHtml(F("&nbsp;<span style='color:orange;'>Not yet calibrated</span>"));
            }
            addFormCheckBox(F("Enable"), F("Plugin_103_enable_cal_H"), false);
            addHtml(F(
                        "\n<script type='text/javascript'>document.getElementById(\"Plugin_103_enable_cal_H\").onclick = function(){document.getElementById(\"Plugin_103_enable_cal_L\").checked = false;document.getElementById(\"Plugin_103_enable_cal_M\").checked = false;};</script>\n"));

            if (nb_calibration_points > 1) {
                char sensordata[32];
                bool status;
                status = _P103_send_I2C_command(I2Cchoice, F("Slope,?"), sensordata);

                if (status) {
                String slopeAnswer = F("Answer to 'Slope' command : ");
                slopeAnswer += sensordata;
                addFormNote(slopeAnswer);
                }
            }
            break;
          }
        
        case ORP:
        {
            int nb_calibration_points = -1;
            status = _P103_send_I2C_command(I2Cchoice, F("Cal,?"), sensordata);

            if (status) {
                if (strncmp(sensordata, "?Cal,", 5)) {
                char tmp[2];
                tmp[0]                = sensordata[5];
                tmp[1]                = '\0',
                nb_calibration_points = atoi(tmp);
                }
            }

            addRowLabel(F("<strong>ORP Calibration</strong>"));
            addFormFloatNumberBox(F("Ref ORP"), F("Plugin_103_ref_cal_O"), PCONFIG_FLOAT(1), 0, 1500, 1, 1);

            if (nb_calibration_points > 0) {
                addHtml(F("&nbsp;<span style='color:green;'>OK</span>"));
            } else {
                addHtml(F("&nbsp;<span style='color:red;'>Not yet calibrated</span>"));
            }
            addFormCheckBox(F("Enable"), F("Plugin_103_enable_cal_O"), false);
            break;
        }

        case EC:
        {
            addRowLabel(F("<strong>Dry calibration</strong>"));
            addFormCheckBox(F("Enable"), F("Plugin_103_enable_cal_dry"), false);
            addHtml(F(
                        "\n<script type='text/javascript'>document.getElementById(\"Plugin_103_enable_cal_dry\").onclick=function() {document.getElementById(\"Plugin_103_enable_cal_single\").checked = false;document.getElementById(\"Plugin_103_enable_cal_L\").checked = false;;document.getElementById(\"Plugin_103_enable_cal_H\").checked = false;};</script>"));
            addFormNote(F("Dry calibration must always be done first!"));

            addRowLabel(F("<strong>Single point calibration</strong> "));
            addFormCheckBox(F("Enable"), F("Plugin_103_enable_cal_single"), false);
            addHtml(F(
                        "\n<script type='text/javascript'>document.getElementById(\"Plugin_103_enable_cal_single\").onclick=function() {document.getElementById(\"Plugin_103_enable_cal_dry\").checked = false;document.getElementById(\"Plugin_103_enable_cal_L\").checked = false;;document.getElementById(\"Plugin_103_enable_cal_H\").checked = false;};</script>"));
            addFormNumericBox(F("Ref EC"), F("Plugin_103_ref_cal_single"), PCONFIG(3));
            addUnit(F("&micro;S"));

            addRowLabel(F("<strong>Low calibration</strong>"));
            addFormCheckBox(F("Enable"),
                            F(
                                "Plugin_103_enable_cal_L' onClick='document.getElementById(\"Plugin_103_enable_cal_dry\").checked = false;document.getElementById(\"Plugin_103_enable_cal_single\").checked = false;;document.getElementById(\"Plugin_103_enable_cal_H\").checked = false;"),
                            false);
            addHtml(F(
                        "\n<script type='text/javascript'>document.getElementById(\"Plugin_103_enable_cal_L\").onclick=function() {document.getElementById(\"Plugin_103_enable_cal_dry\").checked = false;document.getElementById(\"Plugin_103_enable_cal_single\").checked = false;;document.getElementById(\"Plugin_103_enable_cal_H\").checked = false;};</script>"));
            addFormNumericBox(F("Ref EC"), F("Plugin_103_ref_cal_L"), PCONFIG(4));
            addUnit(F("&micro;S"));

            addRowLabel(F("<strong>High calibration</strong>"));
            addFormCheckBox(F("Enable"), F("Plugin_103_enable_cal_H"), false);
            addHtml(F(
                        "\n<script type='text/javascript'>document.getElementById(\"Plugin_103_enable_cal_H\").onclick=function() {document.getElementById(\"Plugin_103_enable_cal_dry\").checked = false;document.getElementById(\"Plugin_103_enable_cal_single\").checked = false;;document.getElementById(\"Plugin_103_enable_cal_L\").checked = false;};</script>"));
            addFormNumericBox(F("Ref EC"), F("Plugin_103_ref_cal_H"), PCONFIG(5));
            addUnit(F("&micro;S"));

            status = _P103_send_I2C_command(I2Cchoice, F("Cal,?"), sensordata);

            if (status) {
                switch (sensordata[5]) {
                case '0':
                    addFormNote(F("<span style='color:red'>Calibration needed</span>"));
                    break;
                case '1':
                    addFormNote(F("<span style='color:green'>Single point calibration ok</span>"));
                    break;
                case '2':
                    addFormNote(F("<span style='color:green'>Two points calibration ok</span>"));
                    break;
                }
            }
        }
        break;
      }

        // Temperature compensation
      if(board_type == PH || board_type == EC) {
        
        addFormSubHeader(F("Temperature compensation"));
        char deviceTemperatureTemplate[40];
        LoadCustomTaskSettings(event->TaskIndex, (byte *)&deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));
            addFormTextBox(F("Temperature "), F("Plugin_103_temperature_template"), deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));
            addFormNote(F(
                            "You can use a formulae and idealy refer to a temp sensor (directly, via ESPEasyP2P or MQTT import) ,e.g. '[Pool#Temperature]'. If you don't have a sensor, you could type a fixed value like '25' for '25.5'."));
            double value;
            char strValue[5];
            String deviceTemperatureTemplateString(deviceTemperatureTemplate);
            String pooltempString(parseTemplate(deviceTemperatureTemplateString, 40));
            addHtml(F("<div class='note'>"));

            if (Calculate(pooltempString.c_str(), value) != CalculateReturnCode::OK) {
                addHtml(F("It seems I can't parse your formulae. Fixed value will be used!"));
                value = FIXED_TEMP_VALUE;
            }
            addHtml(F("</div>"));

            addHtml(F("<div class='note'>"));
            addHtml(F("Actual value : "));
            dtostrf(value, 5, 2, strValue);
            addHtml(strValue);

            addHtml(F("</div>"));
        }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      board_type = PCONFIG(0);

      I2Cchoice = getFormItemInt(F("plugin_103_i2c"));
      
      PCONFIG(1) = I2Cchoice;

      PCONFIG_FLOAT(0) = getFormItemFloat(F("plugin_103_sensorVersion"));

      char sensordata[32];

      if (isFormItemChecked(F("Plugin_103_status_led"))) {
        _P103_send_I2C_command(I2Cchoice, F("L,1"), sensordata);
      } else {
        _P103_send_I2C_command(I2Cchoice, F("L,0"), sensordata);
      }
      PCONFIG(2) = isFormItemChecked(F("Plugin_103_status_led"));

      String cmd("Cal,");
      bool   triggerCalibrate = false;

      switch (board_type)
      {
          case PH: {
            PCONFIG_FLOAT(1) = getFormItemFloat(F("Plugin_103_ref_cal_M"));
            PCONFIG_FLOAT(2) = getFormItemFloat(F("Plugin_103_ref_cal_L"));
            PCONFIG_FLOAT(3) = getFormItemFloat(F("Plugin_103_ref_cal_H"));

            if (isFormItemChecked(F("Plugin_103_enable_cal_M"))) {
                cmd             += F("mid,");
                cmd             += PCONFIG_FLOAT(1);
                triggerCalibrate = true;
            } else if (isFormItemChecked(F("Plugin_103_enable_cal_L"))) {
                cmd             += F("low,");
                cmd             += PCONFIG_FLOAT(2);
                triggerCalibrate = true;
            } else if (isFormItemChecked(F("Plugin_103_enable_cal_H"))) {
                cmd             += F("high,");
                cmd             += PCONFIG_FLOAT(3);
                triggerCalibrate = true;
            }
            break;
          }

          case ORP:
          {
            PCONFIG_FLOAT(1) = getFormItemFloat(F("Plugin_103_ref_cal_O"));

            if (isFormItemChecked(F("Plugin_103_enable_cal_O"))) {
                cmd             += PCONFIG_FLOAT(1);
                triggerCalibrate = true;
            }
            break;
          }

          case EC:
          {
            PCONFIG(3) = getFormItemInt(F("Plugin_103_ref_cal_single"));
            PCONFIG(4) = getFormItemInt(F("Plugin_103_ref_cal_L"));
            PCONFIG(5) = getFormItemInt(F("Plugin_103_ref_cal_H"));

            if (isFormItemChecked(F("Plugin_103_enable_cal_dry"))) {
                cmd             += F("dry");
                triggerCalibrate = true;
            } else if (isFormItemChecked(F("Plugin_103_enable_cal_single"))) {
                cmd             += PCONFIG(3);
                triggerCalibrate = true;
            } else if (isFormItemChecked(F("Plugin_103_enable_cal_L"))) {
                cmd             += F("low,");
                cmd             += PCONFIG(4);
                triggerCalibrate = true;
            } else if (isFormItemChecked(F("Plugin_103_enable_cal_H"))) {
                cmd             += F("high,");
                cmd             += PCONFIG(5);
                triggerCalibrate = true;
            }
            break;
          }
      }

      if (triggerCalibrate) {
        char sensordata[32];
        _P103_send_I2C_command(I2Cchoice, cmd, sensordata);
      }

      if(board_type == PH || board_type == EC) {
        char   deviceTemperatureTemplate[40];
        String tmpString = web_server.arg(F("Plugin_103_temperature_template"));
        strncpy(deviceTemperatureTemplate, tmpString.c_str(), sizeof(deviceTemperatureTemplate) - 1);
        deviceTemperatureTemplate[sizeof(deviceTemperatureTemplate) - 1] = 0; // be sure that our string ends with a \0

        addHtmlError(SaveCustomTaskSettings(event->TaskIndex, (byte *)&deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate)));
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      break;
    }

    case PLUGIN_READ:
    {
      board_type = PCONFIG(0);
      I2Cchoice  = PCONFIG(1);

      char sensordata[32];
      bool status;
      String readCommand;

      if(board_type == PH || board_type == EC) {
        // first set the temperature of reading
        char deviceTemperatureTemplate[40];
        LoadCustomTaskSettings(event->TaskIndex, (byte *)&deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));

        String deviceTemperatureTemplateString(deviceTemperatureTemplate);
        String pooltempString(parseTemplate(deviceTemperatureTemplateString, 40));

        readCommand = F("RT,");
        double  temperatureReading;

        if (Calculate(pooltempString.c_str(), temperatureReading) != CalculateReturnCode::OK) {
            temperatureReading = FIXED_TEMP_VALUE;
        }

        readCommand += temperatureReading;

      } else if(board_type == ORP) {
        readCommand = F("R,");
      }

      // ok, now we can read the value
      status = _P103_send_I2C_command(I2Cchoice, readCommand, sensordata);


      // we read the voltagedata char statussensordata[32];
      char voltagedata[32];
      status = _P103_send_I2C_command(I2Cchoice, F("Status"), voltagedata);


      if (status) {
        String sensorString(sensordata);
        String voltage(voltagedata);
        int    pos = voltage.lastIndexOf(',');
        UserVar[event->BaseVarIndex]     = sensorString.toFloat();
        UserVar[event->BaseVarIndex + 1] = voltage.substring(pos + 1).toFloat();
      }
      else {
        UserVar[event->BaseVarIndex]     = -1;
        UserVar[event->BaseVarIndex + 1] = -1;
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

bool _P103_send_I2C_command(uint8_t I2Caddress, const String& cmd, char *sensordata) {
  uint16_t sensor_bytes_received = 0;

  byte error;
  byte i2c_response_code = 0;
  byte in_char           = 0;

  String log = F("> cmd = ");
          log += String(cmd);
          addLog(LOG_LEVEL_DEBUG, log);

  addLog(LOG_LEVEL_DEBUG, String(cmd));
  Wire.beginTransmission(I2Caddress);
  Wire.write(cmd.c_str());
  error = Wire.endTransmission();

  if (error != 0) {
    // addLog(LOG_LEVEL_ERROR, error);
    addLog(LOG_LEVEL_ERROR, F("Wire.endTransmission() returns error: Check Atlas shield, pH, ORP and EC are supported."));
    return false;
  }

  // don't read answer if we want to go to sleep
  if (cmd.substring(0, 5).equalsIgnoreCase(F("Sleep"))) {
    return true;
  }

  i2c_response_code = 254;

  while (i2c_response_code == 254) { // in case the command takes longer to process, we keep looping here until we get a success or an error
    if  (
      (((cmd[0] == 'r') || (cmd[0] == 'R')) && (cmd[1] == '\0'))
      ||
      (cmd.substring(0, 3).equalsIgnoreCase(F("cal")) && !cmd.substring(0, 5).equalsIgnoreCase(F("Cal,?")))
      )
    {
      // FIXME TD-er: No way we will wait this long.
      //delay(900);
    }
    else {
      // FIXME TD-er: No way we will wait this long.
      //delay(300);
    }

    Wire.requestFrom(I2Caddress, (uint8_t)32); // call the circuit and request 32 bytes (this is more then we need).
    i2c_response_code = Wire.read();           // read response code

    while (Wire.available()) {                 // read response
      in_char = Wire.read();

      if (in_char == 0) {                      // if we receive a null caracter, we're done
        while (Wire.available()) {             // purge the data line if needed
          Wire.read();
        }

        break;                                       // exit the while loop.
      }
      else {
        sensordata[sensor_bytes_received] = in_char; // load this byte into our array.
        sensor_bytes_received++;
      }
    }
    sensordata[sensor_bytes_received] = '\0';

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      switch (i2c_response_code) {
        case 1:
        {
          String log = F("< success, answer = ");
          log += sensordata;
          addLog(LOG_LEVEL_DEBUG, log);
          break;
        }

        case 2:
          addLog(LOG_LEVEL_DEBUG, F("< command failed"));
          return false;

        case 254:
          addLog(LOG_LEVEL_DEBUG_MORE, F("< command pending"));
          break;

        case 255:
          addLog(LOG_LEVEL_DEBUG, F("< no data"));
          return false;
      }
    }
  }

  return true;
}

#endif // ifdef USES_P103