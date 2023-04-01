#include "../PluginStructs/P118_data_struct.h"

#ifdef USES_P118

// **************************************************************************/
// Constructor
// **************************************************************************/
P118_data_struct::P118_data_struct(int8_t csPin,
                                   int8_t irqPin,
                                   bool   logData,
                                   bool   rfLog)
  : _csPin(csPin), _irqPin(irqPin), _log(logData), _rfLog(rfLog) {}

// **************************************************************************/
// Destructor
// **************************************************************************/
P118_data_struct::~P118_data_struct() {
  delete _rf;
  _rf = nullptr;
}

bool P118_data_struct::plugin_init(struct EventStruct *event) {
  bool success = false;

  LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&_ExtraSettings, sizeof(_ExtraSettings));
  # ifdef P118_DEBUG_LOG
  addLog(LOG_LEVEL_INFO, F("ITHO: Extra Settings PLUGIN_118 loaded"));
  # endif // ifdef P118_DEBUG_LOG

  int8_t   spi_pins[3];
  uint32_t startInit = 0;

  if (Settings.getSPI_pins(spi_pins) && validGpio(spi_pins[1])) {
    startInit = millis();
    _rf       = new (std::nothrow) IthoCC1101(_csPin, spi_pins[1]); // Pass CS and MISO
  } else {
    addLog(LOG_LEVEL_ERROR, F("ITHO: SPI configuration not correct!"));
  }

  if (nullptr != _rf) {
    success = true;
    # if P118_FEATURE_ORCON
    _rf->enableOrcon(P118_CONFIG_ORCON == 1); // Enabled?
    # endif // if P118_FEATURE_ORCON

    // DeviceID used to send commands, can also be changed on the fly for multi itho control, 10,87,81 corresponds with old library
    _rf->setDeviceID(P118_CONFIG_DEVID1, P118_CONFIG_DEVID2, P118_CONFIG_DEVID3);
    _rf->init();
    uint32_t finishInit = millis();

    if (finishInit - startInit > P118_TIMEOUT_LIMIT) {
      String log = F("ITHO: Init duration was: ");
      log += finishInit - startInit;
      log += F("msec. suggesting that the CC1101 board is not (correctly) connected.");
      addLog(LOG_LEVEL_ERROR, log);
      success = false;
    }

    if (success) {
      if (validGpio(_irqPin)) {
        attachInterruptArg(digitalPinToInterrupt(_irqPin),
                           reinterpret_cast<void (*)(void *)>(ISR_ithoCheck),
                           this,
                           FALLING);
        addLog(LOG_LEVEL_INFO, F("ITHO: Interrupts enabled."));
      } else {
        addLog(LOG_LEVEL_ERROR, F("ITHO: Interrupt pin disabled, sending is OK, not receiving data!"));
      }
      _rf->initReceive();
      _InitRunned = true;
    }
  }
  return success;
}

bool P118_data_struct::plugin_exit(struct EventStruct *event) {
  // remove interupt when plugin is removed
  if (validGpio(_irqPin)) {
    detachInterrupt(digitalPinToInterrupt(_irqPin));
  }

  return true;
}

bool P118_data_struct::plugin_once_a_second(struct EventStruct *event) {
  // decrement timer when timermode is running
  if ((_State >= 10) && (_Timer > 0)) {
    _Timer--;

    if (_Timer == 0) { _Timer--; }
  }

  // if timer has elapsed set Fan state to low
  if ((_State >= 10) && (_Timer < 0))
  {
    if (_State < 100) {
      _State = 1;   // Itho low
    } else {
      _State = 101; // Orcon low
    }
    _Timer = 0;     // Avoid doing this again
  }

  // Publish new data when vars are changed or init has runned or timer is running (update every 2 sec)
  if  ((_OldState != _State) || ((_Timer > 0) && (_Timer % 2 == 0)) ||
       (_OldLastIDindex != _LastIDindex) || _InitRunned)
  {
    # ifdef P118_DEBUG_LOG
    addLog(LOG_LEVEL_DEBUG, F("ITHO: UPDATE by PLUGIN_ONCE_A_SECOND"));
    # endif // ifdef P118_DEBUG_LOG
    PublishData(event);
    sendData(event);

    // reset flag set by init
    _InitRunned = false;
  }

  // Remeber current state for next cycle
  _OldState       = _State;
  _OldLastIDindex = _LastIDindex;
  return true;
}

bool P118_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  if (_Int) {
    ITHOcheck();
    _Int = false; // reset flag
  }

  return true;
}

bool P118_data_struct::plugin_read(struct EventStruct *event) {
  // This ensures that even when Values are not changing, data is send at the configured interval for aquisition
  # ifdef P118_DEBUG_LOG
  addLog(LOG_LEVEL_DEBUG, F("ITHO: UPDATE by PLUGIN_READ"));
  # endif // ifdef P118_DEBUG_LOG
  PublishData(event);

  return true;
}

bool P118_data_struct::plugin_write(struct EventStruct *event, const String& string) {
  bool   success = false;
  String cmd     = parseString(string, 1);

  bool stateCmd = equals(cmd, F("state"));

  if (equals(cmd, F("itho")) || stateCmd) {
    # ifndef BUILD_NO_DEBUG

    if (stateCmd) { addLogMove(LOG_LEVEL_ERROR, F("ITHO: Command 'state' is deprecated, use 'itho' instead, see documentation.")); }
    # endif // ifndef BUILD_NO_DEBUG
    success = true;

    switch (event->Par1) {
      case 1111: // Join command
      {
        _rf->sendCommand(IthoJoin);
        _rf->initReceive();
        PluginWriteLog(F("join"));
        break;
      }
      case 9999: // Leave command
      {
        _rf->sendCommand(IthoLeave);
        _rf->initReceive();
        PluginWriteLog(F("leave"));
        break;
      }
      case 0: // Off command
      {
        _rf->sendCommand(IthoStandby);
        _State       = 0;
        _Timer       = 0;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("standby"));
        break;
      }
      case 1: // Fan low
      {
        _rf->sendCommand(IthoLow);
        _State       = 1;
        _Timer       = 0;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("low speed"));
        break;
      }
      case 2: // Fan medium
      {
        _rf->sendCommand(IthoMedium);
        _State       = 2;
        _Timer       = 0;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("medium speed"));
        break;
      }
      case 3: // Fan high
      {
        _rf->sendCommand(IthoHigh);
        _State       = 3;
        _Timer       = 0;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("high speed"));
        break;
      }
      case 4: // Fan full
      {
        _rf->sendCommand(IthoFull);
        _State       = 4;
        _Timer       = 0;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("full speed"));
        break;
      }
      case 13: // Timer1 - 10 min
      {
        _rf->sendCommand(IthoTimer1);
        _State       = 13;
        _Timer       = PLUGIN_118_Time1;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("timer 1"));
        break;
      }
      case 23: // Timer2 - 20 min
      {
        _rf->sendCommand(IthoTimer2);
        _State       = 23;
        _Timer       = PLUGIN_118_Time2;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("timer 2"));
        break;
      }
      case 33: // Timer3 - 30 min
      {
        _rf->sendCommand(IthoTimer3);
        _State       = 33;
        _Timer       = PLUGIN_118_Time3;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("timer 3"));
        break;
      }
      # if P118_FEATURE_ORCON
      case 100: // Fan StandBy
      {
        uint8_t srcID[3], destID[3];
        SetDestIDSrcID(event, srcID, destID, _ExtraSettings.ID1);
        _rf->sendCommand(OrconStandBy, srcID, destID);
        _State       = 100;
        _Timer       = 0;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("Orcon standBy"));
        break;
      }
      case 101: // Fan low
      {
        uint8_t srcID[3], destID[3];
        SetDestIDSrcID(event, srcID, destID, _ExtraSettings.ID1);
        _rf->sendCommand(OrconLow, srcID, destID);
        _State       = 101;
        _Timer       = 0;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("Orcon low speed"));
        break;
      }
      case 102: // Fan medium
      {
        uint8_t srcID[3], destID[3];
        SetDestIDSrcID(event, srcID, destID, _ExtraSettings.ID1);
        _rf->sendCommand(OrconMedium, srcID, destID);
        _State       = 102;
        _Timer       = 0;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("Orcon medium speed"));
        break;
      }
      case 103: // Fan high
      {
        uint8_t srcID[3], destID[3];
        SetDestIDSrcID(event, srcID, destID, _ExtraSettings.ID1);
        _rf->sendCommand(OrconHigh, srcID, destID);
        _State       = 103;
        _Timer       = 0;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("Orcon high speed"));
        break;
      }
      case 104: // Fan auto
      {
        uint8_t srcID[3], destID[3];
        SetDestIDSrcID(event, srcID, destID, _ExtraSettings.ID1);
        _rf->sendCommand(OrconAuto, srcID, destID);
        _State       = 104;
        _Timer       = 0;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("Orcon auto speed"));
        break;
      }
      case 110: //  Timer 12*60 minutes @ speed 0
      {
        uint8_t srcID[3], destID[3];
        SetDestIDSrcID(event, srcID, destID, _ExtraSettings.ID1);
        _rf->sendCommand(OrconTimer0, srcID, destID);
        _State       = 110;
        _Timer       = PLUGIN_118_OrconTime0;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("Orcon Timer 0"));
        break;
      }
      case 111: //  Timer 60 minutes @ speed 1
      {
        uint8_t srcID[3], destID[3];
        SetDestIDSrcID(event, srcID, destID, _ExtraSettings.ID1);
        _rf->sendCommand(OrconTimer1, srcID, destID);
        _State       = 111;
        _Timer       = PLUGIN_118_OrconTime1;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("Orcon Timer 1"));
        break;
      }
      case 112: //  Timer 13*60 minutes @ speed 2
      {
        uint8_t srcID[3], destID[3];
        SetDestIDSrcID(event, srcID, destID, _ExtraSettings.ID1);
        _rf->sendCommand(OrconTimer2, srcID, destID);
        _State       = 112;
        _Timer       = PLUGIN_118_OrconTime2;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("Orcon Timer 2"));
        break;
      }
      case 113: //  Timer 60 minutes @ speed 3
      {
        uint8_t srcID[3], destID[3];
        SetDestIDSrcID(event, srcID, destID, _ExtraSettings.ID1);
        _rf->sendCommand(OrconTimer3, srcID, destID);
        _State       = 113;
        _Timer       = PLUGIN_118_OrconTime3;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("Orcon Timer 3"));
        break;
      }
      case 114:
      {
        uint8_t srcID[3], destID[3];
        SetDestIDSrcID(event, srcID, destID, _ExtraSettings.ID1);
        _rf->sendCommand(OrconAutoCO2, srcID, destID);
        _State       = 114;
        _Timer       = PLUGIN_118_OrconTime3;
        _LastIDindex = 0;
        _rf->initReceive();
        PluginWriteLog(F("Orcon Auto CO2"));
        break;
      }
      # else // if P118_FEATURE_ORCON
      case 100:
      case 101:
      case 102:
      case 103:
      case 104:
      case 110:
      case 111:
      case 112:
      case 113:
      case 114:
        PluginWriteLog(F("Orcon support not included!"));
        success = false;
        break;
      # endif // if P118_FEATURE_ORCON
      default:
      {
        PluginWriteLog(F("INVALID"));
        success = false;
        break;
      }
    }
  }
  return success;
}

void P118_data_struct::ITHOcheck() {
  bool _dbgLog = _log
  # ifndef BUILD_NO_DEBUG
                 && loglevelActiveFor(LOG_LEVEL_DEBUG)
  # endif // ifndef BUILD_NO_DEBUG
  ;

  # ifndef BUILD_NO_DEBUG

  if (_dbgLog) {
    addLog(LOG_LEVEL_DEBUG, "ITHO: RF signal received"); // All logs statements contain if-statement to disable logging to
  }                                                      // reduce log clutter when many RF sources are present
  # endif // ifndef BUILD_NO_DEBUG

  if (_rf->checkForNewPacket()) {
    IthoCommand cmd = _rf->getLastCommand();
    String Id       = _rf->getLastIDstr();

    if (_rfLog && loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("ITHO: Received from ID: ");
      log += Id;
      log += F("; raw cmd: ");
      log += cmd;
      addLog(LOG_LEVEL_INFO, log);
    }

    // Move check here to prevent function calling within ISR
    byte index = 0;

    if (Id == _ExtraSettings.ID1) {
      index = 1;
    }
    else if (Id == _ExtraSettings.ID2) {
      index = 2;
    }
    else if (Id == _ExtraSettings.ID3) {
      index = 3;
    }

    String log;

    if (index > 0) {
      if (_dbgLog) {
        log += F("Command received from remote-ID: ");
        log += Id;
        log += F(", command: ");
      }

      switch (cmd) {
        case IthoUnknown:

          if (_dbgLog) { log += F("unknown"); }
          break;
        case IthoStandby:
        case DucoStandby:

          if (_dbgLog) { log += F("standby"); }
          _State       = 0;
          _Timer       = 0;
          _LastIDindex = index;
          break;
        case IthoLow:
        case DucoLow:

          if (_dbgLog) { log += F("low"); }
          _State       = 1;
          _Timer       = 0;
          _LastIDindex = index;
          break;
        case IthoMedium:
        case DucoMedium:

          if (_dbgLog) { log += F("medium"); }
          _State       = 2;
          _Timer       = 0;
          _LastIDindex = index;
          break;
        case IthoHigh:
        case DucoHigh:

          if (_dbgLog) { log += F("high"); }
          _State       = 3;
          _Timer       = 0;
          _LastIDindex = index;
          break;
        case IthoFull:

          if (_dbgLog) { log += F("full"); }
          _State       = 4;
          _Timer       = 0;
          _LastIDindex = index;
          break;
        case IthoTimer1:

          if (_dbgLog) { log += +F("timer1"); }
          _State       = 13;
          _Timer       = PLUGIN_118_Time1;
          _LastIDindex = index;
          break;
        case IthoTimer2:

          if (_dbgLog) { log += F("timer2"); }
          _State       = 23;
          _Timer       = PLUGIN_118_Time2;
          _LastIDindex = index;
          break;
        case IthoTimer3:

          if (_dbgLog) { log += F("timer3"); }
          _State       = 33;
          _Timer       = PLUGIN_118_Time3;
          _LastIDindex = index;
          break;
        case IthoJoin:

          if (_dbgLog) { log += F("join"); }
          break;
        case IthoLeave:

          if (_dbgLog) { log += F("leave"); }
          break;
        # if P118_FEATURE_ORCON
        case OrconStandBy:

          if (_dbgLog) { log += F("Orcon standby"); }
          _State       = 100;
          _Timer       = 0;
          _LastIDindex = index;
          break;
        case OrconLow:

          if (_dbgLog) { log += F("Orcon low"); }
          _State       = 101;
          _Timer       = 0;
          _LastIDindex = index;
          break;
        case OrconMedium:

          if (_dbgLog) { log += F("Orcon medium"); }
          _State       = 102;
          _Timer       = 0;
          _LastIDindex = index;
          break;
        case OrconHigh:

          if (_dbgLog) { log += F("Orcon high"); }
          _State       = 103;
          _Timer       = 0;
          _LastIDindex = index;
          break;
        case OrconAuto:

          if (_dbgLog) { log += F("Orcon auto"); }
          _State       = 104;
          _Timer       = 0;
          _LastIDindex = index;
          break;
        case OrconTimer0:

          if (_dbgLog) { log += +F("Orcon Timer0"); }
          _State       = 110;
          _Timer       = PLUGIN_118_OrconTime0;
          _LastIDindex = index;
          break;
        case OrconTimer1:

          if (_dbgLog) { log += +F("Orcon Timer1"); }
          _State       = 111;
          _Timer       = PLUGIN_118_OrconTime1;
          _LastIDindex = index;
          break;
        case OrconTimer2:

          if (_dbgLog) { log += +F("Orcon Timer2"); }
          _State       = 112;
          _Timer       = PLUGIN_118_OrconTime2;
          _LastIDindex = index;
          break;
        case OrconTimer3:

          if (_dbgLog) { log += +F("Orcon Timer3"); }
          _State       = 113;
          _Timer       = PLUGIN_118_OrconTime3;
          _LastIDindex = index;
          break;
        case OrconAutoCO2:

          if (_dbgLog) { log += +F("Orcon AutoCO2"); }
          _State       = 114;
          _Timer       = 0;
          _LastIDindex = index;
          break;
        # else // if P118_FEATURE_ORCON
        case OrconStandBy:
        case OrconLow:
        case OrconMedium:
        case OrconHigh:
        case OrconAuto:
        case OrconTimer0:
        case OrconTimer1:
        case OrconTimer2:
        case OrconTimer3:
        case OrconAutoCO2:
          break;
        # endif // if P118_FEATURE_ORCON
      }
    } else {
      if (_dbgLog) {
        log += F("Device-ID: ");
        log += Id;
        log += F(" IGNORED");
      }
    }

    # ifndef BUILD_NO_DEBUG

    if (_dbgLog) {
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
    # endif // ifndef BUILD_NO_DEBUG
  }
}

void P118_data_struct::PublishData(struct EventStruct *event) {
  UserVar[event->BaseVarIndex]     = _State;
  UserVar[event->BaseVarIndex + 1] = _Timer;
  UserVar[event->BaseVarIndex + 2] = _LastIDindex;

  # ifndef BUILD_NO_DEBUG

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
  # endif // ifndef BUILD_NO_DEBUG
}

void P118_data_struct::PluginWriteLog(const String& command) {
  String log = F("Send Itho"
                 # if P118_FEATURE_ORCON
                 "/Orcon"
                 # endif // if P118_FEATURE_ORCON
                 " command for: ");

  log += command;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, log);
  }
  printWebString += log;
}

// **************************************************************************/
// Interrupt handler
// **************************************************************************/
void P118_data_struct::ISR_ithoCheck(P118_data_struct *self) {
  self->_Int = true;
}

# if P118_FEATURE_ORCON

/**
 * Orcon specific: Set Destination ID
 */
void P118_data_struct::SetDestIDSrcID(struct EventStruct *event, uint8_t (& srcID)[3], uint8_t (& destID)[3], char (& tmpTmpID)[9])
{
  destID[0] = PCONFIG(1) - 0;
  destID[1] = PCONFIG(2) - 0;
  destID[2] = PCONFIG(3) - 0;

  const char *delimiter = ",";
  char *token;

  // char tmpID[9] = PLUGIN_118_ExtraSettings.ID1; // copy needed otherwise we modify PLUGIN_118_ExtraSettings.ID1 itself
  char tmpID[9];

  memcpy(tmpID, tmpTmpID, 9);
  token = strtok(tmpID, delimiter);     // select the first part

  if (token) {
    srcID[0] = strtol(token, NULL, 16); // convert first string part (hex) to int
  } else {
    srcID[0] = 0;
  }
  token = strtok(NULL, delimiter);

  if (token) {
    srcID[1] = strtol(token, NULL, 16); // convert first string part (hex) to int
  } else {
    srcID[1] = 0;
  }
  token = strtok(NULL, delimiter);

  if (token) {
    srcID[2] = strtol(token, NULL, 16); // convert first string part (hex) to int
  } else {
    srcID[2] = 0;
  }

  #  ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = (F("srcID: "));
    log += String(srcID[0]);
    log += (F(","));
    log += String(srcID[1]);
    log += (F(","));
    log += String(srcID[2]);
    log += (F(" destID: "));
    log += String(destID[0]);
    log += (F(","));
    log += String(destID[1]);
    log += (F(","));
    log += String(destID[2]);
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
  #  endif // ifndef BUILD_NO_DEBUG
}

# endif // if P118_FEATURE_ORCON

#endif // ifdef USES_P118
