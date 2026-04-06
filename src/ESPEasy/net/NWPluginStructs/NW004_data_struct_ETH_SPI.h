#pragma once

#include "../_NWPlugin_Helper.h"
#ifdef USES_NW004

#include "../eth/ETH_NWPluginData_static_runtime.h"

namespace ESPEasy {
namespace net {
namespace eth {

struct NW004_data_struct_ETH_SPI : public NWPluginData_base {

  // See: ESPEasy_key_value_store_import_export::LabelStringFunction
  static const __FlashStringHelper* getLabelString(uint32_t               key,
                                                   bool                   displayString,
                                                   KVS_StorageType::Enum& storageType);

  // When queried with a key of -1, it will return the first key index
  // Return next key, or -2 when no next key exists.
  // See: ESPEasy_key_value_store_import_export::NextKeyFunction
  static int32_t getNextKey(int32_t key);

  static void    loadDefaults(ESPEasy_key_value_store     *kvs,
                              ESPEasy::net::networkIndex_t networkIndex,
                              ESPEasy::net::nwpluginID_t   nwPluginID);


  NW004_data_struct_ETH_SPI(networkIndex_t networkIndex, NetworkInterface *netif);
  ~NW004_data_struct_ETH_SPI();

  void                         webform_load(EventStruct *event);
  void                         webform_save(EventStruct *event);

  bool                         webform_getPort(KeyValueWriter *writer);

  bool                         init(EventStruct *event);

  bool                         exit(EventStruct *event);

  NWPluginData_static_runtime* getNWPluginData_static_runtime();

  bool write_Eth_HW_Address(KeyValueWriter *writer);
  bool write_Eth_port(KeyValueWriter *writer);

private:

  static String formatGpioLabel(uint32_t          key,
                                PinSelectPurpose& purpose,
                                bool              shortNotation = false);

void ethResetGPIOpins();

bool ethCheckSettings();

void ethPrintSettings();

bool ETHConnectRelaxed();

};


} // namespace eth
} // namespace net
} // namespace ESPEasy


#endif // ifdef USES_NW004
