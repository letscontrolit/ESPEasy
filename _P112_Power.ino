#ifdef PLUGIN_BUILD_DEV

//#######################################################################################################
//#################################### Plugin 112: Power Counter ########################################
//#######################################################################################################
//This sketch is based on Plugin 003: Pulse

#define PLUGIN_112
#define PLUGIN_ID_112         112
#define PLUGIN_NAME_112       "Power Counter [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_112 "PowerWh"
#define PLUGIN_VALUENAME2_112 "PowerCountTotal"

unsigned long Plugin_112_pulseCounter[TASKS_MAX];
unsigned long Plugin_112_pulseTotalCounter[TASKS_MAX];
unsigned long Plugin_112_pulseTime[TASKS_MAX];
unsigned long Plugin_112_pulseTimePrevious[TASKS_MAX];
float Plugin_112_pulseUsage[TASKS_MAX];

boolean Plugin_112(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_112;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_DUAL;
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
        string = F(PLUGIN_NAME_112);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_112));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_112));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        char tmpString[128];

		sprintf_P(tmpString, PSTR("<TR><TD>Debounce Time (mSec):<TD><input type='text' name='plugin_112_debounce' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        string += tmpString;

		sprintf_P(tmpString, PSTR("<TR><TD>Pulses per KWh:<TD><input type='text' name='plugin_112_pulsesperkwh' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
        string += tmpString;


        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_112_debounce");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        String plugin2 = WebServer.arg("plugin_112_pulsesperkwh");
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin2.toInt();
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SHOW_VALUES:
      {
        string += ExtraTaskSettings.TaskDeviceValueNames[0];
        string += F(":");
        string += Plugin_112_pulseUsage[event->TaskIndex];
        string += F("<BR>");
        string += ExtraTaskSettings.TaskDeviceValueNames[1];
        string += F(":");
        string += Plugin_112_pulseTotalCounter[event->TaskIndex];
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        String log = F("INIT : Power Counter ");
        log += Settings.TaskDevicePin1[event->TaskIndex];
        addLog(LOG_LEVEL_INFO,log);
        pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
        Plugin_112_pulseinit(Settings.TaskDevicePin1[event->TaskIndex], event->TaskIndex);
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
      	Plugin_112_idleusage(event->TaskIndex);
        UserVar[event->BaseVarIndex] = Plugin_112_pulseUsage[event->TaskIndex];
        UserVar[event->BaseVarIndex+1] = Plugin_112_pulseTotalCounter[event->TaskIndex];
        Plugin_112_pulseCounter[event->TaskIndex] = 0;
        success = true;
        break;
      }
  }
  return success;
}


/*********************************************************************************************\
 * Update usage when no pulse has been received for some time, so it will decrease on every
 * PLUGIN_READ event instead off keeping the last calculated usage value.
\*********************************************************************************************/
void Plugin_112_idleusage(byte Index)
{
  unsigned long PulseTime=millis() - Plugin_112_pulseTimePrevious[Index];
  if(PulseTime > (Settings.TaskDeviceTimer[Index] * 1000) &&        //More than $device_delay passed since last pulse
     PulseTime > Plugin_112_pulseTime[Index] ) {                    //More than last pulse interval

    // Let's prevent divison by zero
    if(Settings.TaskDevicePluginConfig[Index][1]==0) {
      Settings.TaskDevicePluginConfig[Index][1]=1000; // if not configged correctly prevent crashes and set it to 1000 as default value.
    }
    // WH = =3600000/[pulses per kwh]/[time since last pulse (ms)]
    Plugin_112_pulseUsage[Index] = (3600000000./Settings.TaskDevicePluginConfig[Index][1])/PulseTime;
  }
}


/*********************************************************************************************\
 * Check Pulse Counters (called from irq handler)
\*********************************************************************************************/
void Plugin_112_pulsecheck(byte Index)
{
  unsigned long PulseTime=millis() - Plugin_112_pulseTimePrevious[Index];
  if(PulseTime > Settings.TaskDevicePluginConfig[Index][0]) // check with debounce time for this task
    {
      Plugin_112_pulseTimePrevious[Index]=millis(); // moved this up as the operations below might take some time
      Plugin_112_pulseCounter[Index]++;
      Plugin_112_pulseTotalCounter[Index]++;
      Plugin_112_pulseTime[Index] = PulseTime;

      // Let's prevent divison by zero
      if(Settings.TaskDevicePluginConfig[Index][1]==0) {
        Settings.TaskDevicePluginConfig[Index][1]=1000; // if not configged correctly prevent crashes and set it to 1000 as default value.
      }

      // WH = =3600000/[pulses per kwh]/[time since last pulse (ms)]
      Plugin_112_pulseUsage[Index] = (3600000000./Settings.TaskDevicePluginConfig[Index][1])/PulseTime;
    }
}


/*********************************************************************************************\
 * Pulse Counter IRQ handlers
\*********************************************************************************************/
void Plugin_112_pulse_interrupt1()
{
  Plugin_112_pulsecheck(0);
}
void Plugin_112_pulse_interrupt2()
{
  Plugin_112_pulsecheck(1);
}
void Plugin_112_pulse_interrupt3()
{
  Plugin_112_pulsecheck(2);
}
void Plugin_112_pulse_interrupt4()
{
  Plugin_112_pulsecheck(3);
}
void Plugin_112_pulse_interrupt5()
{
  Plugin_112_pulsecheck(4);
}
void Plugin_112_pulse_interrupt6()
{
  Plugin_112_pulsecheck(5);
}
void Plugin_112_pulse_interrupt7()
{
  Plugin_112_pulsecheck(6);
}
void Plugin_112_pulse_interrupt8()
{
  Plugin_112_pulsecheck(7);
}


/*********************************************************************************************\
 * Init Pulse Counters
\*********************************************************************************************/
void Plugin_112_pulseinit(byte Par1, byte Index)
{
  // Init IO pins
  String log = F("Power Counter: Init");
  addLog(LOG_LEVEL_INFO,log);

  switch (Index)
  {
    case 0:
      attachInterrupt(Par1, Plugin_112_pulse_interrupt1, FALLING);
      break;
    case 1:
      attachInterrupt(Par1, Plugin_112_pulse_interrupt2, FALLING);
      break;
    case 2:
      attachInterrupt(Par1, Plugin_112_pulse_interrupt3, FALLING);
      break;
    case 3:
      attachInterrupt(Par1, Plugin_112_pulse_interrupt4, FALLING);
      break;
    case 4:
      attachInterrupt(Par1, Plugin_112_pulse_interrupt5, FALLING);
      break;
    case 5:
      attachInterrupt(Par1, Plugin_112_pulse_interrupt6, FALLING);
      break;
    case 6:
      attachInterrupt(Par1, Plugin_112_pulse_interrupt7, FALLING);
      break;
    case 7:
      attachInterrupt(Par1, Plugin_112_pulse_interrupt8, FALLING);
      break;
  }
}
#endif
