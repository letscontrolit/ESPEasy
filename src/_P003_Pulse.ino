#ifdef USES_P003
//#######################################################################################################
//#################################### Plugin 003: Pulse  ###############################################
//#######################################################################################################

#define PLUGIN_003
#define PLUGIN_ID_003         3
#define PLUGIN_NAME_003       "Generic - Pulse counter"
#define PLUGIN_VALUENAME1_003 "Count"
#define PLUGIN_VALUENAME2_003 "Total"
#define PLUGIN_VALUENAME3_003 "Time"


void Plugin_003_pulse_interrupt1() ICACHE_RAM_ATTR;
void Plugin_003_pulse_interrupt2() ICACHE_RAM_ATTR;
void Plugin_003_pulse_interrupt3() ICACHE_RAM_ATTR;
void Plugin_003_pulse_interrupt4() ICACHE_RAM_ATTR;
//this takes 20 bytes of IRAM per handler
// void Plugin_003_pulse_interrupt5() ICACHE_RAM_ATTR;
// void Plugin_003_pulse_interrupt6() ICACHE_RAM_ATTR;
// void Plugin_003_pulse_interrupt7() ICACHE_RAM_ATTR;
// void Plugin_003_pulse_interrupt8() ICACHE_RAM_ATTR;

unsigned long Plugin_003_pulseCounter[TASKS_MAX];
unsigned long Plugin_003_pulseTotalCounter[TASKS_MAX];
unsigned long Plugin_003_pulseTime[TASKS_MAX];
unsigned long Plugin_003_pulseTimePrevious[TASKS_MAX];

boolean Plugin_003(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_003;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_003);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_003));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_003));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_003));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
      	addFormNumericBox(F("Debounce Time (mSec)"), F("plugin_003")
      			, Settings.TaskDevicePluginConfig[event->TaskIndex][0]);

        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        byte choice2 = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        String options[4] = { F("Delta"), F("Delta/Total/Time"), F("Total"), F("Delta/Total") };
        addFormSelector(F("Counter Type"), F("plugin_003_countertype"), 4, options, NULL, choice );

        if (choice !=0)
          addHtml(F("<span style=\"color:red\">Total count is not persistent!</span>"));

        String modeRaise[4];
        modeRaise[0] = F("LOW");
        modeRaise[1] = F("CHANGE");
        modeRaise[2] = F("RISING");
        modeRaise[3] = F("FALLING");
        int modeValues[4];
        modeValues[0] = LOW;
        modeValues[1] = CHANGE;
        modeValues[2] = RISING;
        modeValues[3] = FALLING;

        addFormSelector(F("Mode Type"), F("plugin_003_raisetype"), 4, modeRaise, modeValues, choice2 );

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_003"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_003_countertype"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("plugin_003_raisetype"));
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SHOW_VALUES:
      {
        string += F("<div class=\"div_l\">");
        string += ExtraTaskSettings.TaskDeviceValueNames[0];
        string += F(":</div><div class=\"div_r\">");
        string += Plugin_003_pulseCounter[event->TaskIndex];
        string += F("</div><div class=\"div_br\"></div><div class=\"div_l\">");
        string += ExtraTaskSettings.TaskDeviceValueNames[1];
        string += F(":</div><div class=\"div_r\">");
        string += Plugin_003_pulseTotalCounter[event->TaskIndex];
        string += F("</div><div class=\"div_br\"></div><div class=\"div_l\">");
        string += ExtraTaskSettings.TaskDeviceValueNames[2];
        string += F(":</div><div class=\"div_r\">");
        string += Plugin_003_pulseTime[event->TaskIndex];
        string += F("</div>");
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        String log = F("INIT : Pulse ");
        log += Settings.TaskDevicePin1[event->TaskIndex];
        addLog(LOG_LEVEL_INFO,log);
        pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
        success = Plugin_003_pulseinit(Settings.TaskDevicePin1[event->TaskIndex], event->TaskIndex,Settings.TaskDevicePluginConfig[event->TaskIndex][2]);
        break;
      }

    case PLUGIN_READ:
      {
        UserVar[event->BaseVarIndex] = Plugin_003_pulseCounter[event->TaskIndex];
        UserVar[event->BaseVarIndex+1] = Plugin_003_pulseTotalCounter[event->TaskIndex];
        UserVar[event->BaseVarIndex+2] = Plugin_003_pulseTime[event->TaskIndex];

        switch (Settings.TaskDevicePluginConfig[event->TaskIndex][1])
        {
          case 0:
          {
            event->sensorType = SENSOR_TYPE_SINGLE;
            UserVar[event->BaseVarIndex] = Plugin_003_pulseCounter[event->TaskIndex];
            break;
          }
          case 1:
          {
            event->sensorType = SENSOR_TYPE_TRIPLE;
            UserVar[event->BaseVarIndex] = Plugin_003_pulseCounter[event->TaskIndex];
            UserVar[event->BaseVarIndex+1] = Plugin_003_pulseTotalCounter[event->TaskIndex];
            UserVar[event->BaseVarIndex+2] = Plugin_003_pulseTime[event->TaskIndex];
            break;
          }
          case 2:
          {
            event->sensorType = SENSOR_TYPE_SINGLE;
            UserVar[event->BaseVarIndex] = Plugin_003_pulseTotalCounter[event->TaskIndex];
            break;
          }
          case 3:
          {
            event->sensorType = SENSOR_TYPE_DUAL;
            UserVar[event->BaseVarIndex] = Plugin_003_pulseCounter[event->TaskIndex];
            UserVar[event->BaseVarIndex+1] = Plugin_003_pulseTotalCounter[event->TaskIndex];
            break;
          }
        }
        Plugin_003_pulseCounter[event->TaskIndex] = 0;
        success = true;
        break;
      }
  }
  return success;
}


/*********************************************************************************************\
 * Check Pulse Counters (called from irq handler)
\*********************************************************************************************/
void Plugin_003_pulsecheck(byte Index)
{
  const unsigned long PulseTime=timePassedSince(Plugin_003_pulseTimePrevious[Index]);
  if(PulseTime > (unsigned long)Settings.TaskDevicePluginConfig[Index][0]) // check with debounce time for this task
    {
      Plugin_003_pulseCounter[Index]++;
      Plugin_003_pulseTotalCounter[Index]++;
      Plugin_003_pulseTime[Index] = PulseTime;
      Plugin_003_pulseTimePrevious[Index]=millis();
    }
}


/*********************************************************************************************\
 * Pulse Counter IRQ handlers
\*********************************************************************************************/
void Plugin_003_pulse_interrupt1()
{
  Plugin_003_pulsecheck(0);
}
void Plugin_003_pulse_interrupt2()
{
  Plugin_003_pulsecheck(1);
}
void Plugin_003_pulse_interrupt3()
{
  Plugin_003_pulsecheck(2);
}
void Plugin_003_pulse_interrupt4()
{
  Plugin_003_pulsecheck(3);
}
void Plugin_003_pulse_interrupt5()
{
  Plugin_003_pulsecheck(4);
}
void Plugin_003_pulse_interrupt6()
{
  Plugin_003_pulsecheck(5);
}
void Plugin_003_pulse_interrupt7()
{
  Plugin_003_pulsecheck(6);
}
void Plugin_003_pulse_interrupt8()
{
  Plugin_003_pulsecheck(7);
}


/*********************************************************************************************\
 * Init Pulse Counters
\*********************************************************************************************/
bool Plugin_003_pulseinit(byte Par1, byte Index, byte Mode)
{

  switch (Index)
  {
    case 0:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt1, Mode);
      break;
    case 1:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt2, Mode);
      break;
    case 2:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt3, Mode);
      break;
    case 3:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt4, Mode);
      break;
    // case 4:
    //   attachInterrupt(Par1, Plugin_003_pulse_interrupt5, Mode);
    //   break;
    // case 5:
    //   attachInterrupt(Par1, Plugin_003_pulse_interrupt6, Mode);
    //   break;
    // case 6:
    //   attachInterrupt(Par1, Plugin_003_pulse_interrupt7, Mode);
    //   break;
    // case 7:
    //   attachInterrupt(Par1, Plugin_003_pulse_interrupt8, Mode);
    //   break;
    default:
      addLog(LOG_LEVEL_ERROR,F("PULSE: Error, only the first 4 tasks can be pulse counters."));
      return(false);
  }

  return(true);
}
#endif // USES_P003
