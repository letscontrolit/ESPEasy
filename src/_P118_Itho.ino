// #######################################################################################################
// ############################## Plugin 118: Itho ventilation unit 868Mhz remote ########################
// #######################################################################################################

// author :jodur,       13-1-2018
// changed :jeroen, 2-11-2019
// changed :svollebregt, 30-1-2020 - changes to improve stability: volatile decleration of state,
//          disable logging within interrupts unles enabled, removed some unused code,
//          reduce noInterrupts() blockage of code fragments to prevent crashes
//			svollebregt, 16-2-2020 - ISR now sets flag which is checked by 50 per seconds plugin call as
//			receive ISR with Ticker was the cause of instability. Inspired by:
// https://github.com/arnemauer/Ducobox-ESPEasy-Plugin
//			svollebregt, 11-04-2020 - Minor changes to make code compatible with latest mega 20200410, removed SYNC1 option for
// now;
//			better to change this value in the Itho-lib code and compile it yourself
//			svollebreggt, 13-2-2021 - Now uses rewirtten library made by arjenhiemstra:
// https://github.com/arjenhiemstra/IthoEcoFanRFT
//			svollebregt, 11-2021 - Code improvements

// Recommended to disable RF receive logging to minimize code execution within interrupts

// List of commands:
// 1111 to join ESP8266 with Itho ventilation unit
// 9999 to leaveESP8266 with Itho ventilation unit
// 0 to set Itho ventilation unit to standby
// 1 - set Itho ventilation unit to low speed
// 2 - set Itho ventilation unit to medium speed
// 3 - set Itho ventilation unit to high speed
// 4 - set Itho ventilation unit to full speed
// 13 - set itho to high speed with hardware timer (10 min)
// 23 - set itho to high speed with hardware timer (20 min)
// 33 - set itho to high speed with hardware timer (30 min)

// List of States:

// 1 - Itho ventilation unit to lowest speed
// 2 - Itho ventilation unit to medium speed
// 3 - Itho ventilation unit to high speed
// 4 - Itho ventilation unit to full speed
// 13 -Itho to high speed with hardware timer (10 min)
// 23 -Itho to high speed with hardware timer (20 min)
// 33 -Itho to high speed with hardware timer (30 min)

// Usage for http (not case sensitive):
// http://ip/control?cmd=STATE,1111
// http://ip/control?cmd=STATE,1
// http://ip/control?cmd=STATE,2
// http://ip/control?cmd=STATE,3

// usage for example mosquito MQTT
// mosquitto_pub -t /Fan/cmd -m 'state 1111'
// mosquitto_pub -t /Fan/cmd -m 'state 1'
// mosquitto_pub -t /Fan/cmd -m 'state 2'
// mosquitto_pub -t /Fan/cmd -m 'state 3'


// This code needs the library made by 'arjenhiemstra': https://github.com/arjenhiemstra/IthoEcoFanRFT
// A CC1101 868Mhz transmitter is needed
// See https://gathering.tweakers.net/forum/list_messages/1690945 for more information
// code/idea was inspired by first release of code from 'Thinkpad'

#ifdef USES_P118

#include <SPI.h>
#include "IthoCC1101.h"
#include "IthoPacket.h"
#include "_Plugin_Helper.h"

// This extra settings struct is needed because the default settingsstruct doesn't support strings
struct PLUGIN__ExtraSettingsStruct
{       char ID1[9];
        char ID2[9];
        char ID3[9]; } PLUGIN_118_ExtraSettings;

IthoCC1101 PLUGIN_118_rf;

// extra for interrupt handling
bool   PLUGIN_118_ITHOhasPacket  = false;
int    PLUGIN_118_State          = 1; // after startup it is assumed that the fan is running low
int    PLUGIN_118_OldState       = 1;
int    PLUGIN_118_Timer          = 0;
int    PLUGIN_118_LastIDindex    = 0;
int    PLUGIN_118_OldLastIDindex = 0;
int8_t Plugin_118_IRQ_pin        = -1;
bool   PLUGIN_118_InitRunned     = false;
bool   PLUGIN_118_Log            = false;

// volatile for interrupt function
volatile bool PLUGIN_118_Int = false;

// volatile unsigned long PLUGIN_118_Int_time = 0;

#define PLUGIN_118
#define PLUGIN_ID_118         118
#define PLUGIN_NAME_118       "Communication - Itho ventilation"
#define PLUGIN_VALUENAME1_118 "State"
#define PLUGIN_VALUENAME2_118 "Timer"
#define PLUGIN_VALUENAME3_118 "LastIDindex"

// Timer values for hardware timer in Fan in seconds
#define PLUGIN_118_Time1      (10 * 60)
#define PLUGIN_118_Time2      (20 * 60)
#define PLUGIN_118_Time3      (30 * 60)
#define PLUGIN_118_OrconTime0      (12 * 60 * 60)
#define PLUGIN_118_OrconTime1      (60 * 60)
#define PLUGIN_118_OrconTime2      (13 * 60 * 60)
#define PLUGIN_118_OrconTime3      (60 * 60)

// Forward declarations
void PLUGIN_118_ITHOcheck();
void PLUGIN_118_Publishdata(struct EventStruct *event);
void PLUGIN_118_PluginWriteLog(const String& command);

ICACHE_RAM_ATTR void PLUGIN_118_ITHOinterrupt()
{
  PLUGIN_118_Int = true; // flag
  // PLUGIN_118_Int_time = millis(); //used to register time since interrupt, to make sure we don't read within 10 ms as the RX buffer needs
  // some time to get ready. Update: Disabled as it appear not necessary
}


boolean Plugin_118(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_118;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_118);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_118));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_118));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_118));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_input(F("Interrupt pin (CC1101 GDO2)"));
      break;
    }

    case PLUGIN_SET_DEFAULTS: // Set defaults address to the one used in old versions of the library for backwards compatability
    {
      PCONFIG(0) = 1;
      PCONFIG(1) = 10;
      PCONFIG(2) = 87;
      PCONFIG(3) = 81;
      success    = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // If configured interrupt pin differs from configured, release old pin first
      if ((Settings.TaskDevicePin1[event->TaskIndex] != Plugin_118_IRQ_pin) && (Plugin_118_IRQ_pin != -1))
      {
        #ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("IO-PIN changed, deatachinterrupt old pin"));
        #endif
        detachInterrupt(Plugin_118_IRQ_pin);
      }
      LoadCustomTaskSettings(event->TaskIndex, (byte *)&PLUGIN_118_ExtraSettings, sizeof(PLUGIN_118_ExtraSettings));
      addLog(LOG_LEVEL_INFO, F("Extra Settings PLUGIN_118 loaded"));
      PLUGIN_118_rf.setDeviceID(PCONFIG(1), PCONFIG(2), PCONFIG(3)); // DeviceID used to send commands, can also be changed on the fly for
                                                                     // multi itho control, 10,87,81 corresponds with old library
      PLUGIN_118_rf.init();
      Plugin_118_IRQ_pin = Settings.TaskDevicePin1[event->TaskIndex];
      pinMode(Plugin_118_IRQ_pin, INPUT);
      attachInterrupt(Plugin_118_IRQ_pin, PLUGIN_118_ITHOinterrupt, FALLING);
      addLog(LOG_LEVEL_INFO, F("CC1101 868Mhz transmitter initialized"));
      PLUGIN_118_rf.initReceive();
      PLUGIN_118_InitRunned = true;
      success               = true;
      break;
    }

    case PLUGIN_EXIT:
    {
      addLog(LOG_LEVEL_INFO, F("EXIT PLUGIN_118"));

      // remove interupt when plugin is removed
      detachInterrupt(Plugin_118_IRQ_pin);
      success = true;
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      // decrement timer when timermode is running
      if (PLUGIN_118_State >= 10) { PLUGIN_118_Timer--; }

      // if timer has elapsed set Fan state to low
      if ((PLUGIN_118_State >= 10) && (PLUGIN_118_Timer <= 0))
      {
        PLUGIN_118_State = 1;
        PLUGIN_118_Timer = 0;
      }

      // Publish new data when vars are changed or init has runned or timer is running (update every 2 sec)
      if  ((PLUGIN_118_OldState != PLUGIN_118_State) || ((PLUGIN_118_Timer > 0) && (PLUGIN_118_Timer % 2 == 0)) ||
           (PLUGIN_118_OldLastIDindex != PLUGIN_118_LastIDindex) || PLUGIN_118_InitRunned)
      {
        #ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("UPDATE by PLUGIN_ONCE_A_SECOND"));
        #endif
        PLUGIN_118_Publishdata(event);
        sendData(event);

        // reset flag set by init
        PLUGIN_118_InitRunned = false;
      }

      // Remeber current state for next cycle
      PLUGIN_118_OldState       = PLUGIN_118_State;
      PLUGIN_118_OldLastIDindex = PLUGIN_118_LastIDindex;
      success                   = true;
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      if (PLUGIN_118_Int)
      {
        PLUGIN_118_Int = false; // reset flag
        PLUGIN_118_ITHOcheck();

        /*
           unsigned long time_elapsed = millis() - PLUGIN_118_Int_time; //Disabled as it doesn't appear to be required
           if (time_elapsed >= 10)
           {
                PLUGIN_118_ITHOcheck();
           }
           else
           {
                delay(10-time_elapsed);
                PLUGIN_118_ITHOcheck();
           }*/
      }
      success = true;
      break;
    }


    case PLUGIN_READ: {
      // This ensures that even when Values are not changing, data is send at the configured interval for aquisition
      #ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("UPDATE by PLUGIN_READ"));
      #endif
      PLUGIN_118_Publishdata(event);

      // sendData(event); //SV - Added to send status every xx secnds as set within plugin
      success = true;
      break;
    }

    case PLUGIN_WRITE: {
      String tmpString = string;
      String cmd       = parseString(tmpString, 1);

      if (cmd.equalsIgnoreCase(F("STATE")))
      {
        // noInterrupts();
        switch (event->Par1) {
          case 1111: // Join command
          {
            PLUGIN_118_rf.sendCommand(IthoJoin);
            PLUGIN_118_rf.initReceive();
            PLUGIN_118_PluginWriteLog(F("join"));
            success = true;
            break;
          }
          case 9999: // Leave command
          {
            PLUGIN_118_rf.sendCommand(IthoLeave);
            PLUGIN_118_rf.initReceive();
            PLUGIN_118_PluginWriteLog(F("leave"));
            success = true;
            break;
          }
          case 0: // Off command
          {
            PLUGIN_118_rf.sendCommand(IthoStandby);
            PLUGIN_118_State       = 0;
            PLUGIN_118_Timer       = 0;
            PLUGIN_118_LastIDindex = 0;
            PLUGIN_118_rf.initReceive();
            PLUGIN_118_PluginWriteLog(F("standby"));
            success = true;
            break;
          }
          case 1: // Fan low
          {
            PLUGIN_118_rf.sendCommand(IthoLow);
            PLUGIN_118_State       = 1;
            PLUGIN_118_Timer       = 0;
            PLUGIN_118_LastIDindex = 0;
            PLUGIN_118_rf.initReceive();
            PLUGIN_118_PluginWriteLog(F("low speed"));
            success = true;
            break;
          }
          case 2: // Fan medium
          {
            PLUGIN_118_rf.sendCommand(IthoMedium);
            PLUGIN_118_State       = 2;
            PLUGIN_118_Timer       = 0;
            PLUGIN_118_LastIDindex = 0;
            PLUGIN_118_rf.initReceive();
            PLUGIN_118_PluginWriteLog(F("medium speed"));
            success = true;
            break;
          }
          case 3: // Fan high
          {
            PLUGIN_118_rf.sendCommand(IthoHigh);
            PLUGIN_118_State       = 3;
            PLUGIN_118_Timer       = 0;
            PLUGIN_118_LastIDindex = 0;
            PLUGIN_118_rf.initReceive();
            PLUGIN_118_PluginWriteLog(F("high speed"));
            success = true;
            break;
          }
          case 4: // Fan full
          {
            PLUGIN_118_rf.sendCommand(IthoFull);
            PLUGIN_118_State       = 4;
            PLUGIN_118_Timer       = 0;
            PLUGIN_118_LastIDindex = 0;
            PLUGIN_118_rf.initReceive();
            PLUGIN_118_PluginWriteLog(F("full speed"));
            success = true;
            break;
          }
          case 13: // Timer1 - 10 min
          {
            PLUGIN_118_rf.sendCommand(IthoTimer1);
            PLUGIN_118_State       = 13;
            PLUGIN_118_Timer       = PLUGIN_118_Time1;
            PLUGIN_118_LastIDindex = 0;
            PLUGIN_118_rf.initReceive();
            PLUGIN_118_PluginWriteLog(F("timer 1"));
            success = true;
            break;
          }
          case 23: // Timer2 - 20 min
          {
            PLUGIN_118_rf.sendCommand(IthoTimer2);
            PLUGIN_118_State       = 23;
            PLUGIN_118_Timer       = PLUGIN_118_Time2;
            PLUGIN_118_LastIDindex = 0;
            PLUGIN_118_rf.initReceive();
            PLUGIN_118_PluginWriteLog(F("timer 2"));
            success = true;
            break;
          }
          case 33: // Timer3 - 30 min
          {
            PLUGIN_118_rf.sendCommand(IthoTimer3);
            PLUGIN_118_State       = 33;
            PLUGIN_118_Timer       = PLUGIN_118_Time3;
            PLUGIN_118_LastIDindex = 0;
            PLUGIN_118_rf.initReceive();
            PLUGIN_118_PluginWriteLog(F("timer 3"));
            success = true;
            break;
          }
          case 103: //  Timer 12*60 minuten @ speed 0
          {
            PLUGIN_118_rf.sendCommand(OrconTimer0);
            PLUGIN_118_State       = 103;
            PLUGIN_118_Timer       = PLUGIN_118_OrconTime0;
            PLUGIN_118_LastIDindex = 0;
            PLUGIN_118_rf.initReceive();
            PLUGIN_118_PluginWriteLog(F("orconTimer 0"));
            success = true;
            break;
          }
          case 113: //  Timer 60 minuten @ speed 1
          {
            PLUGIN_118_rf.sendCommand(OrconTimer1);
            PLUGIN_118_State       = 113;
            PLUGIN_118_Timer       = PLUGIN_118_OrconTime1;
            PLUGIN_118_LastIDindex = 0;
            PLUGIN_118_rf.initReceive();
            PLUGIN_118_PluginWriteLog(F("orconTimer 1"));
            success = true;
            break;
          }
          case 123: //  Timer 13*60 minuten @ speed 2
          {
            PLUGIN_118_rf.sendCommand(OrconTimer2);
            PLUGIN_118_State       = 123;
            PLUGIN_118_Timer       = PLUGIN_118_OrconTime2;
            PLUGIN_118_LastIDindex = 0;
            PLUGIN_118_rf.initReceive();
            PLUGIN_118_PluginWriteLog(F("orconTimer 2"));
            success = true;
            break;
          }
          case 133: //  Timer 60 minuten @ speed 3
          {
            PLUGIN_118_rf.sendCommand(OrconTimer3);
            PLUGIN_118_State       = 133;
            PLUGIN_118_Timer       = PLUGIN_118_OrconTime3;
            PLUGIN_118_LastIDindex = 0;
            PLUGIN_118_rf.initReceive();
            PLUGIN_118_PluginWriteLog(F("orconTimer 3"));
            success = true;
            break;
          }
          default:
          {
            PLUGIN_118_PluginWriteLog(F("INVALID"));
            success = true;
          }
        }

        // interrupts();
      }
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Remote RF Controls"));
      addFormTextBox(F("Unit ID remote 1"), F("PLUGIN_118_ID1"), PLUGIN_118_ExtraSettings.ID1, 8);
      addFormTextBox(F("Unit ID remote 2"), F("PLUGIN_118_ID2"), PLUGIN_118_ExtraSettings.ID2, 8);
      addFormTextBox(F("Unit ID remote 3"), F("PLUGIN_118_ID3"), PLUGIN_118_ExtraSettings.ID3, 8);
      addFormCheckBox(F("Enable RF receive log"), F("p118_log"), PCONFIG(0)); // Makes RF logging optional to reduce clutter in the lof file
                                                                              // in RF noisy environments
      addFormNumericBox(F("Device ID byte 1"), F("p118_deviceid1"), PCONFIG(1), 0, 255);
      addFormNumericBox(F("Device ID byte 2"), F("p118_deviceid2"), PCONFIG(2), 0, 255);
      addFormNumericBox(F("Device ID byte 3"), F("p118_deviceid3"), PCONFIG(3), 0, 255);
      addFormNote(F(
                    "Device ID of your ESP, should not be the same as your neighbours ;-). Defaults to 10,87,81 which corresponds to the old Itho library"));
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      strcpy(PLUGIN_118_ExtraSettings.ID1, web_server.arg(F("PLUGIN_118_ID1")).c_str());
      strcpy(PLUGIN_118_ExtraSettings.ID2, web_server.arg(F("PLUGIN_118_ID2")).c_str());
      strcpy(PLUGIN_118_ExtraSettings.ID3, web_server.arg(F("PLUGIN_118_ID3")).c_str());
      SaveCustomTaskSettings(event->TaskIndex, (byte *)&PLUGIN_118_ExtraSettings, sizeof(PLUGIN_118_ExtraSettings));

      PCONFIG(0)     = isFormItemChecked(F("p118_log"));
      PLUGIN_118_Log = PCONFIG(0);
      PCONFIG(1)     = getFormItemInt(F("p118_deviceid1"), 10);
      PCONFIG(2)     = getFormItemInt(F("p118_deviceid2"), 87);
      PCONFIG(3)     = getFormItemInt(F("p118_deviceid3"), 81);
      success        = true;
      break;
    }
  }
  return success;
}

void PLUGIN_118_ITHOcheck()
{
  #ifndef BUILD_NO_DEBUG
  if (PLUGIN_118_Log) { addLog(LOG_LEVEL_DEBUG, "RF signal received"); } // All logs statements contain if-statement to disable logging to
                                                                         // reduce log clutter when many RF sources are present
  #endif

  if (PLUGIN_118_rf.checkForNewPacket())
  {
    IthoCommand cmd = PLUGIN_118_rf.getLastCommand();
    String Id       = PLUGIN_118_rf.getLastIDstr();

    // Move check here to prevent function calling within ISR
    byte index = 0;

    if (Id == PLUGIN_118_ExtraSettings.ID1) {
      index = 1;
    }
    else if (Id == PLUGIN_118_ExtraSettings.ID2) {
      index = 2;
    }
    else if (Id == PLUGIN_118_ExtraSettings.ID3) {
      index = 3;
    }

    // int index = PLUGIN_118_RFRemoteIndex(Id);
    // IF id is know index should be >0
    String log2;

    if (index > 0)
    {
      if (PLUGIN_118_Log) {
        log2 += F("Command received from remote-ID: ");
        log2 += Id;
        log2 += F(", command: ");

        // addLog(LOG_LEVEL_DEBUG, log);
      }

      switch (cmd)
      {
        case IthoUnknown:

          if (PLUGIN_118_Log) { log2 += F("unknown"); }
          break;
        case IthoStandby:
        case DucoStandby:

          if (PLUGIN_118_Log) { log2 += F("standby"); }
          PLUGIN_118_State       = 0;
          PLUGIN_118_Timer       = 0;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoLow:
        case DucoLow:

          if (PLUGIN_118_Log) { log2 += F("low"); }
          PLUGIN_118_State       = 1;
          PLUGIN_118_Timer       = 0;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoMedium:
        case DucoMedium:

          if (PLUGIN_118_Log) { log2 += F("medium"); }
          PLUGIN_118_State       = 2;
          PLUGIN_118_Timer       = 0;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoHigh:
        case DucoHigh:

          if (PLUGIN_118_Log) { log2 += F("high"); }
          PLUGIN_118_State       = 3;
          PLUGIN_118_Timer       = 0;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoFull:

          if (PLUGIN_118_Log) { log2 += F("full"); }
          PLUGIN_118_State       = 4;
          PLUGIN_118_Timer       = 0;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoTimer1:

          if (PLUGIN_118_Log) { log2 += +F("timer1"); }
          PLUGIN_118_State       = 13;
          PLUGIN_118_Timer       = PLUGIN_118_Time1;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoTimer2:

          if (PLUGIN_118_Log) { log2 += F("timer2"); }
          PLUGIN_118_State       = 23;
          PLUGIN_118_Timer       = PLUGIN_118_Time2;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoTimer3:

          if (PLUGIN_118_Log) { log2 += F("timer3"); }
          PLUGIN_118_State       = 33;
          PLUGIN_118_Timer       = PLUGIN_118_Time3;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoJoin:

          if (PLUGIN_118_Log) { log2 += F("join"); }
          break;
        case IthoLeave:

          if (PLUGIN_118_Log) { log2 += F("leave"); }
          break;
        case OrconTimer0:

          if (PLUGIN_118_Log) { log2 += +F("OrconTimer0"); }
          PLUGIN_118_State       = 103;
          PLUGIN_118_Timer       = PLUGIN_118_OrconTime0;
          PLUGIN_118_LastIDindex = index;
          break;
        case OrconTimer1:

          if (PLUGIN_118_Log) { log2 += +F("OrconTimer1"); }
          PLUGIN_118_State       = 113;
          PLUGIN_118_Timer       = PLUGIN_118_OrconTime1;
          PLUGIN_118_LastIDindex = index;
          break;
        case OrconTimer2:

          if (PLUGIN_118_Log) { log2 += +F("OrconTimer2"); }
          PLUGIN_118_State       = 123;
          PLUGIN_118_Timer       = PLUGIN_118_OrconTime2;
          PLUGIN_118_LastIDindex = index;
          break;
        case OrconTimer3:

          if (PLUGIN_118_Log) { log2 += +F("OrconTimer3"); }
          PLUGIN_118_State       = 133;
          PLUGIN_118_Timer       = PLUGIN_118_OrconTime3;
          PLUGIN_118_LastIDindex = index;
          break;


      }
    }
    else {
      if (PLUGIN_118_Log) {
        log2 += F("Device-ID: ");
        log2 += Id;
        log2 += F(" IGNORED");
      }
    }

    if (PLUGIN_118_Log) {
      addLogMove(LOG_LEVEL_DEBUG, log2);
    }
  }
}

void PLUGIN_118_Publishdata(struct EventStruct *event)
{
  UserVar[event->BaseVarIndex]     = PLUGIN_118_State;
  UserVar[event->BaseVarIndex + 1] = PLUGIN_118_Timer;
  UserVar[event->BaseVarIndex + 2] = PLUGIN_118_LastIDindex;

#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("State: ");
    log += UserVar[event->BaseVarIndex];
    addLogMove(LOG_LEVEL_DEBUG, log);
    log  = F("Timer: ");
    log += UserVar[event->BaseVarIndex + 1];
    addLogMove(LOG_LEVEL_DEBUG, log);
    log  = F("LastIDindex: ");
    log += UserVar[event->BaseVarIndex + 2];
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
#endif // ifndef BUILD_NO_DEBUG
}

void PLUGIN_118_PluginWriteLog(const String& command)
{
  String log = F("Send Itho command for: ");

  log += command;
  printWebString += log;
  addLogMove(LOG_LEVEL_INFO, log);
}

#endif // USES_P118
