#include "_Plugin_Helper.h"
#ifdef USES_P076

// #######################################################################################################
// #################### Plugin 076 HLW8012 AC Current and Voltage measurement sensor #####################
// #######################################################################################################
//
// This plugin is interfacing with HLW8012 and BL0937 IC which is use with some
// commercial devices like:
// -- Sonoff POW
// -- ElectroDragon HLW8012 Breakout board
// --- and more
//
// The Sonoff POW uses the following PINs: SEL=GPIO05(D1), CF1=GPIO13(D8),
// CF=GPIO14(D5)
// The ED Module has pinheaders so any available PIN on the ESP8266 can be used.
//
// HLW8012 IC works with 5VDC (it seems at 3.3V is not stable in reading)
//

/** Changelog:
 * 2023-01-03 tonhuisman: Uncrustify source, apply some code improvements
 *                        Older changelog not registered.
 */

# include <HLW8012.h>

HLW8012 *Plugin_076_hlw = nullptr;

# define PLUGIN_076
# define PLUGIN_ID_076 76
# define PLUGIN_076_DEBUG true // activate extra log info in the debug
# define PLUGIN_NAME_076 "Energy (AC) - HLW8012/BL0937"
# define PLUGIN_VALUENAME1_076 "Voltage"
# define PLUGIN_VALUENAME2_076 "Current"
# define PLUGIN_VALUENAME3_076 "Power"
# define PLUGIN_VALUENAME4_076 "PowerFactor"

# define HLW_DELAYREADING 500

// These are the nominal values for the resistors in the circuit
# define HLW_CURRENT_RESISTOR 0.001
# define HLW_VOLTAGE_RESISTOR_UP (5 * 470000) // Real: 2280k
# define HLW_VOLTAGE_RESISTOR_DOWN (1000)     // Real 1.009k
// -----------------------------------------------------------------------------------------------
int StoredTaskIndex = -1;
uint8_t p076_read_stage  = 0;
unsigned long p076_timer = 0;

float p076_hcurrent = 0.0f;
float p076_hvoltage = 0.0f;
float p076_hpower   = 0.0f;
float p076_hpowfact = 0.0f;

# define P076_Custom       0

// HLW8012 Devices
# define P076_Sonoff       1
# define P076_Huafan       2
# define P076_KMC          3
# define P076_Aplic        4
# define P076_SK03         5

// BL093 Devices
# define P076_BlitzWolf    6
# define P076_Teckin       7
# define P076_TeckinUS     8
# define P076_Gosund       9
# define P076_Shelly_PLUG_S 10

// Keep values as they are stored and increase this when adding new ones.
# define MAX_P076_DEVICE   11

void IRAM_ATTR p076_hlw8012_cf1_interrupt();
void IRAM_ATTR p076_hlw8012_cf_interrupt();


bool           p076_getDeviceString(int device, String& name) {
  switch (device) {
    case P076_Custom: name        = F("Custom");          break;
    case P076_Sonoff: name        = F("Sonoff Pow (r1)"); break;
    case P076_Huafan: name        = F("Huafan SS");       break;
    case P076_KMC: name           = F("KMC 70011");       break;
    case P076_Aplic: name         = F("Aplic WDP303075"); break;
    case P076_SK03: name          = F("SK03 Outdoor");    break;
    case P076_BlitzWolf: name     = F("BlitzWolf SHP");   break;
    case P076_Teckin: name        = F("Teckin");          break;
    case P076_TeckinUS: name      = F("Teckin US");       break;
    case P076_Gosund: name        = F("Gosund SP1 v23");  break;
    case P076_Shelly_PLUG_S: name = F("Shelly PLUG-S");   break;
    default:
      return false;
  }
  return true;
}

bool p076_getDeviceParameters(int      device,
                              uint8_t& SEL_Pin,
                              uint8_t& CF_Pin,
                              uint8_t& CF1_Pin,
                              uint8_t& Cur_read,
                              uint8_t& CF_Trigger,
                              uint8_t& CF1_Trigger) {
  switch (device) {
    case P076_Custom: SEL_Pin        =  0; CF_Pin =  0; CF1_Pin =  0; Cur_read =  LOW; CF_Trigger =     LOW; CF1_Trigger =    LOW; break;
    case P076_Sonoff: SEL_Pin        =  5; CF_Pin = 14; CF1_Pin = 13; Cur_read = HIGH; CF_Trigger =  CHANGE; CF1_Trigger = CHANGE; break;
    case P076_Huafan: SEL_Pin        = 13; CF_Pin = 14; CF1_Pin = 12; Cur_read = HIGH; CF_Trigger =  CHANGE; CF1_Trigger = CHANGE; break;
    case P076_KMC: SEL_Pin           = 12; CF_Pin =  4; CF1_Pin =  5; Cur_read = HIGH; CF_Trigger =  CHANGE; CF1_Trigger = CHANGE; break;
    case P076_Aplic: SEL_Pin         = 12; CF_Pin =  4; CF1_Pin =  5; Cur_read =  LOW; CF_Trigger =  CHANGE; CF1_Trigger = CHANGE; break;
    case P076_SK03: SEL_Pin          = 12; CF_Pin =  4; CF1_Pin =  5; Cur_read =  LOW; CF_Trigger =  CHANGE; CF1_Trigger = CHANGE; break;
    case P076_BlitzWolf: SEL_Pin     = 12; CF_Pin =  5; CF1_Pin = 14; Cur_read =  LOW; CF_Trigger = FALLING; CF1_Trigger = CHANGE; break;
    case P076_Teckin: SEL_Pin        = 12; CF_Pin =  4; CF1_Pin =  5; Cur_read =  LOW; CF_Trigger = FALLING; CF1_Trigger = CHANGE; break;
    case P076_TeckinUS: SEL_Pin      = 12; CF_Pin =  5; CF1_Pin = 14; Cur_read =  LOW; CF_Trigger = FALLING; CF1_Trigger = CHANGE; break;
    case P076_Gosund: SEL_Pin        = 12; CF_Pin =  4; CF1_Pin =  5; Cur_read =  LOW; CF_Trigger = FALLING; CF1_Trigger = CHANGE; break;
    case P076_Shelly_PLUG_S: SEL_Pin = 12; CF_Pin =  5; CF1_Pin = 14; Cur_read =  LOW; CF_Trigger = FALLING; CF1_Trigger = CHANGE; break;
    default:
      return false;
  }
  return true;
}

boolean Plugin_076(uint8_t function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_076;
      Device[deviceCount].Type               = DEVICE_TYPE_TRIPLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = false;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_076);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0],
               PSTR(PLUGIN_VALUENAME1_076));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1],
               PSTR(PLUGIN_VALUENAME2_076));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2],
               PSTR(PLUGIN_VALUENAME3_076));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3],
               PSTR(PLUGIN_VALUENAME4_076));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
      event->String1 = formatGpioName_output(F("SEL"));
      event->String2 = formatGpioName_input(F("CF1"));
      event->String3 = formatGpioName_input(F("CF"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      uint8_t devicePinSettings = PCONFIG(7);

      addFormSubHeader(F("Predefined Pin settings"));
      {
        // Place this in a scope, to keep memory usage low.
        String predefinedNames[MAX_P076_DEVICE];
        int    predefinedId[MAX_P076_DEVICE];

        int index = 0;

        for (int i = 0; i < MAX_P076_DEVICE; i++)
        {
          if (p076_getDeviceString(i, predefinedNames[index])) {
            predefinedId[index] = i;
            ++index;
          }
        }
        addFormSelector(F("Device"),
                        F("preDefDevSel"), index,
                        predefinedNames, predefinedId, devicePinSettings);
        addFormNote(F("Enable device and select device type first"));
      }

      {
        // Place this in a scope, to keep memory usage low.
        const __FlashStringHelper *modeRaise[4] = {
          F("LOW"),
          F("CHANGE"),
          F("RISING"),
          F("FALLING"),
        };

        const int modeValues[4] = {
          LOW,
          CHANGE,
          RISING,
          FALLING,
        };

        const __FlashStringHelper *modeCurr[2] = {
          F("LOW"),
          F("HIGH"),
        };

        const int modeCurrValues[2] = {
          LOW,
          HIGH,
        };

        uint8_t currentRead = PCONFIG(4);

        if ((currentRead != LOW) && (currentRead != HIGH)) {
          currentRead = LOW;
        }
        addFormSubHeader(F("Custom Pin settings (choose Custom above)"));
        addFormSelector(F("SEL Current (A) Reading"), F("curr_read"), 2,
                        modeCurr, modeCurrValues, currentRead);
        addFormSelector(F("CF1  Interrupt Edge"),     F("cf1_edge"),  4,
                        modeRaise, modeValues, PCONFIG(6));
        addFormSelector(F("CF Interrupt Edge"),       F("cf_edge"),   4,
                        modeRaise, modeValues, PCONFIG(5));
      }


      double current, voltage, power;

      if (Plugin076_LoadMultipliers(event->TaskIndex, current, voltage, power)) {
        addFormSubHeader(F("Calibration Values"));
        addFormTextBox(F("Current Multiplier"), F("currmult"),
                       doubleToString(current, 2), 25);
        addFormTextBox(F("Voltage Multiplier"), F("voltmult"),
                       doubleToString(voltage, 2), 25);
        addFormTextBox(F("Power Multiplier"), F("powmult"),
                       doubleToString(power, 2), 25);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      // Set Pin settings
      uint8_t selectedDevice = getFormItemInt(F("preDefDevSel"));

      PCONFIG(7) = selectedDevice;
      {
        uint8_t SEL_Pin, CF_Pin, CF1_Pin, Cur_read, CF_Trigger, CF1_Trigger;

        if ((selectedDevice != 0) && p076_getDeviceParameters(selectedDevice, SEL_Pin, CF_Pin, CF1_Pin, Cur_read, CF_Trigger, CF1_Trigger)) {
          PCONFIG(4) = Cur_read;
          PCONFIG(5) = CF_Trigger;
          PCONFIG(6) = CF1_Trigger;

          CONFIG_PIN1 = SEL_Pin;
          CONFIG_PIN2 = CF1_Pin;
          CONFIG_PIN3 = CF_Pin;
        } else {
          PCONFIG(4) = getFormItemInt(F("curr_read"));
          PCONFIG(5) = getFormItemInt(F("cf_edge"));
          PCONFIG(6) = getFormItemInt(F("cf1_edge"));
        }
      }

      // Set Multipliers
      double hlwMultipliers[3];
      hlwMultipliers[0] = getFormItemFloat(F("currmult"));
      hlwMultipliers[1] = getFormItemFloat(F("voltmult"));
      hlwMultipliers[2] = getFormItemFloat(F("powmult"));

      if ((hlwMultipliers[0] > 1.0) && (hlwMultipliers[1] > 1.0) && (hlwMultipliers[2] > 1.0)) {
        SaveCustomTaskSettings(event->TaskIndex, reinterpret_cast<const uint8_t *>(&hlwMultipliers),
                               sizeof(hlwMultipliers));
        # ifndef BUILD_NO_DEBUG

        if (PLUGIN_076_DEBUG) {
          addLog(LOG_LEVEL_INFO, F("P076: Saved Calibration from Config Page"));
        }
        # endif // ifndef BUILD_NO_DEBUG

        if (Plugin_076_hlw) {
          Plugin_076_hlw->setCurrentMultiplier(hlwMultipliers[0]);
          Plugin_076_hlw->setVoltageMultiplier(hlwMultipliers[1]);
          Plugin_076_hlw->setPowerMultiplier(hlwMultipliers[2]);
        }

        # ifndef BUILD_NO_DEBUG

        if (PLUGIN_076_DEBUG) {
          addLog(LOG_LEVEL_INFO, F("P076: Multipliers Reassigned"));
        }
        # endif // ifndef BUILD_NO_DEBUG
      }

      # ifndef BUILD_NO_DEBUG

      if (PLUGIN_076_DEBUG) {
        String log = F("P076: PIN Settings ");

        log +=  F(" curr_read: ");
        log +=  PCONFIG(4);
        log +=  F(" cf_edge: ");
        log +=  PCONFIG(5);
        log +=  F(" cf1_edge: ");
        log +=  PCONFIG(6);
        addLogMove(LOG_LEVEL_INFO, log);
      }
      # endif // ifndef BUILD_NO_DEBUG

      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:

      if (Plugin_076_hlw) {
        switch (p076_read_stage) {
          case 0:
            // The stage where we have to wait for a measurement to be started.
            break;
          case 1: // Set mode to read current
            Plugin_076_hlw->setMode(MODE_CURRENT);
            p076_timer = millis() + HLW_DELAYREADING;
            ++p076_read_stage;
            break;
          case 2: // Read current + set mode to read voltage

            if (timeOutReached(p076_timer)) {
              p076_hcurrent = Plugin_076_hlw->getCurrent();
              Plugin_076_hlw->setMode(MODE_VOLTAGE);
              p076_timer = millis() + HLW_DELAYREADING;
              ++p076_read_stage;
            }
            break;
          case 3: // Read voltage + active power + power factor

            if (timeOutReached(p076_timer)) {
              p076_hvoltage = Plugin_076_hlw->getVoltage();
              p076_hpower   = Plugin_076_hlw->getActivePower();
              p076_hpowfact = static_cast<int>(100 * Plugin_076_hlw->getPowerFactor());
              ++p076_read_stage;

              // Measurement is done, schedule a new PLUGIN_READ call
              Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
            }
            break;
          default:
            // The PLUGIN_READ should set the new timer and reset the read stage.
            break;
        }
      }
      success = true;
      break;

    case PLUGIN_READ:

      if (Plugin_076_hlw) {
        if (p076_read_stage == 0) {
          // Force a measurement start.
          //        ++p076_read_stage;
          //      } else if (p076_read_stage > 3) {
          p076_hpower   = Plugin_076_hlw->getActivePower();
          p076_hvoltage = Plugin_076_hlw->getVoltage();
          p076_hcurrent = Plugin_076_hlw->getCurrent();
          p076_hpowfact = static_cast<int>(100 * Plugin_076_hlw->getPowerFactor());

          // Measurement is complete.
          p076_read_stage = 0;
          # ifndef BUILD_NO_DEBUG

          if (PLUGIN_076_DEBUG) {
            String log = F("P076: Read values");
            log += F(" - V=");
            log += p076_hvoltage;
            log += F(" - A=");
            log += p076_hcurrent;
            log += F(" - W=");
            log += p076_hpower;
            log += F(" - Pf%=");
            log += p076_hpowfact;
            addLogMove(LOG_LEVEL_INFO, log);
          }
          # endif // ifndef BUILD_NO_DEBUG
          UserVar[event->BaseVarIndex]     = p076_hvoltage;
          UserVar[event->BaseVarIndex + 1] = p076_hcurrent;
          UserVar[event->BaseVarIndex + 2] = p076_hpower;
          UserVar[event->BaseVarIndex + 3] = p076_hpowfact;

          // Plugin_076_hlw->toggleMode();
          success = true;
        }
      }
      break;

    case PLUGIN_EXIT: {
      Plugin076_Reset(event->TaskIndex);
      success = true;
      break;
    }

    case PLUGIN_INIT: {
      Plugin076_Reset(event->TaskIndex);

      // This initializes the HWL8012 library.
      const uint8_t CF_PIN  = CONFIG_PIN3;
      const uint8_t CF1_PIN = CONFIG_PIN2;
      const uint8_t SEL_PIN = CONFIG_PIN1;

      if (validGpio(CF_PIN) && validGpio(CF1_PIN) && validGpio(SEL_PIN)) {
        Plugin_076_hlw = new (std::nothrow) HLW8012;

        if (Plugin_076_hlw) {
          uint8_t currentRead = PCONFIG(4);
          uint8_t cf_trigger  = PCONFIG(5);
          uint8_t cf1_trigger = PCONFIG(6);

          Plugin_076_hlw->begin(CF_PIN, CF1_PIN, SEL_PIN, currentRead,
                                true); // set use_interrupts to true to use
                                       // interrupts to monitor pulse widths
          # ifndef BUILD_NO_DEBUG

          if (PLUGIN_076_DEBUG) {
            addLog(LOG_LEVEL_INFO, F("P076: Init object done"));
          }
          # endif // ifndef BUILD_NO_DEBUG
          Plugin_076_hlw->setResistors(HLW_CURRENT_RESISTOR,
                                       HLW_VOLTAGE_RESISTOR_UP,
                                       HLW_VOLTAGE_RESISTOR_DOWN);
          # ifndef BUILD_NO_DEBUG

          if (PLUGIN_076_DEBUG) {
            addLog(LOG_LEVEL_INFO, F("P076: Init Basic Resistor Values done"));
          }
          # endif // ifndef BUILD_NO_DEBUG

          double current, voltage, power;

          if (Plugin076_LoadMultipliers(event->TaskIndex, current, voltage, power)) {
            # ifndef BUILD_NO_DEBUG

            if (PLUGIN_076_DEBUG) {
              addLog(LOG_LEVEL_INFO, F("P076: Saved Calibration after INIT"));
            }
            # endif // ifndef BUILD_NO_DEBUG

            Plugin_076_hlw->setCurrentMultiplier(current);
            Plugin_076_hlw->setVoltageMultiplier(voltage);
            Plugin_076_hlw->setPowerMultiplier(power);
          } else {
            Plugin076_ResetMultipliers();
          }

          # ifndef BUILD_NO_DEBUG

          if (PLUGIN_076_DEBUG) {
            addLog(LOG_LEVEL_INFO, F("P076: Applied Calibration after INIT"));
          }
          # endif // ifndef BUILD_NO_DEBUG
          StoredTaskIndex = event->TaskIndex; // store task index value in order to
                                              // use it in the PLUGIN_WRITE routine

          // Library expects an interrupt on both edges
          attachInterrupt(CF1_PIN, p076_hlw8012_cf1_interrupt, cf1_trigger);
          attachInterrupt(CF_PIN,  p076_hlw8012_cf_interrupt,  cf_trigger);

          success = true;
        }
      }
      break;
    }

    case PLUGIN_WRITE:

      if (Plugin_076_hlw) {
        String command = parseString(string, 1);

        if (command.equalsIgnoreCase(F("hlwreset"))) {
          Plugin076_ResetMultipliers();
          success = true;
        }

        if (command.equalsIgnoreCase(F("hlwcalibrate"))) {
          float CalibVolt  = 0.0f;
          float CalibCurr  = 0.0f;
          float CalibAcPwr = 0.0f;

          if (validFloatFromString(parseString(string, 2), CalibVolt)) {
            if (validFloatFromString(parseString(string, 3), CalibCurr)) {
              validFloatFromString(parseString(string, 4), CalibAcPwr);
            }
          }
          # ifndef BUILD_NO_DEBUG

          if (PLUGIN_076_DEBUG) {
            String log = F("P076: Calibration to values");
            log += F(" - Expected-V=");
            log += CalibVolt;
            log += F(" - Expected-A=");
            log += CalibCurr;
            log += F(" - Expected-W=");
            log += CalibAcPwr;
            addLogMove(LOG_LEVEL_INFO, log);
          }
          # endif // ifndef BUILD_NO_DEBUG
          bool changed = false;

          if (CalibVolt != 0) {
            Plugin_076_hlw->expectedVoltage(CalibVolt);
            changed = true;
          }

          if (definitelyGreaterThan(CalibCurr, 0.0f)) {
            Plugin_076_hlw->expectedCurrent(CalibCurr);
            changed = true;
          }

          if (!essentiallyEqual(CalibAcPwr, 0.0f)) {
            Plugin_076_hlw->expectedActivePower(CalibAcPwr);
            changed = true;
          }

          // if at least one calibration value has been provided then save the new
          // multipliers //
          if (changed) {
            Plugin076_SaveMultipliers();
          }
          success = true;
        }
      }
      break;
  }
  return success;
}

void Plugin076_ResetMultipliers() {
  if (Plugin_076_hlw) {
    Plugin_076_hlw->resetMultipliers();
    Plugin076_SaveMultipliers();
    # ifndef BUILD_NO_DEBUG

    if (PLUGIN_076_DEBUG) {
      addLog(LOG_LEVEL_INFO, F("P076: Reset Multipliers to DEFAULT"));
    }
    # endif // ifndef BUILD_NO_DEBUG
  }
}

void Plugin076_SaveMultipliers() {
  if (StoredTaskIndex < 0) {
    return; // Not yet initialized.
  }
  double hlwMultipliers[3]{};

  if (Plugin076_ReadMultipliers(hlwMultipliers[0], hlwMultipliers[1], hlwMultipliers[2])) {
    SaveCustomTaskSettings(StoredTaskIndex, reinterpret_cast<const uint8_t *>(&hlwMultipliers),
                           sizeof(hlwMultipliers));
  }
}

bool Plugin076_ReadMultipliers(double& current, double& voltage, double& power) {
  current = 0.0f;
  voltage = 0.0f;
  power   = 0.0f;

  if (Plugin_076_hlw) {
    current = Plugin_076_hlw->getCurrentMultiplier();
    voltage = Plugin_076_hlw->getVoltageMultiplier();
    power   = Plugin_076_hlw->getPowerMultiplier();
    return true;
  }
  return false;
}

bool Plugin076_LoadMultipliers(taskIndex_t TaskIndex, double& current, double& voltage, double& power) {
  // If multipliers are empty load default ones and save all of them as
  // "CustomTaskSettings"
  if (!Plugin076_ReadMultipliers(current, voltage, power)) {
    return false;
  }
  double hlwMultipliers[3];

  LoadCustomTaskSettings(TaskIndex, reinterpret_cast<uint8_t *>(&hlwMultipliers),
                         sizeof(hlwMultipliers));

  if (hlwMultipliers[0] > 1.0) {
    current = hlwMultipliers[0];
  }

  if (hlwMultipliers[1] > 1.0) {
    voltage = hlwMultipliers[1];
  }

  if (hlwMultipliers[2] > 1.0) {
    power = hlwMultipliers[2];
  }
  return (current > 1.0) && (voltage > 1.0) && (power > 1.0);
}

void Plugin076_Reset(taskIndex_t TaskIndex) {
  if (Plugin_076_hlw) {
    const uint8_t CF_PIN  = Settings.TaskDevicePin3[TaskIndex];
    const uint8_t CF1_PIN = Settings.TaskDevicePin2[TaskIndex];
    detachInterrupt(CF_PIN);
    detachInterrupt(CF1_PIN);
    delete Plugin_076_hlw;
    Plugin_076_hlw = nullptr;
  }
  StoredTaskIndex = -1;
  p076_read_stage = 0;
  p076_timer      = 0;

  p076_hcurrent = 0.0f;
  p076_hvoltage = 0.0f;
  p076_hpower   = 0.0f;
  p076_hpowfact = 0.0f;
}

// When using interrupts we have to call the library entry point
// whenever an interrupt is triggered
void IRAM_ATTR p076_hlw8012_cf1_interrupt() {
  if (Plugin_076_hlw) {
    Plugin_076_hlw->cf1_interrupt();
  }
}

void IRAM_ATTR p076_hlw8012_cf_interrupt() {
  if (Plugin_076_hlw) {
    Plugin_076_hlw->cf_interrupt();
  }
}

#endif // USES_P076
