#ifdef USES_P078

//#######################################################################################################
//######################## Plugin 078: SDM120C/220T/230/630 Eastron Energy Meter ########################
//#######################################################################################################
/*
  Plugin written by: Sergio Faustino sjfaustino__AT__gmail.com

  This plugin reads available values of an Eastron SDM120C Energy Meter.
  It will also work with all the other superior model such as SDM220T, SDM230 AND SDM630 series.
*/

#define PLUGIN_078
#define PLUGIN_ID_078         78
#define PLUGIN_NAME_078       "Energy (AC) - Eastron SDM120C/220T/230/630 [TESTING]"

#define P078_DEV_ID   Settings.TaskDevicePluginConfig[event->TaskIndex][0]
#define P078_MODEL    Settings.TaskDevicePluginConfig[event->TaskIndex][1]
#define P078_BAUDRATE Settings.TaskDevicePluginConfig[event->TaskIndex][2]
#define P078_QUERY1   Settings.TaskDevicePluginConfig[event->TaskIndex][3]
#define P078_QUERY2   Settings.TaskDevicePluginConfig[event->TaskIndex][4]
#define P078_QUERY3   Settings.TaskDevicePluginConfig[event->TaskIndex][5]
#define P078_QUERY4   Settings.TaskDevicePluginConfig[event->TaskIndex][6]
#define P078_DEPIN    Settings.TaskDevicePin3[event->TaskIndex]

#define P078_DEV_ID_DFLT     1
#define P078_MODEL_DFLT      0  // SDM120C
#define P078_BAUDRATE_DFLT   1  // 9600 baud
#define P078_QUERY1_DFLT     0  // Voltage (V)
#define P078_QUERY2_DFLT     1  // Current (A)
#define P078_QUERY3_DFLT     2  // Power (W)
#define P078_QUERY4_DFLT     5  // Power Factor (cos-phi)



#include <SDM.h>    // Requires SDM library from Reaper7 - https://github.com/reaper7/SDM_Energy_Meter/
#include <ESPeasySoftwareSerial.h>

// These pointers may be used among multiple instances of the same plugin,
// as long as the same serial settings are used.
ESPeasySoftwareSerial* Plugin_078_SoftSerial = NULL;
SDM* Plugin_078_SDM = NULL;
boolean Plugin_078_init = false;

boolean Plugin_078(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_078;
        Device[deviceCount].Type = DEVICE_TYPE_TRIPLE;     // connected through 3 datapins
        Device[deviceCount].VType = SENSOR_TYPE_QUAD;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 4;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_078);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        if (P078_QUERY1 == 0 && P078_QUERY2 == 0 && P078_QUERY3 == 0 && P078_QUERY4 == 0) {
          P078_QUERY1 = P078_QUERY1_DFLT;
          P078_QUERY2 = P078_QUERY2_DFLT;
          P078_QUERY3 = P078_QUERY3_DFLT;
          P078_QUERY4 = P078_QUERY4_DFLT;
        }
        safe_strncpy(ExtraTaskSettings.TaskDeviceValueNames[0],
          p078_getQueryValueString(P078_QUERY1), sizeof(ExtraTaskSettings.TaskDeviceValueNames[0]));
        safe_strncpy(ExtraTaskSettings.TaskDeviceValueNames[1],
          p078_getQueryValueString(P078_QUERY2), sizeof(ExtraTaskSettings.TaskDeviceValueNames[1]));
        safe_strncpy(ExtraTaskSettings.TaskDeviceValueNames[2],
          p078_getQueryValueString(P078_QUERY3), sizeof(ExtraTaskSettings.TaskDeviceValueNames[2]));
        safe_strncpy(ExtraTaskSettings.TaskDeviceValueNames[3],
          p078_getQueryValueString(P078_QUERY4), sizeof(ExtraTaskSettings.TaskDeviceValueNames[3]));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_RX(false);
        event->String2 = formatGpioName_TX(false);
        event->String3 = formatGpioName_output_optional("DE");
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        if (P078_DEV_ID == 0 || P078_DEV_ID > 247 || P078_BAUDRATE >= 6) {
          // Load some defaults
          P078_DEV_ID = P078_DEV_ID_DFLT;
          P078_MODEL = P078_MODEL_DFLT;
          P078_BAUDRATE = P078_BAUDRATE_DFLT;
          P078_QUERY1 = P078_QUERY1_DFLT;
          P078_QUERY2 = P078_QUERY2_DFLT;
          P078_QUERY3 = P078_QUERY3_DFLT;
          P078_QUERY4 = P078_QUERY4_DFLT;
        }
        addFormNumericBox(F("Modbus Address"), F("p078_dev_id"), P078_DEV_ID, 1, 247);

        String options_model[4] = { F("SDM120C"), F("SDM220T"), F("SDM230"), F("SDM630") };
        addFormSelector(F("Model Type"), F("p078_model"), 4, options_model, NULL, P078_MODEL );

        String options_baudrate[6];
        for (int i = 0; i < 6; ++i) {
          options_baudrate[i] = String(p078_storageValueToBaudrate(i));
        }
        addFormSelector(F("Baud Rate"), F("p078_baudrate"), 6, options_baudrate, NULL, P078_BAUDRATE );

        if (P078_MODEL == 0 && P078_BAUDRATE > 3)
          addFormNote(F("<span style=\"color:red\"> SDM120 only allows up to 9600 baud with default 2400!</span>"));

        if (P078_MODEL == 3 && P078_BAUDRATE == 0)
          addFormNote(F("<span style=\"color:red\"> SDM630 only allows 2400 to 38400 baud with default 9600!</span>"));

        String options_query[10];
        for (int i = 0; i < 10; ++i) {
          options_query[i] = p078_getQueryString(i);
        }
        addFormSelector(F("Variable 1"), F("p078_query1"), 10, options_query, NULL, P078_QUERY1);
        addFormSelector(F("Variable 2"), F("p078_query2"), 10, options_query, NULL, P078_QUERY2);
        addFormSelector(F("Variable 3"), F("p078_query3"), 10, options_query, NULL, P078_QUERY3);
        addFormSelector(F("Variable 4"), F("p078_query4"), 10, options_query, NULL, P078_QUERY4);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
          P078_DEV_ID = getFormItemInt(F("p078_dev_id"));
          P078_MODEL = getFormItemInt(F("p078_model"));
          P078_BAUDRATE = getFormItemInt(F("p078_baudrate"));
          P078_QUERY1 = getFormItemInt(F("p078_query1"));
          P078_QUERY2 = getFormItemInt(F("p078_query2"));
          P078_QUERY3 = getFormItemInt(F("p078_query3"));
          P078_QUERY4 = getFormItemInt(F("p078_query4"));

          Plugin_078_init = false; // Force device setup next time
          success = true;
          break;
      }

    case PLUGIN_INIT:
      {
        Plugin_078_init = true;
        if (Plugin_078_SoftSerial != NULL) {
          delete Plugin_078_SoftSerial;
          Plugin_078_SoftSerial=NULL;
        }
        Plugin_078_SoftSerial = new ESPeasySoftwareSerial(Settings.TaskDevicePin1[event->TaskIndex], Settings.TaskDevicePin2[event->TaskIndex]);
        unsigned int baudrate = p078_storageValueToBaudrate(P078_BAUDRATE);
        Plugin_078_SoftSerial->begin(baudrate);

        if (Plugin_078_SDM != NULL) {
          delete Plugin_078_SDM;
          Plugin_078_SDM=NULL;
        }
        Plugin_078_SDM = new SDM(*Plugin_078_SoftSerial, baudrate, P078_DEPIN);
        Plugin_078_SDM->begin();
        success = true;
        break;
      }

    case PLUGIN_EXIT:
    {
      Plugin_078_init = false;
      if (Plugin_078_SoftSerial != NULL) {
        delete Plugin_078_SoftSerial;
        Plugin_078_SoftSerial=NULL;
      }
      if (Plugin_078_SDM != NULL) {
        delete Plugin_078_SDM;
        Plugin_078_SDM=NULL;
      }
      break;
    }

    case PLUGIN_READ:
      {
        if (Plugin_078_init)
        {
          int model = P078_MODEL;
          byte dev_id = P078_DEV_ID;
          UserVar[event->BaseVarIndex]     = p078_readVal(P078_QUERY1, dev_id, model);
          UserVar[event->BaseVarIndex + 1] = p078_readVal(P078_QUERY2, dev_id, model);
          UserVar[event->BaseVarIndex + 2] = p078_readVal(P078_QUERY3, dev_id, model);
          UserVar[event->BaseVarIndex + 3] = p078_readVal(P078_QUERY4, dev_id, model);
          success = true;
          break;
        }
        break;
      }
  }
  return success;
}

float p078_readVal(byte query, byte node, unsigned int model) {
  if (Plugin_078_SDM == NULL) return 0.0;
  const float _tempvar = Plugin_078_SDM->readVal(p078_getRegister(query, model), node);
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("EASTRON: (");
    log += node;
    log += ',';
    log += model;
    log += ") ";
    log += p078_getQueryString(query);
    log += ": ";
    log += _tempvar;
    addLog(LOG_LEVEL_INFO, log);
  }
  return _tempvar;
}

unsigned int p078_getRegister(byte query, byte model) {
  if (model == 0) { // SDM120C
    switch (query) {
      case 0: return SDM120C_VOLTAGE;
      case 1: return SDM120C_CURRENT;
      case 2: return SDM120C_POWER;
      case 3: return SDM120C_ACTIVE_APPARENT_POWER;
      case 4: return SDM120C_REACTIVE_APPARENT_POWER;
      case 5: return SDM120C_POWER_FACTOR;
      case 6: return SDM120C_FREQUENCY;
      case 7: return SDM120C_IMPORT_ACTIVE_ENERGY;
      case 8: return SDM120C_EXPORT_ACTIVE_ENERGY;
      case 9: return SDM120C_TOTAL_ACTIVE_ENERGY;
    }
  } else if (model == 1) { // SDM220T
    switch (query) {
      case 0: return SDM220T_VOLTAGE;
      case 1: return SDM220T_CURRENT;
      case 2: return SDM220T_POWER;
      case 3: return SDM220T_ACTIVE_APPARENT_POWER;
      case 4: return SDM220T_REACTIVE_APPARENT_POWER;
      case 5: return SDM220T_POWER_FACTOR;
      case 6: return SDM220T_FREQUENCY;
      case 7: return SDM220T_IMPORT_ACTIVE_ENERGY;
      case 8: return SDM220T_EXPORT_ACTIVE_ENERGY;
      case 9: return SDM220T_TOTAL_ACTIVE_ENERGY;
    }
  } else if (model == 2) { // SDM230
    switch (query) {
      case 0: return SDM230_VOLTAGE;
      case 1: return SDM230_CURRENT;
      case 2: return SDM230_POWER;
      case 3: return SDM230_ACTIVE_APPARENT_POWER;
      case 4: return SDM230_REACTIVE_APPARENT_POWER;
      case 5: return SDM230_POWER_FACTOR;
      case 6: return SDM230_FREQUENCY;
      case 7: return SDM230_IMPORT_ACTIVE_ENERGY;
      case 8: return SDM230_EXPORT_ACTIVE_ENERGY;
      case 9: return SDM230_CURRENT_RESETTABLE_TOTAL_ACTIVE_ENERGY;
    }
  } else if (model == 3) { // SDM630
    switch (query) {
      case 0: return SDM630_VOLTAGE_AVERAGE;
      case 1: return SDM630_CURRENTSUM;
      case 2: return SDM630_POWERTOTAL;
      case 3: return SDM630_VOLT_AMPS_TOTAL;
      case 4: return SDM630_VOLT_AMPS_REACTIVE_TOTAL;
      case 5: return SDM630_POWER_FACTOR_TOTAL;
      case 6: return SDM630_FREQUENCY;
      case 7: return SDM630_IMPORT_ACTIVE_ENERGY;
      case 8: return SDM630_EXPORT_ACTIVE_ENERGY;
      case 9: return SDM630_IMPORT_ACTIVE_ENERGY;  // No equivalent for TOTAL_ACTIVE_ENERGY present in the SDM630
    }
  }
  return 0;
}

String p078_getQueryString(byte query) {
  switch(query)
  {
    case 0: return F("Voltage (V)");
    case 1: return F("Current (A)");
    case 2: return F("Power (W)");
    case 3: return F("Active Apparent Power (VA)");
    case 4: return F("Reactive Apparent Power (VAr)");
    case 5: return F("Power Factor (cos-phi)");
    case 6: return F("Frequency (Hz)");
    case 7: return F("Import Active Energy (Wh)");
    case 8: return F("Export Active Energy (Wh)");
    case 9: return F("Total Active Energy (Wh)");
  }
  return "";
}

String p078_getQueryValueString(byte query) {
  switch(query)
  {
    case 0: return F("V");
    case 1: return F("A");
    case 2: return F("W");
    case 3: return F("VA");
    case 4: return F("VAr");
    case 5: return F("cos_phi");
    case 6: return F("Hz");
    case 7: return F("Wh_imp");
    case 8: return F("Wh_exp");
    case 9: return F("Wh_tot");
  }
  return "";
}


int p078_storageValueToBaudrate(byte baudrate_setting) {
  unsigned int baudrate = 9600;
  switch (baudrate_setting) {
    case 0:  baudrate = 1200; break;
    case 1:  baudrate = 2400; break;
    case 2:  baudrate = 4800; break;
    case 3:  baudrate = 9600; break;
    case 4:  baudrate = 19200; break;
    case 5:  baudrate = 38400; break;
  }
  return baudrate;
}



#endif // USES_P078
