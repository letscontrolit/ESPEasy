//#######################################################################################################
//#################################### Plugin 063: _P067_HX711_Load_Cell ########################################
//#######################################################################################################

// ESPEasy Plugin to scan a 16 key touch pad chip TTP229
// written by Jochen Krapf (jk@nerd2nerd.org)

// Important: There are several types of TTP299 chips with different features available. They are namead all TTP229 but differ in the letter(s) followed.
// On the china boards (found on eBay and AliExpress) the TTP229-B is used which has NO! I2C-interface. It uses a properitary serial protocol with clock (SCL) and bidirectional data (SDO)

// ScanCode;
// Value 1...16 for the key number
// No key - the code 0
// If more than one key is pressed, the scan code is the code with the lowest value

// If ScanCode is unchecked the value is the KeyMap 1.Key=1, 2.Key=2, 3.Key=4, 4.Key=8 ... 16.Key=32768
// If more than one key is pressed, the value is sum of all KeyMap-values

// Electronics:
// Connect SCL to 1st GPIO and SDO to 2nd GPIO. Use 3.3 volt for VCC.
// Set the jumper for 16 key mode (TP2=jumper3). Additional set jumper for multi-key (TP3=jumper4, TP4=jumper5).
// Scematics: https://www.openimpulse.com/blog/wp-content/uploads/wpsc/downloadables/TTP229B-Schematic-Diagram.pdf
// Datasheet: http://www.datasheet4u.com/download_new.php?id=996751


#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_067
#define PLUGIN_ID_067         67
#define PLUGIN_NAME_067       "HX711 Load Cell [TESTING]"
#define PLUGIN_VALUENAME1_067 "Weight"

// #include <*.h>   no lib required


#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif
#ifndef PIN
#define PIN(n) (Settings.TaskDevicePin[n][event->TaskIndex])
#endif

int32_t Plugin_067_OversamplingValue = 0;
int16_t Plugin_067_OversamplingCount = 0;


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

int32_t readHX711(int16_t pinSCL, int16_t pinDOUT, uint8_t mode)
{
  int32_t value = 0;
  int32_t mask = 0x00800000;

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

  for (byte i = 0; i < (mode+1); i++)
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
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
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
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormCheckBox(string, F("Oversampling"), F("Plugin_067_oversampling"), CONFIG(0));

        String optionsMode[3] = { F("Channel A, Gain 128"), F("Channel B, Gain 32"), F("Channel A, Gain 64") };
        addFormSelector(string, F("Integration Time (Max Lux)"), F("mode"), 3, optionsMode, NULL, CONFIG(1));

        addFormSubHeader(string, F("Two Point Calibration"));

        addFormCheckBox(string, F("Calibration Enabled"), F("Plugin_067_cal"), Settings.TaskDevicePluginConfig[event->TaskIndex][3]);

        addFormNumericBox(string, F("Point 1"), F("Plugin_067_adc1"), Settings.TaskDevicePluginConfigLong[event->TaskIndex][0], 0, 1023);
        string += F(" &#8793; ");
        addTextBox(string, F("Plugin_067_out1"), String(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0], 3), 10);

        addFormNumericBox(string, F("Point 2"), F("Plugin_067_adc2"), Settings.TaskDevicePluginConfigLong[event->TaskIndex][1], 0, 1023);
        string += F(" &#8793; ");
        addTextBox(string, F("Plugin_067_out2"), String(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1], 3), 10);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        CONFIG(0) = isFormItemChecked(F("Plugin_067_oversampling"));

        CONFIG(1) = getFormItemInt(F("mode"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = isFormItemChecked(F("Plugin_067_cal"));

        Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] = getFormItemInt(F("Plugin_067_adc1"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0] = getFormItemFloat(F("Plugin_067_out1"));

        Settings.TaskDevicePluginConfigLong[event->TaskIndex][1] = getFormItemInt(F("Plugin_067_adc2"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = getFormItemFloat(F("Plugin_067_out2"));

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
          initHX711(pinSCL, pinDOUT);
        }

        success = true;
        break;
      }

    case PLUGIN_FIFTY_PER_SECOND:
      {
        int16_t pinSCL = PIN(0);
        int16_t pinDOUT = PIN(1);

        if (Plugin_067_OversamplingCount < 250)
        if (pinSCL >= 0 && pinDOUT >= 0)
        if (isReadyHX711(pinSCL, pinDOUT))
        {
          int32_t value = readHX711(pinSCL, pinDOUT, CONFIG(1));

          if (CONFIG(0))   //Oversampling?
          {
            Plugin_067_OversamplingValue += value;
            Plugin_067_OversamplingCount ++;
          }
          else   //use last value
          {
            Plugin_067_OversamplingValue = value;
            Plugin_067_OversamplingCount = 1;
          }
        }

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        String log = F("HX711: Value: ");

        if (Plugin_067_OversamplingCount > 0)
        {
          UserVar[event->BaseVarIndex] = (float)Plugin_067_OversamplingValue / Plugin_067_OversamplingCount;
          Plugin_067_OversamplingValue = 0;
          Plugin_067_OversamplingCount = 0;

          log += String(UserVar[event->BaseVarIndex], 3);

          if (Settings.TaskDevicePluginConfig[event->TaskIndex][3])   //Calibration?
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
        success = true;
        break;
      }

  }
  return success;
}

#endif
