#ifdef USES_P076
//#######################################################################################################
//#################### Plugin 076 HLW8012 AC Current and Voltage measurement
// sensor #####################
//#######################################################################################################
//
// This plugin is interfacing with HLW8012 IC which is use with some commercial
// devices:
// -- Sonoff POW
// -- ElectroDragon HLW8012 Breakout board
//
// The Sonoff POW uses the following PINs: SEL=GPIO05(D1), CF1=GPIO13(D8),
// CF=GPIO14(D5)
// The ED Module has pinheaders so any available PIN on the ESP8266 can be used.
//
// HLW8012 IC works with 5VDC (it seems at 3.3V is not stable in reading)
//

#include <HLW8012.h>
HLW8012 *Plugin_076_hlw = NULL;

#define PLUGIN_076
#define PLUGIN_ID_076 76
#define PLUGIN_076_DEBUG true // activate extra log info in the debug
#ifdef PLUGIN_SET_SONOFF_POW
#define PLUGIN_NAME_076 "Energy (AC) - HLW8012 (POW r1) [TESTING]"
#else
#define PLUGIN_NAME_076 "Energy (AC) - HLW8012 [TESTING]"
#endif
#define PLUGIN_VALUENAME1_076 "Voltage (V)"
#define PLUGIN_VALUENAME2_076 "Current (A)"
#define PLUGIN_VALUENAME3_076 "Active Power (W)"
#define PLUGIN_VALUENAME4_076 "Power Factor (%)"

//----------------- HLW8012 Default parameters
//--------------------------------------------------
// Set SEL_PIN to HIGH to sample current
// This is the case for Itead's Sonoff POW, where the SEL_PIN drives a
// transistor that pulls down
// the SEL pin in the HLW8012 when closed
#define HLW_CURRENT_MODE HIGH
#define HLW_DELAYREADING 500

// These are the nominal values for the resistors in the circuit
#define HLW_CURRENT_RESISTOR 0.001
#define HLW_VOLTAGE_RESISTOR_UP (5 * 470000) // Real: 2280k
#define HLW_VOLTAGE_RESISTOR_DOWN (1000)     // Real 1.009k
//-----------------------------------------------------------------------------------------------

byte StoredTaskIndex;
byte p076_read_stage = 0;
unsigned long p076_timer = 0;

double p076_hcurrent = 0.0;
unsigned int p076_hvoltage = 0;
unsigned int p076_hpower = 0;
unsigned int p076_hpowfact = 0;

boolean Plugin_076(byte function, struct EventStruct *event, String &string) {
  boolean success = false;

  switch (function) {
  case PLUGIN_DEVICE_ADD: {
    Device[++deviceCount].Number = PLUGIN_ID_076;
    Device[deviceCount].Type = DEVICE_TYPE_TRIPLE;
    Device[deviceCount].VType = SENSOR_TYPE_QUAD;
    Device[deviceCount].Ports = 0;
    Device[deviceCount].PullUpOption = false;
    Device[deviceCount].InverseLogicOption = false;
    Device[deviceCount].FormulaOption = true;
    Device[deviceCount].ValueCount = 4;
    Device[deviceCount].SendDataOption = true;
    Device[deviceCount].TimerOption = true;
    Device[deviceCount].GlobalSyncOption = false;
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
    event->String1 = formatGpioName_output("SEL");
    event->String2 = formatGpioName_input("CF1");
    event->String3 = formatGpioName_input("CF");
    break;
  }

  case PLUGIN_WEBFORM_LOAD: {
    addFormNote(
        F("Sonoff POW: 1st(SEL)=GPIO-5, 2nd(CF1)=GPIO-13, 3rd(CF)=GPIO-14"));
    addFormSubHeader(F("Calibration Values"));
    double hlwMultipliers[3];
    LoadCustomTaskSettings(event->TaskIndex, (byte *)&hlwMultipliers,
                           sizeof(hlwMultipliers));
    addFormTextBox(F("Current Multiplier"), F("p076_currmult"),
                   String(hlwMultipliers[0], 2), 25);
    addFormTextBox(F("Voltage Multiplier"), F("p076_voltmult"),
                   String(hlwMultipliers[1], 2), 25);
    addFormTextBox(F("Power Multiplier"), F("p076_powmult"),
                   String(hlwMultipliers[2], 2), 25);
    success = true;
    break;
  }

  case PLUGIN_WEBFORM_SAVE: {
    double hlwMultipliers[3];
    String tmpString, arg1;
    arg1 = F("p076_currmult");
    tmpString = WebServer.arg(arg1);
    hlwMultipliers[0] = atof(tmpString.c_str());
    arg1 = F("p076_voltmult");
    tmpString = WebServer.arg(arg1);
    hlwMultipliers[1] = atof(tmpString.c_str());
    arg1 = F("p076_powmult");
    tmpString = WebServer.arg(arg1);
    hlwMultipliers[2] = atof(tmpString.c_str());
    SaveCustomTaskSettings(event->TaskIndex, (byte *)&hlwMultipliers,
                           sizeof(hlwMultipliers));
    if (PLUGIN_076_DEBUG) {
      String log = F("HLW8012: Saved Calibration from Config Page");
      addLog(LOG_LEVEL_INFO, log);
    }
    if (Plugin_076_hlw) {
      Plugin_076_hlw->setCurrentMultiplier(hlwMultipliers[0]);
      Plugin_076_hlw->setVoltageMultiplier(hlwMultipliers[1]);
      Plugin_076_hlw->setPowerMultiplier(hlwMultipliers[2]);
    }
    if (PLUGIN_076_DEBUG) {
      String log = F("HLW8012: Multipliers Reassigned");
      addLog(LOG_LEVEL_INFO, log);
    }
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
          p076_hpower = Plugin_076_hlw->getActivePower();
          p076_hpowfact = (int)(100 * Plugin_076_hlw->getPowerFactor());
          ++p076_read_stage;
          // Measurement is done, schedule a new PLUGIN_READ call
          schedule_task_device_timer(event->TaskIndex, millis() + 10);
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
        ++p076_read_stage;
      } else if (p076_read_stage > 3) {
        // Measurement is complete.
        p076_read_stage = 0;
        if (PLUGIN_076_DEBUG) {
          String log = F("HLW8012: Read values");
          log += F(" - V=");
          log += p076_hvoltage;
          log += F(" - A=");
          log += p076_hcurrent;
          log += F(" - W=");
          log += p076_hpower;
          log += F(" - Pf%=");
          log += p076_hpowfact;
          addLog(LOG_LEVEL_INFO, log);
        }
        UserVar[event->BaseVarIndex] = p076_hvoltage;
        UserVar[event->BaseVarIndex + 1] = p076_hcurrent;
        UserVar[event->BaseVarIndex + 2] = p076_hpower;
        UserVar[event->BaseVarIndex + 3] = p076_hpowfact;
        // Plugin_076_hlw->toggleMode();
        success = true;
      }
    }
    break;

  case PLUGIN_INIT:
    if (!Plugin_076_hlw) {
      p076_read_stage = 0;
      Plugin_076_hlw = new HLW8012;
      // This initializes the HWL8012 library.
      const byte CF_PIN = Settings.TaskDevicePin3[event->TaskIndex];
      const byte CF1_PIN = Settings.TaskDevicePin2[event->TaskIndex];
      const byte SEL_PIN = Settings.TaskDevicePin1[event->TaskIndex];

      Plugin_076_hlw->begin(CF_PIN, CF1_PIN, SEL_PIN, HLW_CURRENT_MODE,
                            true); // set use_interrupts to true to use
                                   // interrupts to monitor pulse widths
      if (PLUGIN_076_DEBUG)
        addLog(LOG_LEVEL_INFO, F("HLW8012: Init object done"));
      Plugin_076_hlw->setResistors(HLW_CURRENT_RESISTOR,
                                   HLW_VOLTAGE_RESISTOR_UP,
                                   HLW_VOLTAGE_RESISTOR_DOWN);
      if (PLUGIN_076_DEBUG)
        addLog(LOG_LEVEL_INFO, F("HLW8012: Init Basic Resistor Values done"));
      // If multipliers are empty load default ones and save all of them as
      // "CustomTaskSettings"
      double hlwMultipliers[3];
      LoadCustomTaskSettings(event->TaskIndex, (byte *)&hlwMultipliers,
                             sizeof(hlwMultipliers));
      if (hlwMultipliers[0] == 0) {
        hlwMultipliers[0] = Plugin_076_hlw->getCurrentMultiplier();
      }
      if (hlwMultipliers[1] == 0) {
        hlwMultipliers[1] = Plugin_076_hlw->getVoltageMultiplier();
      }
      if (hlwMultipliers[2] == 0) {
        hlwMultipliers[2] = Plugin_076_hlw->getPowerMultiplier();
      }
      if (PLUGIN_076_DEBUG)
        addLog(LOG_LEVEL_INFO, F("HLW8012: Saved Calibration after INIT"));
      Plugin_076_hlw->setCurrentMultiplier(hlwMultipliers[0]);
      Plugin_076_hlw->setVoltageMultiplier(hlwMultipliers[1]);
      Plugin_076_hlw->setPowerMultiplier(hlwMultipliers[2]);
      if (PLUGIN_076_DEBUG)
        addLog(LOG_LEVEL_INFO, F("HLW8012: Applied Calibration after INIT"));
      StoredTaskIndex = event->TaskIndex; // store task index value in order to
                                          // use it in the PLUGIN_WRITE routine

      // Library expects an interrupt on both edges
      attachInterrupt(CF1_PIN, p076_hlw8012_cf1_interrupt, CHANGE);
      attachInterrupt(CF_PIN, p076_hlw8012_cf_interrupt, CHANGE);
    }
    success = true;
    break;


  case PLUGIN_WRITE:
    if (Plugin_076_hlw) {
      String tmpString = string;
      int argIndex = tmpString.indexOf(',');
      if (argIndex)
        tmpString = tmpString.substring(0, argIndex);

      if (tmpString.equalsIgnoreCase(F("hlwreset"))) {
        Plugin_076_hlw->resetMultipliers();
        Plugin076_SaveMultipliers();
        if (PLUGIN_076_DEBUG)
          addLog(LOG_LEVEL_INFO, F("HLW8012: Reset Multipliers to DEFAULT"));
        success = true;
      }

      if (tmpString.equalsIgnoreCase(F("hlwcalibrate"))) {
        String tmpStr = string;
        unsigned int CalibVolt = 0;
        double CalibCurr = 0;
        unsigned int CalibAcPwr = 0;
        int comma1 = tmpStr.indexOf(',');
        int comma2 = tmpStr.indexOf(',', comma1 + 1);
        int comma3 = tmpStr.indexOf(',', comma2 + 1);
        if (comma1 != 0) {
          if (comma2 == 0) {
            CalibVolt = tmpStr.substring(comma1 + 1).toInt();
          } else if (comma3 == 0) {
            CalibVolt = tmpStr.substring(comma1 + 1, comma2).toInt();
            CalibCurr = atof(tmpStr.substring(comma2 + 1).c_str());
          } else {
            CalibVolt = tmpStr.substring(comma1 + 1, comma2).toInt();
            CalibCurr = atof(tmpStr.substring(comma2 + 1, comma3).c_str());
            CalibAcPwr = tmpStr.substring(comma3 + 1).toInt();
          }
        }
        if (PLUGIN_076_DEBUG) {
          String log = F("HLW8012: Calibration to values");
          log += F(" - Expected-V=");
          log += CalibVolt;
          log += F(" - Expected-A=");
          log += CalibCurr;
          log += F(" - Expected-W=");
          log += CalibAcPwr;
          addLog(LOG_LEVEL_INFO, log);
        }
        if (CalibVolt != 0) {
          Plugin_076_hlw->expectedVoltage(CalibVolt);
        }
        if (CalibCurr != 0) {
          Plugin_076_hlw->expectedCurrent(CalibCurr);
        }
        if (CalibAcPwr != 0) {
          Plugin_076_hlw->expectedActivePower(CalibAcPwr);
        }
        // if at least one calibration value has been provided then save the new
        // multipliers //
        if ((CalibVolt + CalibCurr + CalibAcPwr) != 0) {
          Plugin076_SaveMultipliers();
        }
        success = true;
      }
    }
    break;

  }
  return success;
}

void Plugin076_SaveMultipliers() {
  double hlwMultipliers[3];
  hlwMultipliers[0] = Plugin_076_hlw->getCurrentMultiplier();
  hlwMultipliers[1] = Plugin_076_hlw->getVoltageMultiplier();
  hlwMultipliers[2] = Plugin_076_hlw->getPowerMultiplier();
  SaveCustomTaskSettings(StoredTaskIndex, (byte *)&hlwMultipliers,
                         sizeof(hlwMultipliers));
}

// When using interrupts we have to call the library entry point
// whenever an interrupt is triggered
void ICACHE_RAM_ATTR p076_hlw8012_cf1_interrupt() {
  if (Plugin_076_hlw) {
    Plugin_076_hlw->cf1_interrupt();
  }
}

void ICACHE_RAM_ATTR p076_hlw8012_cf_interrupt() {
  if (Plugin_076_hlw) {
    Plugin_076_hlw->cf_interrupt();
  }
}

#endif // USES_P076
