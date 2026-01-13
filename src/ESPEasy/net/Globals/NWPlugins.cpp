#include "../Globals/NWPlugins.h"

#include "../../../_Plugin_Helper.h"
#include "../../../src/DataStructs/ESPEasy_EventStruct.h"
#include "../../../src/DataStructs/TimingStats.h"
#include "../../../src/DataTypes/ESPEasy_plugin_functions.h"
#include "../../../src/ESPEasyCore/ESPEasy_Log.h"
#include "../../../src/Globals/SecuritySettings.h"
#include "../../../src/Globals/Settings.h"
#include "../../../src/Helpers/PrintToString.h"
#include "../Helpers/_NWPlugin_init.h"
#include "../Helpers/NWAccessControl.h"
#include "../_NWPlugin_Helper.h"

#if FEATURE_ETHERNET
# include "../eth/ETH_NWPluginData_static_runtime.h"
#endif


namespace ESPEasy {
namespace net {

bool NWPluginCall(NWPlugin::Function Function, EventStruct *event) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  String dummy;

  return NWPluginCall(Function, event, dummy);
}

bool NWPluginCall(NWPlugin::Function Function, EventStruct *event, String& str)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  EventStruct TempEvent;

  if (event == 0) {
    event = &TempEvent;
  }

  switch (Function)
  {
    case NWPlugin::Function::NWPLUGIN_DRIVER_ADD:
      // only called from NWPluginSetup() directly using networkDriverIndex
      break;

    case NWPlugin::Function::NWPLUGIN_GET_PARAMETER_DISPLAY_NAME:
      // Only called from _NWPlugin_Helper_webform  getNetworkParameterName
      break;

    // calls to all active networks
    case NWPlugin::Function::NWPLUGIN_INIT_ALL:
    case NWPlugin::Function::NWPLUGIN_EXIT_ALL:
    case NWPlugin::Function::NWPLUGIN_TEN_PER_SECOND:
    case NWPlugin::Function::NWPLUGIN_FIFTY_PER_SECOND:
    case NWPlugin::Function::NWPLUGIN_WRITE:
    case NWPlugin::Function::NWPLUGIN_WEBSERVER_SHOULD_RUN:
#ifdef ESP32
    case NWPlugin::Function::NWPLUGIN_PRIORITY_ROUTE_CHANGED:
#endif
    {
      // Set to true where return value doesn't matter
      bool success =
#ifdef ESP32
        Function != NWPlugin::Function::NWPLUGIN_PRIORITY_ROUTE_CHANGED &&
#endif
        Function != NWPlugin::Function::NWPLUGIN_WRITE &&
        Function != NWPlugin::Function::NWPLUGIN_WEBSERVER_SHOULD_RUN;

      if (Function == NWPlugin::Function::NWPLUGIN_EXIT_ALL) {
        // Called before shutdown, so must immediately exit network adapters
        // Especially PPP modem may no longer respond if not turned off properly before reboot
        Function = NWPlugin::Function::NWPLUGIN_EXIT;
      }

      for (networkIndex_t x = 0; x < NETWORK_MAX; x++) {
        const bool checkedEnabled =
          Settings.getNetworkEnabled(x) ||
          Function == NWPlugin::Function::NWPLUGIN_EXIT;

        if (Settings.getNWPluginID_for_network(x) && checkedEnabled) {
          if (Function == NWPlugin::Function::NWPLUGIN_INIT_ALL) {
            Scheduler.setNetworkInitTimer(Settings.getNetworkInterfaceStartupDelayAtBoot(x), x);
          } else {
            event->NetworkIndex = x;
            String command;

            if (Function == NWPlugin::Function::NWPLUGIN_WRITE) {
              command = str;
            }

            if (do_NWPluginCall(
                  getNetworkDriverIndex_from_NetworkIndex(x),
                  Function,
                  event,
                  command)) {
              if ((Function == NWPlugin::Function::NWPLUGIN_WRITE) ||
                  (Function == NWPlugin::Function::NWPLUGIN_WEBSERVER_SHOULD_RUN)) {
                // Need to stop when call was handled
                return true;
              }
              success = true;
            }
          }
        }
      }
#ifdef ESP32
      if (Function == NWPlugin::Function::NWPLUGIN_PRIORITY_ROUTE_CHANGED) {
        CheckRunningServices();
      }
#endif
      return success;
    }

      // calls to specific network's NWPluginData_base (must be enabled)
#if FEATURE_NETWORK_TRAFFIC_COUNT
    case NWPlugin::Function::NWPLUGIN_GET_TRAFFIC_COUNT:
#endif
#if FEATURE_NETWORK_STATS
    case NWPlugin::Function::NWPLUGIN_RECORD_STATS:
    case NWPlugin::Function::NWPLUGIN_WEBFORM_LOAD_SHOW_STATS:
#endif // if FEATURE_NETWORK_STATS
    case NWPlugin::Function::NWPLUGIN_PROCESS_EVENT:
    case NWPlugin::Function::NWPLUGIN_GET_CONNECTED_DURATION:
    {
      bool success = false;

      if (!validNetworkIndex(event->NetworkIndex) ||
          !Settings.getNetworkEnabled(event->NetworkIndex)) { return false; }

      auto *NW_data = getNWPluginData(event->NetworkIndex);

      if (NW_data) {
        switch (Function)
        {
#if FEATURE_NETWORK_TRAFFIC_COUNT
          case NWPlugin::Function::NWPLUGIN_GET_TRAFFIC_COUNT:
          {
            TX_RX_traffic_count traffic{};

            if (NW_data->getTrafficCount(traffic)) {
              event->Par64_1 = traffic._tx_count;
              event->Par64_2 = traffic._rx_count;
              event->Par5    = traffic._tx_packets;
              event->Par6    = traffic._rx_packets;
              success        = true;

              if (event->kvWriter) {
                if (event->kvWriter->dataOnlyOutput()) {
                  {
                    auto frames = event->kvWriter->createChild(F("Frames"));

                    if (frames) {
                      frames->write({ F("TX"), traffic._tx_packets });
                      frames->write({ F("RX"), traffic._rx_packets });
                    }
                  }
                  {
                    auto bytes = event->kvWriter->createChild(F("Bytes"));

                    if (bytes) {
                      bytes->write({ F("TX"), traffic._tx_count });
                      bytes->write({ F("RX"), traffic._rx_count });
                    }
                  }
                } else {
                  event->kvWriter->write({
                        F("TX / RX Frames"),
                        strformat(
                          F("%d / %d"),
                          event->Par5,
                          event->Par6) });

                  event->kvWriter->write({
                        F("TX / RX Frame Bytes"),
                        strformat(
                          F("%s%cB / %s%cB"),
                          formatHumanReadable(event->Par64_1, 1024).c_str(),
                          event->Par64_1 < 1024 ? ' ' : 'i',
                          formatHumanReadable(event->Par64_2, 1024).c_str(),
                          event->Par64_2 < 1024 ? ' ' : 'i'
                          ) });
                }
              }
            }
            break;
          }
#endif // if FEATURE_NETWORK_TRAFFIC_COUNT
#if FEATURE_NETWORK_STATS
          case NWPlugin::Function::NWPLUGIN_RECORD_STATS:
          {
            //            NWPluginData_static_runtime& runtime_data = NW_data->getNWPluginData_static_runtime();
            success = NW_data->record_stats();
            break;
          }
          case NWPlugin::Function::NWPLUGIN_WEBFORM_LOAD_SHOW_STATS:
          {
            //            NWPluginData_static_runtime& runtime_data = NW_data->getNWPluginData_static_runtime();
            success = NW_data->webformLoad_show_stats(event);
            break;
          }
#endif // if FEATURE_NETWORK_STATS

          case NWPlugin::Function::NWPLUGIN_PROCESS_EVENT:
          {
            auto runtimeData = NW_data->getNWPluginData_static_runtime();

            if (runtimeData) {
              runtimeData->processEvents();
            }
            break;
          }

          case NWPlugin::Function::NWPLUGIN_GET_CONNECTED_DURATION:
          {
            auto runtime_data = NW_data->getNWPluginData_static_runtime();

            if (runtime_data && runtime_data->_connectedStats.isSet()) {
              const bool connected = runtime_data->_connectedStats.isOn();
              auto duration = connected 
              ? runtime_data->_connectedStats.getLastOnDuration_ms()
              : runtime_data->_connectedStats.getLastOffDuration_ms();

              if (duration > 0) {
                event->Par64_1 = duration;
                event->Par64_2 = runtime_data->_connectedStats.getCycleCount();

                if (event->kvWriter) {
                  event->kvWriter->write({ 
                    connected ? F("Connection Duration") : F("Disconnected Duration"), 
                    format_msec_duration_HMS(duration) });

                  if (!event->kvWriter->summaryValueOnly()) {
                    event->kvWriter->write({ F("Number of Reconnects"), event->Par64_2 });
                  }
                }
                success = true;
              }
            }
            break;
          }
          default: break;
        }
      }

      return success;
    }


    // calls to specific network which need to be enabled before calling
    case NWPlugin::Function::NWPLUGIN_INIT:
    case NWPlugin::Function::NWPLUGIN_CONNECT_SUCCESS:
    case NWPlugin::Function::NWPLUGIN_CONNECT_FAIL:
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_ACTIVE:
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_CONNECTED:
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_EXTENDED:
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HW_ADDRESS:
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_IP:
#ifndef LIMIT_BUILD_SIZE
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_PORT:
#endif
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HOSTNAME:
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_NAME:
    case NWPlugin::Function::NWPLUGIN_CLIENT_IP_WEB_ACCESS_ALLOWED:

#ifdef ESP32
    case NWPlugin::Function::NWPLUGIN_GET_INTERFACE:
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_ROUTE_PRIO:
#endif // ifdef ESP32

      if (!validNetworkIndex(event->NetworkIndex) ||
          !Settings.getNetworkEnabled(event->NetworkIndex)) { return false; }

    // fall through

    // calls to specific network
    case NWPlugin::Function::NWPLUGIN_LOAD_DEFAULTS:
    case NWPlugin::Function::NWPLUGIN_EXIT:
    case NWPlugin::Function::NWPLUGIN_GET_DEVICENAME:
    case NWPlugin::Function::NWPLUGIN_WEBFORM_LOAD:
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SAVE:

    case NWPlugin::Function::NWPLUGIN_DRIVER_TEMPLATE:
    {
      const networkIndex_t networkIndex = event->NetworkIndex;
      bool success                      = false;

      if (validNetworkIndex(networkIndex)) {
        if (supportedNWPluginID(Settings.getNWPluginID_for_network(networkIndex)))
        {
          const networkDriverIndex_t networkDriverIndex =
            getNetworkDriverIndex_from_NetworkIndex(networkIndex);

          success = do_NWPluginCall(
            networkDriverIndex,
            Function,
            event,
            str);
#ifdef ESP32

          if (!success && NWPlugin::canQueryViaNetworkInterface(Function)) {
            String dummy_str;

            if (do_NWPluginCall(
                  networkDriverIndex,
                  NWPlugin::Function::NWPLUGIN_GET_INTERFACE,
                  event,
                  dummy_str)) {
              if (event->networkInterface != nullptr) {
                switch (Function)
                {
                  case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_ROUTE_PRIO:
# if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 2)

                    // TODO TD-er: Must also add option to set route prio
                    // See: https://github.com/espressif/arduino-esp32/pull/11617
                    event->Par1 = event->networkInterface->getRoutePrio();
# else // if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
                    event->Par1 = event->networkInterface->route_prio();
# endif // if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
                    event->Par2 = event->networkInterface->isDefault();
                    success     = true;
                    break;

                  case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_ACTIVE:
                  {
                    success = event->networkInterface->started();
                    break;
                  }                    

                  case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_NAME:

                    if (event->kvWriter) {
                      event->kvWriter->write({ F("Name"), event->networkInterface->desc() });
                      success = true;
                    }
                    break;

                  case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HOSTNAME:

                    if (event->kvWriter) {
                      event->kvWriter->write({ F("Hostname"), event->networkInterface->getHostname() });
                      success = true;
                    }
                    break;

                  case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HW_ADDRESS:

                    if (event->kvWriter) {
                      if (event->kvWriter->summaryValueOnly()) {
                        event->kvWriter->write({
                            EMPTY_STRING,
                            concat(F("MAC: "), event->networkInterface->macAddress()) });
                      }
                      else {
                        event->kvWriter->write({
                            F("MAC"),
                            event->networkInterface->macAddress(),
                            KeyValueStruct::Format::PreFormatted
                          });
                      }
                      success = true;
                    }
                    break;

                  case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_IP:
                  {
                    if (event->kvWriter) {
                      PrintToString prstr;
                      success = NWPlugin::print_IP_address(
                        static_cast<NWPlugin::IP_type>(event->Par1),
                        event->networkInterface,
                        prstr);
                      event->kvWriter->write({ F("ip"), prstr.getMove() });
                      success = true;
                    }
                    break;
                  }

                  case NWPlugin::Function::NWPLUGIN_CLIENT_IP_WEB_ACCESS_ALLOWED:
                  {
                    IPAddress client_ip;
                    client_ip.fromString(str);

                    if ((SecuritySettings.IPblockLevel == LOCAL_SUBNET_ALLOWED) &&
                        !Settings.getNetworkInterfaceSubnetBlockClientIP(event->NetworkIndex)) {
                      success = NWPlugin::IP_in_subnet(client_ip, event->networkInterface);
                    } else if (SecuritySettings.IPblockLevel == ONLY_IP_RANGE_ALLOWED) {
                      const IPAddress low(SecuritySettings.AllowedIPrangeLow);
                      const IPAddress high(SecuritySettings.AllowedIPrangeHigh);
                      success = NWPlugin::ipInRange(client_ip, low, high) &&
                                NWPlugin::IP_in_subnet(low,  event->networkInterface) &&
                                NWPlugin::IP_in_subnet(high, event->networkInterface);
                    } else {
                      success = true;
                    }
                    break;
                  }

                  default: break;
                }
              }
            }
          }
#endif // ifdef ESP32
        }
#ifdef ESP32

        if (Function == NWPlugin::Function::NWPLUGIN_EXIT) {
          //          Cache.clearNetworkSettings(networkIndex);
          auto data = getNWPluginData_static_runtime(event->NetworkIndex);

          if (data) {
            delay(100); // Allow some time to process events
            data->processEvent_and_clear();
          }
          clearNWPluginData(event->NetworkIndex);
        }
#endif // ifdef ESP32
      }
      return success;
    }

  }
  return false;
}

bool validNetworkDriverIndex(networkDriverIndex_t index) { return do_check_validNetworkDriverIndex(index); }


// bool getIP(networkDriverIndex_t index, NWPlugin::IP_type ip_type){}

/*
   bool validNetworkIndex(networkIndex_t index)
   {
   return index < NETWORK_MAX;
   }
 */
bool validNWPluginID(nwpluginID_t nwpluginID) {
  return do_getNetworkDriverIndex_from_NWPluginID(nwpluginID) !=
         INVALID_NETWORKDRIVER_INDEX;
}

bool supportedNWPluginID(nwpluginID_t nwpluginID) {
  return validNetworkDriverIndex(
    do_getNetworkDriverIndex_from_NWPluginID(nwpluginID));
}

networkDriverIndex_t getNetworkDriverIndex_from_NetworkIndex(networkIndex_t index) {
  if (validNetworkIndex(index)) {
    return do_getNetworkDriverIndex_from_NWPluginID(Settings.getNWPluginID_for_network(index));
  }
  return INVALID_NETWORKDRIVER_INDEX;
}

networkDriverIndex_t getNetworkDriverIndex_from_NWPluginID(nwpluginID_t nwpluginID) {
  return do_getNetworkDriverIndex_from_NWPluginID(nwpluginID);
}

nwpluginID_t getNWPluginID_from_NetworkDriverIndex(networkDriverIndex_t index)      { return do_getNWPluginID_from_NetworkDriverIndex(index);
}

nwpluginID_t getNWPluginID_from_NetworkIndex(networkIndex_t index) {
  const networkDriverIndex_t networkDriverIndex = getNetworkDriverIndex_from_NetworkIndex(index);

  return do_getNWPluginID_from_NetworkDriverIndex(networkDriverIndex);
}

String getNWPluginNameFromNetworkDriverIndex(networkDriverIndex_t NetworkDriverIndex) {
  String networkName;

  if (validNetworkDriverIndex(NetworkDriverIndex)) {
    do_NWPluginCall(NetworkDriverIndex, NWPlugin::Function::NWPLUGIN_GET_DEVICENAME, nullptr, networkName);
  }
  return networkName;
}

String getNWPluginNameFromNWPluginID(nwpluginID_t nwpluginID) {
  networkDriverIndex_t networkDriverIndex = getNetworkDriverIndex_from_NWPluginID(nwpluginID);

  if (!validNetworkDriverIndex(networkDriverIndex)) {
    return strformat(F("NWPlugin %d not included in build"), nwpluginID.value);
  }
  return getNWPluginNameFromNetworkDriverIndex(networkDriverIndex);
}

NWPluginData_static_runtime* getWiFi_STA_NWPluginData_static_runtime() { return getNWPluginData_static_runtime(NETWORK_INDEX_WIFI_STA); }

NWPluginData_static_runtime* getWiFi_AP_NWPluginData_static_runtime()  { return getNWPluginData_static_runtime(NETWORK_INDEX_WIFI_AP); }

#if FEATURE_ETHERNET

ETHClass* getFirst_Enabled_ETH_interface()
{
  for (networkIndex_t i = 0; validNetworkIndex(i); ++i) {
    if (Settings.getNetworkEnabled(i)) {
      const auto nwpluginID = Settings.getNWPluginID_for_network(i);

      if ((nwpluginID.value == 3) || (nwpluginID.value == 4)) {
        return ESPEasy::net::eth::ETH_NWPluginData_static_runtime::getInterface(i);
      }
    }
  }
  return nullptr;
}

#endif // if FEATURE_ETHERNET

NWPluginData_static_runtime* getNWPluginData_static_runtime(networkIndex_t index)
{
  auto NW_data = getNWPluginData(index);

  if (!NW_data) { return nullptr; }
  return NW_data->getNWPluginData_static_runtime();
}

const NWPluginData_static_runtime* getDefaultRoute_NWPluginData_static_runtime()
{
  #ifdef ESP32
  networkIndex_t index_highest_prio = NETWORK_MAX;
  int highest_prio                  = -1;
  #endif // ifdef ESP32

  for (networkIndex_t i = 0; validNetworkIndex(i); ++i) {
    auto NW_data = getNWPluginData_static_runtime(i);

    if (NW_data && NW_data->isDefaultRoute()) { return NW_data; }
    #ifdef ESP32

    if (NW_data) {
      const int route_prio = NW_data->_routePrio;
      if (route_prio > highest_prio) {
        index_highest_prio = i;
        highest_prio       = route_prio;
      }
    }
    #endif // ifdef ESP32
  }
  #ifdef ESP32

  if (validNetworkIndex(index_highest_prio)) {
    return getNWPluginData_static_runtime(index_highest_prio);
  }

  #endif // ifdef ESP32

  return nullptr;
}

void processNetworkEvents()
{
  START_TIMER;

  for (networkIndex_t i = 0; i < NETWORK_MAX; ++i) {
    EventStruct tmpEvent;
    tmpEvent.NetworkIndex = i;
    NWPluginCall(NWPlugin::Function::NWPLUGIN_PROCESS_EVENT, &tmpEvent);
  }
  STOP_TIMER(NWPLUGIN_PROCESS_NETWORK_EVENTS);
}

} // namespace net
} // namespace ESPEasy
