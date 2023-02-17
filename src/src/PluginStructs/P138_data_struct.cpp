#include "../PluginStructs/P138_data_struct.h"

#ifdef USES_P138

// **************************************************************************/
// toString: convert P138_valueOptions_e enum to value-name or display-name
// **************************************************************************/
const __FlashStringHelper* toString(const P138_valueOptions_e value, bool displayString) {
  switch (value) {
    case P138_valueOptions_e::None: return displayString ? F("None") : F("none");
    case P138_valueOptions_e::BatteryCurrent: return displayString ? F("Battery current") : F("batterycurrent");
    case P138_valueOptions_e::ChargeUnderVoltage: return displayString ? F("Charge undervoltage") : F("chargeundervoltage");
    case P138_valueOptions_e::StopVoltage: return displayString ? F("Stop voltage") : F("stopvoltage");
    case P138_valueOptions_e::InCurrent: return displayString ? F("Input current") : F("inputcurrent");
    case P138_valueOptions_e::ChargeLevel: return displayString ? F("Charge level") : F("chargelevel");
    case P138_valueOptions_e::PowerSource: return displayString ? F("Power source") : F("powersource");
  }
  return F("*Undefined*");
}

// **************************************************************************/
// Constructor
// **************************************************************************/
P138_data_struct::P138_data_struct(struct EventStruct *event) {
  addLog(LOG_LEVEL_INFO, F("IP5306: Init."));

  _ip5306 = new (std::nothrow) arduino::ip5306(); // Default address and I2C Wire object
}

// **************************************************************************/
// Destructor
// **************************************************************************/
P138_data_struct::~P138_data_struct() {
  delete _ip5306;
  _ip5306 = nullptr;
}

// **************************************************************************/
// plugin_read: Read the values and send to controller(s)
// **************************************************************************/
bool P138_data_struct::plugin_read(struct EventStruct *event) {
  bool success = true;

  for (uint8_t i = 0; i < P138_NR_OUTPUT_VALUES; i++) {
    UserVar[event->BaseVarIndex + i] = read_value(static_cast<P138_valueOptions_e>(PCONFIG(P138_CONFIG_BASE + i)));
  }

  return success;
}

// **************************************************************************/
// plugin_fifty_per_second: check if the powersource changed and generate an event for that
// **************************************************************************/
bool P138_data_struct::plugin_fifty_per_second(struct EventStruct *event) {
  bool success = true;

  if (bitRead(P138_CONFIG_FLAGS, P138_FLAG_POWERCHANGE) && isInitialized()) {
    int8_t src = static_cast<int8_t>(_ip5306->power_source());

    if (_lastPowerSource != src) { // Changed?
      eventQueue.add(event->TaskIndex, F("PowerChanged"), src); // 0 = battery, 1 = Vin
      _lastPowerSource = src;      // Keep current
    }
  }
  return success;
}

// **************************************************************************/
// read_value: Read the requested value
// **************************************************************************/
float P138_data_struct::read_value(P138_valueOptions_e value) {
  if (isInitialized()) {
    switch (value) {
      case P138_valueOptions_e::None: return 0.0f;
      case P138_valueOptions_e::BatteryCurrent: {
        const float charge_current[] = { 200.0f, 400.0f, 500.0f, 600.0f };
        return charge_current[static_cast<int>(_ip5306->end_charge_current_detection())];
      }
      case P138_valueOptions_e::ChargeUnderVoltage: return _ip5306->charge_under_voltage_loop();
      case P138_valueOptions_e::StopVoltage: {
        const float stop_voltage[] = { 4.14f, 4.17f, 4.185f, 4.2f };
        return stop_voltage[static_cast<int>(_ip5306->charging_full_stop_voltage())];
      }
      case P138_valueOptions_e::InCurrent: return static_cast<float>(_ip5306->vin_current());
      case P138_valueOptions_e::ChargeLevel: return _ip5306->charge_level() * 100.0f;            // Make percentage
      case P138_valueOptions_e::PowerSource: return static_cast<float>(_ip5306->power_source()); // 0 = batt, 1 = Vin
    }
  }
  return 0.0f;
}

// **************************************************************************/
// plugin_write: Process commands
// **************************************************************************/
bool P138_data_struct::plugin_write(struct EventStruct *event,
                                    const String      & string) {
  bool success = false;

  // String cmd     = parseString(string, 1);

  // if (isInitialized() && equals(cmd, F("ip5306"))) { // Command trigger
  //   cmd = parseString(string, 2);                // sub command
  //   const bool empty3 = parseString(string, 3).isEmpty();
  //   const bool empty4 = parseString(string, 4).isEmpty();
  //   success = true;

  //   if (equals(cmd, F("??")) && !empty3) {         // ip5306,??,<value>
  //   } else {
  //     success = false;
  //   }
  // }

  return success;
}

/****************************************************************************
 * plugin_get_config_value: Retrieve values like [<taskname>#<valuename>]
 ***************************************************************************/
bool P138_data_struct::plugin_get_config_value(struct EventStruct *event,
                                               String            & string) {
  bool   success = true;
  String command = parseString(string, 1);
  float  value;

  if (equals(command, F("batcurrent"))) {         // batcurrent
    value = read_value(P138_valueOptions_e::BatteryCurrent);
  } else if (equals(command, F("chundervolt"))) { // chundervolt
    value = read_value(P138_valueOptions_e::ChargeUnderVoltage);
  } else if (equals(command, F("stopvolt"))) {  // stopvolt
    value = read_value(P138_valueOptions_e::StopVoltage);
  } else if (equals(command, F("inpcurrent"))) {  // inpcurrent
    value = read_value(P138_valueOptions_e::InCurrent);
  } else if (equals(command, F("chargelvl"))) {   // chargelvl
    value = read_value(P138_valueOptions_e::ChargeLevel);
  } else if (equals(command, F("pwrsource"))) {   // pwrsource
    value = read_value(P138_valueOptions_e::PowerSource);
  } else {
    success = false;
  }

  if (success) {
    string = toString(value, static_cast<unsigned int>(P138_CONFIG_DECIMALS));
  }
  return success;
}

#endif // ifdef USES_P138
