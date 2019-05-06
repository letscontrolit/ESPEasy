//########################################################################
//################## Plugin 214 : Atlas Scientific EZO Ph sensor  ########
//########################################################################

// datasheet at https://www.atlas-scientific.com/_files/_datasheets/_circuit/pH_EZO_datasheet.pdf
// works only in i2c mode

#define PLUGIN_214
#define PLUGIN_ID_214 214
#define PLUGIN_NAME_214       "Environment - Atlas Scientific pH EZO [TESTING]"
#define PLUGIN_VALUENAME1_214 "pH"
#define PLUGIN_VALUENAME2_214 "Voltage"

boolean Plugin_214_init = false;

boolean Plugin_214(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_214;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
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
        string = F(PLUGIN_NAME_214);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_214));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_214));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        #define _P214_ATLASEZO_I2C_NB_OPTIONS 4
        byte I2Cchoice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        int optionValues[_P214_ATLASEZO_I2C_NB_OPTIONS] = { 0x63, 0x64, 0x65, 0x66 };
        addFormSelectorI2C(F("plugin_214_i2c"), _P214_ATLASEZO_I2C_NB_OPTIONS, optionValues, I2Cchoice);

        addFormSubHeader(F("General"));

        char sensordata[32];
        bool info;
        info = _P214_send_I2C_command(Settings.TaskDevicePluginConfig[event->TaskIndex][0],"i",sensordata);

        if (info) {
          String boardInfo(sensordata);

          addHtml(F("<TR><TD>Board type : </TD><TD>"));
          int pos1 = boardInfo.indexOf(',');
          int pos2 = boardInfo.lastIndexOf(',');
          addHtml(boardInfo.substring(pos1+1,pos2));
          if (boardInfo.substring(pos1+1,pos2) != "pH"){
            addHtml(F("<span style='color:red'>  WARNING : Board type should be 'pH', check your i2c Address ? </span>"));
          }
          addHtml(F("</TD></TR><TR><TD>Board version :</TD><TD>"));
          addHtml(boardInfo.substring(pos2+1));
          addHtml(F("</TD></TR>"));

          addHtml(F("<input type='hidden' name='plugin_214_sensorVersion' value='"));
          addHtml(boardInfo.substring(pos2+1));
          addHtml(F("'>"));

        } else {
          addHtml(F("<span style='color:red;'>Unable to send command to device</span>"));
          success = false;
          break;
        }

        addFormCheckBox(F("Status LED"),F("Plugin_214_status_led"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);

        char statussensordata[32];
        bool status;
        status = _P214_send_I2C_command(Settings.TaskDevicePluginConfig[event->TaskIndex][0],"Status",statussensordata);

        if (status) {
          String boardStatus(statussensordata);

          addHtml(F("<TR><TD>Board restart code: </TD><TD>"));
          int pos1 = boardStatus.indexOf(',');
          int pos2 = boardStatus.lastIndexOf(',');
          switch ((char)boardStatus.substring(pos1+1,pos2)[0])
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
          addHtml(boardStatus.substring(pos2+1));
          addHtml(F(" V</TD></TR>"));

          addHtml(F("<input type='hidden' name='plugin_214_sensorVoltage' value='"));
          addHtml(boardStatus.substring(pos2+1));
          addHtml(F("'>"));

        } else {
          addHtml(F("<span style='color:red;'>Unable to send status command to device</span>"));
          success = false;
          break;
        }

        addFormSubHeader(F("Calibration"));

        int nb_calibration_points = -1;
        status = _P214_send_I2C_command(Settings.TaskDevicePluginConfig[event->TaskIndex][0], "Cal,?",sensordata);

        if (status){
          if (strncmp(sensordata,"?Cal,",5)){
            char tmp[2];
            tmp[0] = sensordata[5];
            tmp[1] = '\0',
            nb_calibration_points = atoi(tmp);
          }
        }

        addRowLabel(F("<strong>Middle</strong>"));
        addFormNumericBox(F("Ref Ph"),F("Plugin_214_ref_cal_M' step='0.01"),Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1],1,14);
        if (nb_calibration_points > 0) {
          addHtml(F("&nbsp;<span style='color:green;'>OK</span>"));
        } else {
          addHtml(F("&nbsp;<span style='color:red;'>Not yet calibrated</span>"));
        }
        addFormCheckBox(F("Enable"),F("Plugin_214_enable_cal_M"), false);
        addHtml(F("\n<script type='text/javascript'>document.getElementById(\"Plugin_214_enable_cal_M\").onclick = function(){document.getElementById(\"Plugin_214_enable_cal_L\").checked = false;document.getElementById(\"Plugin_214_enable_cal_H\").checked = false;};</script>\n"));

        addRowLabel(F("<strong>Low</strong>"));
        addFormNumericBox(F("Ref Ph"),F("Plugin_214_ref_cal_L' step='0.01"), Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2],1,14);
        if (nb_calibration_points > 1) {
          addHtml(F("&nbsp;<span style='color:green;'>OK</span>"));
        } else {
          addHtml(F("&nbsp;<span style='color:red;'>Not yet calibrated</span>"));
        }
        addFormCheckBox(F("Enable"),F("Plugin_214_enable_cal_L"), false);
        addHtml(F("\n<script type='text/javascript'>document.getElementById(\"Plugin_214_enable_cal_L\").onclick = function(){document.getElementById(\"Plugin_214_enable_cal_M\").checked = false;document.getElementById(\"Plugin_214_enable_cal_H\").checked = false;};</script>\n"));

        addHtml(F("<TR><TD><strong>High</strong></TD>"));
        addFormNumericBox(F("Ref Ph"),F("Plugin_214_ref_cal_H' step='0.01"), Settings.TaskDevicePluginConfigFloat[event->TaskIndex][3],1,14);
        if (nb_calibration_points > 2) {
          addHtml(F("&nbsp;<span style='color:green;'>OK</span>"));
        } else {
          addHtml(F("&nbsp;<span style='color:orange;'>Not yet calibrated</span>"));
        }
        addFormCheckBox(F("Enable"),F("Plugin_214_enable_cal_H"), false);
        addHtml(F("\n<script type='text/javascript'>document.getElementById(\"Plugin_214_enable_cal_H\").onclick = function(){document.getElementById(\"Plugin_214_enable_cal_L\").checked = false;document.getElementById(\"Plugin_214_enable_cal_M\").checked = false;};</script>\n"));

        if (nb_calibration_points > 1){
          char sensordata[32];
          char cmd[8] = "Slope,?";
          bool status;
          status = _P214_send_I2C_command(Settings.TaskDevicePluginConfig[event->TaskIndex][0],cmd,sensordata);

          if (status){
            String slopeAnswer("Answer to 'Slope' command : ");
            slopeAnswer += sensordata;
            addFormNote(slopeAnswer);
          }
        }

        addFormSubHeader(F("Temperature compensation"));
        char deviceTemperatureTemplate[40];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));
        addFormTextBox(F("Temperature "), F("Plugin_214_temperature_template"), deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));
        addFormNote(F("You can use a formulae (and idealy refer to a temp sensor). "));
        float value;
        char strValue[5];
        String deviceTemperatureTemplateString(deviceTemperatureTemplate);
        String pooltempString(parseTemplate(deviceTemperatureTemplateString, 40));
        addHtml(F("<div class='note'>"));
        if (Calculate(pooltempString.c_str(),&value) == CALCULATE_OK ){
          addHtml(F("Actual value : "));
          dtostrf(value,5,2,strValue);
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
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_214_i2c"));

        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0] = getFormItemFloat(F("plugin_214_sensorVersion"));

        char sensordata[32];
        if (isFormItemChecked(F("Plugin_214_status_led"))) {
          _P214_send_I2C_command(Settings.TaskDevicePluginConfig[event->TaskIndex][0],"L,1",sensordata);
        } else {
          _P214_send_I2C_command(Settings.TaskDevicePluginConfig[event->TaskIndex][0],"L,0",sensordata);
        }
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = isFormItemChecked(F("Plugin_214_status_led"));


        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = getFormItemFloat(F("Plugin_214_ref_cal_M"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] = getFormItemFloat(F("Plugin_214_ref_cal_L"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][3] = getFormItemFloat(F("Plugin_214_ref_cal_H"));

        String cmd ("Cal,");
        bool triggerCalibrate = false;
        if (isFormItemChecked("Plugin_214_enable_cal_M")) {
          cmd += "mid,";
          cmd += Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1];
          triggerCalibrate = true;
        } else if (isFormItemChecked("Plugin_214_enable_cal_L")){
          cmd += "low,";
          cmd += Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2];
          triggerCalibrate = true;
        } else if (isFormItemChecked("Plugin_214_enable_cal_H")){
          cmd += "high,";
          cmd += Settings.TaskDevicePluginConfigFloat[event->TaskIndex][3];
          triggerCalibrate = true;
        }
        if (triggerCalibrate){
          char sensordata[32];
          _P214_send_I2C_command(Settings.TaskDevicePluginConfig[event->TaskIndex][0],cmd.c_str(),sensordata);
        }

        char deviceTemperatureTemplate[40];
        String tmpString = WebServer.arg(F("Plugin_214_temperature_template"));
        strncpy(deviceTemperatureTemplate, tmpString.c_str(), sizeof(deviceTemperatureTemplate)-1);
        deviceTemperatureTemplate[sizeof(deviceTemperatureTemplate)-1]=0; //be sure that our string ends with a \0

        SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));

        Plugin_214_init = false;
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_214_init = true;
      }

    case PLUGIN_READ:
      {
        char sensordata[32];
        bool status;

        //first set the temperature of reading
        char deviceTemperatureTemplate[40];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemperatureTemplate, sizeof(deviceTemperatureTemplate));

        String deviceTemperatureTemplateString(deviceTemperatureTemplate);
        String pooltempString(parseTemplate(deviceTemperatureTemplateString, 40));
        //String setTemperature("T,");
        String setTemperature("RT,");
        float temperatureReading;
        if (Calculate(pooltempString.c_str(),&temperatureReading) == CALCULATE_OK ){
          setTemperature += temperatureReading;
        } else {
          success = false;
          break;
        }

        //ok, now we can read the pH value with Temperature compensation
        status = _P214_send_I2C_command(Settings.TaskDevicePluginConfig[event->TaskIndex][0],setTemperature.c_str(),sensordata);

        //ok, now we can read the pH value
        //status = _P214_send_I2C_command(Settings.TaskDevicePluginConfig[event->TaskIndex][0],"r",sensordata);

        //we read the voltagedata char statussensordata[32];
        char voltagedata[32];
        status = _P214_send_I2C_command(Settings.TaskDevicePluginConfig[event->TaskIndex][0],"Status",voltagedata);


        if (status){
          String sensorString(sensordata);
          String voltage(voltagedata);
          int pos = voltage.lastIndexOf(',');
          UserVar[event->BaseVarIndex] = sensorString.toFloat();
          UserVar[event->BaseVarIndex + 1] = voltage.substring(pos+1).toFloat();
        }
        else {
          UserVar[event->BaseVarIndex] = -1;
          UserVar[event->BaseVarIndex + 1] = -1;
        }

        //go to sleep
        //status = _P214_send_I2C_command(Settings.TaskDevicePluginConfig[event->TaskIndex][0],"Sleep",sensordata);

        success = true;
        break;
      }
      case PLUGIN_WRITE:
        {
          //TODO : do something more usefull ...

          String tmpString  = string;
          int argIndex = tmpString.indexOf(',');
          if (argIndex)
            tmpString = tmpString.substring(0, argIndex);
          if (tmpString.equalsIgnoreCase(F("ATLASCMD")))
          {
            success = true;
            argIndex = string.lastIndexOf(',');
            tmpString = string.substring(argIndex + 1);
            if (tmpString.equalsIgnoreCase(F("CalMid"))){
              String log("Asking for Mid calibration ");
              addLog(LOG_LEVEL_INFO, log);
            }
            else if (tmpString.equalsIgnoreCase(F("CalLow"))){
              String log("Asking for Low calibration ");
              addLog(LOG_LEVEL_INFO, log);
            }
            else if (tmpString.equalsIgnoreCase(F("CalHigh"))){
              String log("Asking for High calibration ");
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

bool _P214_send_I2C_command(uint8_t I2Caddress,const char * cmd, char* sensordata) {
    uint16_t sensor_bytes_received = 0;

    byte error;
    byte i2c_response_code = 0;
    byte in_char = 0;

    Serial.println(cmd);
    Wire.beginTransmission(I2Caddress);
    Wire.write(cmd);
    error = Wire.endTransmission();

    if (error != 0) {
      return false;
    }

    //don't read answer if we want to go to sleep
    if (strncmp(cmd,"Sleep",5) == 0) {
      return true;
    }

    i2c_response_code = 254;
    while (i2c_response_code == 254) {      // in case the cammand takes longer to process, we keep looping here until we get a success or an error

      if  (
             (  (cmd[0] == 'r' || cmd[0] == 'R') && cmd[1] == '\0'  )
             ||
             (  ( strncmp(cmd,"cal",3) || strncmp(cmd,"Cal",3) ) && !strncmp(cmd,"Cal,?",5) )
           )
      {
        delay(900);
      }
      else {
        delay(300);
      }

      Wire.requestFrom(I2Caddress, (uint8_t) 32);    //call the circuit and request 32 bytes (this is more then we need).
      i2c_response_code = Wire.read();      //read response code

      while (Wire.available()) {            //read response
        in_char = Wire.read();

        if (in_char == 0) {                 //if we receive a null caracter, we're done
          while (Wire.available()) {  //purge the data line if needed
            Wire.read();
          }

          break;                            //exit the while loop.
        }
        else {
          sensordata[sensor_bytes_received] = in_char;        //load this byte into our array.
          sensor_bytes_received++;
        }
      }
      sensordata[sensor_bytes_received] = '\0';

      switch (i2c_response_code) {
        case 1:
          Serial.print( F("< success, answer = "));
          Serial.println(sensordata);
          break;

        case 2:
          Serial.println( F("< command failed"));
          return false;

        case 254:
          Serial.println( F("< command pending"));
          break;

        case 255:
          Serial.println( F("< no data"));
          return false;
      }
    }

    Serial.println(sensordata);
    return true;
}
