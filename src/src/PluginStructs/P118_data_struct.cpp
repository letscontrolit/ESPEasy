#include "../PluginStructs/P118_data_struct.h"

#ifdef USES_P118

// **************************************************************************/
// Constructor
// **************************************************************************/
P118_data_struct::P118_data_struct(uint8_t logData)
  : PLUGIN_118_Log(logData) {}

// **************************************************************************/
// Destructor
// **************************************************************************/
P118_data_struct::~P118_data_struct() {
  if (isInitialized()) {
    delete PLUGIN_118_rf;
    PLUGIN_118_rf = nullptr;
  }
}

bool P118_data_struct::plugin_init(struct EventStruct *event) {
  bool success = false;

  LoadCustomTaskSettings(event->TaskIndex, (byte *)&PLUGIN_118_ExtraSettings, sizeof(PLUGIN_118_ExtraSettings));
  # ifdef P118_DEBUG_LOG
  addLog(LOG_LEVEL_INFO, F("Extra Settings PLUGIN_118 loaded"));
  # endif // ifdef P118_DEBUG_LOG

  PLUGIN_118_rf = new (std::nothrow) IthoCC1101(PIN(1));

  if (nullptr != PLUGIN_118_rf) {
    PLUGIN_118_rf->setDeviceID(PCONFIG(1), PCONFIG(2), PCONFIG(3)); // DeviceID used to send commands, can also be changed on the fly for
                                                                    // multi itho control, 10,87,81 corresponds with old library
    PLUGIN_118_rf->init();

    attachInterruptArg(digitalPinToInterrupt(Plugin_118_IRQ_pin),
                       reinterpret_cast<void (*)(void *)>(ISR_ithoCheck),
                       this,
                       FALLING);

    PLUGIN_118_rf->initReceive();
    PLUGIN_118_InitRunned = true;

    success = true;
  }
  return success;
}

bool P118_data_struct::plugin_exit(struct EventStruct *event) {
  // remove interupt when plugin is removed
  detachInterrupt(digitalPinToInterrupt(Plugin_118_IRQ_pin));

  if (nullptr != PLUGIN_118_rf) {
    delete PLUGIN_118_rf;
    PLUGIN_118_rf = nullptr;
  }
  return true;
}

bool P118_data_struct::plugin_once_a_second(struct EventStruct *event) {
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
    # ifdef P118_DEBUG_LOG
    addLog(LOG_LEVEL_DEBUG, F("UPDATE by PLUGIN_ONCE_A_SECOND"));
    # endif // ifdef P118_DEBUG_LOG
    PublishData(event);
    sendData(event);

    // reset flag set by init
    PLUGIN_118_InitRunned = false;
  }

  // Remeber current state for next cycle
  PLUGIN_118_OldState       = PLUGIN_118_State;
  PLUGIN_118_OldLastIDindex = PLUGIN_118_LastIDindex;
  return true;
}

bool P118_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  if (PLUGIN_118_Int) {
    ITHOcheck();
    PLUGIN_118_Int = false; // reset flag
  }

  return true;
}

bool P118_data_struct::plugin_read(struct EventStruct *event) {
  // This ensures that even when Values are not changing, data is send at the configured interval for aquisition
  # ifdef P118_DEBUG_LOG
  addLog(LOG_LEVEL_DEBUG, F("UPDATE by PLUGIN_READ"));
  # endif // ifdef P118_DEBUG_LOG
  PublishData(event);

  // sendData(event); //SV - Added to send status every xx secnds as set within plugin
  return true;
}

bool P118_data_struct::plugin_write(struct EventStruct *event, const String& string) {
  bool   success   = false;
  String tmpString = string;
  String cmd       = parseString(tmpString, 1);

  if (cmd.equalsIgnoreCase(F("STATE")))
  {
    switch (event->Par1) {
      case 1111: // Join command
      {
        PLUGIN_118_rf->sendCommand(IthoJoin);
        PLUGIN_118_rf->initReceive();
        PluginWriteLog(F("join"));
        success = true;
        break;
      }
      case 9999: // Leave command
      {
        PLUGIN_118_rf->sendCommand(IthoLeave);
        PLUGIN_118_rf->initReceive();
        PluginWriteLog(F("leave"));
        success = true;
        break;
      }
      case 0: // Off command
      {
        PLUGIN_118_rf->sendCommand(IthoStandby);
        PLUGIN_118_State       = 0;
        PLUGIN_118_Timer       = 0;
        PLUGIN_118_LastIDindex = 0;
        PLUGIN_118_rf->initReceive();
        PluginWriteLog(F("standby"));
        success = true;
        break;
      }
      case 1: // Fan low
      {
        PLUGIN_118_rf->sendCommand(IthoLow);
        PLUGIN_118_State       = 1;
        PLUGIN_118_Timer       = 0;
        PLUGIN_118_LastIDindex = 0;
        PLUGIN_118_rf->initReceive();
        PluginWriteLog(F("low speed"));
        success = true;
        break;
      }
      case 2: // Fan medium
      {
        PLUGIN_118_rf->sendCommand(IthoMedium);
        PLUGIN_118_State       = 2;
        PLUGIN_118_Timer       = 0;
        PLUGIN_118_LastIDindex = 0;
        PLUGIN_118_rf->initReceive();
        PluginWriteLog(F("medium speed"));
        success = true;
        break;
      }
      case 3: // Fan high
      {
        PLUGIN_118_rf->sendCommand(IthoHigh);
        PLUGIN_118_State       = 3;
        PLUGIN_118_Timer       = 0;
        PLUGIN_118_LastIDindex = 0;
        PLUGIN_118_rf->initReceive();
        PluginWriteLog(F("high speed"));
        success = true;
        break;
      }
      case 4: // Fan full
      {
        PLUGIN_118_rf->sendCommand(IthoFull);
        PLUGIN_118_State       = 4;
        PLUGIN_118_Timer       = 0;
        PLUGIN_118_LastIDindex = 0;
        PLUGIN_118_rf->initReceive();
        PluginWriteLog(F("full speed"));
        success = true;
        break;
      }
      case 13: // Timer1 - 10 min
      {
        PLUGIN_118_rf->sendCommand(IthoTimer1);
        PLUGIN_118_State       = 13;
        PLUGIN_118_Timer       = PLUGIN_118_Time1;
        PLUGIN_118_LastIDindex = 0;
        PLUGIN_118_rf->initReceive();
        PluginWriteLog(F("timer 1"));
        success = true;
        break;
      }
      case 23: // Timer2 - 20 min
      {
        PLUGIN_118_rf->sendCommand(IthoTimer2);
        PLUGIN_118_State       = 23;
        PLUGIN_118_Timer       = PLUGIN_118_Time2;
        PLUGIN_118_LastIDindex = 0;
        PLUGIN_118_rf->initReceive();
        PluginWriteLog(F("timer 2"));
        success = true;
        break;
      }
      case 33: // Timer3 - 30 min
      {
        PLUGIN_118_rf->sendCommand(IthoTimer3);
        PLUGIN_118_State       = 33;
        PLUGIN_118_Timer       = PLUGIN_118_Time3;
        PLUGIN_118_LastIDindex = 0;
        PLUGIN_118_rf->initReceive();
        PluginWriteLog(F("timer 3"));
        success = true;
        break;
      }
      default:
      {
        PluginWriteLog(F("INVALID"));
      }
    }
  }
  return success;
}

void P118_data_struct::ITHOcheck() {
  if (PLUGIN_118_Log) {
    addLog(LOG_LEVEL_DEBUG, "RF signal received"); // All logs statements contain if-statement to disable logging to
  }                                                // reduce log clutter when many RF sources are present

  if (PLUGIN_118_rf->checkForNewPacket()) {
    IthoCommand cmd = PLUGIN_118_rf->getLastCommand();
    String Id       = PLUGIN_118_rf->getLastIDstr();

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
    String log;

    if (index > 0) {
      if (PLUGIN_118_Log) {
        log += F("Command received from remote-ID: ");
        log += Id;
        log += F(", command: ");

        // addLog(LOG_LEVEL_DEBUG, log);
      }

      switch (cmd) {
        case IthoUnknown:

          if (PLUGIN_118_Log) { log += F("unknown"); }
          break;
        case IthoStandby:
        case DucoStandby:

          if (PLUGIN_118_Log) { log += F("standby"); }
          PLUGIN_118_State       = 0;
          PLUGIN_118_Timer       = 0;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoLow:
        case DucoLow:

          if (PLUGIN_118_Log) { log += F("low"); }
          PLUGIN_118_State       = 1;
          PLUGIN_118_Timer       = 0;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoMedium:
        case DucoMedium:

          if (PLUGIN_118_Log) { log += F("medium"); }
          PLUGIN_118_State       = 2;
          PLUGIN_118_Timer       = 0;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoHigh:
        case DucoHigh:

          if (PLUGIN_118_Log) { log += F("high"); }
          PLUGIN_118_State       = 3;
          PLUGIN_118_Timer       = 0;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoFull:

          if (PLUGIN_118_Log) { log += F("full"); }
          PLUGIN_118_State       = 4;
          PLUGIN_118_Timer       = 0;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoTimer1:

          if (PLUGIN_118_Log) { log += +F("timer1"); }
          PLUGIN_118_State       = 13;
          PLUGIN_118_Timer       = PLUGIN_118_Time1;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoTimer2:

          if (PLUGIN_118_Log) { log += F("timer2"); }
          PLUGIN_118_State       = 23;
          PLUGIN_118_Timer       = PLUGIN_118_Time2;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoTimer3:

          if (PLUGIN_118_Log) { log += F("timer3"); }
          PLUGIN_118_State       = 33;
          PLUGIN_118_Timer       = PLUGIN_118_Time3;
          PLUGIN_118_LastIDindex = index;
          break;
        case IthoJoin:

          if (PLUGIN_118_Log) { log += F("join"); }
          break;
        case IthoLeave:

          if (PLUGIN_118_Log) { log += F("leave"); }
          break;
      }
    } else {
      if (PLUGIN_118_Log) {
        log += F("Device-ID: ");
        log += Id;
        log += F(" IGNORED");
      }
    }

    if (PLUGIN_118_Log) {
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  }
}

void P118_data_struct::PublishData(struct EventStruct *event) {
  UserVar[event->BaseVarIndex]     = PLUGIN_118_State;
  UserVar[event->BaseVarIndex + 1] = PLUGIN_118_Timer;
  UserVar[event->BaseVarIndex + 2] = PLUGIN_118_LastIDindex;

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("State: ");

    log += UserVar[event->BaseVarIndex];
    addLog(LOG_LEVEL_DEBUG, log);
    log.clear();
    log += F("Timer: ");
    log += UserVar[event->BaseVarIndex + 1];
    addLog(LOG_LEVEL_DEBUG, log);
    log.clear();
    log += F("LastIDindex: ");
    log += UserVar[event->BaseVarIndex + 2];
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
}

void P118_data_struct::PluginWriteLog(const String& command) {
  String log = F("Send Itho command for: ");

  log += command;
  addLog(LOG_LEVEL_INFO, log);
  printWebString += log;
}

// **************************************************************************/
// Interrupt handler
// **************************************************************************/
void P118_data_struct::ISR_ithoCheck(P118_data_struct *self) {
  self->PLUGIN_118_Int = true;
}

#endif // ifdef USES_P118
