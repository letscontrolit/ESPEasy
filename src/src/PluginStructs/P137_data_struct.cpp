#include "../PluginStructs/P137_data_struct.h"

#ifdef USES_P137

// **************************************************************************/
// toString: convert P137_valueOptions_e enum to value-name or display-name
// **************************************************************************/
const __FlashStringHelper* toString(const P137_valueOptions_e value, bool displayString) {
  switch (value) {
    case P137_valueOptions_e::None: return displayString ? F("None") : F("none");
    case P137_valueOptions_e::BatteryVoltage: return displayString ? F("Battery voltage") : F("batteryvoltage");
    case P137_valueOptions_e::BatteryDischargeCurrent: return displayString ? F("Battery discharge current") : F("batterydischargecurrent");
    case P137_valueOptions_e::BatteryChargeCurrent: return displayString ? F("Battery charge current") : F("batterychargecurrent");
    case P137_valueOptions_e::BatteryPower: return displayString ? F("Battery power") : F("batterypower");
    case P137_valueOptions_e::AcinVoltage: return displayString ? F("Input voltage") : F("inputvoltage");
    case P137_valueOptions_e::AcinCurrent: return displayString ? F("Input current") : F("inputcurrent");
    case P137_valueOptions_e::VbusVoltage: return displayString ? F("VBus voltage") : F("vbusvoltage");
    case P137_valueOptions_e::VbusCurrent: return displayString ? F("VBus current") : F("vbuscurrent");
    case P137_valueOptions_e::InternalTemperature: return displayString ? F("Internal temperature") : F("internaltemperature");
    case P137_valueOptions_e::ApsVoltage: return displayString ? F("APS Voltage") : F("apsvoltage");
    case P137_valueOptions_e::LDO2: return displayString ? F("LDO2 Voltage") : F("ldo2voltage");
    case P137_valueOptions_e::LDO3: return displayString ? F("LDO3 Voltage") : F("ldo3voltage");
    case P137_valueOptions_e::GPIO0: return displayString ? F("GPIO0 Voltage") : F("gpio0voltage");
    case P137_valueOptions_e::OptionCount: return F("*Invalid*");
  }
  return F("*Undefined*");
}

// **************************************************************************/
// Constructor
// **************************************************************************/
P137_data_struct::P137_data_struct(struct EventStruct *event) {
  addLog(LOG_LEVEL_INFO, F("AXP192: Init."));

  axp192 = new (std::nothrow) I2C_AXP192(); // Default address and I2C Wire object

  if (isInitialized()) {
    I2C_AXP192_InitDef initDef = {          // Functions based on M5Stack StickC
      .EXTEN  = true,                       // Enable ESP Power
      .BACKUP = true,                       // Enable RTC power
      .DCDC1  = 3300,                       // ESP Power
      .DCDC2  = 0,                          // Unused
      .DCDC3  = 0,                          // Unused
      .LDO2   = P137_CONFIG_LDO2,           // Backlight power (3000 mV)
      .LDO3   = P137_CONFIG_LDO3,           // TFT Power (3000 mV)
      .GPIO0  = P137_CONFIG_GPIO0,          // Microphone power (2800 mV)
      .GPIO1  = -1,                         // Unused/not connected
      .GPIO2  = -1,
      .GPIO3  = -1,
      .GPIO4  = -1,
    };
    ldo2_value  = P137_CONFIG_LDO2;
    ldo3_value  = P137_CONFIG_LDO3;
    gpio0_value = P137_CONFIG_GPIO0;

    axp192->begin(initDef);
  }
}

// **************************************************************************/
// Destructor
// **************************************************************************/
P137_data_struct::~P137_data_struct() {
  delete axp192;
  axp192 = nullptr;
}

// **************************************************************************/
// plugin_read: Read the values and send to controller(s)
// **************************************************************************/
bool P137_data_struct::plugin_read(struct EventStruct *event) {
  bool success = true;

  for (uint8_t i = 0; i < P137_NR_OUTPUT_VALUES; i++) {
    UserVar[event->BaseVarIndex + i] = read_value(static_cast<P137_valueOptions_e>(PCONFIG(P137_CONFIG_BASE + i)));
  }

  return success;
}

// **************************************************************************/
// read_value: Read the requested value
// **************************************************************************/
float P137_data_struct::read_value(P137_valueOptions_e value) {
  if (isInitialized()) {
    switch (value) {
      case P137_valueOptions_e::None: return 0.0f;
      case P137_valueOptions_e::BatteryVoltage: return axp192->getBatteryVoltage();
      case P137_valueOptions_e::BatteryDischargeCurrent: return axp192->getBatteryDischargeCurrent();
      case P137_valueOptions_e::BatteryChargeCurrent: return axp192->getBatteryChargeCurrent();
      case P137_valueOptions_e::BatteryPower: return axp192->getBatteryPower();
      case P137_valueOptions_e::AcinVoltage: return axp192->getAcinVolatge();
      case P137_valueOptions_e::AcinCurrent: return axp192->getAcinCurrent();
      case P137_valueOptions_e::VbusVoltage: return axp192->getVbusVoltage();
      case P137_valueOptions_e::VbusCurrent: return axp192->getVbusCurrent();
      case P137_valueOptions_e::InternalTemperature: return axp192->getInternalTemperature();
      case P137_valueOptions_e::ApsVoltage: return axp192->getApsVoltage();
      case P137_valueOptions_e::LDO2: return static_cast<float>(ldo2_value);
      case P137_valueOptions_e::LDO3: return static_cast<float>(ldo3_value);
      case P137_valueOptions_e::GPIO0: return static_cast<float>(gpio0_value);
      case P137_valueOptions_e::OptionCount: return 0.0f;
    }
  }
  return 0.0f;
}

// **************************************************************************/
// plugin_write: Process commands
// **************************************************************************/
bool P137_data_struct::plugin_write(struct EventStruct *event,
                                    const String      & string) {
  bool   success = false;
  String cmd     = parseString(string, 1);

  if (isInitialized() && cmd.equals(F("axp"))) { // Command trigger
    cmd = parseString(string, 2);                // sub command
    const bool empty3 = parseString(string, 3).isEmpty();
    const bool empty4 = parseString(string, 4).isEmpty();
    success = true;

    if (cmd.equals(F("ldo2")) && !empty3) {         // axp,ldo2,<voltage>
      axp192->setLDO2(event->Par2);                 // Range checking done by function
      ldo2_value = event->Par2;
    } else if (cmd.equals(F("ldo3")) && !empty3) {  // axp,ldo3,<voltage>
      axp192->setLDO3(event->Par2);                 // Range checking done by function
      ldo3_value = event->Par2;
    } else if (cmd.equals(F("gpio0")) && !empty3) { // axp,gpio0,<voltage>
      axp192->setGPIO0(event->Par2);                // Range checking done by function
      gpio0_value = event->Par2;
    } else if ((event->Par2 >= 0) && (event->Par2 <= P137_CONST_100_PERCENT) && !empty3 && empty4) {
      // percentage 0..100, 0 turns off
      if (cmd.equals(F("ldo2perc"))) { // axp,ldo2perc,<percentage>
        ldo2_value = event->Par2 > 0 ? map(event->Par2,
                                           P137_CONST_1_PERCENT, P137_CONST_100_PERCENT,
                                           ldo2_range[0], ldo2_range[1]) : 0;
        axp192->setLDO2(ldo2_value);
      } else if (cmd.equals(F("ldo3perc"))) { // axp,ldo3perc,<percentage>
        ldo3_value = event->Par2 > 0 ? map(event->Par2,
                                           P137_CONST_1_PERCENT, P137_CONST_100_PERCENT,
                                           ldo3_range[0], ldo3_range[1]) : 0;
        axp192->setLDO3(ldo3_value);
      } else if (cmd.equals(F("gpio0perc"))) { // axp,gpio0perc,<percentage>
        gpio0_value = event->Par2 > 0 ? map(event->Par2,
                                            P137_CONST_1_PERCENT, P137_CONST_100_PERCENT,
                                            gpio0_range[0], gpio0_range[1]) : 0;
        axp192->setGPIO0(gpio0_value);
      } else {
        success = false;
      }
    } else if ((event->Par2 >= 0) && (event->Par3 <= P137_CONST_MAX_LDO) && (event->Par2 < event->Par3) && !empty3 && !empty4) {
      // map range <low>,<high>
      if (cmd.equals(F("ldo2map"))) {         // axp,ldo2map,<low>,<high>
        ldo2_range[0] = event->Par2;
        ldo2_range[1] = event->Par3;
      } else if (cmd.equals(F("ldo3map"))) {  // axp,ldo3map,<low>,<high>
        ldo3_range[0] = event->Par2;
        ldo3_range[1] = event->Par3;
      } else if (cmd.equals(F("gpio0map"))) { // axp,gpio0map,<low>,<high>
        gpio0_range[0] = event->Par2;
        gpio0_range[1] = event->Par3;
      } else {
        success = false;
      }
    } else {
      success = false;
    }
  }

  return success;
}

/****************************************************************************
 * plugin_get_config_value: Retrieve values like [<taskname>#<valuename>]
 ***************************************************************************/
bool P137_data_struct::plugin_get_config_value(struct EventStruct *event,
                                               String            & string) {
  bool   success = true;
  String command = parseString(string, 1);
  float  value;

  if (command == F("batvoltage")) {          // batvoltage
    value = read_value(P137_valueOptions_e::BatteryVoltage);
  } else if (command == F("batdischarge")) { // batdischarge
    value = read_value(P137_valueOptions_e::BatteryDischargeCurrent);
  } else if (command == F("batcharge")) {    // batcharge
    value = read_value(P137_valueOptions_e::BatteryChargeCurrent);
  } else if (command == F("batpower")) {     // batpower
    value = read_value(P137_valueOptions_e::BatteryPower);
  } else if (command == F("inpvoltage")) {   // inpvoltage
    value = read_value(P137_valueOptions_e::AcinVoltage);
  } else if (command == F("inpcurrent")) {   // inpcurrent
    value = read_value(P137_valueOptions_e::AcinCurrent);
  } else if (command == F("vbusvolt")) {     // vbusvolt
    value = read_value(P137_valueOptions_e::VbusVoltage);
  } else if (command == F("vbuscurr")) {     // vbuscurr
    value = read_value(P137_valueOptions_e::VbusCurrent);
  } else if (command == F("inttemp")) {      // inttemp
    value = read_value(P137_valueOptions_e::InternalTemperature);
  } else if (command == F("apsvolt")) {      // apsvolt
    value = read_value(P137_valueOptions_e::ApsVoltage);
  } else if (command == F("ldo2volt")) {     // ldo2volt
    value = read_value(P137_valueOptions_e::LDO2);
  } else if (command == F("ldo3volt")) {     // ldo3volt
    value = read_value(P137_valueOptions_e::LDO3);
  } else if (command == F("gpio0volt")) {    // gpio0volt
    value = read_value(P137_valueOptions_e::GPIO0);
  } else {
    success = false;
  }

  if (success) {
    string = String(value, static_cast<unsigned int>(P137_CONFIG_DECIMALS));
  }
  return success;
}

#endif // ifdef USES_P137
