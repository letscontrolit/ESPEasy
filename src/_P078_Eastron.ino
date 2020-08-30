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

#define P078_DEV_ID          PCONFIG(0)
#define P078_DEV_ID_LABEL    PCONFIG_LABEL(0)
#define P078_MODEL           PCONFIG(1)
#define P078_MODEL_LABEL     PCONFIG_LABEL(1)
#define P078_BAUDRATE        PCONFIG(2)
#define P078_BAUDRATE_LABEL  PCONFIG_LABEL(2)
#define P078_QUERY1          PCONFIG(3)
#define P078_QUERY2          PCONFIG(4)
#define P078_QUERY3          PCONFIG(5)
#define P078_QUERY4          PCONFIG(6)
#define P078_DEPIN           CONFIG_PIN3

#define P078_DEV_ID_DFLT     1
#define P078_MODEL_DFLT      0  // SDM120C
#define P078_BAUDRATE_DFLT   1  // 9600 baud
#define P078_QUERY1_DFLT     0  // Voltage (V)
#define P078_QUERY2_DFLT     1  // Current (A)
#define P078_QUERY3_DFLT     2  // Power (W)
#define P078_QUERY4_DFLT     5  // Power Factor (cos-phi)

#define P078_NR_OUTPUT_VALUES          4
#define P078_NR_OUTPUT_OPTIONS        10
#define P078_QUERY1_CONFIG_POS  3


#include <SDM.h>    // Requires SDM library from Reaper7 - https://github.com/reaper7/SDM_Energy_Meter/
#include <ESPeasySerial.h>
#include "_Plugin_Helper.h"

// These pointers may be used among multiple instances of the same plugin,
// as long as the same serial settings are used.
ESPeasySerial* Plugin_078_SoftSerial = NULL;
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
        Device[deviceCount].Type = DEVICE_TYPE_SERIAL_PLUS1;     // connected through 3 datapins
        Device[deviceCount].VType = SENSOR_TYPE_QUAD;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = P078_NR_OUTPUT_VALUES;
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
        for (byte i = 0; i < VARS_PER_TASK; ++i) {
          if ( i < P078_NR_OUTPUT_VALUES) {
            byte choice = PCONFIG(i + P078_QUERY1_CONFIG_POS);
            safe_strncpy(
              ExtraTaskSettings.TaskDeviceValueNames[i],
              p078_getQueryValueString(choice),
              sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
          } else {
            ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
          }
        }
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        serialHelper_getGpioNames(event);
        event->String3 = formatGpioName_output_optional("DE");
        break;
      }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
      {
        string += serialHelper_getSerialTypeLabel(event);
        success = true;
        break;
      }

    case PLUGIN_SET_DEFAULTS:
      {
        P078_DEV_ID = P078_DEV_ID_DFLT;
        P078_MODEL = P078_MODEL_DFLT;
        P078_BAUDRATE = P078_BAUDRATE_DFLT;
        P078_QUERY1 = P078_QUERY1_DFLT;
        P078_QUERY2 = P078_QUERY2_DFLT;
        P078_QUERY3 = P078_QUERY3_DFLT;
        P078_QUERY4 = P078_QUERY4_DFLT;

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        serialHelper_webformLoad(event);

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
        addFormNumericBox(F("Modbus Address"), P078_DEV_ID_LABEL, P078_DEV_ID, 1, 247);

        {
          String options_model[4] = { F("SDM120C"), F("SDM220T"), F("SDM230"), F("SDM630") };
          addFormSelector(F("Model Type"), P078_MODEL_LABEL, 4, options_model, NULL, P078_MODEL );
        }
        {
          String options_baudrate[6];
          for (int i = 0; i < 6; ++i) {
            options_baudrate[i] = String(p078_storageValueToBaudrate(i));
          }
          addFormSelector(F("Baud Rate"), P078_BAUDRATE_LABEL, 6, options_baudrate, NULL, P078_BAUDRATE );
        }

        if (P078_MODEL == 0 && P078_BAUDRATE > 3)
          addFormNote(F("<span style=\"color:red\"> SDM120 only allows up to 9600 baud with default 2400!</span>"));

        if (P078_MODEL == 3 && P078_BAUDRATE == 0)
          addFormNote(F("<span style=\"color:red\"> SDM630 only allows 2400 to 38400 baud with default 9600!</span>"));

        if (Plugin_078_SDM != nullptr) {
          addRowLabel(F("Checksum (pass/fail)"));
          String chksumStats;
          chksumStats = Plugin_078_SDM->getSuccCount();
          chksumStats += '/';
          chksumStats += Plugin_078_SDM->getErrCount();
          addHtml(chksumStats);
        }

        {
          // In a separate scope to free memory of String array as soon as possible
          sensorTypeHelper_webformLoad_header();
          String options[P078_NR_OUTPUT_OPTIONS];
          for (int i = 0; i < P078_NR_OUTPUT_OPTIONS; ++i) {
            options[i] = p078_getQueryString(i);
          }
          for (byte i = 0; i < P078_NR_OUTPUT_VALUES; ++i) {
            const byte pconfigIndex = i + P078_QUERY1_CONFIG_POS;
            sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, P078_NR_OUTPUT_OPTIONS, options);
          }
        }


        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
          serialHelper_webformSave(event);
          // Save output selector parameters.
          for (byte i = 0; i < P078_NR_OUTPUT_VALUES; ++i) {
            const byte pconfigIndex = i + P078_QUERY1_CONFIG_POS;
            const byte choice = PCONFIG(pconfigIndex);
            sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, p078_getQueryValueString(choice));
          }

          P078_DEV_ID = getFormItemInt(P078_DEV_ID_LABEL);
          P078_MODEL = getFormItemInt(P078_MODEL_LABEL);
          P078_BAUDRATE = getFormItemInt(P078_BAUDRATE_LABEL);

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
        Plugin_078_SoftSerial = new (std::nothrow) ESPeasySerial(CONFIG_PIN1, CONFIG_PIN2);
        if (Plugin_078_SoftSerial == nullptr) {
          break;
        }
        unsigned int baudrate = p078_storageValueToBaudrate(P078_BAUDRATE);
        Plugin_078_SoftSerial->begin(baudrate);

        if (Plugin_078_SDM != NULL) {
          delete Plugin_078_SDM;
          Plugin_078_SDM=NULL;
        }
        Plugin_078_SDM = new SDM(*Plugin_078_SoftSerial, baudrate, P078_DEPIN);
        if (Plugin_078_SDM != nullptr) {
          Plugin_078_SDM->begin();
          success = true;
        }
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

  byte retry_count = 3;
  bool success = false;
  float _tempvar = NAN;
  while (retry_count > 0 && !success) {
    Plugin_078_SDM->clearErrCode();
    _tempvar = Plugin_078_SDM->readVal(p078_getRegister(query, model), node);
    --retry_count;
    if (Plugin_078_SDM->getErrCode() == SDM_ERR_NO_ERROR) {
      success = true;
    }
  }
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
