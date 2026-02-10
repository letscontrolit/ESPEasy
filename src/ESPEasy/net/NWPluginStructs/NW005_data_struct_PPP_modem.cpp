#include "../NWPluginStructs/NW005_data_struct_PPP_modem.h"

#ifdef USES_NW005

# include "../../../src/ESPEasyCore/Controller.h" // For SendStatus  (should we move that function?)

# include "../../../src/Globals/Settings.h"

# include "../../../src/Helpers/ESPEasy_time_calc.h"
# include "../../../src/Helpers/StringConverter.h"
# include "../../../src/Helpers/_Plugin_Helper_serial.h"

# include "../../../src/WebServer/HTML_wrappers.h"
# include "../../../src/WebServer/Markup.h"
# include "../../../src/WebServer/Markup_Forms.h"
# include "../../../src/WebServer/ESPEasy_key_value_store_webform.h"

# include "../Globals/NetworkState.h"
# include "../Helpers/_NWPlugin_Helper_webform.h"
# include <ESPEasySerialPort.h>
# include <PPP.h>

// # include <esp_modem_api.h>
// #include <esp_modem_c_api_types.h>

# define NW_PLUGIN_ID  5
# define NW_PLUGIN_INTERFACE   PPP

namespace ESPEasy {
namespace net {
namespace ppp {

static NWPluginData_static_runtime stats_and_cache(&NW_PLUGIN_INTERFACE, F("PPP"));


// Keys as used in the Key-value-store
# define NW005_KEY_SERIAL_PORT          1
# define NW005_KEY_PIN_RX               2
# define NW005_KEY_PIN_TX               3
# define NW005_KEY_PIN_RTS              4
# define NW005_KEY_PIN_CTS              5
# define NW005_KEY_PIN_RESET            6
# define NW005_KEY_PIN_RESET_ACTIVE_LOW 7
# define NW005_KEY_PIN_RESET_DELAY      8
# define NW005_KEY_BAUDRATE             9
# define NW005_KEY_FLOWCTRL             10
# define NW005_KEY_MODEM_MODEL          11
# define NW005_KEY_APN                  12
# define NW005_KEY_SIM_PIN              13
# define NW005_KEY_PIN_DTR              14

const __FlashStringHelper * NW005_data_struct_PPP_modem::getLabelString(uint32_t key, bool displayString, KVS_StorageType::Enum& storageType)
{
  storageType = KVS_StorageType::Enum::int8_type;

  switch (key)
  {
    case NW005_KEY_SERIAL_PORT:
      return displayString ? F("Serial Port") : F("serPort");
    case NW005_KEY_PIN_RX: return F("RX");
    case NW005_KEY_PIN_TX: return F("TX");
    case NW005_KEY_PIN_RTS: return F("RTS");
    case NW005_KEY_PIN_CTS: return F("CTS");
    case NW005_KEY_PIN_DTR: return F("DTR");
    case NW005_KEY_PIN_RESET: return displayString ? F("Reset") : F("rst");
    case NW005_KEY_PIN_RESET_ACTIVE_LOW:
      storageType = KVS_StorageType::Enum::bool_type;
      return displayString ? F("Reset Active Low") : F("rst_act_low");
    case NW005_KEY_PIN_RESET_DELAY:
      storageType = KVS_StorageType::Enum::uint16_type;
      return displayString ? F("Reset Delay") : F("rst_delay");
    case NW005_KEY_BAUDRATE:
      storageType = KVS_StorageType::Enum::uint32_type;
      return displayString ? F("Baud rate") : F("baudrate");
    case NW005_KEY_FLOWCTRL:
      return displayString ? F("Flow Control") : F("flowctrl");
    case NW005_KEY_MODEM_MODEL: return displayString ? F("Modem Model") : F("mmodel");
    case NW005_KEY_APN:
      storageType = KVS_StorageType::Enum::string_type;
      return displayString ? F("Access Point Name (APN)") : F("apn");
    case NW005_KEY_SIM_PIN:
      storageType = KVS_StorageType::Enum::string_type;
      return displayString ? F("SIM Card Pin") : F("pin");

  }
  return F("");
}

int32_t NW005_data_struct_PPP_modem::getNextKey(int32_t key)
{
  if (key == -1) { return NW005_KEY_SERIAL_PORT; }

  if (key < NW005_KEY_PIN_DTR) { return key + 1; }
  return -2;
}

int32_t NW005_data_struct_PPP_modem::getNextKey_noCredentials(int32_t key)
{
  if (key == -1) { return NW005_KEY_SERIAL_PORT; }

  if (key < NW005_KEY_MODEM_MODEL) { return key + 1; }

  if (key < NW005_KEY_PIN_DTR) { return NW005_KEY_PIN_DTR; }
  return -2;
}

NW005_data_struct_PPP_modem::NW005_data_struct_PPP_modem(networkIndex_t networkIndex)
  : NWPluginData_base(nwpluginID_t(NW_PLUGIN_ID), networkIndex, &NW_PLUGIN_INTERFACE)
{
  stats_and_cache.clear(networkIndex);
  nw_event_id = Network.onEvent(NW005_data_struct_PPP_modem::onEvent);
}

WebFormItemParams NW005_makeWebFormItemParams(uint32_t key) {
  KVS_StorageType::Enum storageType;
  const __FlashStringHelper*label = NW005_data_struct_PPP_modem::getLabelString(key, true, storageType);
  const __FlashStringHelper*id    = NW005_data_struct_PPP_modem::getLabelString(key, false, storageType);

  return WebFormItemParams(label, id, storageType, key);
}

NW005_data_struct_PPP_modem::~NW005_data_struct_PPP_modem() {
  if (_modem_task_data.modem_taskHandle) {
    vTaskDelete(_modem_task_data.modem_taskHandle);
    _modem_task_data.modem_taskHandle = NULL;
  }
  NW_PLUGIN_INTERFACE.mode(ESP_MODEM_MODE_COMMAND);

  const int dtrPin = _kvs->getValueAsInt_or_default(NW005_KEY_PIN_DTR, -1);

  if (dtrPin != -1) {
    digitalWrite(dtrPin, LOW);
  }

  NW_PLUGIN_INTERFACE.end();
  _modem_task_data.modem_initialized = false;

  if (nw_event_id != 0) {
    Network.removeEvent(nw_event_id);
  }
  nw_event_id = 0;
  stats_and_cache.processEvent_and_clear();
}

enum class NW005_modem_model {
  generic = 1,
  SIM7600 = 2,
  SIM7070 = 3,
  SIM7000 = 4,
  BG96    = 5,
  SIM800  = 6

};

const __FlashStringHelper* toString(NW005_modem_model NW005_modemmodel)
{
  switch (NW005_modemmodel)
  {
    case NW005_modem_model::generic: return F("Generic");
    case NW005_modem_model::SIM7600: return F("SIM7600");
    case NW005_modem_model::SIM7070: return F("SIM7070");
    case NW005_modem_model::SIM7000: return F("SIM7000");
    case NW005_modem_model::BG96:    return F("BG96");
    case NW005_modem_model::SIM800:  return F("SIM800");
  }
  return F("unknown");
}

ppp_modem_model_t to_ppp_modem_model_t(NW005_modem_model NW005_modemmodel)
{
  switch (NW005_modemmodel)
  {
    case NW005_modem_model::generic: return PPP_MODEM_GENERIC;
    case NW005_modem_model::SIM7600: return PPP_MODEM_SIM7600;
    case NW005_modem_model::SIM7070: return PPP_MODEM_SIM7070;
    case NW005_modem_model::SIM7000: return PPP_MODEM_SIM7000;
    case NW005_modem_model::BG96:    return PPP_MODEM_BG96;
    case NW005_modem_model::SIM800:  return PPP_MODEM_SIM800;
  }
  return PPP_MODEM_GENERIC;
}

int doGetRSSI()
{
  const int rssi_raw = NW_PLUGIN_INTERFACE.RSSI();

  if (rssi_raw == 0) { return -113; }

  if (rssi_raw == 99) { return 0; }
  return map(rssi_raw, 2, 30, -109, -53);
}

String NW005_data_struct_PPP_modem::getRSSI() const
{
  if (!_modem_task_data.modem_initialized) { return F("-"); }
  const int rssi_raw = NW_PLUGIN_INTERFACE.RSSI();

  if (rssi_raw == 99) { return F("-"); } // Not known or not detectable

  if (rssi_raw == 0) { return F("<= -113"); }

  if (rssi_raw >= 31) { return F(">= -51"); }
  return String(map(rssi_raw, 2, 30, -109, -53));
}

String NW005_data_struct_PPP_modem::getBER() const
{
  if (_modem_task_data.modem_initialized) {
    switch (NW_PLUGIN_INTERFACE.BER())
    {
      case 0: return F("<0.01 %");
      case 1: return F("0.01 % ... 0.1 %");
      case 2: return F("0.1 % ... 0.5 %");
      case 3: return F("0.5 % ... 1 %");
      case 4: return F("1 % ... 2 %");
      case 5: return F("2 % ... 4 %");
      case 6: return F("4 % ... 8 %");
      case 7: return F(">= 8 %");
      case 99: break; // Not known or not detectable
    }
  }
  return F("-");
}

float NW005_data_struct_PPP_modem::getBER_float() const
{
  if (_modem_task_data.modem_initialized) {
    switch (NW_PLUGIN_INTERFACE.BER())
    {
      case 0: return 0.01f; // F("<0.01 %");
      case 1: return 0.1f;  // F("0.01 % ... 0.1 %");
      case 2: return 0.5f;  // F("0.1 % ... 0.5 %");
      case 3: return 1.0f;  // F("0.5 % ... 1 %");
      case 4: return 2.0f;  // F("1 % ... 2 %");
      case 5: return 4.0f;  // F("2 % ... 4 %");
      case 6: return 8.0f;  // F("4 % ... 8 %");
      case 7: return 16.0f; // F(">= 8 %");
      case 99: break;       // Not known or not detectable
    }
  }
  return NAN;
}

bool NW005_data_struct_PPP_modem::attached() const
{
  if (!_modem_task_data.modem_initialized) { return false; }
  return NW_PLUGIN_INTERFACE.attached();
}

String NW005_data_struct_PPP_modem::IMEI() const
{
  if (!_modem_task_data.modem_initialized) { return EMPTY_STRING; }
  return NW_PLUGIN_INTERFACE.IMEI();
}

String NW005_data_struct_PPP_modem::operatorName() const
{
  if (!_modem_task_data.modem_initialized) { return EMPTY_STRING; }
  return NW_PLUGIN_INTERFACE.operatorName();
}

const __FlashStringHelper* NW005_decode_label(int sysmode_index, uint8_t i, String& value_str, bool& has_dBm_unit)
{
  const __FlashStringHelper*res = F("");

  switch (sysmode_index)
  {
    case 0:
    {
      // No service system mode
      const __FlashStringHelper*labels[]
      {
        F("System Mode"), F("Operation Mode")
      };

      if (i < NR_ELEMENTS(labels)) { res = labels[i]; }
      break;
    }
    case 1:
    {
      // GSM system mode
      const __FlashStringHelper*labels[]
      {
        F("System Mode"), F("Operation Mode"), F("MCC-MNC"), F("LAC"), F("Cell ID"), F("Absolute RF Ch Num"), F("RxLev"),
        F("Track LO Adjust"), F("C1-C2")
      };

      if (i < NR_ELEMENTS(labels)) { res = labels[i]; }
      break;
    }
    case 2:
    {
      // WCDMA system mode
      const __FlashStringHelper*labels[]
      {
        F("System Mode"), F("Operation Mode"), F("MCC-MNC"), F("LAC"), F("Cell ID"), F("Frequency Band"), F("PSC"), F("Freq"), F("SSC"),
        F("EC/IO"), F("RSCP"), F("Qual"), F("RxLev"), F("TXPWR")
      };

      if (i < NR_ELEMENTS(labels)) { res = labels[i]; }
      break;
    }
    case 3:
    {
      // LTE system mode
      const __FlashStringHelper*labels[]
      {
        F("System Mode"), F("Operation Mode"), F("MCC-MNC"), F("TAC"), F("SCellID"), F("PCellID"), F("Frequency Band"), F("earfcn"),
        F("dlbw"), F("ulbw"), F("RSRQ"), F("RSRP"), F("RSSI"), F("RSSNR")
      };

      if (i < NR_ELEMENTS(labels)) {
        res = labels[i];

        switch (i)
        {
          case 10:
            // RSRQ
          {
            float val = value_str.toInt();
            val         -= 40;
            val         /= 2.0f;
            value_str    = String(val, 1);
            has_dBm_unit = true;
            break;
          }
          case 11:
            // RSRP
            value_str    = value_str.toInt() - 140;
            has_dBm_unit = true;
            break;
          case 12:
            // RSSI
            value_str    =  value_str.toInt() - 110;
            has_dBm_unit = true;
            break;
        }

      }
      break;
    }
  }
  return res;
}

void NW005_data_struct_PPP_modem::webform_load_UE_system_information(KeyValueWriter*writer)
{
  if (!NW_PLUGIN_INTERFACE.attached()) {
    // clear cached string
    _modem_task_data.AT_CPSI.clear();
    return;
  }

  if (NW_PLUGIN_INTERFACE.mode() != ESP_MODEM_MODE_CMUX) {
    NW_PLUGIN_INTERFACE.mode(ESP_MODEM_MODE_CMUX);
  }


  String res = write_AT_cmd(F("AT+CPSI?"), 1000);

  if (res.isEmpty()) { res = _modem_task_data.AT_CPSI; }

  if (!res.isEmpty() /* && res.startsWith(F("+CPSI"))*/) {
    int start_index         = 0;
    int end_index           = res.indexOf(',');
    const String systemMode = res.substring(start_index, end_index);
    addLog(LOG_LEVEL_INFO, concat(F("PPP: UE sysinfo: "), systemMode));

    const __FlashStringHelper*sysmode_str[] = {
      F("NO SERVICE"), F("GSM"), F("WCDMA"), F("LTE")
    };
    {
      String provider_name = write_AT_cmd(F("AT+CSPN?"), 1000);

      if (!provider_name.isEmpty()) {
        int pos1 = provider_name.indexOf('"');
        int pos2 = provider_name.lastIndexOf('"');

        if ((pos1 != -1) && (pos2 != pos1)) {
          writer->write({ F("Provider"), provider_name.substring(pos1 + 1, pos2) });
        }
      }
    }
    int sysmode_index = -1;

    for (int i = 0; i < NR_ELEMENTS(sysmode_str) && sysmode_index == -1; ++i) {
      if (systemMode.endsWith(sysmode_str[i])) { sysmode_index = i; }

    }

    if (sysmode_index == -1) { return; }

    // Update cached string
    _modem_task_data.AT_CPSI = res;

    //    addFormSubHeader(F("UE System Information"));

    res += ','; // Add trailing comma so we're not missing the last element

    for (int i = 0; start_index < res.length() && end_index != -1 && i < 15; ++i)
    {
      String value_str    = res.substring(start_index, end_index);
      bool   has_dBm_unit = false;
      const String label  = NW005_decode_label(sysmode_index, i, value_str, has_dBm_unit);

      if (!label.isEmpty()) {
        if (i == 0) {
          // We have some leading characters left here, so use the 'clean' strings
          KeyValueStruct kv(label, sysmode_str[sysmode_index]);

          if (has_dBm_unit) {
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
            kv.setUnit(UOM_dBm);
# endif
          }

          writer->write(kv);
        } else {
          KeyValueStruct kv(label, value_str, KeyValueStruct::Format::PreFormatted);

          if (has_dBm_unit) {
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
            kv.setUnit(UOM_dBm);
# endif
          }

          writer->write(kv);
        }
      }
      start_index = end_index + 1;
      end_index   = res.indexOf(',', start_index);
    }

  }

  if (NW_PLUGIN_INTERFACE.attached()) {
    NW_PLUGIN_INTERFACE.mode(ESP_MODEM_MODE_DATA);
  }
}

void NW005_data_struct_PPP_modem::webform_load(EventStruct *event)
{
  _load();

  addFormSubHeader(F("Device Settings"));
  {
    const int ids[] = {
      static_cast<int>(NW005_modem_model::generic),
      static_cast<int>(NW005_modem_model::SIM7600),
      static_cast<int>(NW005_modem_model::SIM7070),
      static_cast<int>(NW005_modem_model::SIM7000),
      static_cast<int>(NW005_modem_model::BG96),
      static_cast<int>(NW005_modem_model::SIM800)
    };

    const __FlashStringHelper*options[] = {
      toString(NW005_modem_model::generic),
      toString(NW005_modem_model::SIM7600),
      toString(NW005_modem_model::SIM7070),
      toString(NW005_modem_model::SIM7000),
      toString(NW005_modem_model::BG96),
      toString(NW005_modem_model::SIM800)
    };

    FormSelectorOptions selector(NR_ELEMENTS(ids), options, ids);
    showFormSelector(*_kvs, selector, NW005_makeWebFormItemParams(NW005_KEY_MODEM_MODEL));
  }

  {
    // TODO TD-er: We cannot use ESPEasySerialPort here as PPPClass needs to handle the pins using periman
    const int ids[] = {
      static_cast<int>(ESPEasySerialPort::serial0)
# if USABLE_SOC_UART_NUM > 1
      , static_cast<int>(ESPEasySerialPort::serial1)
# endif
# if USABLE_SOC_UART_NUM > 2
      , static_cast<int>(ESPEasySerialPort::serial2)
# endif
# if USABLE_SOC_UART_NUM > 3
      , static_cast<int>(ESPEasySerialPort::serial3)
# endif
# if USABLE_SOC_UART_NUM > 4
      , static_cast<int>(ESPEasySerialPort::serial4)
# endif
# if USABLE_SOC_UART_NUM > 5
      , static_cast<int>(ESPEasySerialPort::serial5)
# endif
    };

    constexpr int NR_ESPEASY_SERIAL_TYPES = NR_ELEMENTS(ids);
    const __FlashStringHelper*options[]   = {
      serialHelper_getSerialTypeLabel(ESPEasySerialPort::serial0)
# if USABLE_SOC_UART_NUM > 1
      , serialHelper_getSerialTypeLabel(ESPEasySerialPort::serial1)
# endif
# if USABLE_SOC_UART_NUM > 2
      , serialHelper_getSerialTypeLabel(ESPEasySerialPort::serial2)
# endif
# if USABLE_SOC_UART_NUM > 3
      , serialHelper_getSerialTypeLabel(ESPEasySerialPort::serial3)
# endif
# if USABLE_SOC_UART_NUM > 4
      , serialHelper_getSerialTypeLabel(ESPEasySerialPort::serial4)
# endif
# if USABLE_SOC_UART_NUM > 5
      , serialHelper_getSerialTypeLabel(ESPEasySerialPort::serial5)
# endif
    };

    FormSelectorOptions selector(NR_ELEMENTS(ids), options, ids);
    showFormSelector(*_kvs, selector, NW005_makeWebFormItemParams(NW005_KEY_SERIAL_PORT));
  }

  const int gpio_keys[] = {
    NW005_KEY_PIN_RX,
    NW005_KEY_PIN_TX,
    NW005_KEY_PIN_RTS,
    NW005_KEY_PIN_CTS,
    NW005_KEY_PIN_DTR,
    NW005_KEY_PIN_RESET
  };

  for (int i = 0; i < NR_ELEMENTS(gpio_keys); ++i)
  {
    const int key = gpio_keys[i];
    KVS_StorageType::Enum storageType;
    const __FlashStringHelper *id = getLabelString(key, false, storageType);
    PinSelectPurpose purpose      = PinSelectPurpose::Generic;
    String label                  = NW005_formatGpioLabel(key, purpose);

    int8_t pin = -1;
    _kvs->getValue(key, pin);

    addFormPinSelect(purpose, label, id, pin);
  }
  showWebformItem(
    *_kvs,
    NW005_makeWebFormItemParams(NW005_KEY_PIN_RESET_ACTIVE_LOW));
  {
    auto params = NW005_makeWebFormItemParams(NW005_KEY_PIN_RESET_DELAY);
    params._max             = 2000;
    params._defaultIntValue = 200;
    showWebformItem(*_kvs, params);
    addUnit(F("ms"));

  }
  {
    const int ids[] = {
      ESP_MODEM_FLOW_CONTROL_NONE,
      ESP_MODEM_FLOW_CONTROL_SW,
      ESP_MODEM_FLOW_CONTROL_HW
    };
    const __FlashStringHelper*options[] = {
      F("None"),
      F("Software"),
      F("Hardware")
    };

    FormSelectorOptions selector(NR_ELEMENTS(ids), options, ids);
    showFormSelector(*_kvs, selector, NW005_makeWebFormItemParams(NW005_KEY_FLOWCTRL));
    addFormNote(F("Only set flow control if RTS and CTS are used"));
  }
  {
    auto params = NW005_makeWebFormItemParams(NW005_KEY_BAUDRATE);
    params._max             = 10000000;
    params._defaultIntValue = 115200;
    showWebformItem(*_kvs, params);
  }

  addFormSubHeader(F("Connection Settings"));
  {
    auto params = NW005_makeWebFormItemParams(NW005_KEY_APN);
    params._maxLength = 64;
    showWebformItem(*_kvs, params);
    addFormNote(F("Optional, can be left empty"));
  }
  {
    auto params = NW005_makeWebFormItemParams(NW005_KEY_SIM_PIN);
    params._maxLength = 4;
    showWebformItem(*_kvs, params);
    addFormNote(F("Only numerical digits"));
  }

  if (!Settings.getNetworkEnabled(event->NetworkIndex)) { return; }


  if (!_modem_task_data.modem_initialized) {
    addFormSubHeader(F("Modem State"));
    addRowLabel(F("Modem Model"));

    if (_modem_task_data.initializing) {
      addHtml(F("Initializing ..."));
    } else {
      addHtml(F("Not found"));
    }
    return;
  }
}

void NW005_data_struct_PPP_modem::webform_save(EventStruct *event)
{
  // TODO TD-er: Move this to a central function, like done with import/export
  int32_t key = getNextKey(-1);

  while (key >= 0) {
    KVS_StorageType::Enum storageType;
    const __FlashStringHelper *id = getLabelString(key, false, storageType);
    storeWebformItem(*_kvs, key, storageType, id);
    key = getNextKey(key);
  }
  _store();
}

bool NW005_data_struct_PPP_modem::webform_getPort(KeyValueWriter *writer)
{
  if (_KVS_initialized() && _load() && writer) {
    int  serialPort      = _kvs->getValueAsInt_or_default(NW005_KEY_SERIAL_PORT, -1);
    auto serialTypeLabel = serialHelper_getSerialTypeLabel(static_cast<ESPEasySerialPort>(serialPort));

    if (serialPort >= 0) {
      const uint32_t keys[] {
        NW005_KEY_PIN_RX,
        NW005_KEY_PIN_TX,
        NW005_KEY_PIN_RTS,
        NW005_KEY_PIN_CTS,
        NW005_KEY_PIN_DTR,
        NW005_KEY_PIN_RESET
      };

      KeyValueStruct kv(EMPTY_STRING, serialTypeLabel);

      if (!writer->summaryValueOnly()) {
        writer->write({ F("Serial Type"), serialTypeLabel });
      }

      for (int i = 0; i < NR_ELEMENTS(keys); ++i) {
        int pin = _kvs->getValueAsInt_or_default(keys[i], -1);

        if (pin >= 0) {
          PinSelectPurpose purpose   = PinSelectPurpose::Generic;
          const bool   shortNotation = true;
          const String label         = NW005_formatGpioLabel(keys[i], purpose, shortNotation);

          if (writer->summaryValueOnly()) {
            kv.appendValue(strformat(F("%s: %d"),
                                     label.c_str(),
                                     pin));
          } else {
            if (writer->plainText() || !writer->allowHTML()) {
              KVS_StorageType::Enum storageType;
              writer->write({ getLabelString(keys[i], false, storageType), pin });
            } else {
              writer->write({ label, pin });
            }
          }
        }
      }

      if (writer->summaryValueOnly()) {
        writer->write(kv);
      }
    }
    return true;
  }
  return false;
}

bool NW005_data_struct_PPP_modem::write_ModemState(KeyValueWriter*modemState)
{
  //  if (!writer) { return false; }
  //  auto modemState = writer->createChild(F("Modem State"));

  if (!modemState) { return false; }

  if (!_modem_task_data.modem_initialized) {
    modemState->write({
            F("Modem Model"),
            _modem_task_data.initializing
        ? F("Initializing ...")
        : F("Not found")
          });

    return false;
  }
    # define STATE_WRITE(X, Y) modemState->write({ X, Y \
                                                 })
    # define STATE_WRITE_PRE(X, Y) modemState->write({ X, Y, KeyValueStruct::Format::PreFormatted \
                                                     })
  STATE_WRITE_PRE(F("Modem Model"), NW_PLUGIN_INTERFACE.moduleName());
  STATE_WRITE_PRE(F("IMEI"),        NW_PLUGIN_INTERFACE.IMEI());
  STATE_WRITE_PRE(F("IMSI"),        NW_PLUGIN_INTERFACE.IMSI());

  if (NW_PLUGIN_INTERFACE.attached()) {
    const String operatorName = NW_PLUGIN_INTERFACE.operatorName();
    STATE_WRITE_PRE(F("Mobile Country Code (MCC)"), operatorName.substring(0, 3));
    STATE_WRITE_PRE(F("Mobile Network Code (MNC)"), operatorName.substring(3));
    modemState->writeNote(
      F("See <a href=\"https://en.wikipedia.org/wiki/Mobile_country_code\">Wikipedia - Mobile Country Code</a>")
      );

    if (NW_PLUGIN_INTERFACE.mode() != ESP_MODEM_MODE_CMUX) {
      NW_PLUGIN_INTERFACE.mode(ESP_MODEM_MODE_CMUX);
    }
  }
  {
    KeyValueStruct kv(F("RSSI"), getRSSI());
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
    kv.setUnit(UOM_dBm);
# endif
    modemState->write(kv);
  }
  STATE_WRITE(F("BER"), getBER());

  STATE_WRITE(
    F("Radio State"),
    (NW_PLUGIN_INTERFACE.radioState() == 0) ? F("Minimal") : F("Full"));

  {
    int networkMode = NW_PLUGIN_INTERFACE.networkMode();

    if (networkMode >= 0) {
      const __FlashStringHelper*mode;

      // Result from AT+CNSMOD Show network system mode
      if (networkMode <= 8) {
        switch (networkMode)
        {
          case 0: mode = F("no service");
            break;
          case 1: mode = F("GSM");
            break;
          case 2: mode = F("GPRS");
            break;
          case 3: mode = F("EGPRS (EDGE)");
            break;
          case 4: mode = F("WCDMA");
            break;
          case 5: mode = F("HSDPA only(WCDMA)");
            break;
          case 6: mode = F("HSUPA only(WCDMA)");
            break;
          case 7: mode = F("HSPA (HSDPA and HSUPA, WCDMA)");
            break;
          case 8: mode = F("LTE");
            break;
        }
        STATE_WRITE(F("Network Mode"), mode);
      } else {
        STATE_WRITE(F("Network Mode"), networkMode);
      }
    }
  }
  {
    int batStatus                 = NW_PLUGIN_INTERFACE.batteryStatus();
    const __FlashStringHelper*str = F("Not Available");

    switch (batStatus)
    {
      case 0: str = F("Not Charging");
        break;
      case 1: str = F("Charging");
        break;
      case 2: str = F("Charging Done");
        break;
    }
    STATE_WRITE(F("Battery Status"), str);

    if (batStatus >= 0) {
      int batVolt = NW_PLUGIN_INTERFACE.batteryVoltage();

      if (batVolt >= 0) {
        KeyValueStruct kv(F("Battery Voltage"), batVolt);
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
        kv.setUnit(UOM_milliVolt);
# endif
        modemState->write(kv);
      }

      int batLevel = NW_PLUGIN_INTERFACE.batteryLevel();

      if (batLevel >= 0) {
        STATE_WRITE(F("Battery Level"), batLevel);
      }
    }
  }
    # undef STATE_WRITE
    # undef STATE_WRITE_PRE


  // TODO TD-er: Disabled for now, missing PdpContext

  /*
     esp_modem_dce_t *handle = NW_PLUGIN_INTERFACE.handle();

     if (handle) {
     int state{};

     if (esp_modem_get_network_registration_state(handle, state) == ESP_OK) {
      addRowLabel(F("Network Registration State"));
      const __FlashStringHelper*str = F("Unknown");

      switch (state)
      {
        case 0: str = F("Not registered, MT is not currently searching an operator to register to");
          break;
        case 1: str = F("Registered, home network");
          break;
        case 2: str = F("Not registered, but MT is currently trying to attach or searching an operator to register to");
          break;
        case 3: str = F("Registration denied");
          break;
        case 4: str = F("Unknown");
          break;
        case 5: str = F("Registered, Roaming");
          break;
        case 6: str = F("Registered, for SMS only, home network (NB-IoT only)");
          break;
        case 7: str = F("Registered, for SMS only, roaming (NB-IoT only)");
          break;
        case 8: str = F("Attached for emergency bearer services only");
          break;
        case 9: str = F("Registered for CSFB not preferred, home network");
          break;
        case 10: str = F("Registered for CSFB not preferred, roaming");
          break;
      }
      addHtml(str);
     }
     }
   */
  if (NW_PLUGIN_INTERFACE.attached()) {
    NW_PLUGIN_INTERFACE.mode(ESP_MODEM_MODE_DATA);
  }

  return true;
}

void NW005_begin_modem_task(void *parameter)
{
  NW005_modem_task_data*modem_task_data = static_cast<NW005_modem_task_data *>(parameter);

  if (!modem_task_data->modem_initialized) {
    modem_task_data->initializing = true;

    if (modem_task_data->dtrPin != -1) {
      digitalWrite(modem_task_data->dtrPin, LOW);
    }

    const bool res =
      NW_PLUGIN_INTERFACE.begin(
        modem_task_data->model,
        modem_task_data->uart_num,
        modem_task_data->baud_rate);
    modem_task_data->initializing = false;
    modem_task_data->logString    =  strformat(
      F("PPP: Module Name: %s, IMEI: %s"),
      NW_PLUGIN_INTERFACE.moduleName().c_str(),
      NW_PLUGIN_INTERFACE.IMEI().c_str());

    if (res) {

      uint32_t start = millis();

      do
      {
        delay(100);
      }
      while (timePassedSince(start) < 5000 && !NW_PLUGIN_INTERFACE.attached());

      NW_PLUGIN_INTERFACE.mode(ESP_MODEM_MODE_CMUX);
      modem_task_data->AT_CPSI = NW_PLUGIN_INTERFACE.cmd(F("AT+CPSI?"), 3000);

      if (modem_task_data->dtrPin != -1) {
        NW_PLUGIN_INTERFACE.cmd(F("AT&D1"), 9000);
        digitalWrite(modem_task_data->dtrPin, HIGH);
      }
    }
    modem_task_data->modem_initialized = res;
  }
  modem_task_data->modem_taskHandle = NULL;
  vTaskDelete(modem_task_data->modem_taskHandle);
}

bool NW005_data_struct_PPP_modem::init(EventStruct *event)
{
  if (!_KVS_initialized()) {
    addLog(LOG_LEVEL_ERROR, F("PPP: Could not initialize storage"));
    return false;
  }

  if (!_load()) {
    addLog(LOG_LEVEL_ERROR, F("PPP: Could not load settings"));
    return false;
  }

  NW_PLUGIN_INTERFACE.setResetPin(
    _kvs->getValueAsInt_or_default(NW005_KEY_PIN_RESET, -1),
    _kvs->getValueAsInt_or_default(NW005_KEY_PIN_RESET_ACTIVE_LOW, 0),
    _kvs->getValueAsInt_or_default(NW005_KEY_PIN_RESET_DELAY, 200));

  const int rtsPin               = _kvs->getValueAsInt_or_default(NW005_KEY_PIN_RTS, -1);
  const int ctsPin               = _kvs->getValueAsInt_or_default(NW005_KEY_PIN_CTS, -1);
  esp_modem_flow_ctrl_t flow_ctl = static_cast<esp_modem_flow_ctrl_t>(_kvs->getValueAsInt_or_default(NW005_KEY_FLOWCTRL,
                                                                                                     ESP_MODEM_FLOW_CONTROL_NONE));

  if ((rtsPin < 0) || (ctsPin < 0)) {
    if (flow_ctl != ESP_MODEM_FLOW_CONTROL_NONE) {
      addLog(LOG_LEVEL_INFO, F("PPP: Disable flow control as RTS/CTS are not set"));
      flow_ctl = ESP_MODEM_FLOW_CONTROL_NONE;
    }
  }

  if (!NW_PLUGIN_INTERFACE.setPins(
        _kvs->getValueAsInt_or_default(NW005_KEY_PIN_TX, -1),
        _kvs->getValueAsInt_or_default(NW005_KEY_PIN_RX, -1),
        rtsPin,
        ctsPin,
        flow_ctl))
  {
    addLog(LOG_LEVEL_ERROR, F("PPP: Could not set pins"));
    return false;
  }
  {
    String apn;
    _kvs->getValue(NW005_KEY_APN, apn);
    NW_PLUGIN_INTERFACE.setApn(apn.c_str());
  }
  {
    String pin;
    _kvs->getValue(NW005_KEY_SIM_PIN, pin);

    if (pin.length() >= 4) {
      NW_PLUGIN_INTERFACE.setPin(pin.c_str());
    }
  }

  _modem_task_data.model = to_ppp_modem_model_t(static_cast<NW005_modem_model>(
                                                  _kvs->getValueAsInt_or_default(NW005_KEY_MODEM_MODEL, PPP_MODEM_GENERIC)));

  _modem_task_data.uart_num = 1;
  {
    int8_t serial = 1;

    if (_kvs->getValue(NW005_KEY_SERIAL_PORT, serial)) {
      switch (static_cast<ESPEasySerialPort>(serial))
      {
        case ESPEasySerialPort::serial0: _modem_task_data.uart_num = 0;
          break;
# if USABLE_SOC_UART_NUM > 1
        case ESPEasySerialPort::serial1: _modem_task_data.uart_num = 1;
          break;
# endif // if USABLE_SOC_UART_NUM > 1
# if USABLE_SOC_UART_NUM > 2
        case ESPEasySerialPort::serial2: _modem_task_data.uart_num = 2;
          break;
# endif // if USABLE_SOC_UART_NUM > 2
# if USABLE_SOC_UART_NUM > 3
        case ESPEasySerialPort::serial3: _modem_task_data.uart_num = 3;
          break;
# endif // if USABLE_SOC_UART_NUM > 3
# if USABLE_SOC_UART_NUM > 4
        case ESPEasySerialPort::serial4: _modem_task_data.uart_num = 4;
          break;
# endif // if USABLE_SOC_UART_NUM > 4
# if USABLE_SOC_UART_NUM > 5
        case ESPEasySerialPort::serial5: _modem_task_data.uart_num = 5;
          break;
# endif // if USABLE_SOC_UART_NUM > 5
        default: break;
      }
    }
  }
  _modem_task_data.baud_rate         = _kvs->getValueAsInt_or_default(NW005_KEY_BAUDRATE, 115200);
  _modem_task_data.modem_initialized = false;
  _modem_task_data.initializing      = false;
  _modem_task_data.dtrPin            = _kvs->getValueAsInt_or_default(NW005_KEY_PIN_DTR, -1);

  stats_and_cache.mark_begin_establish_connection();

  xTaskCreatePinnedToCore(
    NW005_begin_modem_task,             // Function that should be called
    "PPP.begin()",                      // Name of the task (for debugging)
    4000,                               // Stack size (bytes)
    &_modem_task_data,                  // Parameter to pass
    1,                                  // Task priority
    &_modem_task_data.modem_taskHandle, // Task handle
    xPortGetCoreID()                    // Core you want to run the task on (0 or 1)
    );

  return true;
}

bool NW005_data_struct_PPP_modem::exit(EventStruct *event)
{
  if (_modem_task_data.modem_taskHandle) {
    vTaskDelete(_modem_task_data.modem_taskHandle);
    _modem_task_data.modem_taskHandle = NULL;
  }
  NW_PLUGIN_INTERFACE.mode(ESP_MODEM_MODE_COMMAND);
  NW_PLUGIN_INTERFACE.end();
  _modem_task_data.modem_initialized = false;
  return true;
}

String NW005_data_struct_PPP_modem::write_AT_cmd(const String& cmd, int timeout)
{
  String res;
  auto   cur_mode = NW_PLUGIN_INTERFACE.mode();

  if (NW_PLUGIN_INTERFACE.mode() != ESP_MODEM_MODE_CMUX) {
    NW_PLUGIN_INTERFACE.mode(ESP_MODEM_MODE_CMUX);
  }
  res = NW_PLUGIN_INTERFACE.cmd(cmd.c_str(), timeout);
  NW_PLUGIN_INTERFACE.mode(cur_mode);
  return res;
}

bool NW005_data_struct_PPP_modem::handle_nwplugin_write(EventStruct *event, String& str)
{
  bool success         = false;
  const String command = parseString(str, 1);

  if (equals(command, F("ppp"))) {
    const String subcommand = parseString(str, 2);

    if (equals(subcommand, F("write"))) {
      const String writeCommand = parseStringToEnd(str, 3);
      const String res          = write_AT_cmd(writeCommand);
      addLog(LOG_LEVEL_INFO, strformat(
               F("PPP cmd: %s -> %s\n"),
               writeCommand.c_str(),
               res.c_str()));
      success = true;
      SendStatus(event, res);
    }
  }
  return success;
}

# if FEATURE_NETWORK_STATS

bool NW005_data_struct_PPP_modem::initPluginStats()
{
  networkStatsVarIndex_t networkStatsVarIndex{};
  PluginStats_Config_t   displayConfig;

  displayConfig.setAxisPosition(PluginStats_Config_t::AxisPosition::Left);
  displayConfig.setEnabled(true);

  displayConfig.setAxisIndex(networkStatsVarIndex);
  NWPluginData_base::initPluginStats(
    networkStatsVarIndex,
    F("RSSI"),
    1,
    NAN,
    displayConfig);

  ++networkStatsVarIndex;
  displayConfig.setAxisIndex(networkStatsVarIndex);
  displayConfig.setHidden(true);
  NWPluginData_base::initPluginStats(
    networkStatsVarIndex,
    F("Bit Error Rate (BER)"),
    1,
    NAN,
    displayConfig);
#  if FEATURE_NETWORK_TRAFFIC_COUNT
  initPluginStats_trafficCount(++networkStatsVarIndex, true);  // TX
  initPluginStats_trafficCount(++networkStatsVarIndex, false); // RX
#  endif // if FEATURE_NETWORK_TRAFFIC_COUNT
  return true;
}

bool NW005_data_struct_PPP_modem::record_stats()
{
  if ((_plugin_stats_array != nullptr) && _modem_task_data.modem_initialized) {
    EventStruct tmpEvent;
    size_t valueCount{};
    const int rssi = doGetRSSI();
    tmpEvent.ParfN[valueCount++] = rssi == 0 ? NAN : rssi;
    tmpEvent.ParfN[valueCount++] = getBER_float();
    bool trackPeaks                  = true;
    bool onlyUpdateTimestampWhenSame = true;
    return pushStatsValues(&tmpEvent, valueCount, trackPeaks, onlyUpdateTimestampWhenSame);
  }
  return false;
}

# endif // if FEATURE_NETWORK_STATS

NWPluginData_static_runtime * NW005_data_struct_PPP_modem::getNWPluginData_static_runtime() { return &stats_and_cache; }

void                          NW005_data_struct_PPP_modem::onEvent(arduino_event_id_t event, arduino_event_info_t info) {
  // TODO TD-er: Must store flags from events in static (or global) object to act on it later.
  switch (event)
  {
    case ARDUINO_EVENT_PPP_START:
      stats_and_cache.mark_start();
      break;
    case ARDUINO_EVENT_PPP_STOP:
      stats_and_cache.mark_stop();
      break;
    case ARDUINO_EVENT_PPP_CONNECTED:
      stats_and_cache.mark_connected();
      break;
    case ARDUINO_EVENT_PPP_DISCONNECTED:
      stats_and_cache.mark_disconnected();
      WiFi.AP.enableNAPT(false);
      break;
    case ARDUINO_EVENT_PPP_GOT_IP:
      stats_and_cache.mark_got_IP();

      //      NW_PLUGIN_INTERFACE.setDefault();

      if (WiFi.AP.enableNAPT(true)) {
        addLog(LOG_LEVEL_INFO, F("WiFi.AP.enableNAPT"));
      }
      break;
# if FEATURE_USE_IPV6
    case ARDUINO_EVENT_PPP_GOT_IP6:
      stats_and_cache.mark_got_IPv6(&info.got_ip6);
      break;
# endif // if FEATURE_USE_IPV6
    case ARDUINO_EVENT_PPP_LOST_IP:
      stats_and_cache.mark_lost_IP();
      WiFi.AP.enableNAPT(false);
      break;

    default: break;
  }
}

String NW005_data_struct_PPP_modem::NW005_formatGpioLabel(uint32_t key, PinSelectPurpose& purpose, bool shortNotation) const
{
  String label;

  if ((key == NW005_KEY_PIN_DTR) ||
      ((key >= NW005_KEY_PIN_RX) && (key <= NW005_KEY_PIN_RESET)))
  {
    KVS_StorageType::Enum storageType;
    purpose = PinSelectPurpose::Generic;
    label   = getLabelString(key, true, storageType);

    const bool optional = !shortNotation;

    switch (key)
    {
      case NW005_KEY_PIN_RX:
        purpose = PinSelectPurpose::Serial_input;
        label   = shortNotation
          ? formatGpioName_RX(false)
          : formatGpioName_serialRX(false);
        break;
      case NW005_KEY_PIN_TX:
        purpose = PinSelectPurpose::Serial_output;
        label   = shortNotation
          ? formatGpioName_TX(false)
          : formatGpioName_serialTX(false);
        break;
      case NW005_KEY_PIN_CTS:
        purpose = PinSelectPurpose::Generic_input;
        label   = formatGpioName(
          label, gpio_direction::gpio_input, optional);
        break;
      case NW005_KEY_PIN_RTS:
      case NW005_KEY_PIN_DTR:
      case NW005_KEY_PIN_RESET:
        purpose = PinSelectPurpose::Generic_output;
        label   = formatGpioName(
          label, gpio_direction::gpio_output, optional);
        break;
    }

  }
  return label;
}

} // namespace ppp

} // namespace net
} // namespace ESPEasy

#endif // ifdef USES_NW005
