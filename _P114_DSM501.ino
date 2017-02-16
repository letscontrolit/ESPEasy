#ifdef PLUGIN_BUILD_DEV

//#######################################################################################################
//#################################### Plugin 114: DMS501A #############################################
//#################################### by serpa #############################################
//#######################################################################################################

#define PLUGIN_114
#define PLUGIN_ID_114 114
#define PLUGIN_NAME_114 "Dust sensor - DSM501a [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_114 "PM1.0" // from the datasheet the detection is from PM1 and up. You could have from PM1 to PM2.5, on subtracting PM2.5 value on PM1 value. This value come from the pin #4
#define PLUGIN_VALUENAME2_114 "PM2.5" // from the datasheet the detection is from PM2.5 and up. This value come from the pin #2. With different resistor topn the pin #1, you could adjust the size threshold detection

unsigned long Plugin_114_pulseCounter[TASKS_MAX];
unsigned long Plugin_114_pulseTotalCounter[TASKS_MAX];
unsigned long Plugin_114_pulseTime[TASKS_MAX];
unsigned long Plugin_114_pulseTimePrevious[TASKS_MAX];
unsigned long tstart1, tstart2;
unsigned long tduration = 30000; // duration of measurement in ms
//unsigned long triggerOn; // start of pulse time in us
//unsigned long triggerOff; // end of pulse time in us
//unsigned long lowpulseoccupancy; // duration of pulse in us
volatile unsigned long thigh1, thigh2;
volatile unsigned long tlow1, tlow2;
volatile unsigned long startlow1, startlow2;
volatile unsigned long starthigh1, starthigh2;
volatile boolean done1, done2;
volatile boolean value1, value2;
//boolean trigger = false;
volatile float ratio1, ratio2;
volatile int DMSpin1, DMSpin2;
float dens1, dens2;
boolean Plugin_114(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_114;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = SENSOR_TYPE_TEMP_HUM;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_114);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_114));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_114));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        char tmpString[128];
        sprintf_P(tmpString, PSTR("<TR><TD>Averaging Time (mSec):<TD><input type='text' name='plugin_114' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        string += tmpString;
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_114");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        // tduration= Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        success = true;
        break;
      }


    case PLUGIN_INIT:
      {
        String log = F("INIT : DSM501A ");
        log += Settings.TaskDevicePin1[event->TaskIndex];
        addLog(LOG_LEVEL_INFO, log);
        // tduration= Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        tstart1 = millis();
        startlow1 = micros();
        starthigh1 = startlow1;
        DMSpin1 = Settings.TaskDevicePin1[event->TaskIndex];
        pinMode(DMSpin1, INPUT);
        attachInterrupt(digitalPinToInterrupt(DMSpin1), Plugin_114_ISR1, CHANGE);
        tstart2 = millis();
        startlow2 = micros();
        starthigh2 = startlow2;
        DMSpin2 = Settings.TaskDevicePin2[event->TaskIndex];
        pinMode(DMSpin2, INPUT);
        attachInterrupt(digitalPinToInterrupt(DMSpin2), Plugin_114_ISR2, CHANGE);
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        if (done1 && done2) {
          done1 = FALSE;
          dens1 = ratio1 * 110; // ug/m^3
          done2 = FALSE;
          dens2 = ratio2 * 110; // ug/m^3

          String log = F("DSM501A: PM1.0=");
          log += dens1;
          log += F(" PM2.5=");
          log += dens2;
          addLog(LOG_LEVEL_INFO, log);
          UserVar[event->BaseVarIndex] = (float) dens1;
          UserVar[event->BaseVarIndex + 1] = (float) dens2;
        }
        success = true;
        break;
      }
  }
  return success;
}


/*********************************************************************************************\
  Check Pulse (called from irq handler)
  \*********************************************************************************************/
void Plugin_114_ISR1()
{
  value1 = digitalRead(DMSpin1); //read input pin just changed
  if (value1 == 0) { // gone low
    startlow1 = micros(); // record starting of low period
    thigh1 += startlow1 - starthigh1; // record duration of past high state
  } else { // gone high
    starthigh1 = micros(); // record starting of high period
    tlow1 += starthigh1 - startlow1; // record duration of past low state
  }
  if (millis() > tstart1 + tduration) { // check if average time has past
    tstart1 = millis(); // reset time period
    ratio1 = float(tlow1) / float(thigh1 + tlow1) * 100; // compute ratio low to total
    tlow1 = 0; // reset low time counter
    thigh1 = 0; // reset high time counter
    done1 = TRUE; // set reading complete flag
  }
}
/*********************************************************************************************\
  Check Pulse (called from irq handler)
  \*********************************************************************************************/
void Plugin_114_ISR2()
{
  value2 = digitalRead(DMSpin2); //read input pin just changed
  if (value2 == 0) { // gone low
    startlow2 = micros(); // record starting of low period
    thigh2 += startlow2 - starthigh2; // record duration of past high state
  } else { // gone high
    starthigh2 = micros(); // record starting of high period
    tlow2 += starthigh2 - startlow2; // record duration of past low state
  }
  if (millis() > tstart2 + tduration) { // check if average time has past
    tstart2 = millis(); // reset time period
    ratio2 = float(tlow2) / float(thigh2 + tlow2) * 100; // compute ratio low to high
    tlow2 = 0; // reset low time counter
    thigh2 = 0; // reset high time counter
    done2 = TRUE; // set reading complete flag
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
