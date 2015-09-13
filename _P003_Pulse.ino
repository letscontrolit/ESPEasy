//#######################################################################################################
//#################################### Plugin 003: Pulse  ###############################################
//#######################################################################################################

#define PLUGIN_003
#define PLUGIN_ID_003        3

unsigned long Plugin_003_pulseCounter[TASKS_MAX];
unsigned long Plugin_003_pulseTotalCounter[TASKS_MAX];

boolean Plugin_003(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_003;
        strcpy(Device[deviceCount].Name, "Pulse Counter");
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        strcpy(Device[deviceCount].ValueNames[0], "Count");
        break;
      }

    case PLUGIN_WEBFORM_VALUES:
      {
        string += F("<TD>");
        string += Plugin_003_pulseCounter[event->TaskIndex];
        string += F("<TD>");
        string += Plugin_003_pulseTotalCounter[event->TaskIndex];
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Serial.print(F("INIT : Pulse "));
        Serial.println(Settings.TaskDevicePin1[event->TaskIndex]);
        pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
        Plugin_003_pulseinit(Settings.TaskDevicePin1[event->TaskIndex], event->TaskIndex);
        success = true;
        break;
      }

    case PLUGIN_COMMAND:
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
 * Pulse Counters
\*********************************************************************************************/
void Plugin_003_pulse_interrupt1()
{
  Plugin_003_pulseCounter[0]++;
  Plugin_003_pulseTotalCounter[0]++;
}
void Plugin_003_pulse_interrupt2()
{
  Plugin_003_pulseCounter[1]++;
  Plugin_003_pulseTotalCounter[1]++;
}
void Plugin_003_pulse_interrupt3()
{
  Plugin_003_pulseCounter[2]++;
  Plugin_003_pulseTotalCounter[2]++;
}
void Plugin_003_pulse_interrupt4()
{
  Plugin_003_pulseCounter[3]++;
  Plugin_003_pulseTotalCounter[3]++;
}
void Plugin_003_pulse_interrupt5()
{
  Plugin_003_pulseCounter[4]++;
  Plugin_003_pulseTotalCounter[4]++;
}
void Plugin_003_pulse_interrupt6()
{
  Plugin_003_pulseCounter[5]++;
  Plugin_003_pulseTotalCounter[5]++;
}
void Plugin_003_pulse_interrupt7()
{
  Plugin_003_pulseCounter[6]++;
  Plugin_003_pulseTotalCounter[6]++;
}
void Plugin_003_pulse_interrupt8()
{
  Plugin_003_pulseCounter[7]++;
  Plugin_003_pulseTotalCounter[7]++;
}

void Plugin_003_pulseinit(byte Par1, byte Index)
{
  // Init IO pins
  Serial.println("PULSE: Init");
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

