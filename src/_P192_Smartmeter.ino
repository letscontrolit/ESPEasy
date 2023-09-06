#include "_Plugin_Helper.h"

#ifdef USES_P192

// #######################################################################################################
// ######################## Plugin 192:Energy - Smartmeter ########################
// #######################################################################################################

#define PLUGIN_192
#define PLUGIN_ID_192 192
#define PLUGIN_NAME_192 "Energy - Smartmeter [TESTING]"

#define P192_MODEL PCONFIG(0)
#define P192_MODEL_LABEL PCONFIG_LABEL(0)
#define P192_QUERY1 PCONFIG(1)
#define P192_QUERY2 PCONFIG(2)
#define P192_QUERY3 PCONFIG(3)
#define P192_QUERY4 PCONFIG(4)

#define P192_MODEL_DFLT 1  // Q3D
#define P192_BAUDRATE 9600 // 9600 baud
#define P192_QUERY1_DFLT 1 // Energy (kWh)
#define P192_QUERY2_DFLT 3 //
#define P192_QUERY3_DFLT 2 //
#define P192_QUERY4_DFLT 0 //

#define P192_NR_OUTPUT_VALUES 4
#define P192_NR_OUTPUT_OPTIONS_MODEL0 6
#define P192_NR_OUTPUT_OPTIONS_MODEL1 13 // SML Standard Bsp: DD3
#define P192_NR_OUTPUT_OPTIONS_MODEL2 4  // Holley DTZ541
#define P192_QUERY1_CONFIG_POS 1
#define P192_RX_BUFFER 256 // 1024

// IDs fuer die Zuordnung der Werte in verschiedenen Querys
#define Q3D_TOTAL_ACTIVE_ENERGY 1;
#define Q3D_POWER_L1 2;
#define Q3D_POWER_L2 3;
#define Q3D_POWER_L3 4;
#define Q3D_POWER_TOTAL 5;

#define DD3_TOTAL_ACTIVE_ENERGY_PLUS 1;
#define DD3_POWER_L1 2;
#define DD3_POWER_L2 3;
#define DD3_POWER_L3 4;
#define DD3_POWER_TOTAL 5;
#define DD3_TOTAL_ACTIVE_ENERGY_MINUS 6;
#define DD3_VOLT_L1 7;
#define DD3_VOLT_L2 8;
#define DD3_VOLT_L3 9;
#define DD3_CURRENT_L1 10;
#define DD3_CURRENT_L2 11;
#define DD3_CURRENT_L3 12;

#include <ESPeasySerial.h>

// These pointers may be used among multiple instances of the same plugin,
// as long as the same serial settings are used.
ESPeasySerial *P192_MySerial = nullptr;

// Forward declaration helper functions
const __FlashStringHelper *p192_getQueryString(uint8_t query, uint8_t model);
const __FlashStringHelper *p192_getQueryValueString(uint8_t query, uint8_t model);
unsigned int p192_getRegister(uint8_t query, uint8_t model);
float p192_readVal(uint8_t query, unsigned int model);
void p192_handleSerialInD0();
void p192_handleSerialInSML();
void p192_parseValuesD0();
void p192_parseValuesSML();
bool p192_byteArrayCompare(byte a1[], int a1len, byte a2[], int a2len);
void p192_deleteValues();

struct p192_dataStructD0
{
  String p192_rxID = "x-x:x.x.x*x";

  float value;
  // Konstruktor
  p192_dataStructD0(String xp192_rxID, float xvalue)
  {
    p192_rxID = xp192_rxID;
    value = xvalue;
  }
};

p192_dataStructD0 p192_myDataD0[P192_NR_OUTPUT_OPTIONS_MODEL0] =
    {
        p192_dataStructD0("x-x:x.x.x*x", 0.0),
        p192_dataStructD0("1-0:1.8.0*255", 0.0),  // Total_Active_Energy_Consumption
        p192_dataStructD0("1-0:21.7.0*255", 0.0), // Power L1
        p192_dataStructD0("1-0:41.7.0*255", 0.0), // Power L2
        p192_dataStructD0("1-0:61.7.0*255", 0.0), // Power L3
        p192_dataStructD0("1-0:1.7.0*255", 0.0)   // Power L123
};

struct p192_dataStructSML
{
  int posData;
  float factor;
  int datatyp; // 0 = float; 1 = int32
  byte p192_rxOrbis[6];
  String p192_rxID = "x-x:x.x.x*x";

  float value;
  // Konstruktor
  p192_dataStructSML(int xposData, float xfactor, byte xp192_rxOrbis[6], float xvalue)
  {
    posData = xposData;
    factor = xfactor;
    for (int i = 0; i < 6; i++)
    {
      p192_rxOrbis[i] = xp192_rxOrbis[i];
    }
    value = xvalue;
  }
};
byte p192_rxOrbis0[6] = {0, 0, 0, 0, 0, 0};
byte p192_rxOrbis1[6] = {1, 0, 1, 8, 0, 255};   // Total_Active_Energy_Consumption
byte p192_rxOrbis2[6] = {1, 0, 21, 7, 0, 255};  // PowerL1
byte p192_rxOrbis3[6] = {1, 0, 41, 7, 0, 255};  // PowerL2
byte p192_rxOrbis4[6] = {1, 0, 61, 7, 0, 255};  // PowerL3
byte p192_rxOrbis5[6] = {1, 0, 16, 7, 0, 255};  // PowerL123 "1-0:1.7.0*255"
byte p192_rxOrbis6[6] = {1, 0, 2, 8, 0, 255};   // Total_Active_Energy_FeedIn "1-0:2.8.0*255"
byte p192_rxOrbis7[6] = {1, 0, 32, 7, 0, 255};  // Volt_L1
byte p192_rxOrbis8[6] = {1, 0, 52, 7, 0, 255};  // Volt_L2
byte p192_rxOrbis9[6] = {1, 0, 72, 7, 0, 255};  // Volt_L3 "1-0:72.7.0*255"
byte p192_rxOrbis10[6] = {1, 0, 31, 7, 0, 255}; // Current_L1
byte p192_rxOrbis11[6] = {1, 0, 51, 7, 0, 255}; // Current_L2 "1-0:51.7.0*255"
byte p192_rxOrbis12[6] = {1, 0, 71, 7, 0, 255}; // Current_L3

p192_dataStructSML p192_myDataSML[P192_NR_OUTPUT_OPTIONS_MODEL1] =
    {
        p192_dataStructSML(0, 1.0, p192_rxOrbis0, 0.0),
        p192_dataStructSML(11, 0.0001, p192_rxOrbis1, 0.0), // 15
        p192_dataStructSML(7, 1.0, p192_rxOrbis2, 0.0),
        p192_dataStructSML(7, 1.0, p192_rxOrbis3, 0.0),
        p192_dataStructSML(7, 1.0, p192_rxOrbis0, 0.0),
        p192_dataStructSML(7, 1.0, p192_rxOrbis5, 0.0),
        p192_dataStructSML(7, 0.0001, p192_rxOrbis6, 0.0),
        p192_dataStructSML(7, 0.1, p192_rxOrbis7, 0.0),
        p192_dataStructSML(7, 0.1, p192_rxOrbis8, 0.0),
        p192_dataStructSML(7, 0.1, p192_rxOrbis9, 0.0),
        p192_dataStructSML(7, 0.01, p192_rxOrbis10, 0.0),
        p192_dataStructSML(7, 0.01, p192_rxOrbis11, 0.0),
        p192_dataStructSML(7, 0.01, p192_rxOrbis12, 0.0)};

p192_dataStructSML p192_myDataDTZ[P192_NR_OUTPUT_OPTIONS_MODEL2] =
    {
        p192_dataStructSML(0, 1.0, p192_rxOrbis0, 0.0),
        p192_dataStructSML(18, 0.0001, p192_rxOrbis1, 0.0),  // 4,19
        p192_dataStructSML(7, 1.0, p192_rxOrbis5, 0.0),      // 4,15
        p192_dataStructSML(14, 0.0001, p192_rxOrbis6, 0.0)}; // 4,15

// Aussortieren
// Variables
boolean p192_MyInit = false;
uint8_t p192_step = 0;
uint8_t p192_charsRead = 0;
char p192_rxBuffer[30];
char p192_ringBuffer[8];
byte p192_rxOrbis[6]; //|1|0|2|8|0|255
String p192_rxID;

int p192_anzBytes;  
int p192_posDataAct;
int p192_registerAct;
int p192_outputOptionsAct;

boolean Plugin_192(uint8_t function, struct EventStruct *event, String &string)
{
  boolean success = false;

  switch (function)
  {

  case PLUGIN_DEVICE_ADD:
  {
    Device[++deviceCount].Number = PLUGIN_ID_192;
    Device[deviceCount].Type = DEVICE_TYPE_DUMMY; // connected through 3 datapins
    Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_QUAD;
    Device[deviceCount].Ports = 0;
    Device[deviceCount].PullUpOption = false;
    Device[deviceCount].InverseLogicOption = false;
    Device[deviceCount].FormulaOption = true;
    Device[deviceCount].ValueCount = P192_NR_OUTPUT_VALUES;
    Device[deviceCount].SendDataOption = true;
    Device[deviceCount].TimerOption = true;
    Device[deviceCount].GlobalSyncOption = true;
    break;
  }

  case PLUGIN_GET_DEVICENAME:
  {
    string = F(PLUGIN_NAME_192);
    break;
  }

  case PLUGIN_GET_DEVICEVALUENAMES:
  {
    for (uint8_t i = 0; i < VARS_PER_TASK; ++i)
    {
      if (i < P192_NR_OUTPUT_VALUES)
      {
        uint8_t choice = PCONFIG(i + P192_QUERY1_CONFIG_POS);
        uint8_t model = PCONFIG(0); // TODO
        safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            p192_getQueryValueString(choice, model),
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
    P192_MODEL = P192_MODEL_DFLT;
    P192_QUERY1 = P192_QUERY1_DFLT;
    P192_QUERY2 = P192_QUERY2_DFLT;
    P192_QUERY3 = P192_QUERY3_DFLT;
    P192_QUERY4 = P192_QUERY4_DFLT;

    success = true;
    break;
  }

  case PLUGIN_WEBFORM_LOAD:
  {
    {
      const __FlashStringHelper *options_model[3] = {F("D0"), F("SML"), F("DTZ541")};
      addFormSelector(F("Model Type"), P192_MODEL_LABEL, 3, options_model, nullptr, P192_MODEL);
    }
    {
      const uint8_t model = PCONFIG(0);                      // TODO
      uint8_t outputOptions = P192_NR_OUTPUT_OPTIONS_MODEL0; // Default immer Model0
      if (model == 1)
        outputOptions = P192_NR_OUTPUT_OPTIONS_MODEL1;
      else if (model == 2)
        outputOptions = P192_NR_OUTPUT_OPTIONS_MODEL2;
      // In a separate scope to free memory of String array as soon as possible
      //sensorTypeHelper_webformLoad_simple();//sensorTypeHelper_webformLoad_header();
      const __FlashStringHelper *options[outputOptions];

      for (int i = 0; i < outputOptions; ++i)
      {
        const uint8_t model = PCONFIG(0); // TODO
        options[i] = p192_getQueryString(i, model);
      }
      for (uint8_t i = 0; i < P192_NR_OUTPUT_VALUES; ++i)
      {
        const uint8_t pconfigIndex = i + P192_QUERY1_CONFIG_POS;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, outputOptions, options);
      }
    }
    success = true;
    break;
  }

  case PLUGIN_WEBFORM_SAVE:
  {
    // Save output selector parameters.
    for (uint8_t i = 0; i < P192_NR_OUTPUT_VALUES; ++i)
    {
      const uint8_t pconfigIndex = i + P192_QUERY1_CONFIG_POS;
      const uint8_t choice = PCONFIG(pconfigIndex);
      const uint8_t model = PCONFIG(0); // TODO
      sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, p192_getQueryValueString(choice, model));
    }

    P192_MODEL = getFormItemInt(P192_MODEL_LABEL);

    p192_MyInit = false; // Force device setup next time
    success = true;
    break;
  }

  case PLUGIN_INIT:
  {
    if (P192_MySerial != nullptr)
    {
      delete P192_MySerial;
      P192_MySerial = nullptr;
    }
    p192_deleteValues(P192_MODEL);
    if (P192_MODEL == 1)
      p192_outputOptionsAct = P192_NR_OUTPUT_OPTIONS_MODEL1;
    else if (P192_MODEL == 2)
      p192_outputOptionsAct = P192_NR_OUTPUT_OPTIONS_MODEL2;
    else
      p192_outputOptionsAct = P192_NR_OUTPUT_OPTIONS_MODEL0;
    // serial0:port=2 rxPin=3 txPin=1; serial1:port=4 rxPin=13 txPin=15; Serial2: port=5 rxPin=16 txPin=17
    CONFIG_PORT = 5;  // Fest auf Serial2
    CONFIG_PIN1 = 16; // Fest auf Serial2
    CONFIG_PIN2 = 17; // Fest auf Serial2
    P192_MySerial = new (std::nothrow) ESPeasySerial(static_cast<ESPEasySerialPort>(CONFIG_PORT), CONFIG_PIN1, CONFIG_PIN2, false, P192_RX_BUFFER);
    if (P192_MySerial == nullptr)
    {
      break;
    }

    unsigned int baudrate = P192_BAUDRATE;
    uint32_t config;
    if (P192_MODEL == 0)
      config = SERIAL_7E1;
    else
      config = SERIAL_8N1;

    P192_MySerial->begin(baudrate, config); // 7E1

    p192_step = 0;
    p192_MyInit = true;

    if (p192_MyInit)
    {
      String log = F("Smartmeter: Init=");
      log += event->TaskIndex;
      log += F(" port=");
      log += CONFIG_PORT;
      log += F(" rxPin=");
      log += CONFIG_PIN1;
      log += F(" txPin=");
      log += CONFIG_PIN2;
      log += F(" BAUDRATE=");
      log += baudrate;
      log += F(" Model=");
      log += P192_MODEL;
      addLogMove(LOG_LEVEL_INFO, log);
    }
    break;
  }

  case PLUGIN_EXIT:
  {
    p192_MyInit = false;
    if (P192_MySerial != nullptr)
    {
      delete P192_MySerial;
      P192_MySerial = nullptr;
    }
    p192_deleteValues(P192_MODEL);
    break;
  }

  case PLUGIN_READ:
  {
    if (p192_MyInit)
    {
      int model = P192_MODEL;
      UserVar[event->BaseVarIndex] = p192_readVal(P192_QUERY1, model);
      UserVar[event->BaseVarIndex + 1] = p192_readVal(P192_QUERY2, model);
      UserVar[event->BaseVarIndex + 2] = p192_readVal(P192_QUERY3, model);
      UserVar[event->BaseVarIndex + 3] = p192_readVal(P192_QUERY4, model);
      success = true;
      break;
    }
    break;
  }

  case PLUGIN_TEN_PER_SECOND:
  {
    if (P192_MODEL == 1 || P192_MODEL == 2)
      p192_handleSerialInSML(P192_MODEL);
    else
      p192_handleSerialInD0();
    success = true;
    break;
  }
  }
  return success;
}

float p192_readVal(uint8_t query, unsigned int model)
{
  if (model == 0)
  { // Q3D
    return p192_myDataD0[query].value;
  }
  else if (model == 1)
  { // DD3
    return p192_myDataSML[query].value;
  }
  else if (model == 2)
  { // DTZ541
    return p192_myDataDTZ[query].value;
  }
  return 0;
}

unsigned int p192_getRegister(uint8_t query, uint8_t model)
{
  if (model == 0)
  { // Q3D
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
  else if (model == 1)
  { // DD3
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
  { // DTZ541
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

const __FlashStringHelper *p192_getQueryString(uint8_t query, uint8_t model)
{

  if (model == 0)
  { // Q3D
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
  else if (model == 1)
  { // DD3
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
      return F("Voltage L1");
    case 8:
      return F("Voltage L2");
    case 9:
      return F("Voltage L3");
    case 10:
      return F("Current L1");
    case 11:
      return F("Current L2");
    case 12:
      return F("Current L3");
    }
  }
  else if (model == 2)
  { // DTZ541
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

const __FlashStringHelper *p192_getQueryValueString(uint8_t query, uint8_t model)
{
  if (model == 0)
  { // Q3D
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
  else if (model == 1)
  { // DD3
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
  { // DTZ541
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

void p192_handleSerialInSML(unsigned int model)
{
  String log1 = F("SML: Log1=");
  String logdata1 = F("SML: 1=");
  String logdata2 = F("SML: 2=");
  String logdata3 = F("SML: 3=");
  String logdata4 = F("SML: 4=");
  String logdata5 = F("SML: 5=");
  String logdata6 = F("SML: 6=");
  String logdata7 = F("SML: 7=");
  String logdata8 = F("SML: 8=");
  if (nullptr == P192_MySerial)
  {
    addLog(LOG_LEVEL_INFO, F("SML: Task handleSerialIn nullptr"));
    return;
  }
  unsigned long RXWait = 100;
  unsigned long timeOut = millis() + RXWait; // Zeit um Serial Buffer zu lesen in ms
  byte ltyp = 0;
  log1 += P192_MySerial->available();
  log1 += "-";
  while (P192_MySerial->available() && millis() < timeOut) // Test fehlende Daten
  {
    byte b = P192_MySerial->read();
    if (true)
    {
      if (logdata1.length() < 110)
      {
        logdata1 += "|";
        logdata1 += b;
      }
      else if (logdata2.length() < 110)
      {
        logdata2 += "|";
        logdata2 += b;
      }
      else if (logdata3.length() < 110)
      {
        logdata3 += "|";
        logdata3 += b;
      }
      else if (logdata4.length() < 110)
      {
        logdata4 += "|";
        logdata4 += b;
      }
      else if (logdata5.length() < 110)
      {
        logdata5 += "|";
        logdata5 += b;
      }
      else if (logdata6.length() < 110)
      {
        logdata6 += "|";
        logdata6 += b;
      }
      else if (logdata7.length() < 110)
      {
        logdata7 += "|";
        logdata7 += b;
      }
      else
      {
        logdata8 += "|";
        logdata8 += b;
      }
    }

    for (int i = 7; i > 0; i--)
    {
      p192_ringBuffer[i] = p192_ringBuffer[i - 1];
    }
    p192_ringBuffer[0] = b;

    if ((p192_ringBuffer[0] == 1) &&
        (p192_ringBuffer[1] == 1) &&
        (p192_ringBuffer[2] == 1) &&
        (p192_ringBuffer[3] == 1) &&
        (p192_ringBuffer[4] == 27) &&
        (p192_ringBuffer[5] == 27) &&
        (p192_ringBuffer[6] == 27) &&
        (p192_ringBuffer[7] == 27))
    { //zum testen Log ab hier leeren
      logdata1 = F("SML: 1:");
      logdata2 = F("SML: 2:");
      logdata3 = F("SML: 3:");
      logdata4 = F("SML: 4:");
      logdata5 = F("SML: 5:");
      logdata6 = F("SML: 6:");
      logdata7 = F("SML: 7:");
      logdata8 = F("SML: 8:");
      p192_step = 11;
    }

    switch (p192_step)
    {
    case 11: // Anfangszeichen 119;0x77 finden
      if (b == 119)
      {
        p192_step = 12;
        p192_charsRead = 0;
        p192_registerAct = 0;
        p192_anzBytes = 0;
        p192_posDataAct = 0;
      }
      break;
    case 12: // Nach dem Startzeichen kommt erst die Laenge hier 7
      if (b == 7)
        p192_step = 13;
      else
        p192_step = 11; // Wenn meine Zeichen folge nicht 119 ; 7 ist dann wieder auf 119 warten
      break;
    case 13:                                          // Alle Zeichen der ORBIS Kennung sammeln
      if (p192_charsRead >= 0 && p192_charsRead <= 5) // Bereich des Array abfragen
      {
        p192_rxOrbis[p192_charsRead++] = b;
      }
      if (p192_charsRead >= 6) // Zeichen 0 - 5 --> 6 Zeichen also fertig
      {
        log1 += F("|");
        log1 += p192_rxOrbis[0];
        log1 += F("-");
        log1 += p192_rxOrbis[1];
        log1 += F(":");
        log1 += p192_rxOrbis[2];
        log1 += F(".");
        log1 += p192_rxOrbis[3];
        log1 += F(".");
        log1 += p192_rxOrbis[4];
        log1 += F(".");
        log1 += p192_rxOrbis[5];
        log1 += F("|");

        p192_charsRead = 0;
        // Find Kennung

        for (int i = 1; i < p192_outputOptionsAct; i++)
        {
          if (model == 1)
          {
            if (p192_byteArrayCompare(p192_rxOrbis, sizeof(p192_rxOrbis), p192_myDataSML[i].p192_rxOrbis, sizeof(p192_myDataSML[i].p192_rxOrbis)))
            {
              p192_registerAct = i;
              p192_posDataAct = p192_myDataSML[i].posData;
              break;
            }
          }
          else if (model == 2)
          {
            if (p192_byteArrayCompare(p192_rxOrbis, sizeof(p192_rxOrbis), p192_myDataDTZ[i].p192_rxOrbis, sizeof(p192_myDataDTZ[i].p192_rxOrbis)))
            {
              p192_registerAct = i;
              p192_posDataAct = p192_myDataDTZ[i].posData;
              break;
            }
          }
        }
        if (p192_registerAct != 0) // Kennung gefunden also auswerten
        {
          p192_step = 20;
          p192_charsRead = 0;
        }
        else
        {
          p192_step = 11;
        }
      }
      break;
    case 20: //Datentyp und Anzahl der Bytes ermitteln 
      p192_rxBuffer[p192_charsRead++] = b;
      p192_anzBytes = ltyp = 0;
      if ((p192_charsRead) >= (p192_posDataAct))
      {
        p192_step = 21;
        ltyp = b;
        if (ltyp == 82 || ltyp == 98) // 8Bit Datentypen
          p192_anzBytes = 1;
        else if (ltyp == 83 || ltyp == 99) // 16Bit Datentypen
          p192_anzBytes = 2;
        else if (ltyp == 85 || ltyp == 101) // 32Bit Datentypen
          p192_anzBytes = 4;
        else if (ltyp == 89 || ltyp == 105) // 64Bit Datentypen
          p192_anzBytes = 8;
        else
          p192_anzBytes = 4;
      }    
      break;
    case 21:
      p192_rxBuffer[p192_charsRead++] = b;

      if ((p192_charsRead >= (p192_posDataAct + p192_anzBytes)))
      {
        p192_step = 11;        
        p192_charsRead = 0;
        p192_parseValuesSML(model);
      }
      else if ((p192_charsRead >= 30)) //Abbruch
      {
        p192_step = 11;        
      }
      break;
    default: // Warten bis Startzeichen Schrittkette startet
      p192_step = 0;
      p192_charsRead = 0;
      p192_anzBytes = 0;
      p192_registerAct = 0;
      p192_posDataAct = 0;
      break;
    }
    log1 += p192_step;
    log1 += F(".");
    log1 += p192_charsRead;
    log1 += F(";");
  }
  
  addLogMove(LOG_LEVEL_DEBUG, log1);
  addLogMove(LOG_LEVEL_DEBUG, logdata1);
  addLogMove(LOG_LEVEL_DEBUG, logdata2);
  addLogMove(LOG_LEVEL_DEBUG, logdata3);
  addLogMove(LOG_LEVEL_DEBUG, logdata4);
  addLogMove(LOG_LEVEL_DEBUG, logdata5);
  addLogMove(LOG_LEVEL_DEBUG, logdata6);
  addLogMove(LOG_LEVEL_DEBUG, logdata7);
  addLogMove(LOG_LEVEL_DEBUG, logdata8);
}

void p192_handleSerialInD0()
{
  // addLog(LOG_LEVEL_INFO, F("  : Fct handleSerialIn"));
  String log1 = F("D0: Log1=");
  String logdata1 = F("D0: SerialData=");

  if (nullptr == P192_MySerial)
  {
    addLog(LOG_LEVEL_INFO, F("Smartmeter D0: Task handleSerialIn nullptr"));
    return;
  }
  unsigned long RXWait = 10;
  unsigned long timeOut = millis() + RXWait; // Zeit um Serial Buffer zu lesen in ms

  while (P192_MySerial->available() && millis() < timeOut)
  {
    char c = (char)P192_MySerial->read();
    logdata1 += c;
    if (c == '(')
    {
      p192_rxBuffer[p192_charsRead] = '\0';
      p192_rxID = String(p192_rxBuffer);
      p192_charsRead = 0;
    }
    else if (c == ')')
    {
      p192_rxBuffer[p192_charsRead] = '\0';
      log1 += F(" p192_rxID= ");
      log1 += String(p192_rxID);

      log1 += F(" p192_rxBuffer= ");
      log1 += String(p192_rxBuffer);

      log1 += F(" Counter= ");
      log1 += String(p192_charsRead);

      if (p192_charsRead > 1 && sizeof(p192_rxID) > 1)
      {
        p192_parseValuesD0();
      }
      p192_charsRead = 0;
    }
    else if (c == 0x0D || c == 0x0A)
    {
      // on CR or LF
      p192_charsRead = 0;
      log1 += F(" CR or LF ");
    }
    else
    {
      p192_rxBuffer[p192_charsRead++] = c;
    }
  }
  addLogMove(LOG_LEVEL_DEBUG, log1);
  addLogMove(LOG_LEVEL_DEBUG, logdata1);
}

void p192_parseValuesSML(unsigned int model)
{
  String logSML = F("Smartmeter SML: Parse ");
  logSML += model;
  logSML += F(":");
  logSML += p192_rxOrbis[0];
  logSML += F("|");
  logSML += p192_rxOrbis[1];
  logSML += F("|");
  logSML += p192_rxOrbis[2];
  logSML += F("|");
  logSML += p192_rxOrbis[3];
  logSML += F("|");
  logSML += p192_rxOrbis[4];
  logSML += F("|");
  logSML += p192_rxOrbis[5];

  logSML += F(",");
  logSML += p192_registerAct;
  logSML += F(",");
  logSML += p192_posDataAct;

  boolean tempbool = false;
  if (model == 1)
    tempbool = p192_byteArrayCompare(p192_rxOrbis, sizeof(p192_rxOrbis), p192_myDataSML[p192_registerAct].p192_rxOrbis, sizeof(p192_myDataSML[p192_registerAct].p192_rxOrbis));
  else if (model == 2)
    tempbool = p192_byteArrayCompare(p192_rxOrbis, sizeof(p192_rxOrbis), p192_myDataDTZ[p192_registerAct].p192_rxOrbis, sizeof(p192_myDataDTZ[p192_registerAct].p192_rxOrbis));
  if (tempbool)
  {
    logSML += F("Gefunden ");
    int lLen = p192_posDataAct;
    logSML += F("|");
    logSML += (byte)p192_rxBuffer[lLen];
    logSML += F(".");
    logSML += (byte)p192_rxBuffer[lLen + 1];
    logSML += F(".");
    logSML += (byte)p192_rxBuffer[lLen + 2];
    logSML += F(".");
    logSML += (byte)p192_rxBuffer[lLen + 3];
    logSML += F(".");
    logSML += (byte)p192_rxBuffer[lLen + 4];
    logSML += F(".");
    logSML += (byte)p192_rxBuffer[lLen + 5];
    logSML += F(".");
    logSML += (byte)p192_rxBuffer[lLen + 6];
    logSML += F(".");
    logSML += (byte)p192_rxBuffer[lLen + 7];
    logSML += F("|");

    byte lTyp = p192_rxBuffer[lLen - 1];
    logSML += lTyp;
    logSML += F("|");
    int8_t lint8 = 0;
    uint8_t luint8 = 0;
    int16_t lint16 = 0;
    uint16_t luint16 = 0;
    int32_t lint32 = 0;
    uint32_t luint32 = 0;
    float lvalue = 0.0;
    switch (lTyp)
    {
    case 82: // Typ integer 8Bit
      lint8 = (p192_rxBuffer[lLen]);
      lvalue = (float)lint8;
      break;
    case 83: // Typ integer 16Bit
      lint16 = (p192_rxBuffer[lLen] << 8) + (p192_rxBuffer[lLen + 1] << 0);
      lvalue = (float)lint16;
      break;
    case 85: // Typ integer 32Bit
      lint32 = (p192_rxBuffer[lLen] << 24) + (p192_rxBuffer[lLen + 1] << 16) + (p192_rxBuffer[lLen + 2] << 8) + (p192_rxBuffer[lLen + 3] << 0);
      lvalue = (float)lint32;
      break;
    case 89: // Typ integer 64Bit
      lint32 = (p192_rxBuffer[lLen + 4] << 24) + (p192_rxBuffer[lLen + 5] << 16) + (p192_rxBuffer[lLen + 6] << 8) + (p192_rxBuffer[lLen + 7] << 0);
      lvalue = (float)lint32;
      break;
    case 98: // Typ unsigned integer 8Bit
      luint8 = (p192_rxBuffer[lLen]);
      lvalue = (float)luint8;
      break;
    case 99: // Typ unsigned integer 16Bit
      luint16 = (p192_rxBuffer[lLen] << 8) + (p192_rxBuffer[lLen + 1] << 0);
      lvalue = (float)luint16;
      break;
    case 101: // Typ unsigned integer 32Bit
      luint32 = (p192_rxBuffer[lLen] << 24) + (p192_rxBuffer[lLen + 1] << 16) + (p192_rxBuffer[lLen + 2] << 8) + (p192_rxBuffer[lLen + 3] << 0);
      lvalue = (float)luint32;
      break;
    case 105: // Typ unsigned integer 64Bit
      luint32 = (p192_rxBuffer[lLen + 4] << 24) + (p192_rxBuffer[lLen + 5] << 16) + (p192_rxBuffer[lLen + 6] << 8) + (p192_rxBuffer[lLen + 7] << 0);
      lvalue = (float)luint32;
      break;
    }
    logSML += lvalue;
    if (model == 1)
      p192_myDataSML[p192_registerAct].value = lvalue * p192_myDataSML[p192_registerAct].factor;
    if (model == 2)
      p192_myDataDTZ[p192_registerAct].value = lvalue * p192_myDataDTZ[p192_registerAct].factor;
  }
  addLogMove(LOG_LEVEL_DEBUG, logSML);
}

void p192_parseValuesD0()
{
  String logD0 = F("Smartmeter D0: Parse ");

  logD0 += p192_rxID;
  for (int i = 1; i < p192_outputOptionsAct; i++)
  {
    if (p192_rxID == p192_myDataD0[i].p192_rxID)
    {
      logD0 += F("Gefunden ");
      p192_myDataD0[i].value = String(p192_rxBuffer).toFloat();
      break;
    }
  }
  addLogMove(LOG_LEVEL_DEBUG, logD0);
}

bool p192_byteArrayCompare(byte a1[], int a1len, byte a2[], int a2len)
{
  int lsize = a1len;
  if (lsize != a2len)
    return false;
  for (int i = 0; i < lsize; i++)
    if (a1[i] != a2[i])
      return false;

  return true;
}

void p192_deleteValues(unsigned int model)
{
  if (model == 0)
  {
    for (int i = 0; i < P192_NR_OUTPUT_OPTIONS_MODEL0; i++)
    {
      p192_myDataD0[i].value = 0;
    }
  }
  else if (model == 1)
  {
    for (int i = 0; i < P192_NR_OUTPUT_OPTIONS_MODEL1; i++)
    {
      p192_myDataSML[i].value = 0;
    }
  }
  else if (model == 2)
  {
    for (int i = 0; i < P192_NR_OUTPUT_OPTIONS_MODEL2; i++)
    {
      p192_myDataDTZ[i].value = 0;
    }
  }
}

#endif // USES_P192
