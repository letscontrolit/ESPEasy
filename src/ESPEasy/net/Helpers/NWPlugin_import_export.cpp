#include "../Helpers/NWPlugin_import_export.h"

#if FEATURE_STORE_NETWORK_INTERFACE_SETTINGS

# include "../../../src/Globals/Settings.h"
# include "../../../src/Helpers/_ESPEasy_key_value_store.h"
# include "../../../src/Helpers/ESPEasy_key_value_store_import_export.h"

# include "../../../src/Helpers/StringConverter.h"

# include "../DataTypes/NWPluginID.h"
# include "../Globals/NWPlugins.h"


# ifdef USES_NW001
#  include "../NWPluginStructs/NW001_data_struct_WiFi_STA.h"
# endif
# ifdef USES_NW002
#  include "../NWPluginStructs/NW002_data_struct_WiFi_AP.h"
# endif
# ifdef USES_NW003
#  include "../NWPluginStructs/NW003_data_struct_ETH_RMII.h"
# endif
# ifdef USES_NW004
#  include "../NWPluginStructs/NW004_data_struct_ETH_SPI.h"
# endif
# ifdef USES_NW005
#  include "../NWPluginStructs/NW005_data_struct_PPP_modem.h"
# endif

namespace ESPEasy {
namespace net {

ESPEasy_key_value_store_import_export::LabelStringFunction getLabelFnc(ESPEasy::net::nwpluginID_t nwpluginID)
{

  switch (nwpluginID.value)
  {
# ifdef USES_NW003
    case 3:
      return ESPEasy::net::eth::NW003_data_struct_ETH_RMII::getLabelString;
# endif
# ifdef USES_NW004
    case 4:
    return ESPEasy::net::eth::NW004_data_struct_ETH_SPI::getLabelString;
# endif

# ifdef USES_NW005
    case 5:
    {
      return ESPEasy::net::ppp::NW005_data_struct_PPP_modem::getLabelString;
    }
# endif // ifdef USES_NW005

# ifdef USES_NW001
    case 1:
# endif
# ifdef USES_NW002
    case 2:
# endif
    default:
      break;

  }
  return nullptr;
}

ESPEasy_key_value_store_import_export::NextKeyFunction getNextKeyFnc(ESPEasy::net::nwpluginID_t nwpluginID, bool includeCredentials)
{

  switch (nwpluginID.value)
  {
# ifdef USES_NW003
    case 3:
    return ESPEasy::net::eth::NW003_data_struct_ETH_RMII::getNextKey;
# endif
# ifdef USES_NW004
    case 4:
    return ESPEasy::net::eth::NW004_data_struct_ETH_SPI::getNextKey;
# endif
# ifdef USES_NW005
    case 5:
    {
      if (includeCredentials) {
        return ESPEasy::net::ppp::NW005_data_struct_PPP_modem::getNextKey;
      }
      return ESPEasy::net::ppp::NW005_data_struct_PPP_modem::getNextKey_noCredentials;
    }
# endif // ifdef USES_NW005

# ifdef USES_NW001
    case 1:
# endif
# ifdef USES_NW002
    case 2:
# endif
    default:
      break;

  }
  return nullptr;
}

String NWPlugin_import_export::exportConfig(
  networkIndex_t  networkIndex,
  KeyValueWriter *writer,
  bool            includeCredentials)
{
  if (writer == nullptr) { return F("KVS : No writer"); }

  if (!validNetworkIndex(networkIndex)) { return F("KVS : Invalid Network Index"); }

  const ESPEasy::net::nwpluginID_t nwpluginID(Settings.NWPluginID[networkIndex]);

  if (!nwpluginID.isValid()) { return F("KVS : Invalid NW-Plugin ID"); }

  ESPEasy_key_value_store kvs;

  if (!kvs.load(
        SettingsType::Enum::NetworkInterfaceSettings_Type,
        networkIndex,
        0,
        nwpluginID.value)) { return F("KVS : Failed loading"); }
  auto labelFnc   = getLabelFnc(nwpluginID);
  auto nextKeyFnc = getNextKeyFnc(nwpluginID, includeCredentials);

  if ((labelFnc == nullptr) || (nextKeyFnc == nullptr)) {
    return F("KVS : NWPlugin ID does not support import/export");
  }

  auto child = writer->createChild();

  if (child) {
    child->write({ F("nwpluginID"), nwpluginID.value });
    child->write({ F("enabled"), Settings.getNetworkEnabled(networkIndex) });
    child->write({ F("route_prio"), Settings.getRoutePrio_for_network(networkIndex) });
    child->write({ F("sn_block"), Settings.getNetworkInterfaceSubnetBlockClientIP(networkIndex) });
    child->write({ F("start_delay"), Settings.getNetworkInterfaceStartupDelayAtBoot(networkIndex) });

# if FEATURE_USE_IPV6
    child->write({ F("en_ipv6"), Settings.getNetworkEnabled_IPv6(networkIndex) });
# endif

    ESPEasy_key_value_store_import_export e(&kvs);

    int32_t key = nextKeyFnc(-1);

    while (key >= 0) {
      e.do_export(key, child.get(), labelFnc);
      key = nextKeyFnc(key);
    }
  }
  return EMPTY_STRING;
}

String NWPlugin_import_export::importConfig(
  networkIndex_t networkIndex,
  const String & json)
{
  if (!validNetworkIndex(networkIndex)) { return F("KVS : Invalid Network Index"); }

  ESPEasy::net::nwpluginID_t nwpluginID(Settings.NWPluginID[networkIndex]);

  if (nwpluginID.isValid()) { return F("KVS : Network Index is already in use"); }

  addLog(LOG_LEVEL_INFO, concat(F("KVS : JSON: "), json));

  ESPEasy_key_value_store kvs;
  ESPEasy_key_value_store_import_export e(&kvs, json);
  {
    String value;

    if (!e.getParsedJSON(F("nwpluginID"), value)) { return F("KVS : No NWPlugin ID"); }
    nwpluginID.value = value.toInt();
  }

  if (!nwpluginID.isValid()) { return F("KVS : No valid NWPlugin ID"); }

  auto labelFnc   = getLabelFnc(nwpluginID);
  auto nextKeyFnc = getNextKeyFnc(nwpluginID, true);

  if ((labelFnc == nullptr) || (nextKeyFnc == nullptr)) {
    return F("KVS : NWPlugin ID does not support import/export");
  }

  String res = e.do_import(labelFnc, nextKeyFnc);

  if (res.isEmpty()) {
    // Add entry, calls NWPLUGIN_LOAD_DEFAULTS
    Settings.setNWPluginID_for_network(networkIndex, nwpluginID);

    if (!kvs.store(
          SettingsType::Enum::NetworkInterfaceSettings_Type,
          networkIndex,
          0,
          nwpluginID.value))
    {
      return F("KVS : Error saving, see log for more details");
    }

    const String non_kvs_keys[] = {
      F("enabled"),
      F("route_prio"),
      F("sn_block"),
      F("start_delay")
# if FEATURE_USE_IPV6
      , F("en_ipv6")
# endif
    };

    for (size_t i = 0; i < NR_ELEMENTS(non_kvs_keys); ++i) {
      String value;

      if (e.getParsedJSON(non_kvs_keys[i], value))
      {
        const bool bool_val = !(value.equalsIgnoreCase(F("false")) || value.equals(F("0")));

        switch (i)
        {
          case 0: Settings.setNetworkEnabled(networkIndex, bool_val);
            break;
          case 1: Settings.setRoutePrio_for_network(networkIndex, value.toInt());
            break;
          case 2: Settings.setNetworkInterfaceSubnetBlockClientIP(networkIndex, bool_val);
            break;
          case 3: Settings.setNetworkInterfaceStartupDelayAtBoot(networkIndex, value.toInt());
            break;
# if FEATURE_USE_IPV6
          case 4: Settings.setNetworkEnabled_IPv6(networkIndex, bool_val);
            break;
# endif // if FEATURE_USE_IPV6
        }
      }
    }
  }

  return res;
}

} // namespace net

} // namespace ESPEasy

#endif // if FEATURE_ESPEASY_KEY_VALUE_STORE
