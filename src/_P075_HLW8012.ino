#ifdef USES_P075
//#######################################################################################################
//#################### Plugin 075 HLW8012 AC Current and Voltage measurement sensor #####################
//#######################################################################################################
//
// This plugin is interfacing with HLW8012 IC which is use with some commercial devices:
// -- Sonoff POW
// -- ElectroDragon HLW8012 Breakout board
//
// The Sonoff POW uses the following PINs: SEL=GPIO05(D1), CF1=GPIO13(D8), CF=GPIO14(D5)
// The ED Module has pinheaders so any available PIN on the ESP8266 can be used.
//
// HLW8012 IC works with 5VDC (it seems at 3.3V is not stable in reading)
//


#include <HLW8012.h>
HLW8012 *Plugin_075_hlw;

#define PLUGIN_075
#define PLUGIN_ID_075        075
#define PLUGIN_075_DEBUG     true    //activate extra log info in the debug
#define PLUGIN_NAME_075       "Energy (AC) - HLW8012 [TESTING]"
#define PLUGIN_VALUENAME1_075 "Voltage (V)"
#define PLUGIN_VALUENAME2_075 "Current (A)"
#define PLUGIN_VALUENAME3_075 "Active Power (W)"
#define PLUGIN_VALUENAME4_075 "Power Factor (%)"

//----------------- HLW8012 Default parameters --------------------------------------------------
// Set SEL_PIN to HIGH to sample current
// This is the case for Itead's Sonoff POW, where the SEL_PIN drives a transistor that pulls down
// the SEL pin in the HLW8012 when closed
#define HLW_CURRENT_MODE         HIGH
#define HLW_DELAYREADING         500

// These are the nominal values for the resistors in the circuit
#define HLW_CURRENT_RESISTOR       0.001
#define HLW_VOLTAGE_RESISTOR_UP    ( 5 * 470000 )   // Real: 2280k
#define HLW_VOLTAGE_RESISTOR_DOWN  ( 1000 )         // Real 1.009k
//-----------------------------------------------------------------------------------------------

byte StoredTaskIndex;

boolean Plugin_075(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_075;
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

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_075);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_075));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_075));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_075));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_075));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormNote(string, F("Sonoff POW: 1st(SEL)=GPIO-5, 2nd(CF1)=GPIO-13, 3rd(CF)=GPIO-14"));
        addFormSubHeader(string, F("Calibration Values"));
        double hlwMultipliers[3];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&hlwMultipliers, sizeof(hlwMultipliers));
        addFormTextBox(string, F("Current Multiplier"), F("plugin_075_currmult"), String(hlwMultipliers[0], 2), 25);
        addFormTextBox(string, F("Voltage Multiplier"), F("plugin_075_voltmult"), String(hlwMultipliers[1], 2), 25);
        addFormTextBox(string, F("Power Multiplier"),   F("plugin_075_powmult"),  String(hlwMultipliers[2], 2), 25);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        double hlwMultipliers[3];
        String tmpString, arg1;
          arg1 = F("plugin_075_currmult"); tmpString = WebServer.arg(arg1);
          hlwMultipliers[0] = atof(tmpString.c_str());
          arg1 = F("plugin_075_voltmult"); tmpString = WebServer.arg(arg1);
          hlwMultipliers[1] = atof(tmpString.c_str());
          arg1 = F("plugin_075_powmult");  tmpString = WebServer.arg(arg1);
          hlwMultipliers[2] = atof(tmpString.c_str());
        SaveCustomTaskSettings(event->TaskIndex, (byte*)&hlwMultipliers, sizeof(hlwMultipliers));
        if (PLUGIN_075_DEBUG) {
          String log = F("HLW8012: Saved Calibration from Config Page");
          addLog(LOG_LEVEL_INFO, log);
        }
        if (Plugin_075_hlw) {
          Plugin_075_hlw->setCurrentMultiplier(hlwMultipliers[0]);
          Plugin_075_hlw->setVoltageMultiplier(hlwMultipliers[1]);
          Plugin_075_hlw->setPowerMultiplier(hlwMultipliers[2]);
        }
        if (PLUGIN_075_DEBUG) {
          String log = F("HLW8012: Multipliers Reassigned");
          addLog(LOG_LEVEL_INFO, log);
        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        Plugin_075_hlw->setMode(MODE_CURRENT); delay(HLW_DELAYREADING); double       hcurrent  = Plugin_075_hlw->getCurrent();
        Plugin_075_hlw->setMode(MODE_VOLTAGE); delay(HLW_DELAYREADING); unsigned int hvoltage  = Plugin_075_hlw->getVoltage();
        unsigned int hpower    = Plugin_075_hlw->getActivePower();
        //unsigned int happpower = Plugin_075_hlw->getApparentPower();
        unsigned int hpowfact  = (int) (100 * Plugin_075_hlw->getPowerFactor());
        if (PLUGIN_075_DEBUG) {
          String log = F("HLW8012: Read values");
          log += F(" - V="); log += hvoltage;
          log += F(" - A="); log += hcurrent;
          log += F(" - W="); log += hpower;
          log += F(" - Pf%="); log += hpowfact;
          addLog(LOG_LEVEL_INFO, log);
        }
        UserVar[event->BaseVarIndex]     = hvoltage;
        UserVar[event->BaseVarIndex + 1] = hcurrent;
        UserVar[event->BaseVarIndex + 2] = hpower;
        UserVar[event->BaseVarIndex + 3] = hpowfact;
        //Plugin_075_hlw->toggleMode();
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (!Plugin_075_hlw)
        {
          Plugin_075_hlw = new HLW8012;
          // This initializes the HWL8012 library.
          Plugin_075_hlw->begin(Settings.TaskDevicePin3[event->TaskIndex], Settings.TaskDevicePin2[event->TaskIndex], Settings.TaskDevicePin1[event->TaskIndex], HLW_CURRENT_MODE, false, 1000000);
          if (PLUGIN_075_DEBUG) addLog(LOG_LEVEL_INFO, F("HLW8012: Init object done"));
          Plugin_075_hlw->setResistors(HLW_CURRENT_RESISTOR, HLW_VOLTAGE_RESISTOR_UP, HLW_VOLTAGE_RESISTOR_DOWN);
          if (PLUGIN_075_DEBUG) addLog(LOG_LEVEL_INFO, F("HLW8012: Init Basic Resistor Values done"));
          // If multipliers are empty load default ones and save all of them as "CustomTaskSettings"
          double hlwMultipliers[3];
          LoadCustomTaskSettings(event->TaskIndex, (byte*)&hlwMultipliers, sizeof(hlwMultipliers));
            if (hlwMultipliers[0] == 0) { hlwMultipliers[0] = Plugin_075_hlw->getCurrentMultiplier(); }
            if (hlwMultipliers[1] == 0) { hlwMultipliers[1] = Plugin_075_hlw->getVoltageMultiplier(); }
            if (hlwMultipliers[2] == 0) { hlwMultipliers[2] = Plugin_075_hlw->getPowerMultiplier();   }
          SaveCustomTaskSettings(event->TaskIndex, (byte*)&hlwMultipliers, sizeof(hlwMultipliers));
          if (PLUGIN_075_DEBUG) addLog(LOG_LEVEL_INFO, F("HLW8012: Saved Calibration after INIT"));
          Plugin_075_hlw->setCurrentMultiplier(hlwMultipliers[0]);
          Plugin_075_hlw->setVoltageMultiplier(hlwMultipliers[1]);
          Plugin_075_hlw->setPowerMultiplier(hlwMultipliers[2]);
          if (PLUGIN_075_DEBUG) addLog(LOG_LEVEL_INFO, F("HLW8012: Applied Calibration after INIT"));
          StoredTaskIndex = event->TaskIndex; // store task index value in order to use it in the PLUGIN_WRITE routine
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if (Plugin_075_hlw)
        {
          String tmpString  = string;
          int argIndex = tmpString.indexOf(',');
          if (argIndex)
            tmpString = tmpString.substring(0, argIndex);

          if (tmpString.equalsIgnoreCase(F("hlwreset")))
          {
            Plugin_075_hlw->resetMultipliers();
            Plugin075_SaveMultipliers();
            if (PLUGIN_075_DEBUG) addLog(LOG_LEVEL_INFO, F("HLW8012: Reset Multipliers to DEFAULT"));
            success = true;
          }

          if (tmpString.equalsIgnoreCase(F("hlwcalibrate")))
          {
            String tmpStr = string;
            unsigned int CalibVolt = 0;
            double       CalibCurr = 0;
            unsigned int CalibAcPwr = 0;
            int comma1 = tmpStr.indexOf(',');
            int comma2 = tmpStr.indexOf(',', comma1+1);
            int comma3 = tmpStr.indexOf(',', comma2+1);
            if (comma1 != 0) {
              if (comma2 == 0) {
                CalibVolt  = tmpStr.substring(comma1+1).toInt();
              } else if (comma3 == 0) {
                CalibVolt  = tmpStr.substring(comma1+1, comma2).toInt();
                CalibCurr  = atof(tmpStr.substring(comma2+1).c_str());
              } else {
                CalibVolt  = tmpStr.substring(comma1+1, comma2).toInt();
                CalibCurr  = atof(tmpStr.substring(comma2+1, comma3).c_str());
                CalibAcPwr = tmpStr.substring(comma3+1).toInt();
              }
            }
            if (PLUGIN_075_DEBUG) {
              String log = F("HLW8012: Calibration to values");
              log += F(" - Expected-V="); log += CalibVolt;
              log += F(" - Expected-A="); log += CalibCurr;
              log += F(" - Expected-W="); log += CalibAcPwr;
              addLog(LOG_LEVEL_INFO, log);
            }
            if (CalibVolt  != 0) { Plugin_075_hlw->expectedVoltage(CalibVolt); }
            if (CalibCurr  != 0) { Plugin_075_hlw->expectedCurrent(CalibCurr); }
            if (CalibAcPwr != 0) { Plugin_075_hlw->expectedActivePower(CalibAcPwr); }
            // if at least one calibration value has been provided then save the new multipliers //
            if ((CalibVolt + CalibCurr + CalibAcPwr) != 0) { Plugin075_SaveMultipliers(); }
            success = true;
          }
        }
        break;
      }

  }
  return success;
}

void Plugin075_SaveMultipliers() {
    double hlwMultipliers[3];
    hlwMultipliers[0] = Plugin_075_hlw->getCurrentMultiplier();
    hlwMultipliers[1] = Plugin_075_hlw->getVoltageMultiplier();
    hlwMultipliers[2] = Plugin_075_hlw->getPowerMultiplier();
    SaveCustomTaskSettings(StoredTaskIndex, (byte*)&hlwMultipliers, sizeof(hlwMultipliers));
}

#endif // USES_P075
