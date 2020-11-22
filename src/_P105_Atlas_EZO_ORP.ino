#include "_Plugin_Helper.h"

#ifdef USES_P105

// ########################################################################
// ################## Plugin 105 : Atlas Scientific EZO ORP sensor  ########
// ########################################################################

// datasheet at https://www.atlas-scientific.com/_files/_datasheets/_circuit/ORP_EZO_datasheet.pdf
// works only in i2c mode

# define PLUGIN_105
# define PLUGIN_ID_105 105
# define PLUGIN_NAME_105       "Environment - Atlas Scientific EZO ORP"
# define PLUGIN_VALUENAME1_105 "ORP"
# define PLUGIN_VALUENAME2_105 "Voltage"


boolean Plugin_105(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_105;
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
      string = F(PLUGIN_NAME_105);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_105));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_105));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
        # define _P105_ATLASEZO_I2C_NB_OPTIONS 4
      byte I2Cchoice                                  = PCONFIG(0);
      int optionValues[_P105_ATLASEZO_I2C_NB_OPTIONS] = { 0x62, 0x63, 0x64, 0x65 };
      addFormSelectorI2C(F("plugin_105_i2c"), _P105_ATLASEZO_I2C_NB_OPTIONS, optionValues, I2Cchoice);

      addFormSubHeader(F("General"));

      char sensordata[32];
      bool info;
      info = _P105_send_I2C_command(PCONFIG(0), F("i"), sensordata);

      if (info) {
        String boardInfo(sensordata);

        addHtml(F("<TR><TD>Board type : </TD><TD>"));
        int pos1 = boardInfo.indexOf(',');
        int pos2 = boardInfo.lastIndexOf(',');
        addHtml(boardInfo.substring(pos1 + 1, pos2));

        if (boardInfo.substring(pos1 + 1, pos2) != "ORP") {
          addHtml(F("<span style='color:red'>  WARNING : Board type should be 'ORP', check your i2c Address ? </span>"));
        }
        addHtml(F("</TD></TR><TR><TD>Board version :</TD><TD>"));
        addHtml(boardInfo.substring(pos2 + 1));
        addHtml(F("</TD></TR>"));

        addHtml(F("<input type='hidden' name='plugin_105_sensorVersion' value='"));
        addHtml(boardInfo.substring(pos2 + 1));
        addHtml(F("'>"));
      } else {
        addHtml(F("<span style='color:red;'>Unable to send command to device</span>"));
        success = false;
        break;
      }

      addFormCheckBox(F("Status LED"), F("Plugin_105_status_led"), PCONFIG(1));

      char statussensordata[32];
      bool status;
      status = _P105_send_I2C_command(PCONFIG(0), F("Status"), statussensordata);

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

        addHtml(F("<input type='hidden' name='plugin_105_sensorVoltage' value='"));
        addHtml(boardStatus.substring(pos2 + 1));
        addHtml(F("'>"));
      } else {
        addHtml(F("<span style='color:red;'>Unable to send status command to device</span>"));
        success = false;
        break;
      }

      addFormSubHeader(F("Calibration"));

      int nb_calibration_points = -1;
      status = _P105_send_I2C_command(PCONFIG(0), F("Cal,?"), sensordata);

      if (status) {
        if (strncmp(sensordata, "?Cal,", 5)) {
          char tmp[2];
          tmp[0]                = sensordata[5];
          tmp[1]                = '\0',
          nb_calibration_points = atoi(tmp);
        }
      }

      addRowLabel(F("<strong>ORP Calibration</strong>"));
      addFormNumericBox(F("Ref ORP"), F("Plugin_105_ref_cal_M' step='1"), PCONFIG_FLOAT(1), 0, 1500);

      if (nb_calibration_points > 0) {
        addHtml(F("&nbsp;<span style='color:green;'>OK</span>"));
      } else {
        addHtml(F("&nbsp;<span style='color:red;'>Not yet calibrated</span>"));
      }
      addFormCheckBox(F("Enable"), F("Plugin_105_enable_cal_M"), false);

      if (nb_calibration_points > 1) {
        char sensordata[32];
        bool status;
        status = _P105_send_I2C_command(PCONFIG(0), F("Slope,?"), sensordata);

        if (status) {
          String slopeAnswer = F("Answer to 'Slope' command : ");
          slopeAnswer += sensordata;
          addFormNote(slopeAnswer);
        }
      }

      addHtml(F("</div>"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("plugin_105_i2c"));

      PCONFIG_FLOAT(0) = getFormItemFloat(F("plugin_105_sensorVersion"));

      char sensordata[32];

      if (isFormItemChecked(F("Plugin_105_status_led"))) {
        _P105_send_I2C_command(PCONFIG(0), F("L,1"), sensordata);
      } else {
        _P105_send_I2C_command(PCONFIG(0), F("L,0"), sensordata);
      }
      PCONFIG(1) = isFormItemChecked(F("Plugin_105_status_led"));


      PCONFIG_FLOAT(1) = getFormItemFloat(F("Plugin_105_ref_cal_M"));

      String cmd = F("Cal,");
      bool   triggerCalibrate = false;

      if (isFormItemChecked(F("Plugin_105_enable_cal_M"))) {
        cmd             += PCONFIG_FLOAT(1);
        triggerCalibrate = true;
      }

      if (triggerCalibrate) {
        char sensordata[32];
        _P105_send_I2C_command(PCONFIG(0), cmd, sensordata);
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
      char sensordata[32];
      bool status;

      // ok, now we can read the ORP value
      status = _P105_send_I2C_command(PCONFIG(0), F("R"), sensordata);

      // we read the voltagedata char statussensordata[32];
      char voltagedata[32];
      status = _P105_send_I2C_command(PCONFIG(0), F("Status"), voltagedata);

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
      // status = _P105_send_I2C_command(PCONFIG(0),F("Sleep"),sensordata);

      success = true;
      break;
    }
    case PLUGIN_WRITE:
    {
      // TODO : do something more usefull ...

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
          String log("Asking for calibration ");
          addLog(LOG_LEVEL_INFO, log);
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

bool _P105_send_I2C_command(uint8_t I2Caddress, const String& cmd, char *sensordata) {
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
    addLog(LOG_LEVEL_ERROR, F("Wire.endTransmission() returns error: Check ORP shield"));
    return false;
  }

  // don't read answer if we want to go to sleep
  if (cmd.substring(0, 5).equalsIgnoreCase(F("Sleep"))) {
    return true;
  }

  i2c_response_code = 254;

  while (i2c_response_code == 254) { // in case the cammand takes longer to process, we keep looping here until we get a success or an error
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

#endif // ifdef USES_P105
