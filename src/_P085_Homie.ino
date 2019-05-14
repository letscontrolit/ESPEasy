#ifdef USES_P085
//#######################################################################################################
//################################## Plugin 085: Homie receiver##########################################
//#######################################################################################################

#define PLUGIN_085
#define PLUGIN_ID_085         85
#define PLUGIN_NAME_085       "Generic - Homie receiver"

// empty default names because settings will be ignored / not used if value name is empty
#define PLUGIN_VALUENAME1_085 ""
#define PLUGIN_VALUENAME2_085 ""
#define PLUGIN_VALUENAME3_085 ""
#define PLUGIN_VALUENAME4_085 ""

#define PLUGIN_085_VALUE_INTEGER    0
#define PLUGIN_085_VALUE_FLOAT      1
#define PLUGIN_085_VALUE_BOOLEAN    2
#define PLUGIN_085_VALUE_STRING     3
#define PLUGIN_085_VALUE_ENUM       4
#define PLUGIN_085_VALUE_RGB        5
#define PLUGIN_085_VALUE_HSV        6

#define PLUGIN_085_VALUE_TYPES      7
#define PLUGIN_085_VALUE_MAX        4

#define PLUGIN_085_DEBUG            true

boolean Plugin_085(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_085;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].VType = SENSOR_TYPE_NONE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].DecimalsOnly = true;
        Device[deviceCount].ValueCount = PLUGIN_085_VALUE_MAX;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        Device[deviceCount].Custom = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_085);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_085));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_085));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_085));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_085));

        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormNote(F("Translation Plugin for controllers able to recieve value updates according to Homie convention."));

        byte choice = 0;
        String labelText = "";
        String keyName = "";
        String options[PLUGIN_085_VALUE_TYPES];
        options[0] = F("integer");
        options[1] = F("float");
        options[2] = F("boolean");
        options[3] = F("string");
        options[4] = F("enum");
        options[5] = F("rgb");
        options[6] = F("hsv");
        int optionValues[PLUGIN_085_VALUE_TYPES];
        optionValues[0] = PLUGIN_085_VALUE_INTEGER;
        optionValues[1] = PLUGIN_085_VALUE_FLOAT;
        optionValues[2] = PLUGIN_085_VALUE_BOOLEAN;
        optionValues[3] = PLUGIN_085_VALUE_STRING;
        optionValues[4] = PLUGIN_085_VALUE_ENUM;
        optionValues[5] = PLUGIN_085_VALUE_RGB;
        optionValues[6] = PLUGIN_085_VALUE_HSV;
        for (int i=0;i<PLUGIN_085_VALUE_MAX;i++) {
          labelText = F("Function #");
          labelText += (i+1);
          addFormSubHeader(labelText);
          choice = PCONFIG(i);
          if (i==0) addFormNote(F("Triggers an event when a ../%event%/set topic arrives"));
          labelText = F("Event Name");
          keyName = F("p085_functionName");
          keyName += i;
          addFormTextBox(labelText, keyName, ExtraTaskSettings.TaskDeviceValueNames[i], NAME_FORMULA_LENGTH_MAX);
          labelText = F("Parameter Type");
          keyName = F("p085_valueType");
          keyName += i;
          addFormSelector(labelText, keyName, PLUGIN_085_VALUE_TYPES, options, optionValues, choice );
          keyName += F("_min");
          addFormNumericBox(F("Min"),keyName,ExtraTaskSettings.TaskDevicePluginConfig[i]);
          keyName = F("p085_valueType");
          keyName += i;
          keyName += F("_max");
          addFormNumericBox(F("Max"),keyName,ExtraTaskSettings.TaskDevicePluginConfig[i+PLUGIN_085_VALUE_MAX]);
          if (i==0) addFormNote(F("min max values only valid for numeric parameter"));
          keyName = F("p085_decimals");
          keyName += i;
          addFormNumericBox(F("Decimals"),keyName,ExtraTaskSettings.TaskDeviceValueDecimals[i],0,8);
          if (i==0) addFormNote(F("Decimal counts for float parameter"));
          keyName = F("p085_string");
          keyName += i;
          addFormTextBox(F("String or enum"), keyName, ExtraTaskSettings.TaskDeviceFormula[i], NAME_FORMULA_LENGTH_MAX);
          if (i==0) addFormNote(F("String content. Enum expects comma seperated list"));
        }
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String keyName = "";
        for (int i=0;i<PLUGIN_085_VALUE_MAX;i++) {
          keyName = F("p085_valueType");
          keyName += i;
          PCONFIG(i) = getFormItemInt(keyName);
          keyName = F("p085_functionName");
          keyName += i;
          strncpy_webserver_arg(ExtraTaskSettings.TaskDeviceValueNames[i], keyName);
          keyName = F("p085_valueType");
          keyName += i;
          keyName += F("_min");
          ExtraTaskSettings.TaskDevicePluginConfig[i]=getFormItemInt(keyName);
          keyName = F("p085_valueType");
          keyName += i;
          keyName += F("_max");
          ExtraTaskSettings.TaskDevicePluginConfig[i+PLUGIN_085_VALUE_MAX]=getFormItemInt(keyName);
          keyName = F("p085_decimals");
          keyName += i;
          ExtraTaskSettings.TaskDeviceValueDecimals[i]=getFormItemInt(keyName);
          keyName = F("p085_string");
          keyName += i;
          strncpy_webserver_arg(ExtraTaskSettings.TaskDeviceFormula[i], keyName);
        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        for (byte x=0; x<PLUGIN_085_VALUE_MAX;x++)
        {
          String log = F("P085 : Value ");
          log += x+1;
          log += F(": ");
          log += UserVar[event->BaseVarIndex+x];
          addLog(LOG_LEVEL_INFO,log);
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String command = parseString(string, 1);
        if (command == F("homievalueset"))
        {
          if (event->Par1 == event->TaskIndex+1) {// make sure that this instance is the target
            LoadTaskSettings(event->Par1 -1);
            String parameter = parseStringToEndKeepCase(string,4);
            String log = "";
/*            if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
              log = F("P085 : Acknowledge :");
              log += string;
              log += F(" / ");
              log += ExtraTaskSettings.TaskDeviceName;
              log += F(" / ");
              log += ExtraTaskSettings.TaskDeviceValueNames[event->Par2-1];
              log += F(" sensorType:");
              log += event->sensorType;
              log += F(" Source:");
              log += event->Source;
              log += F(" idx:");
              log += event->idx;
              log += F(" S1:");
              log += event->String1;
              log += F(" S2:");
              log += event->String2;
              log += F(" S3:");
              log += event->String3;
              log += F(" S4:");
              log += event->String4;
              log += F(" S5:");
              log += event->String5;
              log += F(" P1:");
              log += event->Par1;
              log += F(" P2:");
              log += event->Par2;
              log += F(" P3:");
              log += event->Par3;
              log += F(" P4:");
              log += event->Par4;
              log += F(" P5:");
              log += event->Par5;
              addLog(LOG_LEVEL_DEBUG, log);
            } */
            float floatValue = 0;
            String enumList = "";
            int i = 0;
            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              log = F("P085 : deviceNr:");
              log += event->Par1;
              log += F(" valueNr:");
              log += event->Par2;
              log += F(" valueType:");
              log += Settings.TaskDevicePluginConfig[event->Par1-1][event->Par2-1];
            }

            switch (Settings.TaskDevicePluginConfig[event->Par1-1][event->Par2-1]) {
              case PLUGIN_085_VALUE_INTEGER:
              case PLUGIN_085_VALUE_FLOAT:
                if (parameter!="") {
                  if (string2float(parameter,floatValue)) {
                    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                      log += F(" integer/float set to ");
                      log += floatValue;
                      addLog(LOG_LEVEL_INFO,log);
                    }
                    UserVar[event->BaseVarIndex+event->Par2-1]=floatValue;
                  } else { // float conversion failed!
                    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
                      log += F(" parameter:");
                      log += parameter;
                      log += F(" not a float value!");
                      addLog(LOG_LEVEL_ERROR,log);
                    }
                  }
                } else {
                  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                    log += F(" value:");
                    log += UserVar[event->BaseVarIndex+event->Par2-1];
                    addLog(LOG_LEVEL_INFO,log);
                  }
                }
                break;

              case PLUGIN_085_VALUE_BOOLEAN:
                if (parameter=="false") {
                  floatValue = 0;
                } else {
                  floatValue = 1;
                }
                UserVar[event->BaseVarIndex+event->Par2-1]=floatValue;
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log += F(" boolean set to ");
                  log += floatValue;
                  addLog(LOG_LEVEL_INFO,log);
                }
                break;

              case PLUGIN_085_VALUE_STRING:
                //String values not stored to conserve flash memory
                //safe_strncpy(ExtraTaskSettings.TaskDeviceFormula[event->Par2-1], parameter.c_str(), sizeof(ExtraTaskSettings.TaskDeviceFormula[event->Par2-1]));
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log += F(" string set to ");
                  log += parameter;
                  addLog(LOG_LEVEL_INFO,log);
                }
                break;

              case PLUGIN_085_VALUE_ENUM:
                enumList = ExtraTaskSettings.TaskDeviceFormula[event->Par2-1];
                i = 1;
                while (parseString(enumList,i)!="") { // lookup result in enum List
                  if (parseString(enumList,i)==parameter) {
                    floatValue = i;
                    break;
                  }
                  i++;
                }
                UserVar[event->BaseVarIndex+event->Par2-1]=floatValue;
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log += F(" enum set to ");
                  log += floatValue;
                  log += F(" (");
                  log += parameter;
                  log += F(")");
                  addLog(LOG_LEVEL_INFO,log);
                }
                break;

              case PLUGIN_085_VALUE_RGB:
                //String values not stored to conserve flash memory
                //safe_strncpy(ExtraTaskSettings.TaskDeviceFormula[event->Par2-1], parameter.c_str(), sizeof(ExtraTaskSettings.TaskDeviceFormula[event->Par2-1]));
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log += F(" RGB received ");
                  log += parameter;
                  addLog(LOG_LEVEL_INFO,log);
                }
                break;

              case PLUGIN_085_VALUE_HSV:
                //String values not stored to conserve flash memory
                //safe_strncpy(ExtraTaskSettings.TaskDeviceFormula[event->Par2-1], parameter.c_str(), sizeof(ExtraTaskSettings.TaskDeviceFormula[event->Par2-1]));
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log += F(" HSV received ");
                  log += parameter;
                  addLog(LOG_LEVEL_INFO,log);
                }
                break;
            }
            success = true;
          }
        }
        break;
      }
  }
  return success;
}

// acurate color converter (should be perhaps moved to a more prominent place to be used by others too)
// uses H 0..360 S 1..100 I/V 1..100 (according to homie convention)
// Source https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white

void HSV2RGB(float H, float S, float I, int rgb[3]) {
  int r, g, b;
  H = fmod(H,360); // cycle H around to 0-360 degrees
  H = 3.14159*H/(float)180; // Convert to radians.
  S = S / 100;
  S = S>0?(S<1?S:1):0; // clamp S and I to interval [0,1]
  I = I / 100;
  I = I>0?(I<1?I:1):0;

  // Math! Thanks in part to Kyle Miller.
  if(H < 2.09439) {
    r = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
    g = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
    b = 255*I/3*(1-S);
  } else if(H < 4.188787) {
    H = H - 2.09439;
    g = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
    b = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
    r = 255*I/3*(1-S);
  } else {
    H = H - 4.188787;
    b = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
    r = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
    g = 255*I/3*(1-S);
  }
  rgb[0]=r;
  rgb[1]=g;
  rgb[2]=b;
}

// uses H 0..360 S 1..100 I/V 1..100 (according to homie convention)
// Source https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white

void HSV2RGBW(float H, float S, float I, int rgbw[4]) {
  int r, g, b, w;
  float cos_h, cos_1047_h;
  H = fmod(H,360); // cycle H around to 0-360 degrees
  H = 3.14159*H/(float)180; // Convert to radians.
  S = S / 100;
  S = S>0?(S<1?S:1):0; // clamp S and I to interval [0,1]
  I = I / 100;
  I = I>0?(I<1?I:1):0;

  if(H < 2.09439) {
    cos_h = cos(H);
    cos_1047_h = cos(1.047196667-H);
    r = S*255*I/3*(1+cos_h/cos_1047_h);
    g = S*255*I/3*(1+(1-cos_h/cos_1047_h));
    b = 0;
    w = 255*(1-S)*I;
  } else if(H < 4.188787) {
    H = H - 2.09439;
    cos_h = cos(H);
    cos_1047_h = cos(1.047196667-H);
    g = S*255*I/3*(1+cos_h/cos_1047_h);
    b = S*255*I/3*(1+(1-cos_h/cos_1047_h));
    r = 0;
    w = 255*(1-S)*I;
  } else {
    H = H - 4.188787;
    cos_h = cos(H);
    cos_1047_h = cos(1.047196667-H);
    b = S*255*I/3*(1+cos_h/cos_1047_h);
    r = S*255*I/3*(1+(1-cos_h/cos_1047_h));
    g = 0;
    w = 255*(1-S)*I;
  }

  rgbw[0]=r;
  rgbw[1]=g;
  rgbw[2]=b;
  rgbw[3]=w;
}


#endif // USES_P085
