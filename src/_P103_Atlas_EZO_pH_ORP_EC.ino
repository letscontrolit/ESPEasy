#include "_Plugin_Helper.h"

#ifdef USES_P103

// ########################################################################
// ################## Plugin 103 : Atlas Scientific EZO pH ORP EC sensors #
// ########################################################################

// datasheet at https://atlas-scientific.com/files/pH_EZO_Datasheet.pdf
// datasheet at https://atlas-scientific.com/files/ORP_EZO_Datasheet.pdf
// datasheet at https://atlas-scientific.com/files/EC_EZO_Datasheet.pdf
// works only in i2c mode

#include "src/Helpers/Rules_calculate.h"

#define PLUGIN_103
#define PLUGIN_ID_103 103
#define PLUGIN_NAME_103 "Environment - Atlas EZO pH ORP EC [TESTING]"
#define PLUGIN_VALUENAME1_103 "SensorData"
#define PLUGIN_VALUENAME2_103 "Voltage"
#define UNKNOWN 0
#define PH 1
#define ORP 2
#define EC 3

#define ATLAS_EZO_RETURN_ARRAY_SIZE 33

#define FIXED_TEMP_VALUE 20 // Temperature correction for pH and EC sensor if no temperature is given from calculation

boolean Plugin_103(byte function, struct EventStruct *event, String &string)
{
  boolean success = false;

  byte board_type = UNKNOWN;
  byte I2Cchoice;

  switch (function)
  {
  case PLUGIN_DEVICE_ADD:
  {
    Device[++deviceCount].Number = PLUGIN_ID_103;
    Device[deviceCount].Type = DEVICE_TYPE_I2C;
    Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_DUAL;
    Device[deviceCount].Ports = 0;
    Device[deviceCount].PullUpOption = false;
    Device[deviceCount].InverseLogicOption = false;
    Device[deviceCount].FormulaOption = true;
    Device[deviceCount].ValueCount = 2;
    Device[deviceCount].SendDataOption = true;
    Device[deviceCount].TimerOption = true;
    Device[deviceCount].GlobalSyncOption = true;
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
    I2Cchoice = PCONFIG(1);
#define _P103_ATLASEZO_I2C_NB_OPTIONS 6
    int optionValues[_P103_ATLASEZO_I2C_NB_OPTIONS] = {0x62, 0x63, 0x64, 0x65, 0x66, 0x67};
    addFormSelectorI2C(F("plugin_103_i2c"), _P103_ATLASEZO_I2C_NB_OPTIONS, optionValues, I2Cchoice);
    addFormNote(F("pH: 0x63, ORP: 0x62, EC: 0x64. The plugin is able to detect the type of device automatically."));

    addFormSubHeader(F("Board"));

    char boarddata[ATLAS_EZO_RETURN_ARRAY_SIZE] = {0};

    if (_P103_send_I2C_command(I2Cchoice, F("i"), boarddata))
    {
      String boardInfo(boarddata);
      addRowLabel(F("Board type"));

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

      if (board_type == UNKNOWN)
      {
        addHtml(F("<span style='color:red'>  WARNING : Board type should be 'pH' or 'ORP' or 'EC', check your i2c address? </span>"));
      }
      addRowLabel(F("Board version"));
      addHtml(version);

      addHtml(F("<input type='hidden' name='plugin_214_sensorVersion' value='"));
      addHtml(version);
      addHtml(F("'>"));
    }
    else
    {
      addHtml(F("<span style='color:red;'>Unable to send command to device</span>"));
      if (board_type == UNKNOWN)
      {
        addHtml(F("<span style='color:red'>  WARNING : Board type should be 'pH' or 'ORP' or 'EC', check your i2c address?</span>"));
      }
      success = false;
      break;
    }

    char statussensordata[ATLAS_EZO_RETURN_ARRAY_SIZE] = {0};

    if (_P103_send_I2C_command(I2Cchoice, F("Status"), statussensordata))
    {
      String boardStatus(statussensordata);

      addRowLabel(F("Board restart code"));

      addLog(LOG_LEVEL_DEBUG, boardStatus);

      char *statuschar = strchr(statussensordata, ',');

      if (statuschar > 0)
      {
        switch (statussensordata[statuschar - statussensordata + 1])
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
      addUnit(F("V"));

      addRowLabel(F("Sensor Data"));
      addHtml(String(UserVar[event->BaseVarIndex]));
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
      }
    }
    else
    {
      addHtml(F("<span style='color:red;'>Unable to send status command to device</span>"));
      success = false;
      break;
    }

    // Ability to turn status LED of board on or off
    addFormCheckBox(F("Status LED"), F("Plugin_103_status_led"), PCONFIG(2));

    // Ability to see and change EC Probe Type (e.g., 0.1, 1.0, 10)
    if (board_type == EC)
    {
      char ecprobetypedata[ATLAS_EZO_RETURN_ARRAY_SIZE] = {0};

      if (_P103_send_I2C_command(I2Cchoice, F("K,?"), ecprobetypedata))
      {
        String ecProbeType(ecprobetypedata);

        addFormTextBox(F("EC Probe Type"), F("Plugin_103_ec_probe_type"), ecProbeType.substring(ecProbeType.lastIndexOf(',') + 1), 32);
        addFormCheckBox(F("Set Probe Type"), F("Plugin_103_enable_set_probe_type"), false);
      }
    }

    // calibrate
    switch (board_type)
    {
    case PH:
    {
      addFormSubHeader(F("pH Calibration"));
      addFormNote(F("Calibration for pH-Probe could be 1 (single), 2 (single, low) or 3 point (single, low, high). The sequence is important."));
      int nb_calibration_points = addCreate3PointCalibration(board_type, event, I2Cchoice, F("pH"), 0.0, 14.0, 2, 0.01);
      if (nb_calibration_points > 1)
      {
        char slopedata[ATLAS_EZO_RETURN_ARRAY_SIZE] = {0};

        if (_P103_send_I2C_command(I2Cchoice, F("Slope,?"), slopedata))
        {
          String slopeAnswer = F("Answer to 'Slope' command : ");
          slopeAnswer += slopedata;
          addFormNote(slopeAnswer);
        }
      }
      break;
    }

    case ORP:
    {
      addFormSubHeader(F("ORP Calibration"));
      addCreateSinglePointCalibration(board_type, event, I2Cchoice, F("mV"), 0.0, 1500.0, 0, 1.0);
      break;
    }

    case EC:
    {
      addFormSubHeader(F("EC Calibration"));
      addCreateDryCalibration();
      addCreate3PointCalibration(board_type, event, I2Cchoice, F("&micro;S"), 0.0, 500000.0, 0, 1.0);
    }
    break;
    }

    // Clear calibration
    addClearCalibration();

    // Temperature compensation
    if (board_type == PH || board_type == EC)
    {
      double value;
      char strValue[6] = {0};

      addFormSubHeader(F("Temperature compensation"));
      char deviceTemperatureTemplate[40] = {0};
      LoadCustomTaskSettings(event->TaskIndex, (byte *)&deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));
      ZERO_TERMINATE(deviceTemperatureTemplate);
      addFormTextBox(F("Temperature "), F("Plugin_103_temperature_template"), deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));
      addFormNote(F("You can use a formulae and idealy refer to a temp sensor (directly, via ESPEasyP2P or MQTT import) ,e.g. '[Pool#Temperature]'. If you don't have a sensor, you could type a fixed value like '25' for '25.5'."));

      String deviceTemperatureTemplateString(deviceTemperatureTemplate);
      String pooltempString(parseTemplate(deviceTemperatureTemplateString, 40));

      if (Calculate(pooltempString.c_str(), value) != CalculateReturnCode::OK)
      {
        addFormNote(F("It seems I can't parse your formulae. Fixed value will be used!"));
        value = FIXED_TEMP_VALUE;
      }

      dtostrf(value, 5, 2, strValue);
      ZERO_TERMINATE(strValue);
      String actualValueStr(F("Actual value: "));
      actualValueStr += strValue;
      addFormNote(actualValueStr);
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

    char leddata[ATLAS_EZO_RETURN_ARRAY_SIZE] = {0};

    if (isFormItemChecked(F("Plugin_103_status_led")))
    {
      _P103_send_I2C_command(I2Cchoice, F("L,1"), leddata);
    }
    else
    {
      _P103_send_I2C_command(I2Cchoice, F("L,0"), leddata);
    }
    PCONFIG(2) = isFormItemChecked(F("Plugin_103_status_led"));

    if((board_type == EC) && isFormItemChecked(F("Plugin_103_enable_set_probe_type")))
    {
      addLog(LOG_LEVEL_DEBUG, F("isFormItemChecked"));
      String probeType(F("K,"));
      probeType += webArg(F("Plugin_103_ec_probe_type"));
      char setProbeTypeCmd[ATLAS_EZO_RETURN_ARRAY_SIZE] = {0};
      _P103_send_I2C_command(I2Cchoice, probeType, setProbeTypeCmd);
    }

    String cmd(F("Cal,"));
    bool triggerCalibrate = false;

    PCONFIG_FLOAT(1) = getFormItemFloat(F("Plugin_103_ref_cal_single"));
    PCONFIG_FLOAT(2) = getFormItemFloat(F("Plugin_103_ref_cal_L"));
    PCONFIG_FLOAT(3) = getFormItemFloat(F("Plugin_103_ref_cal_H"));

    if (isFormItemChecked(F("Plugin_103_enable_cal_clear")))
    {
      cmd += F("clear");
      triggerCalibrate = true;
    }
    else if (isFormItemChecked(F("Plugin_103_enable_cal_dry")))
    {
      cmd += F("dry");
      triggerCalibrate = true;
    }
    else if (isFormItemChecked(F("Plugin_103_enable_cal_single")))
    {
      if (board_type == PH)
      {
        cmd += F("mid,");
      }
      cmd += PCONFIG_FLOAT(1);
      triggerCalibrate = true;
    }
    else if (isFormItemChecked(F("Plugin_103_enable_cal_L")))
    {
      cmd += F("low,");
      cmd += PCONFIG_FLOAT(2);
      triggerCalibrate = true;
    }
    else if (isFormItemChecked(F("Plugin_103_enable_cal_H")))
    {
      cmd += F("high,");
      cmd += PCONFIG_FLOAT(3);
      triggerCalibrate = true;
    }

    if (triggerCalibrate)
    {
      char calibration[ATLAS_EZO_RETURN_ARRAY_SIZE] = {0};
      _P103_send_I2C_command(I2Cchoice, cmd, calibration);
    }

    if (board_type == PH || board_type == EC)
    {
      char deviceTemperatureTemplate[40] = {0};
      String tmpString = webArg(F("Plugin_103_temperature_template"));
      safe_strncpy(deviceTemperatureTemplate, tmpString.c_str(), sizeof(deviceTemperatureTemplate) - 1);
      ZERO_TERMINATE(deviceTemperatureTemplate); // be sure that our string ends with a \0

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
    I2Cchoice = PCONFIG(1);

    String readCommand;

    if (board_type == PH || board_type == EC)
    {
      // first set the temperature of reading
      char deviceTemperatureTemplate[40] = {0};
      LoadCustomTaskSettings(event->TaskIndex, (byte *)&deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));
      ZERO_TERMINATE(deviceTemperatureTemplate);

      String deviceTemperatureTemplateString(deviceTemperatureTemplate);
      String pooltempString(parseTemplate(deviceTemperatureTemplateString, 40));

      readCommand = F("RT,");
      double temperatureReading;

      if (Calculate(pooltempString.c_str(), temperatureReading) != CalculateReturnCode::OK)
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
    char sensordata[ATLAS_EZO_RETURN_ARRAY_SIZE] = {0};
    UserVar[event->BaseVarIndex] = -1;
    if (_P103_send_I2C_command(I2Cchoice, readCommand, sensordata))
    {
      String sensorString(sensordata);
      string2float(sensorString, UserVar[event->BaseVarIndex]);
    }

    // we read the voltagedata
    char voltagedata[ATLAS_EZO_RETURN_ARRAY_SIZE] = {0};
    UserVar[event->BaseVarIndex + 1] = -1;
    if (_P103_send_I2C_command(I2Cchoice, F("Status"), voltagedata))
    {
      String voltage(voltagedata);
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

bool _P103_send_I2C_command(uint8_t I2Caddress, const String &cmd, char *sensordata)
{
  sensordata[0] = '\0';

  uint16_t sensor_bytes_received = 0;

  byte error;
  byte i2c_response_code = 0;
  byte in_char = 0;

  String log = F("> cmd = ");
  log += cmd;
  addLog(LOG_LEVEL_DEBUG, log);

  addLog(LOG_LEVEL_DEBUG, String(cmd));
  Wire.beginTransmission(I2Caddress);
  Wire.write(cmd.c_str());
  error = Wire.endTransmission();

  if (error != 0)
  {
    // addLog(LOG_LEVEL_ERROR, error);
    addLog(LOG_LEVEL_ERROR, F("Wire.endTransmission() returns error: Check Atlas shield, pH, ORP and EC are supported."));
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
    Wire.requestFrom(I2Caddress, (uint8_t)(ATLAS_EZO_RETURN_ARRAY_SIZE - 1)); // call the circuit and request ATLAS_EZO_RETURN_ARRAY_SIZE - 1 = 32 bytes (this is more then we need).
    i2c_response_code = Wire.read();                                          // read response code

    while (Wire.available())
    { // read response
      in_char = Wire.read();

      if (in_char == 0)
      { // if we receive a null caracter, we're done
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
        sensordata[sensor_bytes_received] = in_char; // load this byte into our array.
        sensor_bytes_received++;
      }
    }
    sensordata[sensor_bytes_received] = '\0';

    if (loglevelActiveFor(LOG_LEVEL_DEBUG))
    {
      switch (i2c_response_code)
      {
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

int getCalibrationPoints(uint8_t i2cAddress)
{
  int nb_calibration_points = -1;
  char sensordata[ATLAS_EZO_RETURN_ARRAY_SIZE] = {0};

  if (_P103_send_I2C_command(i2cAddress, F("Cal,?"), sensordata))
  {
    if (strncmp(sensordata, "?Cal,", 5))
    {
      char tmp[2];
      tmp[0] = sensordata[5];
      tmp[1] = '\0',
      nb_calibration_points = str2int(tmp);
    }
  }

  return nb_calibration_points;
}

void addClearCalibration()
{
  addRowLabel(F("<strong>Clear calibration</strong>"));
  addFormCheckBox(F("Clear"), F("Plugin_103_enable_cal_clear"), false);
  addHtml(F("\n<script type='text/javascript'>document.getElementById(\"Plugin_103_enable_cal_clear\").onclick=function() {document.getElementById(\"Plugin_103_enable_cal_single\").checked = false;document.getElementById(\"Plugin_103_enable_cal_L\").checked = false;document.getElementById(\"Plugin_103_enable_cal_H\").checked = false;document.getElementById(\"Plugin_103_enable_cal_dry\").checked = false;};</script>"));
  addFormNote(F("Attention! This will reset all calibrated data. New calibration will be needed!!!"));
}

void addCreateDryCalibration()
{
  addRowLabel(F("<strong>Dry calibration</strong>"));
  addFormCheckBox(F("Enable"), F("Plugin_103_enable_cal_dry"), false);
  addHtml(F("\n<script type='text/javascript'>document.getElementById(\"Plugin_103_enable_cal_dry\").onclick=function() {document.getElementById(\"Plugin_103_enable_cal_single\").checked = false;document.getElementById(\"Plugin_103_enable_cal_L\").checked = false;document.getElementById(\"Plugin_103_enable_cal_H\").checked = false;document.getElementById(\"Plugin_103_enable_cal_clear\").checked = false;};</script>"));
  addFormNote(F("Dry calibration must always be done first!"));
  addFormNote(F("Calibration for pH-Probe could be 1 (single) or 2 point (low, high)."));
}

int addCreateSinglePointCalibration(byte board_type, struct EventStruct *event, byte I2Cchoice, String unit, float min, float max, byte nrDecimals, float stepsize)
{
  int nb_calibration_points = getCalibrationPoints(I2Cchoice);

  addRowLabel("Calibrated Points");
  addHtmlInt(nb_calibration_points);
  if (nb_calibration_points < 1)
  {
    addHtml(F("<span style='color:red'>   Calibration needed</span>"));
  }

  addRowLabel(F("<strong>Single point calibration</strong>"));
  addFormFloatNumberBox(F("Ref single point"), F("Plugin_103_ref_cal_single'"), PCONFIG_FLOAT(1), min, max, nrDecimals, stepsize);
  addUnit(unit);

  if ((board_type != EC && nb_calibration_points > 0) || (board_type == EC && nb_calibration_points == 1))
  {
    addHtml(F("&nbsp;<span style='color:green;'>OK</span>"));
  }
  else
  {
    if ((board_type == EC) && (nb_calibration_points > 1))
    {
      addHtml(F("&nbsp;<span style='color:green;'>Not calibrated, because two point calibration is active.</span>"));
    }
    else
    {
      addHtml(F("&nbsp;<span style='color:red;'>Not yet calibrated</span>"));
    }
  }
  addFormCheckBox(F("Enable"), F("Plugin_103_enable_cal_single"), false);
  addHtml(F("\n<script type='text/javascript'>document.getElementById(\"Plugin_103_enable_cal_single\").onclick=function() {document.getElementById(\"Plugin_103_enable_cal_clear\").checked = false;document.getElementById(\"Plugin_103_enable_cal_L\").checked = false;document.getElementById(\"Plugin_103_enable_cal_H\").checked = false;document.getElementById(\"Plugin_103_enable_cal_dry\").checked = false;};</script>"));

  return nb_calibration_points;
}

int addCreate3PointCalibration(byte board_type, struct EventStruct *event, byte I2Cchoice, String unit, float min, float max, byte nrDecimals, float stepsize)
{
  int nb_calibration_points = addCreateSinglePointCalibration(board_type, event, I2Cchoice, unit, min, max, nrDecimals, stepsize);

  addRowLabel(F("<strong>Low calibration</strong>"));
  addFormFloatNumberBox(F("Ref low point"), F("Plugin_103_ref_cal_L"), PCONFIG_FLOAT(2), min, max, nrDecimals, stepsize);
  addUnit(unit);

  if (nb_calibration_points > 1)
  {
    addHtml(F("&nbsp;<span style='color:green;'>OK</span>"));
  }
  else
  {
    addHtml(F("&nbsp;<span style='color:orange;'>Not yet calibrated</span>"));
  }
  addFormCheckBox(F("Enable"), F("Plugin_103_enable_cal_L"), false);
  addHtml(F("\n<script type='text/javascript'>document.getElementById(\"Plugin_103_enable_cal_L\").onclick=function() {document.getElementById(\"Plugin_103_enable_cal_clear\").checked = false;document.getElementById(\"Plugin_103_enable_cal_single\").checked = false;document.getElementById(\"Plugin_103_enable_cal_H\").checked = false;document.getElementById(\"Plugin_103_enable_cal_dry\").checked = false;};</script>"));

  addHtml(F("<TR><TD><strong>High calibration</strong></TD>"));
  addFormFloatNumberBox(F("Ref high point"), F("Plugin_103_ref_cal_H"), PCONFIG_FLOAT(3), min, max, nrDecimals, stepsize);
  addUnit(unit);

  // pH: low, high OK with 3 calibration points (single is the first one); EC: low high OK with 2 calibration points
  if (nb_calibration_points > 2 || (board_type == EC && nb_calibration_points > 1))
  {
    addHtml(F("&nbsp;<span style='color:green;'>OK</span>"));
  }
  else
  {
    addHtml(F("&nbsp;<span style='color:orange;'>Not yet calibrated</span>"));
  }
  addFormCheckBox(F("Enable"), F("Plugin_103_enable_cal_H"), false);
  addHtml(F("\n<script type='text/javascript'>document.getElementById(\"Plugin_103_enable_cal_H\").onclick=function() {document.getElementById(\"Plugin_103_enable_cal_single\").checked = false;document.getElementById(\"Plugin_103_enable_cal_L\").checked = false;document.getElementById(\"Plugin_103_enable_cal_clear\").checked = false;document.getElementById(\"Plugin_103_enable_cal_dry\").checked = false;};</script>"));

  return nb_calibration_points;
}

#endif // ifdef USES_P103
