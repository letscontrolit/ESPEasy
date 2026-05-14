#include "_Plugin_Helper.h"

#ifdef USES_P155

// #######################################################################################################
// ######################## Plugin 155: Energy - Smartmeter ########################
// #######################################################################################################
//
// Models:
//   P155_MODEL_D0  = D0  (IEC 62056-21, 7E1, 9600 Baud)
//   P155_MODEL_SML = SML (dynamic TL-parsing, works with all SML-compliant meters)
//
// SML operation:
//   After the OBIS code, each SML-ListEntry contains these fields,
//   each preceded by a TL-byte (Type-Length):
//     Status | Time | Unit | Scaler (int8) | Value
//   TL-byte: Bits 7-4 = Type (5=int signed, 6=uint, 7=list, 0=optional)
//            Bits 3-0 = Total length incl. TL-byte (0 = field absent)
//            For lists (Type 7): Bits 3-0 = number of child elements (direct)
//   The parser reads these fields dynamically - no hardcoded byte offsets.

// #######################################################################################################

# define PLUGIN_155
# define PLUGIN_ID_155 155
# define PLUGIN_NAME_155 "SML - Smartmeter"

# define P155_MODEL PCONFIG(0)
# define P155_MODEL_LABEL PCONFIG_LABEL(0)
# define P155_QUERY1 PCONFIG(1)
# define P155_QUERY2 PCONFIG(2)
# define P155_QUERY3 PCONFIG(3)
# define P155_QUERY4 PCONFIG(4)

// Model index defines
# define P155_MODEL_D0 0
# define P155_MODEL_SML 1

# define P155_MODEL_DFLT P155_MODEL_SML
# define P155_BAUDRATE 9600
# define P155_QUERY1_DFLT 1
# define P155_QUERY2_DFLT 3
# define P155_QUERY3_DFLT 2
# define P155_QUERY4_DFLT 0

# define P155_NR_OUTPUT_VALUES 4
# define P155_NR_OUTPUT_OPTIONS_D0 6
# define P155_NR_OUTPUT_OPTIONS_SML 13
# define P155_QUERY1_CONFIG_POS 1
# define P155_RX_BUFFER 64 // enlarged for 64-bit values

// OBIS index defines - index 0 is an unused placeholder so that
// the #define constants (1-based) map directly to array indices.
# define Q3D_TOTAL_ACTIVE_ENERGY 1
# define Q3D_POWER_L1 2
# define Q3D_POWER_L2 3
# define Q3D_POWER_L3 4
# define Q3D_POWER_TOTAL 5

# define DD3_TOTAL_ACTIVE_ENERGY_PLUS 1
# define DD3_POWER_L1 2
# define DD3_POWER_L2 3
# define DD3_POWER_L3 4
# define DD3_POWER_TOTAL 5
# define DD3_TOTAL_ACTIVE_ENERGY_MINUS 6
# define DD3_VOLT_L1 7
# define DD3_VOLT_L2 8
# define DD3_VOLT_L3 9
# define DD3_CURRENT_L1 10
# define DD3_CURRENT_L2 11
# define DD3_CURRENT_L3 12

# include <ESPeasySerial.h>

ESPeasySerial *P155_MySerial = nullptr;

// Forward declarations
const __FlashStringHelper* p155_getQueryString(uint8_t query,
                                               uint8_t model);
const __FlashStringHelper* p155_getQueryValueString(uint8_t query,
                                                    uint8_t model);
unsigned int               p155_getRegister(uint8_t query,
                                            uint8_t model);
float                      p155_readVal(uint8_t      query,
                                        unsigned int model);
void                       p155_handleSerialInD0();
void                       p155_handleSerialInSML();
void                       p155_parseValuesD0();
void                       p155_parseValuesSML();
bool                       p155_readUint32(int       startByte,
                                           int       numBytes,
                                           uint32_t& result);
bool                       p155_readInt32(int      startByte,
                                          int      numBytes,
                                          int32_t& result);
bool                       p155_byteArrayCompare(byte a1[],
                                                 int  a1len,
                                                 byte a2[],
                                                 int  a2len);
void p155_deleteValues(unsigned int model);

// ============================================================
// Data structures
// ============================================================

struct p155_dataStructD0
{
  String p155_rxID;
  float  value;
  p155_dataStructD0(const String& xID, float xvalue) : p155_rxID(xID), value(xvalue) {}

};

p155_dataStructD0 p155_myDataD0[P155_NR_OUTPUT_OPTIONS_D0] = {
  p155_dataStructD0("x-x:x.x.x*x",    0.0f), // [0] placeholder
  p155_dataStructD0("1-0:1.8.0*255",  0.0f), // [1] Total active energy import
  p155_dataStructD0("1-0:21.7.0*255", 0.0f), // [2] Power L1
  p155_dataStructD0("1-0:41.7.0*255", 0.0f), // [3] Power L2
  p155_dataStructD0("1-0:61.7.0*255", 0.0f), // [4] Power L3
  p155_dataStructD0("1-0:1.7.0*255",  0.0f), // [5] Power L123
};

struct p155_dataStructSML
{
  byte  p155_rxOrbis[6];
  float value;
  p155_dataStructSML(byte xOrbis[6]) : value(0.0f)
  {
    for (int i = 0; i < 6; i++) {
      p155_rxOrbis[i] = xOrbis[i];
    }
  }

};

// OBIS codes
byte p155_rxOrbis0[6]  = { 0, 0, 0, 0, 0, 0 };
byte p155_rxOrbis1[6]  = { 1, 0, 1, 8, 0, 255 };  // 1-0:1.8.0   Total import
byte p155_rxOrbis2[6]  = { 1, 0, 21, 7, 0, 255 }; // 1-0:21.7.0  Power L1
byte p155_rxOrbis3[6]  = { 1, 0, 41, 7, 0, 255 }; // 1-0:41.7.0  Power L2
byte p155_rxOrbis4[6]  = { 1, 0, 61, 7, 0, 255 }; // 1-0:61.7.0  Power L3
byte p155_rxOrbis5[6]  = { 1, 0, 16, 7, 0, 255 }; // 1-0:16.7.0  Total power
byte p155_rxOrbis6[6]  = { 1, 0, 2, 8, 0, 255 };  // 1-0:2.8.0   Total export
byte p155_rxOrbis7[6]  = { 1, 0, 32, 7, 0, 255 }; // 1-0:32.7.0  Voltage L1
byte p155_rxOrbis8[6]  = { 1, 0, 52, 7, 0, 255 }; // 1-0:52.7.0  Voltage L2
byte p155_rxOrbis9[6]  = { 1, 0, 72, 7, 0, 255 }; // 1-0:72.7.0  Voltage L3
byte p155_rxOrbis10[6] = { 1, 0, 31, 7, 0, 255 }; // 1-0:31.7.0  Current L1
byte p155_rxOrbis11[6] = { 1, 0, 51, 7, 0, 255 }; // 1-0:51.7.0  Current L2
byte p155_rxOrbis12[6] = { 1, 0, 71, 7, 0, 255 }; // 1-0:71.7.0  Current L3

p155_dataStructSML p155_myDataSML[P155_NR_OUTPUT_OPTIONS_SML] = {
  p155_dataStructSML(p155_rxOrbis0),              // [0] placeholder
  p155_dataStructSML(p155_rxOrbis1),              // [1] Total import
  p155_dataStructSML(p155_rxOrbis2),              // [2] Power L1
  p155_dataStructSML(p155_rxOrbis3),              // [3] Power L2
  p155_dataStructSML(p155_rxOrbis4),              // [4] Power L3
  p155_dataStructSML(p155_rxOrbis5),              // [5] Total power
  p155_dataStructSML(p155_rxOrbis6),              // [6] Total export
  p155_dataStructSML(p155_rxOrbis7),              // [7] Voltage L1
  p155_dataStructSML(p155_rxOrbis8),              // [8] Voltage L2
  p155_dataStructSML(p155_rxOrbis9),              // [9] Voltage L3
  p155_dataStructSML(p155_rxOrbis10),             // [10] Current L1
  p155_dataStructSML(p155_rxOrbis11),             // [11] Current L2
  p155_dataStructSML(p155_rxOrbis12),             // [12] Current L3
};

// ============================================================
// State variables
// ============================================================
boolean p155_MyInit    = false;
uint8_t p155_step      = 0;
uint8_t p155_charsRead = 0;
char    p155_rxBuffer[P155_RX_BUFFER]{};
char    p155_ringBuffer[8]{};
byte    p155_rxOrbis[6]{};
String  p155_rxID;

int p155_anzBytes{};
int p155_registerAct{};
int p155_outputOptionsAct{};

// SML parser state
float   p155_scaleFactor  = 1.0f; // 10^scaler, calculated once in state 32
uint8_t p155_autoSubState = 0;    // current field: 0=Status,1=Time,2=Unit,3=Scaler,4=Value
uint8_t p155_skipBytes    = 0;    // bytes still to skip
uint8_t p155_autoDataTyp  = 0;    // SML type of Value field (5=signed int, 6=uint)
uint8_t p155_listElems    = 0;    // remaining child elements for list type (type=7)

// ============================================================
// Plugin main function
// ============================================================
boolean Plugin_155(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number             = PLUGIN_ID_155;
      dev.Type               = DEVICE_TYPE_SERIAL; // enables serial port selector in UI
      dev.VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.Ports              = 0;
      dev.PullUpOption       = false;
      dev.InverseLogicOption = false;
      dev.FormulaOption      = true;
      dev.ValueCount         = P155_NR_OUTPUT_VALUES;
      dev.SendDataOption     = true;
      dev.TimerOption        = true;
      dev.GlobalSyncOption   = true;
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
          safe_strncpy(ExtraTaskSettings.TaskDeviceValueNames[i],
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
      P155_MODEL  = P155_MODEL_DFLT;
      P155_QUERY1 = P155_QUERY1_DFLT;
      P155_QUERY2 = P155_QUERY2_DFLT;
      P155_QUERY3 = P155_QUERY3_DFLT;
      P155_QUERY4 = P155_QUERY4_DFLT;
      success     = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *options_model[] = {
          F("D0"),
          F("SML"),
        };
        FormSelectorOptions selector(NR_ELEMENTS(options_model), options_model);
        selector.reloadonchange = true;
        selector.addFormSelector(F("Model Type"), P155_MODEL_LABEL, P155_MODEL);
      }
      {
        const uint8_t model         = PCONFIG(0);
        const uint8_t outputOptions = (model == P155_MODEL_SML)
                                        ? P155_NR_OUTPUT_OPTIONS_SML
                                        : P155_NR_OUTPUT_OPTIONS_D0;

        const __FlashStringHelper *options[outputOptions];

        for (int i = 0; i < outputOptions; ++i) {
          options[i] = p155_getQueryString(i, model);
        }

        for (uint8_t i = 0; i < P155_NR_OUTPUT_VALUES; ++i) {
          sensorTypeHelper_loadOutputSelector(event, i + P155_QUERY1_CONFIG_POS,
                                              i, outputOptions, options);
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
        const uint8_t choice       = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i,
                                            p155_getQueryValueString(choice, model));
      }
      p155_MyInit = false;
      success     = true;
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

      p155_outputOptionsAct = (P155_MODEL == P155_MODEL_SML)
                                ? P155_NR_OUTPUT_OPTIONS_SML
                                : P155_NR_OUTPUT_OPTIONS_D0;

      const int16_t serial_rx      = CONFIG_PIN1;
      const int16_t serial_tx      = CONFIG_PIN2;
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);

      P155_MySerial = new ESPeasySerial(port, serial_rx, serial_tx, false, P155_RX_BUFFER);

      if (P155_MySerial == nullptr) {
        break;
      }

      const uint32_t serialConfig = (P155_MODEL == P155_MODEL_D0) ? SERIAL_7E1 : SERIAL_8N1;
      P155_MySerial->begin(P155_BAUDRATE, serialConfig);

      p155_step         = 0;
      p155_scaleFactor  = 1.0f;
      p155_autoSubState = 0;
      p155_skipBytes    = 0;
      p155_autoDataTyp  = 0;
      p155_listElems    = 0;
      p155_MyInit       = true;
      success           = true;

      if (loglevelActiveFor(LOG_LEVEL_INFO))
      {
        String log = F("Smartmeter: Init=");
        log += event->TaskIndex;
        log += F(" Model=");
        log += P155_MODEL;
        log += F(" Port=");
        log += (int)port;
        log += F(" RX=");
        log += serial_rx;
        log += F(" TX=");
        log += serial_tx;
        addLogMove(LOG_LEVEL_INFO, log);
      }
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
      p155_deleteValues(P155_MODEL_D0);
      p155_deleteValues(P155_MODEL_SML);
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
      if (P155_MODEL == P155_MODEL_D0) {
        p155_handleSerialInD0();
      }
      else {
        p155_handleSerialInSML();
      }
      success = true;
      break;
    }
  } // switch
  return success;
}

// ============================================================
// Helper functions
// ============================================================
float p155_readVal(uint8_t query, unsigned int model)
{
  if ((model == P155_MODEL_D0) && (query < P155_NR_OUTPUT_OPTIONS_D0)) {
    return p155_myDataD0[query].value;
  }

  if ((model == P155_MODEL_SML) && (query < P155_NR_OUTPUT_OPTIONS_SML)) {
    return p155_myDataSML[query].value;
  }
  return 0.0f;
}

unsigned int p155_getRegister(uint8_t query, uint8_t model)
{
  if (model == P155_MODEL_D0)
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
  else if (model == P155_MODEL_SML)
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
  return 0;
}

const __FlashStringHelper* p155_getQueryString(uint8_t query, uint8_t model)
{
  if (model == P155_MODEL_D0)
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
  else if (model == P155_MODEL_SML)
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
  return F("");
}

const __FlashStringHelper* p155_getQueryValueString(uint8_t query, uint8_t model)
{
  if (model == P155_MODEL_D0)
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
  else if (model == P155_MODEL_SML)
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
  return F("");
}

// SML start sequence (ring buffer order: newest byte first)
// Stream: 1B 1B 1B 1B 01 01 01 01  →  ring buffer reversed: 01 01 01 01 1B 1B 1B 1B
static constexpr uint8_t P155_SML_startseq[] = { 0x01, 0x01, 0x01, 0x01, 0x1B, 0x1B, 0x1B, 0x1B };

// ============================================================
// Serial handler: SML (dynamic TL parsing)
// ============================================================
void p155_handleSerialInSML()
{
  if (nullptr == P155_MySerial)
  {
    static uint32_t lastLogTime{};

    if (timePassedSince(lastLogTime) > 1000)
    {
      addLog(LOG_LEVEL_INFO, F("SML: Serial nullptr"));
      lastLogTime = millis();
    }
    return;
  }

  const unsigned long start = millis();
  size_t available          = P155_MySerial->available();

  while (available && timePassedSince(start) < 10) // while (P155_MySerial->available() && timePassedSince(start) < 100)
  {
    byte b = P155_MySerial->read();
    available--;

    if (available == 0)
    {
      available = P155_MySerial->available();
    }

    // Ring buffer for SML start-sequence detection: 1B 1B 1B 1B 01 01 01 01
    for (int i = 7; i > 0; i--) {
      p155_ringBuffer[i] = p155_ringBuffer[i - 1];
    }
    p155_ringBuffer[0] = b;

    // static uint64_t p155_rxWindow = 0;
    //  per Byte (one Operation instead of 8 writings in loop):
    // p155_rxWindow = (p155_rxWindow << 8) | b;

    //// Compare (one Operation instead memcmp with 8 Bytes):
    // static constexpr uint64_t P155_SML_startseq = 0x1B1B1B1B01010101ULL;
    // if (p155_rxWindow == P155_SML_startseq)

    if (memcmp(p155_ringBuffer, P155_SML_startseq, NR_ELEMENTS(P155_SML_startseq)) == 0)
    {
      p155_step         = 11;
      p155_charsRead    = 0;
      p155_registerAct  = 0;
      p155_anzBytes     = 0;
      p155_scaleFactor  = 1.0f;
      p155_autoSubState = 0;
      p155_skipBytes    = 0;
      p155_listElems    = 0;
    }

    switch (p155_step)
    {
      // -------------------------------------------------------
      // States 11-13: find OBIS code
      // -------------------------------------------------------
      case 11: // find ListEntry tag 0x77

        if (b == 0x77)
        {
          p155_step      = 12;
          p155_charsRead = 0;
        }
        break;

      case 12: // length byte must be 0x07 (6 OBIS bytes + TL)
        p155_step = (b == 0x07) ? 13 : 11;
        break;

      case 13: // collect 6 OBIS bytes

        if (p155_charsRead < 6) {
          p155_rxOrbis[p155_charsRead++] = b;
        }

        if (p155_charsRead >= 6)
        {
          p155_charsRead   = 0;
          p155_registerAct = 0;

          // search OBIS Number in table
          for (int i = 1; i < p155_outputOptionsAct; i++)
          {
            if (p155_byteArrayCompare(p155_rxOrbis, 6, p155_myDataSML[i].p155_rxOrbis, 6))
            {
              p155_registerAct = i;
              break;
            }
          }

          if (p155_registerAct != 0)
          {
            p155_scaleFactor  = 1.0f;
            p155_autoSubState = 0;
            p155_skipBytes    = 0;
            p155_listElems    = 0;
            p155_step         = 30;
            p155_charsRead    = 0;
          }
          else
          {
            p155_step = 11; // not found, keep searching
          }
        }
        break;

      // -------------------------------------------------------
      // States 30-33: dynamic TL parsing
      //
      // TL-byte format:
      //   Bits 7-4: Type (0=optional/absent, 5=signed int, 6=uint, 7=list)
      //   Bits 3-0: Total length incl. TL-byte
      //     Primitive: dataBytes = bits3-0 - 1
      //     List (Type 7): child count = bits3-0  (direct, NOT length-1)
      //
      // autoSubState: 0=Status, 1=Time, 2=Unit, 3=Scaler, 4=Value
      // -------------------------------------------------------
      case 30: // read TL-byte of current field
      {
        uint8_t tlLen     = (b & 0x0F);
        uint8_t tlTyp     = (b >> 4) & 0x07;
        uint8_t dataBytes = (tlLen > 0) ? (tlLen - 1) : 0;

# ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG) && (p155_registerAct > 0))
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
# endif // ifndef BUILD_NO_DEBUG

        if (p155_autoSubState < 3)
        {
          // Status (0), Time (1), Unit (2)
          if (tlTyp == 7)
          {
            // List type: bits3-0 = direct child count (NOT byte length)
            // e.g. 0x72: 2 children (typical SML_Time with timestamp)
            if (tlLen == 0)
            {
              p155_autoSubState++;
            }
            else
            {
              p155_listElems = tlLen;
              p155_step      = 36;
            }
          }
          else if (dataBytes == 0)
          {
            p155_autoSubState++; // Field not present → continue
            // State 30 remains, reads next TL
          }
          else
          {
            p155_skipBytes = dataBytes;
            p155_step      = 31; // skip bytes
          }
        }
        else if (p155_autoSubState == 3)
        {
          // Scaler field
          if (dataBytes == 0)
          {
            p155_scaleFactor = 1.0f; // no scaler present
            p155_autoSubState++;
          }
          else
          {
            p155_step = 32; // read 1 scaler byte
          }
        }
        else if (p155_autoSubState == 4)
        {
          // Value field
          if (dataBytes == 0)
          {
            p155_step = 11;
          } // no value - abort
          else
          {
            p155_autoDataTyp = tlTyp;
            p155_anzBytes    = dataBytes;
            p155_charsRead   = 0;
            p155_step        = 33;
          }
        }
        break;
      }

      case 31: // skip bytes (primitive Status/Time/Unit)
        p155_skipBytes--;

        if (p155_skipBytes == 0)
        {
          p155_autoSubState++;
          p155_step = 30; // read next TL-Byte
        }
        break;

      case 32: // read scaler byte (int8, signed)
      {
        const int8_t scalerRaw = b;
        p155_scaleFactor = (scalerRaw != 0) ? powf(10.0f, (float)(scalerRaw)) : 1.0f;
        p155_autoSubState++;
        p155_step = 30; // continue with Value-TL
        break;
      }

      case 33: // collect value bytes

        if (p155_charsRead < P155_RX_BUFFER - 1) {
          p155_rxBuffer[p155_charsRead++] = b;
        }

        if (p155_charsRead >= p155_anzBytes)
        {
          p155_parseValuesSML();
          p155_step = 11;
        }
        break;

      // -------------------------------------------------------
      // States 36-37: skip list type (type=7) child elements
      // Required for SML_Time with timestamp, e.g.:
      //   72 62 01 65 xx xx xx xx  (list with 2 child elements)
      // -------------------------------------------------------
      case 36: // read TL-byte of a list child element
      {
        uint8_t cLen  = (b & 0x0F);
        uint8_t cTyp  = (b >> 4) & 0x07;
        uint8_t cData = (cLen > 0) ? (cLen - 1) : 0;

        if (cTyp == 7)
        {
          // Nested list: replace current element with its children
          p155_listElems = p155_listElems - 1 + (cLen > 0 ? cLen : 0);
        }
        else
        {
          p155_listElems--;

          if (cData > 0)
          {
            p155_skipBytes = cData;
            p155_step      = 37; // skip Data, then back to 36 or 30
            break;
          }
        }

        if (p155_listElems == 0)
        {
          p155_autoSubState++;
          p155_step = 30;
        }
        break; // otherwise 36
      }

      case 37: // skip data bytes of a list child element
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
            p155_step = 36; // next childelement
          }
        }
        break;

      default:
        p155_step         = 0;
        p155_charsRead    = 0;
        p155_anzBytes     = 0;
        p155_registerAct  = 0;
        p155_autoSubState = 0;
        p155_listElems    = 0;
        break;
    }
  } // while serial available

# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG))
  {
    String log1 = F("SML: step=");
    log1 += p155_step;
    log1 += F(" sub=");
    log1 += p155_autoSubState;
    log1 += F(" reg=");
    log1 += p155_registerAct;
    addLogMove(LOG_LEVEL_DEBUG, log1);
  }
# endif // ifndef BUILD_NO_DEBUG
}

// ============================================================
// D0 serial handler
// ============================================================
void p155_handleSerialInD0()
{
  if (nullptr == P155_MySerial)
  {
    static uint32_t lastLogTime{};

    if (timePassedSince(lastLogTime) > 1000)
    {
      addLog(LOG_LEVEL_INFO, F("D0: Serial nullptr"));
      lastLogTime = millis();
    }
    return;
  }

  const unsigned long start = millis();
  size_t available          = P155_MySerial->available();

  while (available && timePassedSince(start) < 10)
  {
    --available;

    if (available == 0)
    {
      available = P155_MySerial->available();
    }

    // check bounds at the top of the loop
    if (p155_charsRead >= P155_RX_BUFFER - 1)
    {
      p155_charsRead = 0;
    }

    char c = (char)P155_MySerial->read();

    if (c == '(')
    {
      p155_rxBuffer[p155_charsRead] = '\0';
      p155_rxID                     = String(p155_rxBuffer);
      p155_charsRead                = 0;
    }
    else if (c == ')')
    {
      p155_rxBuffer[p155_charsRead] = '\0';

      if (p155_charsRead > 1) {
        p155_parseValuesD0();
      }
      p155_charsRead = 0;
    }
    else if ((c == 0x0D) || (c == 0x0A))
    {
      p155_charsRead = 0;
    }
    else
    {
      p155_rxBuffer[p155_charsRead++] = c;
    }
  }
}

// ============================================================
// Parse values: SML (dynamic TL)
// ============================================================
void p155_parseValuesSML()
{
  if (p155_registerAct >= P155_NR_OUTPUT_OPTIONS_SML) {
    return;
  }

  const int startByte = (p155_anzBytes > 4) ? (p155_anzBytes - 4) : 0;
  const int readBytes = (p155_anzBytes > 4) ? 4 : p155_anzBytes;

  float lvalue = 0.0f;

  if (p155_autoDataTyp == 5) // signed int
  {
    int32_t raw = 0;

    if (!p155_readInt32(startByte, readBytes, raw)) {
      return;
    }
    lvalue = raw;
  }
  else // unsigned (type 6) or unknown
  {
    uint32_t raw = 0;

    if (!p155_readUint32(startByte, readBytes, raw)) {
      return;
    }
    lvalue = raw;
  }

  p155_myDataSML[p155_registerAct].value = lvalue * p155_scaleFactor;

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log = F("SML: reg=");
    log += p155_registerAct;
    log += F(" typ=");
    log += p155_autoDataTyp;
    log += F(" bytes=");
    log += p155_anzBytes;
    log += F(" factor=");
    log += p155_scaleFactor;
    log += F(" val=");
    log += p155_myDataSML[p155_registerAct].value;
    addLogMove(LOG_LEVEL_INFO, log);
  }
}

// ============================================================
// D0 parse values
// ============================================================
void p155_parseValuesD0()
{
  int i = 1;

  for (; i < p155_outputOptionsAct; i++)
  {
    if (p155_rxID == p155_myDataD0[i].p155_rxID)
    {
      p155_myDataD0[i].value = String(p155_rxBuffer).toFloat();
      break;
    }
  }

# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG))
  {
    String log = F("D0 Parse: ID=");
    log += p155_rxID;

    if (i < p155_outputOptionsAct)
    {
      log += F(" reg=");
      log += i;
      log += F(" val=");
      log += p155_myDataD0[i].value;
    }
    else
    {
      log += F(" (not found)");
    }
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
# endif // ifndef BUILD_NO_DEBUG
}

// ============================================================
// Buffer read helpers (bounds-checked)
// ============================================================

// Read numBytes (1-4) big-endian from rxBuffer at startByte → uint32_t
bool p155_readUint32(int startByte, int numBytes, uint32_t& result)
{
  if ((numBytes < 1) || (numBytes > 4)) {
    return false;
  }

  if ((startByte + numBytes) > p155_charsRead) {
    return false;
  }

  if ((startByte + numBytes) > P155_RX_BUFFER) {
    return false;
  }
  result = 0;

  for (int i = 0; i < numBytes; i++) {
    result = (result << 8) | (uint8_t)p155_rxBuffer[startByte + i];
  }
  return true;
}

// Read numBytes (1-4) big-endian from rxBuffer at startByte → int32_t (sign-extended)
//
// Sign extension via arithmetic right shift:
//   1. Shift LEFT so the sign bit of the original data lands at bit 31
//   2. Shift RIGHT (arithmetic) fills all upper bits with the sign bit
//
// Example: 0xFF read as 1 byte
//   shift = (4-1)*8 = 24
//   0x000000FF << 24 = 0xFF000000   (sign bit at bit 31)
//   0xFF000000 >> 24 = 0xFFFFFFFF   (arithmetic fill → -1)
bool p155_readInt32(int startByte, int numBytes, int32_t& result)
{
  uint32_t raw = 0;

  if (!p155_readUint32(startByte, numBytes, raw)) {
    return false;
  }
  const int shift = (4 - numBytes) * 8;
  result = static_cast<int32_t>(raw << shift) >> shift;
  return true;
}

bool p155_byteArrayCompare(byte a1[], int a1len, byte a2[], int a2len)
{
  if (a1len != a2len) {
    return false;
  }

  for (int i = 0; i < a1len; i++) {
    if (a1[i] != a2[i]) {
      return false;
    }
  }
  return true;
}

void p155_deleteValues(unsigned int model)
{
  if (model == P155_MODEL_D0) {
    for (int i = 0; i < P155_NR_OUTPUT_OPTIONS_D0; i++) {
      p155_myDataD0[i].value = 0;
    }
  }
  else if (model == P155_MODEL_SML) {
    for (int i = 0; i < P155_NR_OUTPUT_OPTIONS_SML; i++) {
      p155_myDataSML[i].value = 0;
    }
  }
}

#endif // USES_P155
