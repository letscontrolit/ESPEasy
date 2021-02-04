#include "_Plugin_Helper.h"


#ifdef USES_P103

// ########################################################################
// ################## Plugin 103 : Atlas Scientific EZO Ph sensor  ########
// ########################################################################

// datasheet at https://www.atlas-scientific.com/_files/_datasheets/_circuit/pH_EZO_datasheet.pdf
// works only in i2c mode

# include "src/Helpers/Rules_calculate.h"

# define PLUGIN_103
# define PLUGIN_ID_103         103
# define PLUGIN_NAME_103       "Environment - Atlas Scientific EZO pH"
# define PLUGIN_VALUENAME1_103 "pH"
# define PLUGIN_VALUENAME2_103 "Voltage"

# define FIXED_TEMP_VALUE      20


boolean Plugin_103(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

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
        # define _P103_ATLASEZO_I2C_NB_OPTIONS 4
      byte I2Cchoice                                  = PCONFIG(0);
      int optionValues[_P103_ATLASEZO_I2C_NB_OPTIONS] = { 0x63, 0x64, 0x65, 0x66 };
      addFormSelectorI2C(F("plugin_103_i2c"), _P103_ATLASEZO_I2C_NB_OPTIONS, optionValues, I2Cchoice);

      addFormSubHeader(F("General"));

      char sensordata[32];
      bool info;
      info = _P103_send_I2C_command(PCONFIG(0), F("i"), sensordata);

      if (info) {
        String boardInfo(sensordata);

        addHtml(F("<TR><TD>Board type : </TD><TD>"));
        int pos1 = boardInfo.indexOf(',');
        int pos2 = boardInfo.lastIndexOf(',');
        addHtml(boardInfo.substring(pos1 + 1, pos2));

        if (boardInfo.substring(pos1 + 1, pos2) != F("pH")) {
          addHtml(F("<span style='color:red'>  WARNING : Board type should be 'pH', check your i2c Address ? </span>"));
        }
        addHtml(F("</TD></TR><TR><TD>Board version :</TD><TD>"));
        addHtml(boardInfo.substring(pos2 + 1));
        addHtml(F("</TD></TR>"));

        addHtml(F("<input type='hidden' name='plugin_103_sensorVersion' value='"));
        addHtml(boardInfo.substring(pos2 + 1));
        addHtml(F("'>"));
      } else {
        addHtml(F("<span style='color:red;'>Unable to send command to device</span>"));
        success = false;
        break;
      }

      addFormCheckBox(F("Status LED"), F("Plugin_103_status_led"), PCONFIG(1));

      char statussensordata[32];
      bool status;
      status = _P103_send_I2C_command(PCONFIG(0), F("Status"), statussensordata);

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
      } else {
        addHtml(F("<span style='color:red;'>Unable to send status command to device</span>"));
        success = false;
        break;
      }

      addFormSubHeader(F("Calibration"));

      int nb_calibration_points = -1;
      status = _P103_send_I2C_command(PCONFIG(0), F("Cal,?"), sensordata);

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
                            F("Plugin_103_ref_cal_M' step='0.01"),
                            PCONFIG_FLOAT(1),
                            0,
                            14);

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
                            F("Plugin_103_ref_cal_L' step='0.01"),
                            PCONFIG_FLOAT(2),
                            0,
                            14);

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
                            F("Plugin_103_ref_cal_H' step='0.01"),
                            PCONFIG_FLOAT(3),
                            0,
                            14);

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
        status = _P103_send_I2C_command(PCONFIG(0), F("Slope,?"), sensordata);

        if (status) {
          String slopeAnswer = F("Answer to 'Slope' command : ");
          slopeAnswer += sensordata;
          addFormNote(slopeAnswer);
        }
      }

      addFormSubHeader(F("Temperature compensation"));
      char deviceTemperatureTemplate[40];
      LoadCustomTaskSettings(event->TaskIndex, (byte *)&deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));
      addFormTextBox(F("Temperature "), F("Plugin_103_temperature_template"), deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));
      addFormNote(F(
                    "You can use a formulae and idealy refer to a temp sensor (directly, via ESPEasyP2P or MQTT import) ,e.g. '[Pool#Temperature]'. If you don't have a sensor, you could type a fixed value like '25' for 25°."));
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

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("plugin_103_i2c"));

      PCONFIG_FLOAT(0) = getFormItemFloat(F("plugin_103_sensorVersion"));

      char sensordata[32];

      if (isFormItemChecked(F("Plugin_103_status_led"))) {
        _P103_send_I2C_command(PCONFIG(0), F("L,1"), sensordata);
      } else {
        _P103_send_I2C_command(PCONFIG(0), F("L,0"), sensordata);
      }
      PCONFIG(1) = isFormItemChecked(F("Plugin_103_status_led"));


      PCONFIG_FLOAT(1) = getFormItemFloat(F("Plugin_103_ref_cal_M"));
      PCONFIG_FLOAT(2) = getFormItemFloat(F("Plugin_103_ref_cal_L"));
      PCONFIG_FLOAT(3) = getFormItemFloat(F("Plugin_103_ref_cal_H"));

      String cmd("Cal,");
      bool   triggerCalibrate = false;

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

      if (triggerCalibrate) {
        char sensordata[32];
        _P103_send_I2C_command(PCONFIG(0), cmd, sensordata);
      }

      char   deviceTemperatureTemplate[40];
      String tmpString = web_server.arg(F("Plugin_103_temperature_template"));
      strncpy(deviceTemperatureTemplate, tmpString.c_str(), sizeof(deviceTemperatureTemplate) - 1);
      deviceTemperatureTemplate[sizeof(deviceTemperatureTemplate) - 1] = 0; // be sure that our string ends with a \0

      addHtmlError(SaveCustomTaskSettings(event->TaskIndex, (byte *)&deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate)));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      break;
    }

    case PLUGIN_READ:
    {
      char sensordata[32];
      bool status;

      // first set the temperature of reading
      char deviceTemperatureTemplate[40];
      LoadCustomTaskSettings(event->TaskIndex, (byte *)&deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));

      String deviceTemperatureTemplateString(deviceTemperatureTemplate);
      String pooltempString(parseTemplate(deviceTemperatureTemplateString, 40));

      // String setTemperature = F("T,");
      String setTemperature = F("RT,");
      double  temperatureReading;

      if (Calculate(pooltempString.c_str(), temperatureReading) != CalculateReturnCode::OK) {
        temperatureReading = FIXED_TEMP_VALUE;
      }

      setTemperature += temperatureReading;

      // ok, now we can read the pH value with Temperature compensation
      status = _P103_send_I2C_command(PCONFIG(0), setTemperature, sensordata);

      // ok, now we can read the pH value
      // status = _P103_send_I2C_command(PCONFIG(0),"r",sensordata);

      // we read the voltagedata char statussensordata[32];
      char voltagedata[32];
      status = _P103_send_I2C_command(PCONFIG(0), F("Status"), voltagedata);


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

      // go to sleep
      // status = _P103_send_I2C_command(PCONFIG(0),"Sleep",sensordata);

      success = true;
      break;
    }
    case PLUGIN_WRITE:
    {
      String tmpString = string;
      int    argIndex  = tmpString.indexOf(',');

      if (argIndex) {
        tmpString = tmpString.substring(0, argIndex);
      }

      if (tmpString.equalsIgnoreCase(F("ATLASCMD")))
      {
        success   = true;
        argIndex  = string.lastIndexOf(',');
        tmpString = string.substring(argIndex + 1);

        if (tmpString.equalsIgnoreCase(F("CalMid"))) {
          addLog(LOG_LEVEL_INFO, F("Asking for Mid calibration "));
        }
        else if (tmpString.equalsIgnoreCase(F("CalLow"))) {
          addLog(LOG_LEVEL_INFO, F("Asking for Low calibration "));
        }
        else if (tmpString.equalsIgnoreCase(F("CalHigh"))) {
          addLog(LOG_LEVEL_INFO, F("Asking for High calibration "));
        }
      }
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

  addLog(LOG_LEVEL_DEBUG, String(cmd));
  Wire.beginTransmission(I2Caddress);
  Wire.write(cmd.c_str());
  error = Wire.endTransmission();

  if (error != 0) {
    // addLog(LOG_LEVEL_ERROR, error);
    addLog(LOG_LEVEL_ERROR, F("Wire.endTransmission() returns error: Check pH shield"));
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
      delay(900);
    }
    else {
      // FIXME TD-er: No way we will wait this long.
      delay(300);
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
          addLog(LOG_LEVEL_DEBUG, F("< command pending"));
          break;

        case 255:
          addLog(LOG_LEVEL_DEBUG, F("< no data"));
          return false;
      }
    }
  }

  addLog(LOG_LEVEL_DEBUG, sensordata);
  return true;
}

#endif // ifdef USES_P103
