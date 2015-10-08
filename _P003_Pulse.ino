//#######################################################################################################
//#################################### Plugin 003: Pulse  ###############################################
//#######################################################################################################

#define PLUGIN_003
#define PLUGIN_ID_003         3
#define PLUGIN_NAME_003       "Pulse Counter"
#define PLUGIN_VALUENAME1_003 "Count"
#define PLUGIN_VALUENAME2_003 "Total"

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
        Device[deviceCount].ValueCount = 2;
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
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        char tmpString[80];
        sprintf_P(tmpString, PSTR("<TR><TD>Debounce Time (mSec):<TD><input type='text' name='plugin_003' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        string += tmpString;
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_003");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        success = true;
        break;
      }
      
    case PLUGIN_WEBFORM_SHOW_VALUES:
      {
        string += ExtraTaskSettings.TaskDeviceValueNames[0];
        string += F(":");
        string += Plugin_003_pulseCounter[event->TaskIndex];
        string += F("<BR>");
        string += ExtraTaskSettings.TaskDeviceValueNames[1];
        string += F(":");
        string += Plugin_003_pulseTotalCounter[event->TaskIndex];
        string += F("<BR>Time:");
        string += Plugin_003_pulseTime[event->TaskIndex];
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        String log = F("INIT : Pulse ");
        log += Settings.TaskDevicePin1[event->TaskIndex];
        addLog(LOG_LEVEL_INFO,log);
        pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
        Plugin_003_pulseinit(Settings.TaskDevicePin1[event->TaskIndex], event->TaskIndex);
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        UserVar[event->BaseVarIndex] = Plugin_003_pulseCounter[event->TaskIndex];
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
  unsigned long PulseTime=millis() - Plugin_003_pulseTimePrevious[Index];
  if(PulseTime > Settings.TaskDevicePluginConfig[Index][0]) // check with debounce time for this task
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
void Plugin_003_pulseinit(byte Par1, byte Index)
{
  // Init IO pins
  String log = F("PULSE: Init");
  addLog(LOG_LEVEL_INFO,log);

  switch (Index)
  {
    case 0:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt1, FALLING);
      break;
    case 1:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt2, FALLING);
      break;
    case 2:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt3, FALLING);
      break;
    case 3:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt4, FALLING);
      break;
    case 4:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt5, FALLING);
      break;
    case 5:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt6, FALLING);
      break;
    case 6:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt7, FALLING);
      break;
    case 7:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt8, FALLING);
      break;
  }
}

