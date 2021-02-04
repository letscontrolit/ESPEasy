#include "_Plugin_Helper.h"

#ifdef USES_P104

// ########################################################################
// ################## Plugin 104 : Atlas Scientific EZO EC sensor  ########
// ########################################################################

// datasheet at https://www.atlas-scientific.com/_files/_datasheets/_circuit/EC_EZO_datasheet.pdf
// works only in i2c mode

# define PLUGIN_104
# define PLUGIN_ID_104 104
# define PLUGIN_NAME_104       "Environment - Atlas Scientific EC EZO"
# define PLUGIN_VALUENAME1_104 "EC"

boolean Plugin_104(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_104;
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
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_104);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_104));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
        # define ATLASEZO_EC_I2C_NB_OPTIONS 3
      byte I2Cchoice                               = PCONFIG(0);
      int optionValues[ATLASEZO_EC_I2C_NB_OPTIONS] = { 0x64, 0x65, 0x66 };
      addFormSelectorI2C(F("Plugin_104_i2c"), ATLASEZO_EC_I2C_NB_OPTIONS, optionValues, I2Cchoice);

      addFormSubHeader(F("General"));

      char sensordata[32];
      bool status;
      status = _P104_send_I2C_command(PCONFIG(0), F("i"), sensordata);

      if (status) {
        String boardInfo(sensordata);

        addHtml(F("<TR><TD>Board type : </TD><TD>"));
        int pos1 = boardInfo.indexOf(',');
        int pos2 = boardInfo.lastIndexOf(',');
        addHtml(boardInfo.substring(pos1 + 1, pos2));

        if (boardInfo.substring(pos1 + 1, pos2) != F("EC")) {
          addHtml(F("<span style='color:red'>  WARNING : Board type should be 'EC', check your i2c Address ? </span>"));
        }
        addHtml(F("</TD></TR><TR><TD>Board version :</TD><TD>"));
        addHtml(boardInfo.substring(pos2 + 1));
        addHtml(F("</TD></TR>\n"));

        addHtml(F("<input type='hidden' name='Plugin_104_sensorVersion' value='"));
        addHtml(boardInfo.substring(pos2 + 1));
        addHtml(F("'>"));

        status    = _P104_send_I2C_command(PCONFIG(0), F("K,?"), sensordata);
        boardInfo = sensordata;
        addHtml(F("<TR><TD>Sensor type : </TD><TD>K="));
        pos2 = boardInfo.lastIndexOf(',');
        addHtml(boardInfo.substring(pos2 + 1));
        addHtml(F("</TD></TR>\n"));
      } else {
        addHtml(F("<span style='color:red;'>Unable to send command to device</span>"));
        success = false;
        break;
      }

      addFormCheckBox(F("Status LED"), F("Plugin_104_status_led"), PCONFIG(1));

      addFormSubHeader(F("Calibration"));

      addRowLabel(F("<strong>Dry calibration</strong>"));
      addFormCheckBox(F("Enable"), F("Plugin_104_enable_cal_dry"), false);
      addHtml(F(
                "\n<script type='text/javascript'>document.getElementById(\"Plugin_104_enable_cal_dry\").onclick=function() {document.getElementById(\"Plugin_104_enable_cal_single\").checked = false;document.getElementById(\"Plugin_104_enable_cal_L\").checked = false;;document.getElementById(\"Plugin_104_enable_cal_H\").checked = false;};</script>"));
      addFormNote(F("Dry calibration must always be done first!"));

      addRowLabel(F("<strong>Single point calibration</strong> "));
      addFormCheckBox(F("Enable"), F("Plugin_104_enable_cal_single"), false);
      addHtml(F(
                "\n<script type='text/javascript'>document.getElementById(\"Plugin_104_enable_cal_single\").onclick=function() {document.getElementById(\"Plugin_104_enable_cal_dry\").checked = false;document.getElementById(\"Plugin_104_enable_cal_L\").checked = false;;document.getElementById(\"Plugin_104_enable_cal_H\").checked = false;};</script>"));
      addFormNumericBox(F("Ref EC"), F("Plugin_104_ref_cal_single"), PCONFIG(2));
      addUnit(F("&micro;S"));

      addRowLabel(F("<strong>Low calibration</strong>"));
      addFormCheckBox(F("enable"),
                      F(
                        "Plugin_104_enable_cal_L' onClick='document.getElementById(\"Plugin_104_enable_cal_dry\").checked = false;document.getElementById(\"Plugin_104_enable_cal_single\").checked = false;;document.getElementById(\"Plugin_104_enable_cal_H\").checked = false;"),
                      false);
      addHtml(F(
                "\n<script type='text/javascript'>document.getElementById(\"Plugin_104_enable_cal_L\").onclick=function() {document.getElementById(\"Plugin_104_enable_cal_dry\").checked = false;document.getElementById(\"Plugin_104_enable_cal_single\").checked = false;;document.getElementById(\"Plugin_104_enable_cal_H\").checked = false;};</script>"));
      addFormNumericBox(F("Ref EC"), F("Plugin_104_ref_cal_L"), PCONFIG(3));
      addUnit(F("&micro;S"));

      addRowLabel(F("<strong>High calibration</strong>"));
      addFormCheckBox(F("Enable"), F("Plugin_104_enable_cal_H"), false);
      addHtml(F(
                "\n<script type='text/javascript'>document.getElementById(\"Plugin_104_enable_cal_H\").onclick=function() {document.getElementById(\"Plugin_104_enable_cal_dry\").checked = false;document.getElementById(\"Plugin_104_enable_cal_single\").checked = false;;document.getElementById(\"Plugin_104_enable_cal_L\").checked = false;};</script>"));
      addFormNumericBox(F("Ref EC"), F("Plugin_104_ref_cal_H"), PCONFIG(4));
      addUnit(F("&micro;S"));

      status = _P104_send_I2C_command(PCONFIG(0), F("Cal,?"), sensordata);

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

      addFormSubHeader(F("Temperature compensation"));
      char deviceTemperatureTemplate[40];
      LoadCustomTaskSettings(event->TaskIndex, (byte *)&deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));
      addFormTextBox(F("Temperature "), F("Plugin_104_temperature_template"), deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));
      addFormNote(F("You can use a formulae (and idealy refer to a temp sensor). "));
      double value;
      char strValue[5];
      addHtml(F("<div class='note'>"));

      if (Calculate(deviceTemperatureTemplate, value) == CalculateReturnCode::OK) {
        addHtml(F("Actual value : "));
        dtostrf(value, 5, 2, strValue);
        addHtml(strValue);
      } else {
        addHtml(F("(It seems I can't parse your formulae)"));
      }
      addHtml(F("</div>"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("Plugin_104_i2c"));

      PCONFIG_FLOAT(0) = getFormItemFloat(F("Plugin_104_sensorVersion"));

      char sensordata[32];

      if (isFormItemChecked(F("Plugin_104_status_led"))) {
        _P104_send_I2C_command(PCONFIG(0), F("L,1"), sensordata);
      } else {
        _P104_send_I2C_command(PCONFIG(0), F("L,0"), sensordata);
      }
      PCONFIG(1) = isFormItemChecked(F("Plugin_104_status_led"));

      PCONFIG(2) = getFormItemInt(F("Plugin_104_ref_cal_single"));
      PCONFIG(3) = getFormItemInt(F("Plugin_104_ref_cal_L"));
      PCONFIG(4) = getFormItemInt(F("Plugin_104_ref_cal_H"));

      String cmd              = F("Cal,");
      bool   triggerCalibrate = false;

      if (isFormItemChecked(F("Plugin_104_enable_cal_dry"))) {
        cmd             += F("dry");
        triggerCalibrate = true;
      } else if (isFormItemChecked(F("Plugin_104_enable_cal_single"))) {
        cmd             += PCONFIG(2);
        triggerCalibrate = true;
      } else if (isFormItemChecked(F("Plugin_104_enable_cal_L"))) {
        cmd             += F("low,");
        cmd             += PCONFIG(3);
        triggerCalibrate = true;
      } else if (isFormItemChecked(F("Plugin_104_enable_cal_H"))) {
        cmd             += F("high,");
        cmd             += PCONFIG(4);
        triggerCalibrate = true;
      }

      if (triggerCalibrate) {
        char sensordata[32];
        _P104_send_I2C_command(PCONFIG(0), cmd, sensordata);
      }

      char   deviceTemperatureTemplate[40];
      String tmpString = web_server.arg(F("Plugin_104_temperature_template"));
      strncpy(deviceTemperatureTemplate, tmpString.c_str(), sizeof(deviceTemperatureTemplate) - 1);
      deviceTemperatureTemplate[sizeof(deviceTemperatureTemplate) - 1] = 0; // be sure that our string ends with a \0

      SaveCustomTaskSettings(event->TaskIndex, (byte *)&deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));

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

      String setTemperature = F("T,");
      double temperatureReading;

      if (Calculate(deviceTemperatureTemplate, temperatureReading) == CalculateReturnCode::OK) {
        setTemperature += temperatureReading;
      } else {
        success = false;
        break;
      }

      status = _P104_send_I2C_command(PCONFIG(0), setTemperature, sensordata);

      // ok, now we can read the EC value
      status = _P104_send_I2C_command(PCONFIG(0), F("r"), sensordata);

      if (status) {
        String sensorString(sensordata);
        UserVar[event->BaseVarIndex] = sensorString.toFloat();
      }
      else {
        UserVar[event->BaseVarIndex] = -1;
      }

      // go to sleep
      // status = _P104_send_I2C_command(PCONFIG(0),F("Sleep"),sensordata);

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

bool _P104_send_I2C_command(uint8_t I2Caddress, const String& cmd, char *sensordata) {
  uint16_t sensor_bytes_received = 0;

  byte error;
  byte i2c_response_code = 0;
  byte in_char           = 0;

  Serial.println(cmd);
  Wire.beginTransmission(I2Caddress);
  Wire.write(cmd.c_str());
  error = Wire.endTransmission();

  if (error != 0) {
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
      delay(600);
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

#endif // ifdef USES_P104
