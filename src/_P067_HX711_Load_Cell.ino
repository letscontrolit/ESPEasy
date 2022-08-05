#include "_Plugin_Helper.h"
#ifdef USES_P067
//#######################################################################################################
//#################################### Plugin 063: _P067_HX711_Load_Cell ################################
//#######################################################################################################

// ESPEasy Plugin to scan a 24 bit AD value from a load cell chip HX711
// written by Jochen Krapf (jk@nerd2nerd.org)
//
// Modified by chunter to support dual channel measurements.
// When both channels are enabled, sample-rate drops to approx. 1 sample/s for each channel.

// Electronics:
// Connect SCL to 1st GPIO and DOUT to 2nd GPIO. Use 3.3 volt for VCC.

// Datasheet: https://cdn.sparkfun.com/datasheets/Sensors/ForceFlex/hx711_english.pdf



#define PLUGIN_067
#define PLUGIN_ID_067           67
#define PLUGIN_NAME_067         "Weight - HX711 Load Cell"
#define PLUGIN_VALUENAME1_067   "WeightChanA"
#define PLUGIN_VALUENAME2_067   "WeightChanB"


#define BIT_POS_OS_CHAN_A         0
#define BIT_POS_OS_CHAN_B         1
#define BIT_POS_MODE_CHAN_A64     2
#define BIT_POS_MODE_CHAN_A128    3
#define BIT_POS_MODE_CHAN_B32     4
#define BIT_POS_CALIB_CHAN_A      5
#define BIT_POS_CALIB_CHAN_B      6

std::map<uint8_t, int32_t> Plugin_067_OversamplingValueChanA;
std::map<uint8_t, int16_t> Plugin_067_OversamplingCountChanA;
std::map<uint8_t, int32_t> Plugin_067_OversamplingValueChanB;
std::map<uint8_t, int16_t> Plugin_067_OversamplingCountChanB;

enum {modeAoff, modeA64, modeA128};
enum {modeBoff, modeB32};
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

int32_t readHX711(int16_t pinSCL, int16_t pinDOUT, int16_t config0, uint8_t *channelRead)
{
  static uint8_t channelToggle = 0;
  static uint8_t nextChannel = chanA128;
  int32_t value = 0;
  int32_t mask = 0x00800000;
  int8_t modeChanA = (config0 >> BIT_POS_MODE_CHAN_A64) & 0x03;
  int8_t modeChanB = (config0 >> BIT_POS_MODE_CHAN_B32) & 0x01;


  *channelRead = nextChannel;

  if ((modeChanA == modeAoff) && (modeChanB == modeBoff))
  {
    digitalWrite(pinSCL, HIGH);
    return 0;
  }

  if ((modeChanA != modeAoff) && (modeChanB != modeBoff))
  {
    // Both channels are activated -> do interleaved measurement
    channelToggle = 1 - channelToggle;
    if (channelToggle)
    {
      if (modeChanA == modeA64)
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
    if (modeChanA == modeA64)
      nextChannel = chanA64;
    if (modeChanA == modeA128)
      nextChannel = chanA128;
    if (modeChanB == modeB32)
      nextChannel = chanB32;
  }

  for (uint8_t i = 0; i < 24; i++)
  {
    digitalWrite(pinSCL, HIGH);
    delayMicroseconds(1);
    digitalWrite(pinSCL, LOW);
    if (digitalRead(pinDOUT))
      value |= mask;
    delayMicroseconds(1);
    mask >>= 1;
  }

  for (uint8_t i = 0; i < (nextChannel + 1); i++)
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

void float2int(float valFloat, int16_t *valInt0, int16_t *valInt1)
{
  // FIXME TD-er: Casting from float* to integer* is not portable due to different binary data representations on different platforms.
  int16_t *fti = (int16_t *)&valFloat;
  *valInt0 = *fti++;
  *valInt1 = *fti;
}

void int2float(int16_t valInt0, int16_t valInt1, float *valFloat)
{
  // FIXME TD-er: Casting from float* to integer* is not portable due to different binary data representations on different platforms.
  float offset = 0.0f; // Set to some value to prevent compiler warnings
  int16_t *itf = (int16_t *)&offset;
  *itf++ = valInt0;
  *itf = valInt1;
  *valFloat = offset;
}

boolean Plugin_067(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_067;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_DUAL;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = false;
        Device[deviceCount].GlobalSyncOption = true;
        Device[deviceCount].PluginStats        = true;
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
        event->String1 = formatGpioName_output(F("SCL"));
        event->String2 = formatGpioName_input(F("DOUT"));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        float valFloat;

        addFormSubHeader(F("Measurement Channel A"));

        addFormCheckBox(F("Oversampling"), F("oversamplingChanA"), PCONFIG(0) & (1 << BIT_POS_OS_CHAN_A));

        {
          const __FlashStringHelper * optionsModeChanA[3] = { F("off"), F("Gain 64"), F("Gain 128") };
          addFormSelector(F("Mode"), F("modeChanA"), 3, optionsModeChanA, nullptr, (PCONFIG(0) >> BIT_POS_MODE_CHAN_A64) & 0x03);
        }

        int2float(PCONFIG(1), PCONFIG(2), &valFloat);
        addFormTextBox(F("Offset"), F("p067_offset_chanA"), toString(valFloat, 3), 25);
        addHtml(F(" &nbsp; &nbsp; &#8617; Tare: "));
        addCheckBox(F("tareChanA"), 0);   //always off

        //------------
        addFormSubHeader(F("Measurement Channel B"));

        addFormCheckBox(F("Oversampling"), F("oversamplingChanB"), PCONFIG(0) & (1 << BIT_POS_OS_CHAN_B));

        {
          const __FlashStringHelper * optionsModeChanB[2] = { F("off"), F("Gain 32") };
          addFormSelector(F("Mode"), F("modeChanB"), 2, optionsModeChanB, nullptr, (PCONFIG(0) >> BIT_POS_MODE_CHAN_B32) & 0x01);
        }

        int2float(PCONFIG(3), PCONFIG(4), &valFloat);
        addFormTextBox(F("Offset"), F("p067_offset_chanB"), toString(valFloat, 3), 25);
        addHtml(F(" &nbsp; &nbsp; &#8617; Tare: "));
        addCheckBox(F("tareChanB"), 0);   //always off

        //------------
        addFormSubHeader(F("Two Point Calibration Channel A"));
        addFormCheckBox(F("Calibration Enabled"), F("p067_cal_chanA"), PCONFIG(0) & (1 << BIT_POS_CALIB_CHAN_A));

        addFormNumericBox(F("Point 1"), F("p067_adc1_chanA"), PCONFIG_LONG(0));
        html_add_estimate_symbol();
        addTextBox(F("p067_out1_chanA"), toString(PCONFIG_FLOAT(0), 3), 10);

        addFormNumericBox(F("Point 2"), F("p067_adc2_chanA"), PCONFIG_LONG(1));
        html_add_estimate_symbol();
        addTextBox(F("p067_out2_chanA"), toString(PCONFIG_FLOAT(1), 3), 10);

        //------------
        addFormSubHeader(F("Two Point Calibration Channel B"));

        addFormCheckBox(F("Calibration Enabled"), F("p067_cal_chanB"), PCONFIG(0) & (1 << BIT_POS_CALIB_CHAN_B));

        addFormNumericBox(F("Point 1"), F("p067_adc1_chanB"), PCONFIG_LONG(2));
        html_add_estimate_symbol();
        addTextBox(F("p067_out1_chanB"), toString(PCONFIG_FLOAT(2), 3), 10);

        addFormNumericBox(F("Point 2"), F("p067_adc2_chanB"), PCONFIG_LONG(3));
        html_add_estimate_symbol();
        addTextBox(F("p067_out2_chanB"), toString(PCONFIG_FLOAT(3), 3), 10);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        float valFloat;

        PCONFIG(0) = 0;
        if (isFormItemChecked(F("oversamplingChanA")))
          PCONFIG(0) |= (1 << BIT_POS_OS_CHAN_A);
        if (isFormItemChecked(F("oversamplingChanB")))
          PCONFIG(0) |= (1 << BIT_POS_OS_CHAN_B);
        if (getFormItemInt(F("modeChanA")) == modeA64)
          PCONFIG(0) |= (1 << BIT_POS_MODE_CHAN_A64);
        if (getFormItemInt(F("modeChanA")) == modeA128)
          PCONFIG(0) |= (1 << BIT_POS_MODE_CHAN_A128);
        if (getFormItemInt(F("modeChanB")) == modeB32)
          PCONFIG(0) |= (1 << BIT_POS_MODE_CHAN_B32);

        if (isFormItemChecked(F("p067_cal_chanA")))
          PCONFIG(0) |= (1 << BIT_POS_CALIB_CHAN_A);
        if (isFormItemChecked(F("p067_cal_chanB")))
          PCONFIG(0) |= (1 << BIT_POS_CALIB_CHAN_B);

        if (isFormItemChecked(F("tareChanA")))
        {
          valFloat = -UserVar[event->BaseVarIndex + 2];
          Plugin_067_OversamplingValueChanA[event->TaskIndex] = 0;
          Plugin_067_OversamplingCountChanA[event->TaskIndex] = 0;
        }
        else
        {
          valFloat = getFormItemFloat(F("p067_offset_chanA"));
        }
        float2int(valFloat, &PCONFIG(1), &PCONFIG(2));

        if (isFormItemChecked(F("tareChanB")))
        {
          valFloat = -UserVar[event->BaseVarIndex + 3];
          Plugin_067_OversamplingValueChanB[event->TaskIndex] = 0;
          Plugin_067_OversamplingCountChanB[event->TaskIndex] = 0;
        }
        else
        {
          valFloat = getFormItemFloat(F("p067_offset_chanB"));
        }
        float2int(valFloat, &PCONFIG(3), &PCONFIG(4));

        PCONFIG_LONG(0) = getFormItemInt(F("p067_adc1_chanA"));
        PCONFIG_FLOAT(0) = getFormItemFloat(F("p067_out1_chanA"));

        PCONFIG_LONG(1) = getFormItemInt(F("p067_adc2_chanA"));
        PCONFIG_FLOAT(1) = getFormItemFloat(F("p067_out2_chanA"));

        PCONFIG_LONG(2) = getFormItemInt(F("p067_adc1_chanB"));
        PCONFIG_FLOAT(2) = getFormItemFloat(F("p067_out1_chanB"));

        PCONFIG_LONG(3) = getFormItemInt(F("p067_adc2_chanB"));
        PCONFIG_FLOAT(3) = getFormItemFloat(F("p067_out2_chanB"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        int16_t pinSCL = CONFIG_PIN1;
        int16_t pinDOUT = CONFIG_PIN2;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("HX711: GPIO: SCL=");
          log += pinSCL;
          log += F(" DOUT=");
          log += pinDOUT;
          addLogMove(LOG_LEVEL_INFO, log);
        }

        if (pinSCL >= 0 && pinDOUT >= 0)
        {
          Plugin_067_OversamplingValueChanA[event->TaskIndex] = 0;
          Plugin_067_OversamplingCountChanA[event->TaskIndex] = 0;
          Plugin_067_OversamplingValueChanB[event->TaskIndex] = 0;
          Plugin_067_OversamplingCountChanB[event->TaskIndex] = 0;
          initHX711(pinSCL, pinDOUT);
        }

        success = true;
        break;
      }

    case PLUGIN_FIFTY_PER_SECOND:
      {
        int16_t pinSCL = CONFIG_PIN1;
        int16_t pinDOUT = CONFIG_PIN2;

        if (pinSCL >= 0 && pinDOUT >= 0)
        if (isReadyHX711(pinSCL, pinDOUT))
        {
          uint8_t channelRead;
          int32_t value = readHX711(pinSCL, pinDOUT, PCONFIG(0), &channelRead);

          switch (channelRead)
          {
            case chanA64:   //
            case chanA128:  if (PCONFIG(0) & (1 << BIT_POS_OS_CHAN_A))   //Oversampling on channel A?
                            {
                              if (Plugin_067_OversamplingCountChanA[event->TaskIndex] < 250)
                              {
                                Plugin_067_OversamplingValueChanA[event->TaskIndex] += value;
                                Plugin_067_OversamplingCountChanA[event->TaskIndex]++;
                              }
                            } else {
                              Plugin_067_OversamplingValueChanA[event->TaskIndex] = value;
                              Plugin_067_OversamplingCountChanA[event->TaskIndex] = 1;
                            }
                            break;
            case chanB32:   if (PCONFIG(0) & (1 << BIT_POS_OS_CHAN_B))   //Oversampling on channel B?
                            {
                              if (Plugin_067_OversamplingCountChanB[event->TaskIndex] < 250)
                              {
                                Plugin_067_OversamplingValueChanB[event->TaskIndex] += value;
                                Plugin_067_OversamplingCountChanB[event->TaskIndex]++;
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
        int8_t modeChanA = (PCONFIG(0) >> 2) & 0x03;
        int8_t modeChanB = (PCONFIG(0) >> 4) & 0x01;
        float valFloat;

        if ((modeChanA == modeAoff) && (modeChanB == modeBoff))
        {
          addLog(LOG_LEVEL_INFO, F("HX711: No channel selected"));
        }

        // Channel A activated?
        if (modeChanA != modeAoff)
        {
          String log = F("HX711: ChanA: ");

          if (Plugin_067_OversamplingCountChanA[event->TaskIndex] > 0)
          {
            UserVar[event->BaseVarIndex + 2] = static_cast<float>(Plugin_067_OversamplingValueChanA[event->TaskIndex]) / Plugin_067_OversamplingCountChanA[event->TaskIndex];

            Plugin_067_OversamplingValueChanA[event->TaskIndex] = 0;
            Plugin_067_OversamplingCountChanA[event->TaskIndex] = 0;

            int2float(PCONFIG(1), PCONFIG(2), &valFloat);
            UserVar[event->BaseVarIndex] = UserVar[event->BaseVarIndex + 2] + valFloat;   //Offset

            log += formatUserVarNoCheck(event->TaskIndex, 0);

            if (PCONFIG(0) & (1 << BIT_POS_CALIB_CHAN_A))  //Calibration channel A?
            {
              int adc1 = PCONFIG_LONG(0);
              int adc2 = PCONFIG_LONG(1);
              float out1 = PCONFIG_FLOAT(0);
              float out2 = PCONFIG_FLOAT(1);
              if (adc1 != adc2)
              {
                const float normalized = static_cast<float>(UserVar[event->BaseVarIndex] - adc1) / static_cast<float>(adc2 - adc1);
                UserVar[event->BaseVarIndex] = normalized * (out2 - out1) + out1;

                log += F(" = ");
                log += formatUserVarNoCheck(event->TaskIndex, 0);
              }
            }
          }
          else
          {
            log += F("NO NEW VALUE");
          }
          addLogMove(LOG_LEVEL_INFO, log);
        }

        // Channel B activated?
        if (modeChanB != modeBoff)
        {
          String log = F("HX711: ChanB: ");

          if (Plugin_067_OversamplingCountChanB[event->TaskIndex] > 0)
          {
            UserVar[event->BaseVarIndex + 3] = static_cast<float>(Plugin_067_OversamplingValueChanB[event->TaskIndex]) / Plugin_067_OversamplingCountChanB[event->TaskIndex];

            Plugin_067_OversamplingValueChanB[event->TaskIndex] = 0;
            Plugin_067_OversamplingCountChanB[event->TaskIndex] = 0;

            int2float(PCONFIG(3), PCONFIG(4), &valFloat);
            UserVar[event->BaseVarIndex + 1] = UserVar[event->BaseVarIndex + 3] + valFloat;   //Offset

            log += formatUserVarNoCheck(event->TaskIndex, 1);

            if (PCONFIG(0) & (1 << BIT_POS_CALIB_CHAN_B))  //Calibration channel B?
            {
              int adc1 = PCONFIG_LONG(2);
              int adc2 = PCONFIG_LONG(3);
              float out1 = PCONFIG_FLOAT(2);
              float out2 = PCONFIG_FLOAT(3);
              if (adc1 != adc2)
              {
                float normalized = (UserVar[event->BaseVarIndex + 1] - adc1) / static_cast<float>(adc2 - adc1);
                UserVar[event->BaseVarIndex + 1] = normalized * (out2 - out1) + out1;

                log += F(" = ");
                log += formatUserVarNoCheck(event->TaskIndex, 1);
              }
            }
          }
          else
          {
            log += F("NO NEW VALUE");
          }
          addLogMove(LOG_LEVEL_INFO, log);
        }

        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String command = parseString(string, 1);
        if (command.equalsIgnoreCase(F("tarechana")))
        {
          float2int(-UserVar[event->BaseVarIndex + 2], &PCONFIG(1), &PCONFIG(2));
          Plugin_067_OversamplingValueChanA[event->TaskIndex] = 0;
          Plugin_067_OversamplingCountChanA[event->TaskIndex] = 0;

          addLog(LOG_LEVEL_INFO, F("HX711: tare channel A"));
          success = true;
        }

        if (command.equalsIgnoreCase(F("tarechanb")))
        {
          float2int(-UserVar[event->BaseVarIndex + 3], &PCONFIG(3), &PCONFIG(4));
          Plugin_067_OversamplingValueChanB[event->TaskIndex] = 0;
          Plugin_067_OversamplingCountChanB[event->TaskIndex] = 0;

          addLog(LOG_LEVEL_INFO, F("HX711: tare channel B"));
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
