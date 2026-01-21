#include "../Helpers/NW_info_writer.h"

#include "../../../src/DataStructs/ESPEasy_EventStruct.h"
#include "../../../src/DataTypes/ESPEasy_plugin_functions.h"
#include "../../../src/Globals/Settings.h"
#include "../../../src/Helpers/ESPEasy_UnitOfMeasure.h"
#include "../../../src/Helpers/StringConverter.h"

#include "../Globals/NWPlugins.h"

#include "../Helpers/NW_info_writer.h"

namespace ESPEasy {
namespace net {

#ifdef ESP32

bool write_NetworkAdapterFlags(ESPEasy::net::networkIndex_t networkindex, KeyValueWriter*writer)
{
  if (writer == nullptr) { return false; }
  struct EventStruct TempEvent;
  TempEvent.kvWriter = writer;

  TempEvent.NetworkIndex = networkindex;

  String str;

  if (!NWPluginCall(NWPlugin::Function::NWPLUGIN_GET_INTERFACE, &TempEvent, str)) {
    return false;
  }

  {
    String str;
    NWPluginCall(NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HW_ADDRESS, &TempEvent, str);
  }

  {
    KeyValueStruct kv(F("Flags"), KeyValueStruct::Format::PreFormatted);

    const NWPlugin::NetforkFlags flags[] = {
      NWPlugin::NetforkFlags::DHCP_client,
      NWPlugin::NetforkFlags::DHCP_server,
      NWPlugin::NetforkFlags::AutoUp,
      NWPlugin::NetforkFlags::GratuituousArp,
      NWPlugin::NetforkFlags::EventIPmodified,
      NWPlugin::NetforkFlags::isPPP,
      NWPlugin::NetforkFlags::isBridge,
# if CONFIG_LWIP_IPV6
      NWPlugin::NetforkFlags::MLD_v6_report,
      NWPlugin::NetforkFlags::IPv6_autoconf_enabled,
# endif // if CONFIG_LWIP_IPV6
    };

    for (size_t i = 0; i < NR_ELEMENTS(flags); ++i) {
      if (NWPlugin::isFlagSet(flags[i], TempEvent.networkInterface)) {
        String labels_str = NWPlugin::toString(flags[i]);

        if ((flags[i] == NWPlugin::NetforkFlags::DHCP_client) &&
            (TempEvent.networkInterface->getStatusBits() & ESP_NETIF_HAS_STATIC_IP_BIT)) {
          kv.appendValue(concat(NWPlugin::toString(flags[i]), F(" (static IP)")));
        } else {
          kv.appendValue(NWPlugin::toString(flags[i]));
        }
      }
    }
    writer->write(kv);
  }
  return true;
}
#ifndef LIMIT_BUILD_SIZE
bool write_NetworkAdapterPort(ESPEasy::net::networkIndex_t networkindex,
                              KeyValueWriter              *writer)
{
  if (writer == nullptr) { return false; }
  struct EventStruct TempEvent;
  TempEvent.kvWriter = writer;

  TempEvent.NetworkIndex = networkindex;

  String str;
  NWPluginCall(NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_PORT, &TempEvent, str);
  return true;
}
#endif

bool write_IP_config(ESPEasy::net::networkIndex_t networkindex, KeyValueWriter*writer)
{
  if (writer == nullptr) { return false; }
  struct EventStruct TempEvent;

  TempEvent.NetworkIndex = networkindex;
  String str;

  if (!NWPluginCall(NWPlugin::Function::NWPLUGIN_GET_INTERFACE, &TempEvent, str)) {
    return false;
  }
  const NWPlugin::IP_type ip_types[] = {
    NWPlugin::IP_type::inet,
    NWPlugin::IP_type::network_id_cdr,
    NWPlugin::IP_type::netmask,
    NWPlugin::IP_type::broadcast,
    NWPlugin::IP_type::gateway,
    NWPlugin::IP_type::dns1,
    NWPlugin::IP_type::dns2,
# if CONFIG_LWIP_IPV6
    NWPlugin::IP_type::ipv6_unknown,
    NWPlugin::IP_type::ipv6_global,
    NWPlugin::IP_type::ipv6_link_local,
    NWPlugin::IP_type::ipv6_site_local,
    NWPlugin::IP_type::ipv6_unique_local,
    NWPlugin::IP_type::ipv4_mapped_ipv6,
# endif // if CONFIG_LWIP_IPV6

  };

  for (size_t i = 0; i < NR_ELEMENTS(ip_types); ++i) {

    PrintToString str;

    if (NWPlugin::print_IP_address(ip_types[i], TempEvent.networkInterface, str)) {
      KeyValueStruct kv(NWPlugin::toString(ip_types[i]), str.get(), KeyValueStruct::Format::PreFormatted);
      writer->write(kv);
    }
  }
  return true;
}

#endif // ifdef ESP32

bool write_NetworkConnectionInfo(ESPEasy::net::networkIndex_t networkindex, KeyValueWriter*writer)
{
  if (!writer) { return false; }
  struct EventStruct TempEvent;
  TempEvent.NetworkIndex = networkindex;
  TempEvent.kvWriter     = writer;

  const bool res = NWPluginCall(NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_CONNECTED, &TempEvent);

  if (res) {
    NWPluginCall(NWPlugin::Function::NWPLUGIN_GET_CONNECTED_DURATION, &TempEvent);

#if FEATURE_NETWORK_TRAFFIC_COUNT
    NWPluginCall(NWPlugin::Function::NWPLUGIN_GET_TRAFFIC_COUNT,      &TempEvent);
#endif // if FEATURE_NETWORK_TRAFFIC_COUNT
  }
  return res;
}

#if defined(USES_NW003) || defined(USES_NW004)

bool write_Eth_Show_Connected(const ETHClass& eth, KeyValueWriter *writer)
{
  if ((writer == nullptr) || !eth.connected() || !eth.linkUp()) { return false; }

  if (writer->summaryValueOnly()) {
    String s = concat(
      String(eth.linkSpeed()),
      eth.fullDuplex() ? F("Mbps FD") : F("Mbps HD"));

    if (!eth.autoNegotiation()) { s += F("(manual)"); }
    writer->write({ EMPTY_STRING, s });
  } else {
    KeyValueStruct kv(F("Link Speed"), eth.linkSpeed());
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
    kv.setUnit(UOM_Mbps);
# endif
    writer->write(kv);
    writer->write({ F("Duplex Mode"), eth.fullDuplex() ? F("Full Duplex") : F("Half Duplex") });
    writer->write({ F("Negotiation Mode"), eth.autoNegotiation() ? F("Auto") : F("Manual") });
  }

  return true;
}

bool write_Eth_HW_Address(
  const ESPEasy::net::EthPhyType_t phyType,
  const ETHClass* eth, KeyValueWriter *writer)
{
  if (writer == nullptr) { return false; }

  if (writer->summaryValueOnly()) {
    KeyValueStruct kv(EMPTY_STRING);
    kv.appendValue(toString(phyType));
    kv.appendValue(concat(F("MAC: "), eth->macAddress()));

    writer->write(kv);
  } else {
    writer->write({
          F("Adapter"),
          toString(phyType) });
    writer->write({
          F("MAC"),
          eth->macAddress(),
          KeyValueStruct::Format::PreFormatted });
  }

  return true;
}

#endif // if defined(USES_NW003) || defined(USES_NW004)

bool write_NetworkPort(const __FlashStringHelper*labels[], const int pins[], size_t nrElements, KeyValueWriter *writer)
{
  if (writer == nullptr) { return false; }
  bool success = false;

  if (writer->summaryValueOnly()) {
    KeyValueStruct kv(EMPTY_STRING);

    for (size_t i = 0; i < nrElements; ++i) {
      if (pins[i] >= 0) {
        kv.appendValue(concat(labels[i], F(":&nbsp;")) + formatGpioLabel(pins[i], false));
      }
    }

    if (kv._values.size()) {
      success = true;
      writer->write(kv);
    }
  } else {
    for (size_t i = 0; i < nrElements; ++i) {
      if (pins[i] >= 0) {
        success = true;
        writer->write({
              labels[i],
              pins[i],
              KeyValueStruct::Format::PreFormatted });
      }
    }
  }

  return success;
}

} // namespace net
} // namespace ESPEasy
