#ifdef USES_P076
//#######################################################################################################
//#################### Plugin 076 HLW8012 AC Current and Voltage measurement sensor #####################
//#######################################################################################################
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

#include <HLW8012.h>
HLW8012 *Plugin_076_hlw = NULL;

#define PLUGIN_076
#define PLUGIN_ID_076 76
#define PLUGIN_076_DEBUG true // activate extra log info in the debug
#define PLUGIN_NAME_076 "Energy (AC) - HLW8012/BL0937  [TESTING]"
#define PLUGIN_VALUENAME1_076 "Voltage"
#define PLUGIN_VALUENAME2_076 "Current"
#define PLUGIN_VALUENAME3_076 "Power"
#define PLUGIN_VALUENAME4_076 "PowerFactor"

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

struct p076_PredefinedDevice_struct {
  int Id;
  PGM_P Device_Name;
  byte SEL_Pin;
  byte CF_Pin;
  byte CF1_Pin;
  byte Current_Read;
  byte CF_Trigger;
  byte CF1_Trigger;

};

typedef struct p076_PredefinedDevice_struct P076_PredefinedDevice;

//When adding new device increase counter, too
const int p076_PinSettingsCount = 10;
const char string_Custom[]    PROGMEM = "Custom";
const char string_Sonoff[]    PROGMEM = "Sonoff Pow (r1)";
const char string_Huafan[]    PROGMEM = "Huafan SS";
const char string_KMC[]       PROGMEM = "KMC 70011";
const char string_Aplic[]     PROGMEM = "Aplic WDP303075";
const char string_SK03[]      PROGMEM = "SK03 Outdoor";
const char string_BlitzWolf[] PROGMEM = "BlitzWolf SHP";
const char string_Teckin[]    PROGMEM = "Teckin";
const char string_TeckinUS[]  PROGMEM = "Teckin US";
const char string_Gosund[]    PROGMEM = "Gosund SP1 v23";
static const P076_PredefinedDevice p076_PredefinedDevices[] PROGMEM =
{
   //Device_Name,SEL_PIN, CF_PIN, CF1_PIN, Current_Read, CF_Trigger, CF1_Trigger
   //HLW8012 Devices
   { 0, string_Custom,      0,      0,     0,       LOW,        LOW,    LOW},
   { 1, string_Sonoff,      5,     14,    13,      HIGH,     CHANGE, CHANGE},
   { 2, string_Huafan,     13,     14,    12,      HIGH,     CHANGE, CHANGE},
   { 3, string_KMC,        12,      4,     5,      HIGH,     CHANGE, CHANGE},
   { 4, string_Aplic,      12,      4,     5,       LOW,     CHANGE, CHANGE},
   { 5, string_SK03,       12,      4,     5,       LOW,     CHANGE, CHANGE},

   //BL093 Devices
   { 6, string_BlitzWolf,  12,      5,    14,       LOW,    FALLING, CHANGE},
   { 7, string_Teckin,     12,      4,     5,       LOW,    FALLING, CHANGE},
   { 8, string_TeckinUS,   12,      5,    14,       LOW,    FALLING, CHANGE},
   { 9, string_Gosund,     12,      4,     5,       LOW,    FALLING, CHANGE}
};


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

    byte currentRead = PCONFIG(4);
    byte cf_trigger  = PCONFIG(5);
    byte cf1_trigger = PCONFIG(6);

    byte devicePinSettings = PCONFIG(7);

    String predefinedNames[p076_PinSettingsCount];
    int predefinedId[p076_PinSettingsCount];

    for (int i = 0; i < p076_PinSettingsCount; i++)
    {
      predefinedNames[i] = p076_PredefinedDevices[i].Device_Name;
      predefinedId[i]    = p076_PredefinedDevices[i].Id;
    }

    String modeRaise[4];
    modeRaise[0] = F("LOW");
    modeRaise[1] = F("CHANGE");
    modeRaise[2] = F("RISING");
    modeRaise[3] = F("FALLING");

    int modeValues[4];
    modeValues[0] = LOW;
    modeValues[1] = CHANGE;
    modeValues[2] = RISING;
    modeValues[3] = FALLING;

    String modeCurr[2];
    modeCurr[0] = F("LOW");
    modeCurr[1] = F("HIGH");

    int modeCurrValues[2];
    modeCurrValues[0] = LOW;
    modeCurrValues[1] = HIGH;

    addFormSubHeader(F("Predefined Pin settings"));
    addFormSelector(F("Device"),
                    F("p076_preDefDevSel"), p076_PinSettingsCount,
                    predefinedNames, predefinedId, devicePinSettings );

    addFormSubHeader(F("Custom Pin settings (choose Custom above)"));
    addFormSelector(F("Current (A) Reading"), F("p076_curr_read"), 2,
                    modeCurr, modeCurrValues, currentRead );
    addFormSelector(F("CF Interrupt Edge"), F("p076_cf_edge"), 4,
                    modeRaise, modeValues, cf_trigger );
    addFormSelector(F("CF1  Interrupt Edge"), F("p076_cf1_edge"), 4,
                    modeRaise, modeValues, cf1_trigger);

    addFormSubHeader(F("Calibration Values"));
    double current, voltage, power;
    Plugin076_LoadMultipliers(event->TaskIndex, current, voltage, power);

    addFormTextBox(F("Current Multiplier"), F("p076_currmult"),
                   String(current, 2), 25);
    addFormTextBox(F("Voltage Multiplier"), F("p076_voltmult"),
                   String(voltage, 2), 25);
    addFormTextBox(F("Power Multiplier"), F("p076_powmult"),
                   String(power, 2), 25);

    success = true;
    break;
  }

  case PLUGIN_WEBFORM_SAVE: {

    //Set Multipliers
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
     String log = F("P076: Saved Calibration from Config Page");
     addLog(LOG_LEVEL_INFO, log);
    }

    if (Plugin_076_hlw) {
     Plugin_076_hlw->setCurrentMultiplier(hlwMultipliers[0]);
     Plugin_076_hlw->setVoltageMultiplier(hlwMultipliers[1]);
     Plugin_076_hlw->setPowerMultiplier(hlwMultipliers[2]);
    }

    if (PLUGIN_076_DEBUG) {
     String log = F("P076: Multipliers Reassigned");
     addLog(LOG_LEVEL_INFO, log);
    }

    //Set Pin settings
    byte selectedDevice = getFormItemInt(F("p076_preDefDevSel"));

    PCONFIG(7) = selectedDevice;

    if (selectedDevice == 0){
       PCONFIG(4) = getFormItemInt(F("p076_curr_read"));
       PCONFIG(5) = getFormItemInt(F("p076_cf_edge"));
       PCONFIG(6) = getFormItemInt(F("p076_cf1_edge"));
    }
    else if (selectedDevice < p076_PinSettingsCount){
      //Fetch Predefined Pin Setting from PROGMEM
      PCONFIG(4) = p076_PredefinedDevices[selectedDevice].Current_Read;
      PCONFIG(5) = p076_PredefinedDevices[selectedDevice].CF_Trigger;
      PCONFIG(6) = p076_PredefinedDevices[selectedDevice].CF1_Trigger;

      CONFIG_PIN1 = p076_PredefinedDevices[selectedDevice].SEL_Pin;
      CONFIG_PIN2 = p076_PredefinedDevices[selectedDevice].CF1_Pin;
      CONFIG_PIN3 = p076_PredefinedDevices[selectedDevice].CF_Pin;
    }

    if (PLUGIN_076_DEBUG) {
      String log = F("P076: PIN Settings ");

      log +=  " curr_read: ";
      log +=  PCONFIG(4);
      log +=  " cf_edge: ";
      log +=  PCONFIG(5);
      log +=  " cf1_edge: ";
      log +=  PCONFIG(6);
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
          String log = F("P076: Read values");
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

  case PLUGIN_EXIT: {
    Plugin076_Reset(event->TaskIndex);
    success = true;
    break;
  }

  case PLUGIN_INIT:
    Plugin076_Reset(event->TaskIndex);
    if (!Plugin_076_hlw) {
      p076_read_stage = 0;
      Plugin_076_hlw = new HLW8012;

      // This initializes the HWL8012 library.
      const byte CF_PIN = CONFIG_PIN3;
      const byte CF1_PIN = CONFIG_PIN2;
      const byte SEL_PIN = CONFIG_PIN1;

      byte currentRead = PCONFIG(4);
      byte cf_trigger  = PCONFIG(5);
      byte cf1_trigger = PCONFIG(6);

      Plugin_076_hlw->begin(CF_PIN, CF1_PIN, SEL_PIN, currentRead,
                            true); // set use_interrupts to true to use
                                   // interrupts to monitor pulse widths
      if (PLUGIN_076_DEBUG){
        addLog(LOG_LEVEL_INFO, F("P076: Init object done"));
      }
      Plugin_076_hlw->setResistors(HLW_CURRENT_RESISTOR,
                                   HLW_VOLTAGE_RESISTOR_UP,
                                   HLW_VOLTAGE_RESISTOR_DOWN);
      if (PLUGIN_076_DEBUG){
        addLog(LOG_LEVEL_INFO, F("P076: Init Basic Resistor Values done"));
      }

      double current, voltage, power;
      if (Plugin076_LoadMultipliers(event->TaskIndex, current, voltage, power)) {
        if (PLUGIN_076_DEBUG){
          addLog(LOG_LEVEL_INFO, F("P076: Saved Calibration after INIT"));
        }

        Plugin_076_hlw->setCurrentMultiplier(current);
        Plugin_076_hlw->setVoltageMultiplier(voltage);
        Plugin_076_hlw->setPowerMultiplier(power);
      } else {
        Plugin076_ResetMultipliers();
      }

      if (PLUGIN_076_DEBUG){
        addLog(LOG_LEVEL_INFO, F("P076: Applied Calibration after INIT"));
      }
      StoredTaskIndex = event->TaskIndex; // store task index value in order to
                                          // use it in the PLUGIN_WRITE routine

      // Library expects an interrupt on both edges
      attachInterrupt(CF1_PIN, p076_hlw8012_cf1_interrupt, cf1_trigger);
      attachInterrupt(CF_PIN, p076_hlw8012_cf_interrupt, cf_trigger);
    }
    success = true;
    break;


  case PLUGIN_WRITE:
    if (Plugin_076_hlw) {
      String tmpString = string;
      int argIndex = tmpString.indexOf(',');
      if (argIndex){
        tmpString = tmpString.substring(0, argIndex);
      }
      if (tmpString.equalsIgnoreCase(F("hlwreset"))) {
        Plugin076_ResetMultipliers();
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
          String log = F("P076: Calibration to values");
          log += F(" - Expected-V=");
          log += CalibVolt;
          log += F(" - Expected-A=");
          log += CalibCurr;
          log += F(" - Expected-W=");
          log += CalibAcPwr;
          addLog(LOG_LEVEL_INFO, log);
        }
        bool changed = false;
        if (CalibVolt != 0) {
          Plugin_076_hlw->expectedVoltage(CalibVolt);
          changed = true;
        }
        if (CalibCurr > 0.0) {
          Plugin_076_hlw->expectedCurrent(CalibCurr);
          changed = true;
        }
        if (CalibAcPwr != 0) {
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
    if (PLUGIN_076_DEBUG){
      addLog(LOG_LEVEL_INFO, F("P076: Reset Multipliers to DEFAULT"));
    }
  }
}

void Plugin076_SaveMultipliers() {
  double hlwMultipliers[3];
  Plugin076_ReadMultipliers(hlwMultipliers[0], hlwMultipliers[1], hlwMultipliers[2]);
  SaveCustomTaskSettings(StoredTaskIndex, (byte *)&hlwMultipliers,
                         sizeof(hlwMultipliers));
}

void Plugin076_ReadMultipliers(double& current, double& voltage, double& power) {
  current = 0.0;
  voltage = 0.0;
  power   = 0.0;
  if (Plugin_076_hlw) {
    current = Plugin_076_hlw->getCurrentMultiplier();
    voltage = Plugin_076_hlw->getVoltageMultiplier();
    power   = Plugin_076_hlw->getPowerMultiplier();
  }
}


bool Plugin076_LoadMultipliers(byte TaskIndex, double& current, double& voltage, double& power) {
  // If multipliers are empty load default ones and save all of them as
  // "CustomTaskSettings"
  Plugin076_ReadMultipliers(current, voltage, power);
  double hlwMultipliers[3];
  LoadCustomTaskSettings(TaskIndex, (byte *)&hlwMultipliers,
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

void Plugin076_Reset(byte TaskIndex) {
  if (Plugin_076_hlw) {
    delete Plugin_076_hlw;
    Plugin_076_hlw = nullptr;
  }
  const byte CF_PIN = Settings.TaskDevicePin3[TaskIndex];
  const byte CF1_PIN = Settings.TaskDevicePin2[TaskIndex];
  detachInterrupt(CF_PIN);
  detachInterrupt(CF1_PIN);
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
