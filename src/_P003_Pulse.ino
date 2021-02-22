#include "_Plugin_Helper.h"
#ifdef USES_P003

// #######################################################################################################
// #################################### Plugin 003: Pulse  ###############################################
// #######################################################################################################
//
// Make sure physical connections are electrically well sepparated so no crossover of the signals happen.
// Especially at rates above ~5'000 RPM with longer lines. Best use a cable with ground and signal twisted.

#include "src/Helpers/ESPEasy_time_calc.h"

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
void Plugin_003_pulsecheck(byte Index) ICACHE_RAM_ATTR;

// this takes 20 bytes of IRAM per handler
// void Plugin_003_pulse_interrupt5() ICACHE_RAM_ATTR;
// void Plugin_003_pulse_interrupt6() ICACHE_RAM_ATTR;
// void Plugin_003_pulse_interrupt7() ICACHE_RAM_ATTR;
// void Plugin_003_pulse_interrupt8() ICACHE_RAM_ATTR;

volatile unsigned long Plugin_003_pulseCounter[TASKS_MAX];
volatile unsigned long Plugin_003_pulseTotalCounter[TASKS_MAX];
volatile uint64_t Plugin_003_pulseTime[TASKS_MAX];
volatile uint64_t Plugin_003_pulseTimePrevious[TASKS_MAX];
volatile uint64_t Plugin_003_debounce[TASKS_MAX];

// Mx: 2021-01: additions for enhanced Mode Types PULSE_HIGH and PULSE_LOW
// special Mode Type. Note: only lower 3 bits are significant for GPIO Interupt. upper 4 bits are flag for new modes
#define P003_MODE_TYPE_PULSE_LOW  (0x10|CHANGE)
// special Mode Type. Note: only lower 3 bits are significant for GPIO Interupt. upper 4 bits are flag for new modes
#define P003_MODE_TYPE_PULSE_HIGH (0x20|CHANGE)
#define P003_MODE_TYPE_MODE_MASK 0x30
#define P003_MODE_TYPE_INTERRUPT_MASK 0x03
// pin state detected in previpus interupt call
volatile int Plugin_003_pinStatePrevious[TASKS_MAX];
// Mx: 2021-01: end

boolean Plugin_003(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_003;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
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

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_input(F("Pulse"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Debounce Time (mSec)"), F("p003")
                        , PCONFIG(0));

      byte   choice     = PCONFIG(1);
      byte   choice2    = PCONFIG(2);
      String options[4] = { F("Delta"), F("Delta/Total/Time"), F("Total"), F("Delta/Total") };
      addFormSelector(F("Counter Type"), F("p003_countertype"), 4, options, NULL, choice);

      if (choice != 0) {
        addHtml(F("<span style=\"color:red\">Total count is not persistent!</span>"));
      }

      // Mx: 2021-01: correction of first mode. LOW=0 did not generate interupts and does not make sense. Tus changed to "none"
      // Note: A correction to ONLOW = 0x04 (cf. Arduino.h) causes problems as it fires consecutive interupts, when GPIO is low 
      String modeRaise[6];
      modeRaise[0] = F("none");
      modeRaise[1] = F("CHANGE");
      modeRaise[2] = F("RISING");
      modeRaise[3] = F("FALLING");
      // Mx: 2021-01: addition of two PULSE modes
      modeRaise[4] = F("PULSE low ");   // couting takes place when long enough low pulse ends
      modeRaise[5] = F("PULSE high");   // couting takes place when long enough high pulse ends
      int modeValues[6];
      modeValues[0] = 0;
      modeValues[1] = CHANGE;
      modeValues[2] = RISING;
      modeValues[3] = FALLING;
      // Mx: 2021-01: addition of two PULSE modes
      modeValues[4] = P003_MODE_TYPE_PULSE_LOW;
      modeValues[5] = P003_MODE_TYPE_PULSE_HIGH;
      
      addFormSelector(F("Mode Type"), F("p003_raisetype"), 6, modeRaise, modeValues, choice2);
      // Mx: 2021-01: ends
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("p003"));
      PCONFIG(1) = getFormItemInt(F("p003_countertype"));
      PCONFIG(2) = getFormItemInt(F("p003_raisetype"));
      success                                              = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_VALUES:
    {
      pluginWebformShowValue(ExtraTaskSettings.TaskDeviceValueNames[0], String(Plugin_003_pulseCounter[event->TaskIndex]));
      pluginWebformShowValue(ExtraTaskSettings.TaskDeviceValueNames[1], String(Plugin_003_pulseTotalCounter[event->TaskIndex]));
      pluginWebformShowValue(ExtraTaskSettings.TaskDeviceValueNames[2], String(Plugin_003_pulseTime[event->TaskIndex]/1000.0f), false);
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // Restore any value that may have been read from the RTC.
      switch (PCONFIG(1))
      {
        case 0:
        {
          Plugin_003_pulseCounter[event->TaskIndex] = UserVar[event->BaseVarIndex];
          break;
        }
        case 1:
        {
          Plugin_003_pulseCounter[event->TaskIndex]      = UserVar[event->BaseVarIndex];
          Plugin_003_pulseTotalCounter[event->TaskIndex] = UserVar[event->BaseVarIndex + 1];
          Plugin_003_pulseTime[event->TaskIndex]         = UserVar[event->BaseVarIndex + 2]*1000L;
          break;
        }
        case 2:
        {
          Plugin_003_pulseTotalCounter[event->TaskIndex] = UserVar[event->BaseVarIndex];
          break;
        }
        case 3:
        {
          Plugin_003_pulseCounter[event->TaskIndex]      = UserVar[event->BaseVarIndex];
          Plugin_003_pulseTotalCounter[event->TaskIndex] = UserVar[event->BaseVarIndex + 1];
          break;
        }
      }

      // Restore the total counter from the unused 4th UserVar value.
      // It may be using a formula to generate the output, which makes it impossible to restore
      // the true internal state.
      Plugin_003_pulseTotalCounter[event->TaskIndex] = UserVar[event->BaseVarIndex + 3];
      Plugin_003_debounce[event->TaskIndex] = (uint64_t)Settings.TaskDevicePluginConfig[event->TaskIndex][0]*1000L;

      // Mx: 2021-01: Initialize pinState from previos interrupt with "no pulse" depending on Mode Type
      Plugin_003_pinStatePrevious[event->TaskIndex] = ((PCONFIG(2) == P003_MODE_TYPE_PULSE_LOW) ? HIGH : LOW );
      // Mx: 2021-01: end

      String log = F("INIT : Pulse GPIO ");
      log += Settings.TaskDevicePin1[event->TaskIndex];
      addLog(LOG_LEVEL_INFO, log);
      pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
      // Mx: 2021-01: masking out interrupt type from Mode Type for setting interupts
      success =
        Plugin_003_pulseinit(Settings.TaskDevicePin1[event->TaskIndex], event->TaskIndex,
                             (PCONFIG(2) & P003_MODE_TYPE_INTERRUPT_MASK));
      // Mx: 2021-01: end
      break;
    }

    case PLUGIN_READ:
    {
      // FIXME TD-er: Is it correct to write the first 3  UserVar values, regardless the set counter type?
      UserVar[event->BaseVarIndex]     = Plugin_003_pulseCounter[event->TaskIndex];
      UserVar[event->BaseVarIndex + 1] = Plugin_003_pulseTotalCounter[event->TaskIndex];
      UserVar[event->BaseVarIndex + 2] = Plugin_003_pulseTime[event->TaskIndex]/1000.0f;

      // Store the raw value in the unused 4th position.
      // This is needed to restore the value from RTC as it may be converted into another output value using a formula.
      UserVar[event->BaseVarIndex + 3] = Plugin_003_pulseTotalCounter[event->TaskIndex];

      switch (PCONFIG(1))
      {
        case 0:
        {
          event->sensorType            = Sensor_VType::SENSOR_TYPE_SINGLE;
          UserVar[event->BaseVarIndex] = Plugin_003_pulseCounter[event->TaskIndex];
          break;
        }
        case 1:
        {
          event->sensorType                = Sensor_VType::SENSOR_TYPE_TRIPLE;
          UserVar[event->BaseVarIndex]     = Plugin_003_pulseCounter[event->TaskIndex];
          UserVar[event->BaseVarIndex + 1] = Plugin_003_pulseTotalCounter[event->TaskIndex];
          UserVar[event->BaseVarIndex + 2] = Plugin_003_pulseTime[event->TaskIndex]/1000.0f;
          break;
        }
        case 2:
        {
          event->sensorType            = Sensor_VType::SENSOR_TYPE_SINGLE;
          UserVar[event->BaseVarIndex] = Plugin_003_pulseTotalCounter[event->TaskIndex];
          break;
        }
        case 3:
        {
          event->sensorType                = Sensor_VType::SENSOR_TYPE_DUAL;
          UserVar[event->BaseVarIndex]     = Plugin_003_pulseCounter[event->TaskIndex];
          UserVar[event->BaseVarIndex + 1] = Plugin_003_pulseTotalCounter[event->TaskIndex];
          break;
        }
      }
      Plugin_003_pulseCounter[event->TaskIndex] = 0;
      success                                   = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      String command            = parseString(string, 1);
      bool   mustCallPluginRead = false;

      if (command == F("resetpulsecounter"))
      {
        // Valid commands:
        // - resetpulsecounter
        // - resetpulsecounter,taskindex

        // Allow for an optional taskIndex parameter. When not given it will take the first task with this plugin.
        if (!pluginOptionalTaskIndexArgumentMatch(event->TaskIndex, string, 1)) {
          break;
        }
        Plugin_003_pulseCounter[event->TaskIndex]      = 0;
        Plugin_003_pulseTotalCounter[event->TaskIndex] = 0;
        Plugin_003_pulseTime[event->TaskIndex]         = 0;
        mustCallPluginRead                             = true;
        success                                        = true; // Command is handled.
      } else if (command == F("setpulsecountertotal"))
      {
        // Valid commands:
        // - setpulsecountertotal,value
        // - setpulsecountertotal,value,taskindex

        // First check if (optional) task index matches.
        if (!pluginOptionalTaskIndexArgumentMatch(event->TaskIndex, string, 2)) {
          break;
        }

        int par1;

        if (validIntFromString(parseString(string, 2), par1))
        {
          Plugin_003_pulseTotalCounter[event->TaskIndex] = par1;
          mustCallPluginRead                             = true;
          success                                        = true; // Command is handled.
        }
      }

      if (mustCallPluginRead) {
        // Note that the set time is before the current moment, so we call the read as soon as possible.
        // The read does also use any set formula and stored the value in RTC.
        Scheduler.schedule_task_device_timer(event->TaskIndex, millis() - 10);
      }
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
  noInterrupts(); // s0170071: avoid nested interrups due to bouncing.

  const uint64_t PulseTime = getMicros64() - Plugin_003_pulseTimePrevious[Index];
  int pinState;

  // Mx: 2021-01: added processing for new mode types PULSE_LOW and PULSE_HIGH
  // if Mode Type is P003_MODE_TYPE_PULSE_LOW or P003_MODE_TYPE_PULSE_HIGH
  if ( (Settings.TaskDevicePluginConfig[Index][2] & P003_MODE_TYPE_MODE_MASK) != 0 )
  {
    //  read current state from this tasks's GPIO
    pinState = digitalRead(Settings.TaskDevicePin1[Index]);

    // Was the previous pulse longer than debounce time? (else ignore previous pulse)
    if (PulseTime > Plugin_003_debounce[Index])
    {
      // Has pin state changed ? (else ignore prev. pulse (possibly we missed (disabled) interupt))
      if (pinState != (int)Plugin_003_pinStatePrevious[Index] )
      {  
      // is prev. pulse to be counted, because it applied to the configured Mode Type 
        if ((int)Plugin_003_pinStatePrevious[Index] == ((Settings.TaskDevicePluginConfig[Index][2] == P003_MODE_TYPE_PULSE_LOW) ? LOW : HIGH))
        {
          Plugin_003_pulseCounter[Index]++;
          Plugin_003_pulseTotalCounter[Index]++;
          Plugin_003_pulseTime[Index] = PulseTime;  // length of counted pulse
        }
      }
    }
    
    // save current pinState for next call
    Plugin_003_pinStatePrevious[Index] = pinState;
    Plugin_003_pulseTimePrevious[Index] = getMicros64();  // reset for each received interupt to determine previous pulse's length (counted or not)

  }
  else
  // Mx: 2021-01: end 
  {
    if (PulseTime > Plugin_003_debounce[Index]) // check with debounce time for this task
    {
      Plugin_003_pulseCounter[Index]++;
      Plugin_003_pulseTotalCounter[Index]++;
      Plugin_003_pulseTime[Index]         = PulseTime;
      Plugin_003_pulseTimePrevious[Index] = getMicros64();  // reset when counted only to determine interval between counted pulses
    }
  }

  interrupts(); // enable interrupts again.
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
      addLog(LOG_LEVEL_ERROR, F("PULSE: Error, only the first 4 tasks can be pulse counters."));
      return false;
  }

  return true;
}

#endif // USES_P003