#pragma once

#include "../_NWPlugin_Helper.h"
#ifdef USES_NW005

# include "../../../src/Helpers/StringGenerator_GPIO.h"

# include <PPP.h>


namespace ESPEasy {
namespace net {
namespace ppp {

struct NW005_modem_task_data {
  ppp_modem_model_t model     = PPP_MODEM_GENERIC;
  uint8_t           uart_num  = 1;
  int               baud_rate = 115200;
  bool              initializing{};
  bool              modem_initialized{};
  int               dtrPin = -1;
  String            logString;
  String            AT_CPSI; // Result from "AT+CPSI?"

  // This is C-code, so not set to nullptr, but to NULL
  TaskHandle_t modem_taskHandle = NULL;

};


struct NW005_data_struct_PPP_modem : public NWPluginData_base {

// See: ESPEasy_key_value_store_import_export::LabelStringFunction
static const __FlashStringHelper* getLabelString(uint32_t key, bool displayString, KVS_StorageType::Enum& storageType);

// When queried with a key of -1, it will return the first key index
// Return next key, or -2 when no next key exists.
// See: ESPEasy_key_value_store_import_export::NextKeyFunction
static int32_t getNextKey(int32_t key);
static int32_t getNextKey_noCredentials(int32_t key);


  NW005_data_struct_PPP_modem(networkIndex_t networkIndex);
  ~NW005_data_struct_PPP_modem();

  String getRSSI() const;
  String getBER() const;
  float  getBER_float() const;
  bool   attached() const;
  String IMEI() const;
  String operatorName() const;

  void   webform_load_UE_system_information(KeyValueWriter* writer);

  void   webform_load(EventStruct *event);
  void   webform_save(EventStruct *event);

  bool   webform_getPort(KeyValueWriter *writer);

  bool   write_ModemState(KeyValueWriter*writer);

  bool   init(EventStruct *event);

  bool   exit(EventStruct *event);

  bool   handle_nwplugin_write(EventStruct *event,
                               String     & str) override;

  String write_AT_cmd(const String& cmd,
                      int           timeout = 1000);

# if FEATURE_NETWORK_STATS
  bool initPluginStats() override;
  bool record_stats() override;
# endif // if FEATURE_NETWORK_STATS


  NWPluginData_static_runtime* getNWPluginData_static_runtime();

private:

  static void onEvent(arduino_event_id_t   event,
                      arduino_event_info_t info);

  String      NW005_formatGpioLabel(uint32_t          key,
                                    PinSelectPurpose& purpose,
                                    bool              shortNotation = false) const;

  NW005_modem_task_data _modem_task_data;

  network_event_handle_t nw_event_id = 0;


};

} // namespace ppp
} // namespace net
} // namespace ESPEasy

#endif // ifdef USES_NW005
