#include "../PluginStructs/P162_data_struct.h"

#ifdef USES_P162

# include "../PluginStructs/P162_data_struct.h"

# include <SPI.h>

// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter

P162_data_struct::P162_data_struct(int8_t csPin,
                                   int8_t rstPin,
                                   int8_t shdPin)
  : _csPin(csPin), _rstPin(rstPin), _shdPin(shdPin)
{}

P162_data_struct::~P162_data_struct() {
  //
}

bool P162_data_struct::plugin_init(struct EventStruct *event) {
  if (validGpio(_csPin) && Settings.isSPI_valid()) {
    pinMode(_csPin, OUTPUT);
    _initialized = true;
  }

  if (validGpio(_rstPin)) {
    pinMode(_rstPin, OUTPUT);
    hw_reset();
    updateUserVars(event);
  }

  if (_initialized) {
    // Set default values
    if (P162_SHUTDOWN_W0) {
      _pot0_value = P162_SHUTDOWN_VALUE;
      write_pot(P162_POT0_SHUTDOWN, _pot0_value); // Value ignored for shutdown
    } else {
      _pot0_value = P162_INIT_W0;
      write_pot(P162_POT0_SEL, _pot0_value);
    }

    if (P162_SHUTDOWN_W1) {
      _pot1_value = P162_SHUTDOWN_VALUE;
      write_pot(P162_POT1_SHUTDOWN, _pot1_value); // Value ignored for shutdown
    } else {
      _pot1_value = P162_INIT_W1;
      write_pot(P162_POT1_SEL, _pot1_value);
    }
    updateUserVars(event);

    if (validGpio(_shdPin)) {
      pinMode(_shdPin, OUTPUT);

      if (P162_SHUTDOWN_W0 && P162_SHUTDOWN_W1) {
        _shdState = LOW;
        digitalWrite(_shdPin, _shdState);
      }
    }
  } else {
    addLog(LOG_LEVEL_ERROR, F("Digipot: Initialization failed, SPI/CS not configured."));
  }

  return _initialized;
}

/****************************************************
 * Reset the chip if a reset pin is configured
 ***************************************************/
bool P162_data_struct::hw_reset() {
  if (validGpio(_rstPin)) {
    _pot0_value = P162_RESET_VALUE;
    _pot1_value = P162_RESET_VALUE;
    digitalWrite(_rstPin, LOW);
    delayMicroseconds(1); // Reset requires low signal for at least 150 nsec, so 1 microsecond should suffice
    digitalWrite(_rstPin, HIGH);
    addLog(LOG_LEVEL_INFO, F("Digipot: Hardware reset applied."));
    return true;
  }
  return false;
}

/*********************************************************************************************
 * Handle command processing
 ********************************************************************************************/
bool P162_data_struct::plugin_write(struct EventStruct *event,
                                    String            & string) {
  bool success     = false;
  const String cmd = parseString(string, 1);

  if (equals(cmd, F("digipot"))) {
    const String sub    = parseString(string, 2);
    const bool   hasPar = !parseString(string, 3).isEmpty();

    // digipot,reset : Reset via configured RST pin else via software to initial state (both channels at 128)
    if (equals(sub, F("reset"))) { // Reset the digipot
      success = hw_reset();        // if the reset pin is configured

      if (!success) {
        _pot0_value = P162_RESET_VALUE;
        _pot1_value = P162_RESET_VALUE;
        updateUserVars(event);
        write_pot(P162_BOTH_POT_SEL, _pot0_value); // Single command
        success = true;
        addLog(LOG_LEVEL_INFO, F("Digipot: Software reset applied."));
      }
    } else

    // digipit,shutdown[,pot] : Shutdown pot 0, 1 or both (2), via hardware if both and pin shutdown configured
    if (equals(sub, F("shutdown")) && (event->Par2 >= 0) && (event->Par2 <= 2)) { // Pot range: 0..2
      if (((hasPar && (2 == event->Par2)) || !hasPar) && validGpio(_shdPin)) {    // no params or param = 2 (both) and shutdown pin defined
        _shdState = LOW;
        digitalWrite(_shdPin, _shdState);
        _pot0_value = P162_SHUTDOWN_VALUE;
        _pot1_value = P162_SHUTDOWN_VALUE;
        updateUserVars(event);
        success = true;
      } else {
        uint8_t shd = P162_BOTH_POT_SHUTDOWN;

        if (hasPar && (0 == event->Par2)) {
          shd         = P162_POT0_SHUTDOWN;
          _pot0_value = P162_SHUTDOWN_VALUE;
        } else if (hasPar && (1 == event->Par2)) {
          shd         = P162_POT1_SHUTDOWN;
          _pot1_value = P162_SHUTDOWN_VALUE;
        }
        updateUserVars(event);
        write_pot(shd, 0);
        success = true;
      }
    } else

    // digipot,(0|1|2),<value> : Set pot 0, 1 or both (2) to <value>, where value = 0..255
    if (isdigit(parseString(string, 2)[0]) && (event->Par1 >= 0) && (event->Par1 <= 2)) {
      if (hasPar && (event->Par2 >= 0) && (event->Par2 <= 255)) { // Argument mandatory
        uint8_t sel = P162_BOTH_POT_SEL;

        if (0 == event->Par1) {
          sel         = P162_POT0_SEL;
          _pot0_value = event->Par2;
        } else if (1 == event->Par1) {
          sel         = P162_POT1_SEL;
          _pot1_value = event->Par2;
        } else {
          _pot0_value = event->Par2;
          _pot1_value = event->Par2;
        }
        updateUserVars(event);
        write_pot(sel, event->Par2);
        success = true;
      }
    }
  }

  return success;
}

/********************************************************************************************
 * Write to pot
 *******************************************************************************************/
void P162_data_struct::write_pot(uint8_t cmd,
                                 uint8_t val) {
  if (!_initialized) { return; }

  // set the CS pin to low to select the chip:
  digitalWrite(_csPin, LOW);

  // send the command and value via SPI:
  SPI.transfer(cmd);
  SPI.transfer(val);

  // Set the CS pin high to execute the command:
  digitalWrite(_csPin, HIGH);
}

/*********************************************************************************
 * Set current values to UserVar
 ********************************************************************************/
void P162_data_struct::updateUserVars(struct EventStruct *event) {
  const int16_t pot0_old = UserVar.getFloat(event->TaskIndex, 0, true);
  const int16_t pot1_old = UserVar.getFloat(event->TaskIndex, 1, true);

  UserVar.setFloat(event->TaskIndex, 0, _pot0_value);
  UserVar.setFloat(event->TaskIndex, 1, _pot1_value);

  if (P162_CHANGED_EVENTS && ((pot0_old != _pot0_value) || (pot1_old != _pot1_value))) {
    sendData(event);
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("Digipot: W0 = %d, W1 = %d"), _pot0_value, _pot1_value));
  }
}

#endif // ifdef USES_P162
