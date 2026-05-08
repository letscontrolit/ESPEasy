#include "_Plugin_Helper.h"

#ifdef USES_P155

// #######################################################################################################
// ######################## Plugin 155: Energy - Smartmeter ########################
// #######################################################################################################
//
// Modelle:
//   0 = D0   (IEC 62056-21, 7E1, 9600 Baud)
//   1 = SML  (hardcodierte posData je Hersteller)
//   2 = DTZ541
//   3 = SML-Auto (dynamisches TL-Parsing, herstellerunabhängig)
//
// SML-Auto Funktionsweise:
//   Nach der OBIS-Kennung enthält jeder SML-ListEntry folgende Felder,
//   jeweils eingeleitet durch ein TL-Byte (Type-Length):
//     Status | Time | Unit | Scaler (int8) | Value
//   TL-Byte: Bits 7-4 = Typ (5=int, 6=uint, 7=Liste, 0=optional)
//            Bits 3-0 = Gesamtlänge inkl. TL-Byte (0 = nicht vorhanden)
//            Für Listen (Typ 7): Bits 3-0 = Anzahl Kindelemente + 1
//   SML-Auto liest diese Felder dynamisch → funktioniert bei allen
//   SML-konformen Zählern unabhängig vom Hersteller.
// #######################################################################################################

#define PLUGIN_155
#define PLUGIN_ID_155 155
#define PLUGIN_NAME_155 "SML - Smartmeter [Testing]"

#define P155_MODEL PCONFIG(0)
#define P155_MODEL_LABEL PCONFIG_LABEL(0)
#define P155_QUERY1 PCONFIG(1)
#define P155_QUERY2 PCONFIG(2)
#define P155_QUERY3 PCONFIG(3)
#define P155_QUERY4 PCONFIG(4)

#define P155_MODEL_DFLT 1
#define P155_BAUDRATE 9600
#define P155_QUERY1_DFLT 1
#define P155_QUERY2_DFLT 3
#define P155_QUERY3_DFLT 2
#define P155_QUERY4_DFLT 0

#define P155_NR_OUTPUT_VALUES 4
#define P155_NR_OUTPUT_OPTIONS_MODEL0 6
#define P155_NR_OUTPUT_OPTIONS_MODEL1 13 // SML (DD3 etc.)
#define P155_NR_OUTPUT_OPTIONS_MODEL2 4  // Holley DTZ541
#define P155_NR_OUTPUT_OPTIONS_MODEL3 13 // SML-Auto (gleiche OBIS wie Model1)
#define P155_QUERY1_CONFIG_POS 1
#define P155_RX_BUFFER 64 // vergrößert für 64-bit Werte

// OBIS-Kennung Indizes
#define Q3D_TOTAL_ACTIVE_ENERGY 1
#define Q3D_POWER_L1 2
#define Q3D_POWER_L2 3
#define Q3D_POWER_L3 4
#define Q3D_POWER_TOTAL 5

#define DD3_TOTAL_ACTIVE_ENERGY_PLUS 1
#define DD3_POWER_L1 2
#define DD3_POWER_L2 3
#define DD3_POWER_L3 4
#define DD3_POWER_TOTAL 5
#define DD3_TOTAL_ACTIVE_ENERGY_MINUS 6
#define DD3_VOLT_L1 7
#define DD3_VOLT_L2 8
#define DD3_VOLT_L3 9
#define DD3_CURRENT_L1 10
#define DD3_CURRENT_L2 11
#define DD3_CURRENT_L3 12

#include <ESPeasySerial.h>

ESPeasySerial *P155_MySerial = nullptr;

// Forward declarations
const __FlashStringHelper *p155_getQueryString(uint8_t query, uint8_t model);
const __FlashStringHelper *p155_getQueryValueString(uint8_t query, uint8_t model);
unsigned int p155_getRegister(uint8_t query, uint8_t model);
float p155_readVal(uint8_t query, unsigned int model);
void p155_handleSerialInD0();
void p155_handleSerialInSML(unsigned int model);
void p155_parseValuesD0();
void p155_parseValuesSML(unsigned int model);
void p155_parseValuesSMLAuto();
bool p155_byteArrayCompare(byte a1[], int a1len, byte a2[], int a2len);
void p155_deleteValues(unsigned int model);

// ============================================================
// Datenstrukturen
// ============================================================
struct p155_dataStructD0
{
  String p155_rxID = "x-x:x.x.x*x";
  float value;
  p155_dataStructD0(String xID, float xvalue)
  {
    p155_rxID = xID;
    value = xvalue;
  }
};

p155_dataStructD0 p155_myDataD0[P155_NR_OUTPUT_OPTIONS_MODEL0] = {
    p155_dataStructD0("x-x:x.x.x*x", 0.0),
    p155_dataStructD0("1-0:1.8.0*255", 0.0),  // Total_Active_Energy_Consumption
    p155_dataStructD0("1-0:21.7.0*255", 0.0), // Power L1
    p155_dataStructD0("1-0:41.7.0*255", 0.0), // Power L2
    p155_dataStructD0("1-0:61.7.0*255", 0.0), // Power L3
    p155_dataStructD0("1-0:1.7.0*255", 0.0)   // Power L123
};

struct p155_dataStructSML
{
  int posData;  // Bytes nach OBIS bis Wert
  float factor; // Skalierungsfaktor
  byte p155_rxOrbis[6];
  float value;
  p155_dataStructSML(int xposData, float xfactor, byte xOrbis[6], float xvalue)
  {
    posData = xposData;
    factor = xfactor;
    for (int i = 0; i < 6; i++)
      p155_rxOrbis[i] = xOrbis[i];
    value = xvalue;
  }
};

// OBIS-Kennzahlen
byte p155_rxOrbis0[6] = {0, 0, 0, 0, 0, 0};
byte p155_rxOrbis1[6] = {1, 0, 1, 8, 0, 255};   // 1-0:1.8.0   Bezug gesamt
byte p155_rxOrbis2[6] = {1, 0, 21, 7, 0, 255};  // 1-0:21.7.0  Wirkleistung L1
byte p155_rxOrbis3[6] = {1, 0, 41, 7, 0, 255};  // 1-0:41.7.0  Wirkleistung L2
byte p155_rxOrbis4[6] = {1, 0, 61, 7, 0, 255};  // 1-0:61.7.0  Wirkleistung L3
byte p155_rxOrbis5[6] = {1, 0, 16, 7, 0, 255};  // 1-0:16.7.0  Wirkleistung gesamt
byte p155_rxOrbis6[6] = {1, 0, 2, 8, 0, 255};   // 1-0:2.8.0   Einspeisung gesamt
byte p155_rxOrbis7[6] = {1, 0, 32, 7, 0, 255};  // 1-0:32.7.0  Spannung L1
byte p155_rxOrbis8[6] = {1, 0, 52, 7, 0, 255};  // 1-0:52.7.0  Spannung L2
byte p155_rxOrbis9[6] = {1, 0, 72, 7, 0, 255};  // 1-0:72.7.0  Spannung L3
byte p155_rxOrbis10[6] = {1, 0, 31, 7, 0, 255}; // 1-0:31.7.0  Strom L1
byte p155_rxOrbis11[6] = {1, 0, 51, 7, 0, 255}; // 1-0:51.7.0  Strom L2
byte p155_rxOrbis12[6] = {1, 0, 71, 7, 0, 255}; // 1-0:71.7.0  Strom L3

// Model 1: SML (DD3 etc.) – posData herstellerspezifisch
p155_dataStructSML p155_myDataSML[P155_NR_OUTPUT_OPTIONS_MODEL1] = {
    p155_dataStructSML(0, 1.0, p155_rxOrbis0, 0.0),
    p155_dataStructSML(11, 0.0001, p155_rxOrbis1, 0.0), // kWh
    p155_dataStructSML(7, 1.0, p155_rxOrbis2, 0.0),     // W
    p155_dataStructSML(7, 1.0, p155_rxOrbis3, 0.0),
    p155_dataStructSML(7, 1.0, p155_rxOrbis4, 0.0),
    p155_dataStructSML(7, 1.0, p155_rxOrbis5, 0.0),
    p155_dataStructSML(7, 0.0001, p155_rxOrbis6, 0.0), // kWh
    p155_dataStructSML(7, 0.1, p155_rxOrbis7, 0.0),    // V
    p155_dataStructSML(7, 0.1, p155_rxOrbis8, 0.0),
    p155_dataStructSML(7, 0.1, p155_rxOrbis9, 0.0),
    p155_dataStructSML(7, 0.01, p155_rxOrbis10, 0.0), // A
    p155_dataStructSML(7, 0.01, p155_rxOrbis11, 0.0),
    p155_dataStructSML(7, 0.01, p155_rxOrbis12, 0.0),
};

// Model 2: DTZ541 – andere posData-Werte
p155_dataStructSML p155_myDataDTZ[P155_NR_OUTPUT_OPTIONS_MODEL2] = {
    p155_dataStructSML(0, 1.0, p155_rxOrbis0, 0.0),
    p155_dataStructSML(18, 0.0001, p155_rxOrbis1, 0.0),
    p155_dataStructSML(7, 1.0, p155_rxOrbis5, 0.0),
    p155_dataStructSML(14, 0.0001, p155_rxOrbis6, 0.0),
};

// Model 3: SML-Auto – posData wird ignoriert, Scaler kommt aus Telegramm
p155_dataStructSML p155_myDataAuto[P155_NR_OUTPUT_OPTIONS_MODEL3] = {
    p155_dataStructSML(0, 1.0, p155_rxOrbis0, 0.0),
    p155_dataStructSML(0, 1.0, p155_rxOrbis1, 0.0), // Scaler aus Telegramm
    p155_dataStructSML(0, 1.0, p155_rxOrbis2, 0.0),
    p155_dataStructSML(0, 1.0, p155_rxOrbis3, 0.0),
    p155_dataStructSML(0, 1.0, p155_rxOrbis4, 0.0),
    p155_dataStructSML(0, 1.0, p155_rxOrbis5, 0.0),
    p155_dataStructSML(0, 1.0, p155_rxOrbis6, 0.0),
    p155_dataStructSML(0, 1.0, p155_rxOrbis7, 0.0),
    p155_dataStructSML(0, 1.0, p155_rxOrbis8, 0.0),
    p155_dataStructSML(0, 1.0, p155_rxOrbis9, 0.0),
    p155_dataStructSML(0, 1.0, p155_rxOrbis10, 0.0),
    p155_dataStructSML(0, 1.0, p155_rxOrbis11, 0.0),
    p155_dataStructSML(0, 1.0, p155_rxOrbis12, 0.0),
};

// ============================================================
// Zustandsvariablen
// ============================================================
boolean p155_MyInit = false;
uint8_t p155_step = 0;
uint8_t p155_charsRead = 0;
char p155_rxBuffer[P155_RX_BUFFER];
char p155_ringBuffer[8];
byte p155_rxOrbis[6]; //|1|0|2|8|0|255
String p155_rxID;

int p155_anzBytes;
int p155_posDataAct;
int p155_registerAct;
int p155_outputOptionsAct;

// Zusätzliche Variablen für SML-Auto
int8_t p155_scaler = 0;        // Scaler-Byte aus dem SML-Telegramm
uint8_t p155_autoSubState = 0; // Aktuell zu lesendes Feld (0=Status,1=Time,2=Unit,3=Scaler,4=Value)
uint8_t p155_skipBytes = 0;    // Noch zu überspringende Bytes
uint8_t p155_autoDataTyp = 0;  // SML-Datentyp des Value-Feldes (5=int,6=uint)
uint8_t p155_listElems = 0;    // Verbleibende Kindelemente bei List-Typ (type=7)

// ============================================================
// Plugin-Hauptfunktion
// ============================================================
boolean Plugin_155(uint8_t function, struct EventStruct *event, String &string)
{
  boolean success = false;

  switch (function)
  {
  case PLUGIN_DEVICE_ADD:
  {
    Device[++deviceCount].Number = PLUGIN_ID_155;
    Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
    Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_QUAD;
    Device[deviceCount].Ports = 0;
    Device[deviceCount].PullUpOption = false;
    Device[deviceCount].InverseLogicOption = false;
    Device[deviceCount].FormulaOption = true;
    Device[deviceCount].ValueCount = P155_NR_OUTPUT_VALUES;
    Device[deviceCount].SendDataOption = true;
    Device[deviceCount].TimerOption = true;
    Device[deviceCount].GlobalSyncOption = true;
    break;
  }

  case PLUGIN_GET_DEVICENAME:
  {
    string = F(PLUGIN_NAME_155);
    break;
  }

  case PLUGIN_GET_DEVICEVALUENAMES:
  {
    const uint8_t model = P155_MODEL;
    for (uint8_t i = 0; i < VARS_PER_TASK; ++i)
    {
      if (i < P155_NR_OUTPUT_VALUES)
      {
        uint8_t choice = PCONFIG(i + P155_QUERY1_CONFIG_POS);
        safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            p155_getQueryValueString(choice, model),
            sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
      }
      else
      {
        ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
      }
    }
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
    P155_MODEL = P155_MODEL_DFLT;
    P155_QUERY1 = P155_QUERY1_DFLT;
    P155_QUERY2 = P155_QUERY2_DFLT;
    P155_QUERY3 = P155_QUERY3_DFLT;
    P155_QUERY4 = P155_QUERY4_DFLT;
    success = true;
    break;
  }

  case PLUGIN_WEBFORM_LOAD:
  {
    {
      const __FlashStringHelper *options_model[] = {
          F("D0"),
          F("SML"),
          F("DTZ541"),
          F("SML-Auto"),
      };
      constexpr size_t nrOptions = NR_ELEMENTS(options_model);
      FormSelectorOptions selector(nrOptions, options_model);
      selector.reloadonchange = true;
      selector.addFormSelector(F("Model Type"), P155_MODEL_LABEL, P155_MODEL);
    }
    {
      const uint8_t model = PCONFIG(0);
      uint8_t outputOptions;
      if (model == 1)
        outputOptions = P155_NR_OUTPUT_OPTIONS_MODEL1;
      else if (model == 2)
        outputOptions = P155_NR_OUTPUT_OPTIONS_MODEL2;
      else if (model == 3)
        outputOptions = P155_NR_OUTPUT_OPTIONS_MODEL3;
      else
        outputOptions = P155_NR_OUTPUT_OPTIONS_MODEL0;

      const __FlashStringHelper *options[outputOptions];
      for (int i = 0; i < outputOptions; ++i)
        options[i] = p155_getQueryString(i, model);

      for (uint8_t i = 0; i < P155_NR_OUTPUT_VALUES; ++i)
      {
        const uint8_t pconfigIndex = i + P155_QUERY1_CONFIG_POS;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, outputOptions, options);
      }
    }
    success = true;
    break;
  }

  case PLUGIN_WEBFORM_SAVE:
  {
    P155_MODEL = getFormItemInt(P155_MODEL_LABEL);
    const uint8_t model = P155_MODEL;
    for (uint8_t i = 0; i < P155_NR_OUTPUT_VALUES; ++i)
    {
      const uint8_t pconfigIndex = i + P155_QUERY1_CONFIG_POS;
      const uint8_t choice = PCONFIG(pconfigIndex);
      sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i,
                                          p155_getQueryValueString(choice, model));
    }
    p155_MyInit = false;
    success = true;
    break;
  }

  case PLUGIN_INIT:
  {
    if (P155_MySerial != nullptr)
    {
      delete P155_MySerial;
      P155_MySerial = nullptr;
    }
    p155_deleteValues(P155_MODEL);

    if (P155_MODEL == 1)
      p155_outputOptionsAct = P155_NR_OUTPUT_OPTIONS_MODEL1;
    else if (P155_MODEL == 2)
      p155_outputOptionsAct = P155_NR_OUTPUT_OPTIONS_MODEL2;
    else if (P155_MODEL == 3)
      p155_outputOptionsAct = P155_NR_OUTPUT_OPTIONS_MODEL3;
    else
      p155_outputOptionsAct = P155_NR_OUTPUT_OPTIONS_MODEL0;

    CONFIG_PORT = 5; // Serial2
    CONFIG_PIN1 = 16;
    CONFIG_PIN2 = 17;
    P155_MySerial = new ESPeasySerial(
        static_cast<ESPEasySerialPort>(CONFIG_PORT), CONFIG_PIN1, CONFIG_PIN2, false, P155_RX_BUFFER);

    if (P155_MySerial == nullptr)
      break;

    uint32_t config = (P155_MODEL == 0) ? SERIAL_7E1 : SERIAL_8N1;
    P155_MySerial->begin(P155_BAUDRATE, config);

    p155_step = 0;
    p155_scaler = 0;
    p155_autoSubState = 0;
    p155_skipBytes = 0;
    p155_autoDataTyp = 0;
    p155_listElems = 0;
    p155_MyInit = true;
    success = true;

    String log = F("Smartmeter: Init=");
    log += event->TaskIndex;
    log += F(" Model=");
    log += P155_MODEL;
    log += F(" Port=");
    log += CONFIG_PORT;
    log += F(" RX=");
    log += CONFIG_PIN1;
    log += F(" TX=");
    log += CONFIG_PIN2;
    log += F(" Baud=");
    log += P155_BAUDRATE;
    addLogMove(LOG_LEVEL_INFO, log);
    break;
  }

  case PLUGIN_EXIT:
  {
    p155_MyInit = false;
    if (P155_MySerial != nullptr)
    {
      delete P155_MySerial;
      P155_MySerial = nullptr;
    }

    p155_deleteValues(0);
    p155_deleteValues(1);
    p155_deleteValues(2);
    p155_deleteValues(3);
    break;
  }

  case PLUGIN_READ:
  {
    if (p155_MyInit)
    {
      int model = P155_MODEL;
      UserVar.setFloat(event->TaskIndex, 0, p155_readVal(P155_QUERY1, model));
      UserVar.setFloat(event->TaskIndex, 1, p155_readVal(P155_QUERY2, model));
      UserVar.setFloat(event->TaskIndex, 2, p155_readVal(P155_QUERY3, model));
      UserVar.setFloat(event->TaskIndex, 3, p155_readVal(P155_QUERY4, model));
      success = true;
    }
    break;
  }

  case PLUGIN_TEN_PER_SECOND:
  {
    if (P155_MODEL == 0)
      p155_handleSerialInD0();
    else
      p155_handleSerialInSML(P155_MODEL);
    success = true;
    break;
  }
  } // switch
  return success;
}

// ============================================================
// Hilfsfunktionen
// ============================================================

float p155_readVal(uint8_t query, unsigned int model)
{
  if (model == 0)
    return p155_myDataD0[query].value;
  else if (model == 1)
    return p155_myDataSML[query].value;
  else if (model == 2)
    return p155_myDataDTZ[query].value;
  else if (model == 3)
    return p155_myDataAuto[query].value;
  return 0;
}

unsigned int p155_getRegister(uint8_t query, uint8_t model)
{
  if (model == 0)
  {
    switch (query)
    {
    case 1:
      return Q3D_TOTAL_ACTIVE_ENERGY;
    case 2:
      return Q3D_POWER_L1;
    case 3:
      return Q3D_POWER_L2;
    case 4:
      return Q3D_POWER_L3;
    case 5:
      return Q3D_POWER_TOTAL;
    }
  }
  else if (model == 1 || model == 3)
  {
    switch (query)
    {
    case 1:
      return DD3_TOTAL_ACTIVE_ENERGY_PLUS;
    case 2:
      return DD3_POWER_L1;
    case 3:
      return DD3_POWER_L2;
    case 4:
      return DD3_POWER_L3;
    case 5:
      return DD3_POWER_TOTAL;
    case 6:
      return DD3_TOTAL_ACTIVE_ENERGY_MINUS;
    case 7:
      return DD3_VOLT_L1;
    case 8:
      return DD3_VOLT_L2;
    case 9:
      return DD3_VOLT_L3;
    case 10:
      return DD3_CURRENT_L1;
    case 11:
      return DD3_CURRENT_L2;
    case 12:
      return DD3_CURRENT_L3;
    }
  }
  else if (model == 2)
  {
    switch (query)
    {
    case 1:
      return DD3_TOTAL_ACTIVE_ENERGY_PLUS;
    case 2:
      return DD3_POWER_TOTAL;
    case 3:
      return DD3_TOTAL_ACTIVE_ENERGY_MINUS;
    }
  }
  return 0;
}

const __FlashStringHelper *p155_getQueryString(uint8_t query, uint8_t model)
{
  // D0
  if (model == 0)
  {
    switch (query)
    {
    case 1:
      return F("Total Active Energy (kWh)");
    case 2:
      return F("Power L1 (W)");
    case 3:
      return F("Power L2 (W)");
    case 4:
      return F("Power L3 (W)");
    case 5:
      return F("Power Total (W)");
    }
  }
  // SML / SML-Auto – gleiche OBIS-Beschriftungen
  else if (model == 1 || model == 3)
  {
    switch (query)
    {
    case 1:
      return F("Total Active Energy Plus (kWh)");
    case 2:
      return F("Power L1 (W)");
    case 3:
      return F("Power L2 (W)");
    case 4:
      return F("Power L3 (W)");
    case 5:
      return F("Power Total (W)");
    case 6:
      return F("Total Active Energy Minus (kWh)");
    case 7:
      return F("Voltage L1 (V)");
    case 8:
      return F("Voltage L2 (V)");
    case 9:
      return F("Voltage L3 (V)");
    case 10:
      return F("Current L1 (A)");
    case 11:
      return F("Current L2 (A)");
    case 12:
      return F("Current L3 (A)");
    }
  }
  // DTZ541
  else if (model == 2)
  {
    switch (query)
    {
    case 1:
      return F("Total Active Energy Plus (kWh)");
    case 2:
      return F("Power Total (W)");
    case 3:
      return F("Total Active Energy Minus (kWh)");
    }
  }
  return F("");
}

const __FlashStringHelper *p155_getQueryValueString(uint8_t query, uint8_t model)
{
  if (model == 0)
  {
    switch (query)
    {
    case 1:
      return F("Consumption_kWh");
    case 2:
      return F("L1_W");
    case 3:
      return F("L2_W");
    case 4:
      return F("L3_W");
    case 5:
      return F("L123_W");
    }
  }
  else if (model == 1 || model == 3)
  {
    switch (query)
    {
    case 1:
      return F("Energy_Consumption_kWh");
    case 2:
      return F("L1_W");
    case 3:
      return F("L2_W");
    case 4:
      return F("L3_W");
    case 5:
      return F("L123_W");
    case 6:
      return F("Energy_FeedIn_kWh");
    case 7:
      return F("L1_V");
    case 8:
      return F("L2_V");
    case 9:
      return F("L3_V");
    case 10:
      return F("L1_A");
    case 11:
      return F("L2_A");
    case 12:
      return F("L3_A");
    }
  }
  else if (model == 2)
  {
    switch (query)
    {
    case 1:
      return F("Energy_Consumption_kWh");
    case 2:
      return F("L123_W");
    case 3:
      return F("Energy_FeedIn_kWh");
    }
  }
  return F("");
}

// ============================================================
// Serial-Handler: gemeinsam für SML, DTZ541 und SML-Auto
// ============================================================
void p155_handleSerialInSML(unsigned int model)
{
  if (nullptr == P155_MySerial)
  {
    addLog(LOG_LEVEL_INFO, F("SML: handleSerialIn nullptr"));
    return;
  }

  String log1 = F("SML: step=");
  String logdata1 = F("SML: 1=");
  String logdata2 = F("SML: 2=");
  // ... (weitere Log-Strings wie bisher)

  unsigned long timeOut = millis() + 100;

  while (P155_MySerial->available() && millis() < timeOut)
  {
    byte b = P155_MySerial->read();

    // Ringbuffer für Startsequenz-Erkennung
    for (int i = 7; i > 0; i--)
      p155_ringBuffer[i] = p155_ringBuffer[i - 1];
    p155_ringBuffer[0] = b;

    // SML-Startsequenz: 1B 1B 1B 1B 01 01 01 01
    if ((p155_ringBuffer[0] == 0x01) && (p155_ringBuffer[1] == 0x01) &&
        (p155_ringBuffer[2] == 0x01) && (p155_ringBuffer[3] == 0x01) &&
        (p155_ringBuffer[4] == 0x1B) && (p155_ringBuffer[5] == 0x1B) &&
        (p155_ringBuffer[6] == 0x1B) && (p155_ringBuffer[7] == 0x1B))
    {
      p155_step = 11;
      p155_charsRead = 0;
      p155_registerAct = 0;
      p155_anzBytes = 0;
      p155_posDataAct = 0;
      p155_scaler = 0;
      p155_autoSubState = 0;
      p155_skipBytes = 0;
      p155_listElems = 0;
    }

    switch (p155_step)
    {
    // -------------------------------------------------------
    // Gemeinsame States: OBIS-Kennung finden (alle SML-Models)
    // -------------------------------------------------------
    case 11: // Startzeichen 0x77 finden
      if (b == 0x77)
      {
        p155_step = 12;
        p155_charsRead = 0;
      }
      break;

    case 12: // Längen-Byte: muss 0x07 sein
      p155_step = (b == 0x07) ? 13 : 11;
      break;

    case 13: // 6 OBIS-Bytes sammeln
      if (p155_charsRead <= 5)
        p155_rxOrbis[p155_charsRead++] = b;

      if (p155_charsRead >= 6)
      {
        p155_charsRead = 0;
        p155_registerAct = 0;

        // OBIS-Kennung in Datentabelle suchen
        for (int i = 1; i < p155_outputOptionsAct; i++)
        {
          byte *orbis = nullptr;
          if (model == 1)
            orbis = p155_myDataSML[i].p155_rxOrbis;
          else if (model == 2)
            orbis = p155_myDataDTZ[i].p155_rxOrbis;
          else if (model == 3)
            orbis = p155_myDataAuto[i].p155_rxOrbis;

          if (orbis && p155_byteArrayCompare(p155_rxOrbis, 6, orbis, 6))
          {
            p155_registerAct = i;
            if (model != 3)
              p155_posDataAct = (model == 1)
                                    ? p155_myDataSML[i].posData
                                    : p155_myDataDTZ[i].posData;
            break;
          }
        }

        if (p155_registerAct != 0)
        {
          // Weiter je nach Model
          if (model == 3)
          {
            // SML-Auto: dynamisches TL-Parsing starten
            p155_scaler = 0;
            p155_autoSubState = 0; // beginnt mit Status-Feld
            p155_skipBytes = 0;
            p155_listElems = 0;
            p155_step = 30;
          }
          else
          {
            // Hardcodiertes Model: direkt zu posData-Auswertung
            p155_step = 20;
          }
          p155_charsRead = 0;
        }
        else
        {
          p155_step = 11; // Kennung nicht gefunden, weiter suchen
        }
      }
      break;

    // -------------------------------------------------------
    // States 20-21: Hardcodierte Models (SML, DTZ541)
    // -------------------------------------------------------
    case 20: // Bytes bis posData lesen, dann Datentyp ermitteln
      p155_rxBuffer[p155_charsRead++] = b;
      if (p155_charsRead >= p155_posDataAct)
      {
        // BUGFIX: ltyp = b (das zuletzt gelesene Byte), NICHT ltyp = 0
        byte ltyp = b;
        p155_anzBytes = 4; // Default
        if (ltyp == 0x52 || ltyp == 0x62)
          p155_anzBytes = 1; //  8-bit
        else if (ltyp == 0x53 || ltyp == 0x63)
          p155_anzBytes = 2; // 16-bit
        else if (ltyp == 0x55 || ltyp == 0x65)
          p155_anzBytes = 4; // 32-bit
        else if (ltyp == 0x59 || ltyp == 0x69)
          p155_anzBytes = 8; // 64-bit
        p155_step = 21;
      }
      break;

    case 21: // Nutzdaten sammeln
      p155_rxBuffer[p155_charsRead++] = b;
      if (p155_charsRead >= p155_posDataAct + p155_anzBytes)
      {
        p155_parseValuesSML(model);
        p155_step = 11;
        p155_charsRead = 0;
      }
      else if (p155_charsRead >= P155_RX_BUFFER - 1)
      {
        // Abbruch: Buffer-Überlauf
        p155_step = 11;
      }
      break;

    // -------------------------------------------------------
    // States 30-33: SML-Auto – dynamisches TL-Parsing
    //
    // TL-Byte Format:
    //   Bits 7-4: Typ (0=optional, 5=int signed, 6=uint, 7=Liste)
    //   Bits 3-0: Gesamtlänge inkl. TL-Byte
    //     Primitive Typen: Datenbytes = bits3-0 - 1
    //     Listen (Typ 7): Kindelemente = bits3-0 - 1  (NICHT Byte-Anzahl!)
    //
    // autoSubState: 0=Status, 1=Time, 2=Unit, 3=Scaler, 4=Value
    // -------------------------------------------------------
    case 30: // TL-Byte des aktuellen Feldes lesen
    {
      uint8_t tlLen = (b & 0x0F);                        // Gesamtlänge inkl. TL
      uint8_t tlTyp = (b >> 4) & 0x07;                   // Datentyp
      uint8_t dataBytes = (tlLen > 0) ? (tlLen - 1) : 0; // Datenbytes (bei Primitiven)

      // Debug-Log: zeigt jeden TL-Byte (nur bei Log-Level DEBUG aktiv)
      if (loglevelActiveFor(LOG_LEVEL_DEBUG) && p155_registerAct > 0)
      {
        String tl = F("SML-TL: reg=");
        tl += p155_registerAct;
        tl += F(" sub=");
        tl += p155_autoSubState;
        tl += F(" b=0x");
        tl += String(b, HEX);
        tl += F(" typ=");
        tl += tlTyp;
        tl += F(" data=");
        tl += dataBytes;
        addLogMove(LOG_LEVEL_DEBUG, tl);
      }

      if (p155_autoSubState < 3)
      {
        // Status (0), Time (1), Unit (2)
        if (tlTyp == 7)
        {
          // Listen-Typ: bits3-0 = Anzahl Kindelemente direkt (NICHT Byte-Anzahl!)
          // Beispiel: 72 = type7, 2 Kinder (62 01 + 65 xx xx xx xx fuer SML_Time)
          uint8_t numElems = tlLen;
          if (numElems == 0)
          {
            p155_autoSubState++; // leere Liste → weiter
          }
          else
          {
            p155_listElems = numElems;
            p155_step = 36;
          }
        }
        else if (dataBytes == 0)
        {
          p155_autoSubState++; // Feld nicht vorhanden → weiter
          // state 30 bleibt, liest nächstes TL
        }
        else
        {
          p155_skipBytes = dataBytes;
          p155_step = 31; // Bytes überspringen
        }
      }
      else if (p155_autoSubState == 3)
      {
        // Scaler-Feld
        if (dataBytes == 0)
        {
          p155_scaler = 0; // kein Scaler → Faktor 1
          p155_autoSubState++;
          // state 30 bleibt
        }
        else
        {
          // Scaler ist immer 1 Byte (int8)
          p155_step = 32;
        }
      }
      else if (p155_autoSubState == 4)
      {
        // Value-Feld
        if (dataBytes == 0)
        {
          // Kein Wert → abbrechen
          p155_step = 11;
        }
        else
        {
          p155_autoDataTyp = tlTyp;
          p155_anzBytes = dataBytes;
          p155_charsRead = 0;
          p155_step = 33; // Wert-Bytes lesen
        }
      }
      break;
    }

    case 31: // Bytes überspringen (Status/Time/Unit, primitive Typen)
      p155_skipBytes--;
      if (p155_skipBytes == 0)
      {
        p155_autoSubState++;
        p155_step = 30; // nächstes TL-Byte lesen
      }
      break;

    case 32: // Scaler-Byte lesen (int8, signed)
      p155_scaler = (int8_t)b;
      p155_autoSubState++;
      p155_step = 30; // weiter zum Value-TL
      break;

    case 33: // Wert-Bytes sammeln
      if (p155_charsRead < P155_RX_BUFFER - 1)
        p155_rxBuffer[p155_charsRead++] = b;

      if (p155_charsRead >= p155_anzBytes)
      {
        p155_parseValuesSMLAuto();
        p155_step = 11;
      }
      break;

    // -------------------------------------------------------
    // States 36-37: Listen-Typ (type=7) Kindelemente überspringen
    //
    // Benötigt für SML_Time mit Zeitstempel, z.B.:
    //   72 62 01 65 xx xx xx xx  (Liste mit 2 Kindelementen)
    //   - 72: Liste, 2-1=1 Kind  → falsch! 72 hat 2 Kinder (secType + secValue)
    //   Korrekte Interpretation: bits3-0 - 1 = Anzahl Kinder
    //     72 → 2-1=1 Kind? oder 2 Kinder?
    //   Praxis: State 36 liest TL-Byte jedes Kindes und überspringt seine Daten
    // -------------------------------------------------------
    case 36: // TL-Byte eines Listkind-Elements lesen
    {
      uint8_t cLen = (b & 0x0F);
      uint8_t cTyp = (b >> 4) & 0x07;
      uint8_t cData = (cLen > 0) ? (cLen - 1) : 0;

      if (cTyp == 7)
      {
        // Verschachtelte Liste: aktuelles Element durch seine Kinder ersetzen
        p155_listElems = p155_listElems - 1 + (cLen > 0 ? cLen - 1 : 0);
      }
      else
      {
        p155_listElems--;
        if (cData > 0)
        {
          p155_skipBytes = cData;
          p155_step = 37; // Daten überspringen, dann zurück zu 36 oder 30
          break;
        }
      }

      if (p155_listElems == 0)
      {
        p155_autoSubState++;
        p155_step = 30;
      }
      // sonst: weiter in State 36 für nächstes Kind
      break;
    }

    case 37: // Daten eines Listkind-Elements überspringen
      p155_skipBytes--;
      if (p155_skipBytes == 0)
      {
        if (p155_listElems == 0)
        {
          p155_autoSubState++;
          p155_step = 30;
        }
        else
        {
          p155_step = 36; // nächstes Kindelement
        }
      }
      break;

    default:
      p155_step = 0;
      p155_charsRead = 0;
      p155_anzBytes = 0;
      p155_registerAct = 0;
      p155_posDataAct = 0;
      p155_autoSubState = 0;
      p155_listElems = 0;
      break;
    }
  } // while serial available

  if (loglevelActiveFor(LOG_LEVEL_DEBUG))
  {
    log1 += p155_step;
    log1 += F(" sub=");
    log1 += p155_autoSubState;
    log1 += F(" reg=");
    log1 += p155_registerAct;
    addLogMove(LOG_LEVEL_DEBUG, log1);
  }
}

// ============================================================
// D0-Serial-Handler (unverändert)
// ============================================================
void p155_handleSerialInD0()
{
  if (nullptr == P155_MySerial)
  {
    addLog(LOG_LEVEL_INFO, F("D0: handleSerialIn nullptr"));
    return;
  }
  String log1 = F("D0: Log=");
  String logdata1 = F("D0: Data=");

  unsigned long timeOut = millis() + 10;
  while (P155_MySerial->available() && millis() < timeOut)
  {
    char c = (char)P155_MySerial->read();
    logdata1 += c;

    if (c == '(')
    {
      p155_rxBuffer[p155_charsRead] = '\0';
      p155_rxID = String(p155_rxBuffer);
      p155_charsRead = 0;
    }
    else if (c == ')')
    {
      p155_rxBuffer[p155_charsRead] = '\0';
      log1 += F(" ID=");
      log1 += p155_rxID;
      log1 += F(" val=");
      log1 += String(p155_rxBuffer);
      if (p155_charsRead > 1)
        p155_parseValuesD0();
      p155_charsRead = 0;
    }
    else if (c == 0x0D || c == 0x0A)
    {
      p155_charsRead = 0;
    }
    else if (p155_charsRead < P155_RX_BUFFER - 1)
    {
      p155_rxBuffer[p155_charsRead++] = c;
    }
  }
  addLogMove(LOG_LEVEL_DEBUG, log1);
  addLogMove(LOG_LEVEL_DEBUG, logdata1);
}

// ============================================================
// Werte parsen: hardcodierte SML-Models (1, 2)
// ============================================================
void p155_parseValuesSML(unsigned int model)
{
  String log = F("SML Parse: reg=");
  log += p155_registerAct;
  log += F(" pos=");
  log += p155_posDataAct;

  byte *orbis = (model == 1)
                    ? p155_myDataSML[p155_registerAct].p155_rxOrbis
                    : p155_myDataDTZ[p155_registerAct].p155_rxOrbis;

  if (!p155_byteArrayCompare(p155_rxOrbis, 6, orbis, 6))
    return;

  int lLen = p155_posDataAct;
  byte lTyp = p155_rxBuffer[lLen - 1]; // BUGFIX: ltyp aus Buffer, nicht 0
  float lvalue = 0.0f;

  int8_t lint8;
  uint8_t luint8;
  int16_t lint16;
  uint16_t luint16;
  int32_t lint32;
  uint32_t luint32;

  switch (lTyp)
  {
  case 0x52:
    lint8 = p155_rxBuffer[lLen];
    lvalue = (float)lint8;
    break; //  S8
  case 0x53:
    lint16 = ((uint8_t)p155_rxBuffer[lLen] << 8) | (uint8_t)p155_rxBuffer[lLen + 1];
    lvalue = (float)lint16;
    break; // S16
  case 0x55:
    lint32 = ((uint8_t)p155_rxBuffer[lLen] << 24) |
             ((uint8_t)p155_rxBuffer[lLen + 1] << 16) |
             ((uint8_t)p155_rxBuffer[lLen + 2] << 8) |
             (uint8_t)p155_rxBuffer[lLen + 3];
    lvalue = (float)lint32;
    break; // S32
  case 0x59:
    lint32 = ((uint8_t)p155_rxBuffer[lLen + 4] << 24) | // S64 → nur untere 32bit
             ((uint8_t)p155_rxBuffer[lLen + 5] << 16) |
             ((uint8_t)p155_rxBuffer[lLen + 6] << 8) |
             (uint8_t)p155_rxBuffer[lLen + 7];
    lvalue = (float)lint32;
    break;
  case 0x62:
    luint8 = p155_rxBuffer[lLen];
    lvalue = (float)luint8;
    break; // U8
  case 0x63:
    luint16 = ((uint8_t)p155_rxBuffer[lLen] << 8) | (uint8_t)p155_rxBuffer[lLen + 1];
    lvalue = (float)luint16;
    break; // U16
  case 0x65:
    luint32 = ((uint8_t)p155_rxBuffer[lLen] << 24) |
              ((uint8_t)p155_rxBuffer[lLen + 1] << 16) |
              ((uint8_t)p155_rxBuffer[lLen + 2] << 8) |
              (uint8_t)p155_rxBuffer[lLen + 3];
    lvalue = (float)luint32;
    break; // U32
  case 0x69:
    luint32 = ((uint8_t)p155_rxBuffer[lLen + 4] << 24) | // U64 → untere 32bit
              ((uint8_t)p155_rxBuffer[lLen + 5] << 16) |
              ((uint8_t)p155_rxBuffer[lLen + 6] << 8) |
              (uint8_t)p155_rxBuffer[lLen + 7];
    lvalue = (float)luint32;
    break;
  }

  float factor = (model == 1) ? p155_myDataSML[p155_registerAct].factor
                              : p155_myDataDTZ[p155_registerAct].factor;

  if (model == 1)
    p155_myDataSML[p155_registerAct].value = lvalue * factor;
  if (model == 2)
    p155_myDataDTZ[p155_registerAct].value = lvalue * factor;

  log += F(" typ=0x");
  log += String(lTyp, HEX);
  log += F(" val=");
  log += lvalue;
  addLogMove(LOG_LEVEL_DEBUG, log);
}

// ============================================================
// Werte parsen: SML-Auto (Model 3)
// ============================================================
void p155_parseValuesSMLAuto()
{
  // FIX2: Bei >4 Bytes (z.B. int64) obere Bytes ignorieren, untere 4 nehmen
  // (wie Model 1 – energy-Werte liegen im unteren 32-bit-Bereich)
  int startByte = (p155_anzBytes > 4) ? (p155_anzBytes - 4) : 0;
  int readBytes = (p155_anzBytes > 4) ? 4 : p155_anzBytes;

  uint32_t rawU = 0;
  for (int i = 0; i < readBytes; i++)
    rawU = (rawU << 8) | (uint8_t)p155_rxBuffer[startByte + i];

  float lvalue = 0.0f;
  if (p155_autoDataTyp == 5) // signed int
  {
    int32_t raw = (int32_t)rawU;
    // Vorzeichenerweiterung nur nötig wenn < 4 Bytes gelesen
    if (readBytes == 1 && (rawU & 0x80))
      raw |= (int32_t)0xFFFFFF00;
    else if (readBytes == 2 && (rawU & 0x8000))
      raw |= (int32_t)0xFFFF0000;
    lvalue = (float)raw;
  }
  else // unsigned (typ 6) oder unbekannt
  {
    lvalue = (float)rawU;
  }

  float scaledValue = (p155_scaler != 0)
                          ? lvalue * powf(10.0f, (float)p155_scaler)
                          : lvalue;

  p155_myDataAuto[p155_registerAct].value = scaledValue;

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log = F("SML-Auto: reg=");
    log += p155_registerAct;
    log += F(" typ=");
    log += p155_autoDataTyp;
    log += F(" bytes=");
    log += p155_anzBytes;
    log += F(" start=");
    log += startByte;
    log += F(" scaler=");
    log += p155_scaler;
    log += F(" raw=");
    log += lvalue;
    log += F(" val=");
    log += scaledValue;
    addLogMove(LOG_LEVEL_INFO, log);
  }
}

// ============================================================
// D0 Werte parsen (unverändert)
// ============================================================
void p155_parseValuesD0()
{
  String log = F("D0 Parse: ID=");
  log += p155_rxID;
  for (int i = 1; i < p155_outputOptionsAct; i++)
  {
    if (p155_rxID == p155_myDataD0[i].p155_rxID)
    {
      p155_myDataD0[i].value = String(p155_rxBuffer).toFloat();
      log += F(" → val=");
      log += p155_myDataD0[i].value;
      break;
    }
  }
  addLogMove(LOG_LEVEL_DEBUG, log);
}

bool p155_byteArrayCompare(byte a1[], int a1len, byte a2[], int a2len)
{
  if (a1len != a2len)
    return false;
  for (int i = 0; i < a1len; i++)
    if (a1[i] != a2[i])
      return false;
  return true;
}

void p155_deleteValues(unsigned int model)
{
  if (model == 0)
    for (int i = 0; i < P155_NR_OUTPUT_OPTIONS_MODEL0; i++)
      p155_myDataD0[i].value = 0;
  else if (model == 1)
    for (int i = 0; i < P155_NR_OUTPUT_OPTIONS_MODEL1; i++)
      p155_myDataSML[i].value = 0;
  else if (model == 2)
    for (int i = 0; i < P155_NR_OUTPUT_OPTIONS_MODEL2; i++)
      p155_myDataDTZ[i].value = 0;
  else if (model == 3)
    for (int i = 0; i < P155_NR_OUTPUT_OPTIONS_MODEL3; i++)
      p155_myDataAuto[i].value = 0;
}

#endif // USES_P155
