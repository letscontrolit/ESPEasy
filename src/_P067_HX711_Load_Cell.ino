#ifdef USES_P067
//#######################################################################################################
//#################################### Plugin 063: _P067_HX711_Load_Cell ################################
//#######################################################################################################

// ESPEasy Plugin to scan a 24 bit AD value from a load cell chip HX711
// written by Jochen Krapf (jk@nerd2nerd.org)
// Modified by chunter to support dual channel measurements

// Electronics:
// Connect SCL to 1st GPIO and DOUT to 2nd GPIO. Use 3.3 volt for VCC.

// Datasheet: https://cdn.sparkfun.com/datasheets/Sensors/ForceFlex/hx711_english.pdf



#define PLUGIN_067
#define PLUGIN_ID_067           67
#define PLUGIN_NAME_067         "Weight - HX711 Load Cell [TESTING]"
#define PLUGIN_VALUENAME1_067   "WeightChanA"
#define PLUGIN_VALUENAME2_067   "WeightChanB"

// #include <*.h>   no lib required


#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif
#ifndef PIN
#define PIN(n) (Settings.TaskDevicePin[n][event->TaskIndex])
#endif

std::map<byte, int32_t> Plugin_067_OversamplingValueChanA;
std::map<byte, int16_t> Plugin_067_OversamplingCountChanA;
std::map<byte, int32_t> Plugin_067_OversamplingValueChanB;
std::map<byte, int16_t> Plugin_067_OversamplingCountChanB;

enum {modeOff, mode32, mode64, mode128};
enum {chanA128, chanB32, chanA64};

void initHX711(int16_t pinSCL, int16_t pinDOUT)
{
  digitalWrite(pinSCL, LOW);
  pinMode(pinSCL, OUTPUT);

  pinMode(pinDOUT, INPUT_PULLUP);
}

boolean isReadyHX711(int16_t pinSCL, int16_t pinDOUT)
{
  return (!digitalRead(pinDOUT));
}

int32_t readHX711(int16_t pinSCL, int16_t pinDOUT, uint8_t modeChanA, uint8_t modeChanB, uint8_t *channelRead)
{
  static uint8_t channelToggle = 0;
  static uint8_t nextChannel = chanA128;
  int32_t value = 0;
  int32_t mask = 0x00800000;

  *channelRead = nextChannel;

  if ((modeChanA == modeOff) && (modeChanB == modeOff))
  {
    // Both channels are deactivated -> continue taking measurements
    nextChannel = chanA128;
  }
  else 
  {
    if ((modeChanA != modeOff) && (modeChanB != modeOff))
    {
      // Both channels are activated -> do interleaved measurement
      channelToggle = 1 - channelToggle;
      if (channelToggle)
      {
        if (modeChanA == mode64)
          nextChannel = chanA64;
        else
          nextChannel = chanA128;
      } else {
        nextChannel = chanB32;
      }
    }
    else
    {
      // Only one channel is activated
      if (modeChanA == mode64)
        nextChannel = chanA64;
      if (modeChanA == mode128)
        nextChannel = chanA128;
      if (modeChanB == mode32)
        nextChannel = chanB32;
    }
  }

  for (byte i = 0; i < 24; i++)
  {
    digitalWrite(pinSCL, HIGH);
    delayMicroseconds(1);
    digitalWrite(pinSCL, LOW);
    if (digitalRead(pinDOUT))
      value |= mask;
    delayMicroseconds(1);
    mask >>= 1;
  }
  
  for (byte i = 0; i < (nextChannel + 1); i++)
  {
    digitalWrite(pinSCL, HIGH);
    delayMicroseconds(1);
    digitalWrite(pinSCL, LOW);
    delayMicroseconds(1);
  }

  if (value & 0x00800000)   //negative?
    value |= 0xFF000000;  //expand sign bit to 32 bit

  return value;
}


boolean Plugin_067(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_067;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_DUAL;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = false;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_067);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_067));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_067));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output("SCL");
        event->String2 = formatGpioName_input("DOUT");
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormSubHeader(F("Measurement Channel A"));

        addFormCheckBox(F("Oversampling"), F("oversamplingChanA"), CONFIG(0));

        String optionsModeChanA[3] = { F("off"), F("Gain 128"), F("Gain 64") };
        addFormSelector(F("Mode"), F("modeChanA"), 3, optionsModeChanA, NULL, CONFIG(1));
        
        addFormTextBox(F("Offset"), F("p067_offset_chanA"), String(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][4], 3), 25);
        addHtml(F(" &nbsp; &nbsp; &#8617; Tare: "));
        addCheckBox(F("tareChanA"), 0);   //always off

        //------------
        addFormSubHeader(F("Measurement Channel B"));

        addFormCheckBox(F("Oversampling"), F("oversamplingChanB"), CONFIG(2));

        String optionsModeChanB[2] = { F("off"), F("Gain 32") };
        addFormSelector(F("Mode"), F("modeChanB"), 2, optionsModeChanB, NULL, CONFIG(3));
        
        addFormTextBox(F("Offset"), F("p067_offset_chanB"), String(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][5], 3), 25);
        addHtml(F(" &nbsp; &nbsp; &#8617; Tare: "));
        addCheckBox(F("tareChanB"), 0);   //always off

        //------------
        addFormSubHeader(F("Two Point Calibration Channel A"));

        addFormCheckBox(F("Calibration Enabled"), F("p067_cal_chanA"), Settings.TaskDevicePluginConfig[event->TaskIndex][4]);

        addFormNumericBox(F("Point 1"), F("p067_adc1_chanA"), Settings.TaskDevicePluginConfigLong[event->TaskIndex][0]);
        html_add_estimate_symbol();
        addTextBox(F("p067_out1_chanA"), String(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0], 3), 10);

        addFormNumericBox(F("Point 2"), F("p067_adc2_chanA"), Settings.TaskDevicePluginConfigLong[event->TaskIndex][1]);
        html_add_estimate_symbol();
        addTextBox(F("p067_out2_chanA"), String(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1], 3), 10);

        //------------
        addFormSubHeader(F("Two Point Calibration Channel B"));

        addFormCheckBox(F("Calibration Enabled"), F("p067_cal_chanB"), Settings.TaskDevicePluginConfig[event->TaskIndex][5]);

        addFormNumericBox(F("Point 1"), F("p067_adc1_chanB"), Settings.TaskDevicePluginConfigLong[event->TaskIndex][2]);
        html_add_estimate_symbol();
        addTextBox(F("p067_out1_chanB"), String(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2], 3), 10);

        addFormNumericBox(F("Point 2"), F("p067_adc2_chanB"), Settings.TaskDevicePluginConfigLong[event->TaskIndex][3]);
        html_add_estimate_symbol();
        addTextBox(F("p067_out2_chanB"), String(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][3], 3), 10);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        CONFIG(0) = isFormItemChecked(F("oversamplingChanA"));
        CONFIG(1) = getFormItemInt(F("modeChanA"));
        CONFIG(2) = isFormItemChecked(F("oversamplingChanB"));
        CONFIG(3) = getFormItemInt(F("modeChanB"));

        if (isFormItemChecked(F("tareChanA")))
        {
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][4] = -UserVar[event->BaseVarIndex];
          Plugin_067_OversamplingValueChanA[event->TaskIndex] = 0;
          Plugin_067_OversamplingCountChanA[event->TaskIndex] = 0;
        }
        else
        {
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][4] = getFormItemFloat(F("p067_offset_chanA"));
        }

        if (isFormItemChecked(F("tareChanB")))
        {
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][5] = -UserVar[event->BaseVarIndex + 1];
          Plugin_067_OversamplingValueChanB[event->TaskIndex] = 0;
          Plugin_067_OversamplingCountChanB[event->TaskIndex] = 0;
        }
        else
        {
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][5] = getFormItemFloat(F("p067_offset_chanB"));
        }

        Settings.TaskDevicePluginConfig[event->TaskIndex][4] = isFormItemChecked(F("p067_cal_chanA"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][5] = isFormItemChecked(F("p067_cal_chanB"));

        Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] = getFormItemInt(F("p067_adc1_chanA"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0] = getFormItemFloat(F("p067_out1_chanA"));

        Settings.TaskDevicePluginConfigLong[event->TaskIndex][1] = getFormItemInt(F("p067_adc2_chanA"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = getFormItemFloat(F("p067_out2_chanA"));

        Settings.TaskDevicePluginConfigLong[event->TaskIndex][2] = getFormItemInt(F("p067_adc1_chanB"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] = getFormItemFloat(F("p067_out1_chanB"));

        Settings.TaskDevicePluginConfigLong[event->TaskIndex][3] = getFormItemInt(F("p067_adc2_chanB"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][3] = getFormItemFloat(F("p067_out2_chanB"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        int16_t pinSCL = PIN(0);
        int16_t pinDOUT = PIN(1);

        String log = F("HX711: GPIO: SCL=");
        log += pinSCL;
        log += F(" DOUT=");
        log += pinDOUT;
        addLog(LOG_LEVEL_INFO, log);

        if (pinSCL >= 0 && pinDOUT >= 0)
        {
          Plugin_067_OversamplingValueChanA[event->TaskIndex]=0;
          Plugin_067_OversamplingCountChanA[event->TaskIndex]=0;
          Plugin_067_OversamplingValueChanB[event->TaskIndex]=0;
          Plugin_067_OversamplingCountChanB[event->TaskIndex]=0;
          initHX711(pinSCL, pinDOUT);
        }

        success = true;
        break;
      }

    case PLUGIN_FIFTY_PER_SECOND:
      {
        int16_t pinSCL = PIN(0);
        int16_t pinDOUT = PIN(1);

        if (pinSCL >= 0 && pinDOUT >= 0)
        if (isReadyHX711(pinSCL, pinDOUT))
        {
          uint8_t channelRead;
          int32_t value = readHX711(pinSCL, pinDOUT, CONFIG(1), CONFIG(3), &channelRead);

          switch (channelRead) 
          {
            case chanA64:   //
            case chanA128:  if (CONFIG(0))   //Oversampling on channel A?
                            {
                              if (Plugin_067_OversamplingCountChanA[event->TaskIndex] < 250)
                              {
                                Plugin_067_OversamplingValueChanA[event->TaskIndex] += value;
                                Plugin_067_OversamplingCountChanA[event->TaskIndex] ++;
                              }
                            } else {
                              Plugin_067_OversamplingValueChanA[event->TaskIndex] = value;
                              Plugin_067_OversamplingCountChanA[event->TaskIndex] = 1;
                            }
                            break;
            case chanB32:   if (CONFIG(2))   //Oversampling on channel B?
                            {
                              if (Plugin_067_OversamplingCountChanB[event->TaskIndex] < 250)
                              {
                                Plugin_067_OversamplingValueChanB[event->TaskIndex] += value;
                                Plugin_067_OversamplingCountChanB[event->TaskIndex] ++;
                              }
                            } else {
                              Plugin_067_OversamplingValueChanB[event->TaskIndex] = value;
                              Plugin_067_OversamplingCountChanB[event->TaskIndex] = 1;
                            }
                            break;
          }
        }

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        String log;

        if ((CONFIG(1) == modeOff) && (CONFIG(3) == modeOff))
        {
          log = F("HX711: No channel selected");
          addLog(LOG_LEVEL_INFO,log);
        }
          
        // Channel A activated?
        if (CONFIG(1) != modeOff)
        {
          log = F("HX711: ChanA: ");
          
          if (Plugin_067_OversamplingCountChanA[event->TaskIndex] > 0)
          {
            UserVar[event->BaseVarIndex + 2] = (float)Plugin_067_OversamplingValueChanA[event->TaskIndex] / Plugin_067_OversamplingCountChanA[event->TaskIndex];
            
            Plugin_067_OversamplingValueChanA[event->TaskIndex] = 0;
            Plugin_067_OversamplingCountChanA[event->TaskIndex] = 0;
            
            UserVar[event->BaseVarIndex] = UserVar[event->BaseVarIndex + 2] + Settings.TaskDevicePluginConfigFloat[event->TaskIndex][4];   //Offset

            log += String(UserVar[event->BaseVarIndex], 3);
            
            if (Settings.TaskDevicePluginConfig[event->TaskIndex][4])   //Calibration channel A?
            {
              int adc1 = Settings.TaskDevicePluginConfigLong[event->TaskIndex][0];
              int adc2 = Settings.TaskDevicePluginConfigLong[event->TaskIndex][1];
              float out1 = Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0];
              float out2 = Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1];
              if (adc1 != adc2)
              {
                float normalized = (float)(UserVar[event->BaseVarIndex] - adc1) / (float)(adc2 - adc1);
                UserVar[event->BaseVarIndex] = normalized * (out2 - out1) + out1;
  
                log += F(" = ");
                log += String(UserVar[event->BaseVarIndex], 3);
              }
            }
          }
          else 
          {
            log += F("NO NEW VALUE");
          }
          addLog(LOG_LEVEL_INFO,log);
        }

        // Channel B activated?
        if (CONFIG(3) != modeOff)
        {
          log = F("HX711: ChanB: ");
          
          if (Plugin_067_OversamplingCountChanB[event->TaskIndex] > 0)
          {
            UserVar[event->BaseVarIndex + 3] = (float)Plugin_067_OversamplingValueChanB[event->TaskIndex] / Plugin_067_OversamplingCountChanB[event->TaskIndex];
            
            Plugin_067_OversamplingValueChanB[event->TaskIndex] = 0;
            Plugin_067_OversamplingCountChanB[event->TaskIndex] = 0;
  
            UserVar[event->BaseVarIndex + 1] = UserVar[event->BaseVarIndex + 3] + Settings.TaskDevicePluginConfigFloat[event->TaskIndex][5];   //Offset
  
            log += String(UserVar[event->BaseVarIndex + 1], 3);

            if (Settings.TaskDevicePluginConfig[event->TaskIndex][5])   //Calibration channel B?
            {
              int adc1 = Settings.TaskDevicePluginConfigLong[event->TaskIndex][2];
              int adc2 = Settings.TaskDevicePluginConfigLong[event->TaskIndex][3];
              float out1 = Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2];
              float out2 = Settings.TaskDevicePluginConfigFloat[event->TaskIndex][3];
              if (adc1 != adc2)
              {
                float normalized = (float)(UserVar[event->BaseVarIndex + 1] - adc1) / (float)(adc2 - adc1);
                UserVar[event->BaseVarIndex + 1] = normalized * (out2 - out1) + out1;
  
                log += F(" = ");
                log += String(UserVar[event->BaseVarIndex + 1], 3);
              }
            }
          }
          else 
          {
            log += F("NO NEW VALUE");
          }
          addLog(LOG_LEVEL_INFO,log);
        }

        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String command = parseString(string, 1);
        if (command == F("tareChanA"))
        {
          String log = F("HX711: tare channel A");

          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][4] = -UserVar[event->BaseVarIndex + 2];
          Plugin_067_OversamplingValueChanA[event->TaskIndex] = 0;
          Plugin_067_OversamplingCountChanA[event->TaskIndex] = 0;
          
          addLog(LOG_LEVEL_INFO, log);
          success = true;
        }

        if (command == F("tareChanB"))
        {
          String log = F("HX711: tare channel B");

          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][5] = -UserVar[event->BaseVarIndex + 3];
          Plugin_067_OversamplingValueChanB[event->TaskIndex] = 0;
          Plugin_067_OversamplingCountChanB[event->TaskIndex] = 0;

          addLog(LOG_LEVEL_INFO, log);
          success = true;
        }
        break;
      }

    case PLUGIN_EXIT:
      {
        Plugin_067_OversamplingValueChanA.erase(event->TaskIndex);
        Plugin_067_OversamplingCountChanA.erase(event->TaskIndex);
        Plugin_067_OversamplingValueChanB.erase(event->TaskIndex);
        Plugin_067_OversamplingCountChanB.erase(event->TaskIndex);
        break;
      }

  }
  return success;
}

#endif // USES_P067
