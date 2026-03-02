#include "../../ESPEasy_common.h"

#ifdef USES_NW001

// #######################################################################################################
// ########################### Network Plugin 001: WiFi Station ##########################################
// #######################################################################################################

# define NWPLUGIN_001
# define NWPLUGIN_ID_001         1
# define NWPLUGIN_NAME_001       "WiFi Station"

# include "../../src/DataStructs/ESPEasy_EventStruct.h"
# include "../../src/Globals/RTC.h"
# include "../../src/Globals/SecuritySettings.h"
# include "../../src/Globals/Settings.h"
# include "../../src/Helpers/ESPEasy_Storage.h"
# include "../../src/Helpers/PrintToString.h"
# include "../../src/Helpers/StringConverter.h"
# include "../../src/Helpers/StringGenerator_WiFi.h"
# include "../../src/WebServer/ESPEasy_WebServer.h"
# include "../../src/WebServer/HTML_Print.h"
# include "../../src/WebServer/HTML_wrappers.h"
# include "../../src/WebServer/Markup.h"
# include "../../src/WebServer/Markup_Forms.h"
# include "../../src/WebServer/common.h"
# include "../net/ESPEasyNetwork.h"
# include "../net/Globals/ESPEasyWiFiEvent.h"
# include "../net/Globals/NWPlugins.h"
# include "../net/Globals/WiFi_AP_Candidates.h"
# include "../net/Helpers/_NWPlugin_Helper_webform.h"
# include "../net/Helpers/_NWPlugin_init.h"
# include "../net/NWPluginStructs/NW001_data_struct_WiFi_STA.h"
# include "../net/wifi/ESPEasyWifi.h"

# if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE
#  include "../../src/WebServer/SecurityStruct_deviceSpecific_webform.h"
# endif


# ifdef ESP8266
#  include "../net/Helpers/NWAccessControl.h"
# endif

# ifdef ESP32P4
#  include <esp_hosted.h>
# endif


namespace ESPEasy {
namespace net {

bool NWPlugin_001(NWPlugin::Function function, EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case NWPlugin::Function::NWPLUGIN_DRIVER_ADD:
    {
      NetworkDriverStruct& nw = getNetworkDriverStruct(networkDriverIndex_t::toNetworkDriverIndex(event->idx));
      nw.onlySingleInstance = true;
      nw.alwaysPresent      = true;
# ifdef ESP32P4
      nw.enabledOnFactoryReset = false;

      //        ESPEasy::net::wifi::GetHostedMCUFwVersion() > 0x00020600;
# else // ifdef ESP32P4
      nw.enabledOnFactoryReset = true;
# endif // ifdef ESP32P4
      nw.fixedNetworkIndex = NWPLUGIN_ID_001 - 1; // Start counting at 0
      break;
    }

    case NWPlugin::Function::NWPLUGIN_LOAD_DEFAULTS:
    {
# ifdef ESP32
      Settings.setRoutePrio_for_network(event->NetworkIndex, 100);
# endif // ifdef ESP32
      Settings.setNetworkInterfaceSubnetBlockClientIP(event->NetworkIndex, false);
      Settings.setNetworkInterfaceStartupDelayAtBoot(event->NetworkIndex, 1000);

      Settings.ConnectFailRetryCount = 1;
      break;
    }

    case NWPlugin::Function::NWPLUGIN_GET_DEVICENAME:
    {
      string = F(NWPLUGIN_NAME_001);
      break;
    }
# ifdef ESP32
    case NWPlugin::Function::NWPLUGIN_GET_INTERFACE:
    {
      event->networkInterface = &WiFi.STA;
      success                 = event->networkInterface != nullptr;
      break;
    }
# endif // ifdef ESP32

    case NWPlugin::Function::NWPLUGIN_WEBSERVER_SHOULD_RUN:
    {
# ifdef ESP32
      success = WiFi.STA.connected();
# else // ifdef ESP32
      success = WiFi.isConnected();
# endif // ifdef ESP32
      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_ACTIVE:
    {
      success = ESPEasy::net::wifi::WifiIsSTA(WiFi.getMode());
      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_CONNECTED:
    {
# ifdef ESP32
      success = WiFi.STA.connected();
# else
      success = WiFi.isConnected();
# endif // ifdef ESP32

      if (event->kvWriter == nullptr) {
        break;
      }

      if (success) {
# ifdef ESP32
#  define STA_SSID_STR  WiFi.STA.SSID()
#  define STA_BSSID_STR WiFi.STA.BSSIDstr()
# else // ifdef ESP32
#  define STA_SSID_STR  WiFi.SSID()
#  define STA_BSSID_STR WiFi.BSSIDstr()
# endif // ifdef ESP32


        if (event->kvWriter->summaryValueOnly()) {
          event->kvWriter->write({
                EMPTY_STRING,
                strformat(F("%s (ch: %d)"),
                          STA_SSID_STR.c_str(),
                          WiFi.channel()) });
        } else {
          event->kvWriter->write({
                F("SSID"),
                STA_SSID_STR,
                KeyValueStruct::Format::PreFormatted });
          event->kvWriter->write({
                F("Channel"),
                WiFi.channel() });
# if CONFIG_SOC_WIFI_SUPPORT_5G
          {
            KeyValueStruct kv(
              F("Band"),
              (WiFi.channel() < 36) ? F("2.4") : F("5"));
#  if FEATURE_TASKVALUE_UNIT_OF_MEASURE
            kv.setUnit(UOM_GHz);
#  endif
            event->kvWriter->write(kv);
          }
# endif // if CONFIG_SOC_WIFI_SUPPORT_5G

        }
        {
          KeyValueStruct kv(
            F("RSSI"),
            strformat(
              event->kvWriter->summaryValueOnly() ? F("RSSI: %d dBm") : F("%d"),
              WiFi.RSSI()));
# if FEATURE_TASKVALUE_UNIT_OF_MEASURE
          kv.setUnit(UOM_dBm);
# endif
          event->kvWriter->write(kv);
        }

        if (!event->kvWriter->summaryValueOnly()) {
# if FEATURE_SET_WIFI_TX_PWR
          {
            KeyValueStruct kv(
              F("WiFi TX Power"),
              ESPEasy::net::wifi::GetWiFiTXpower(), 2);
#  if FEATURE_TASKVALUE_UNIT_OF_MEASURE
            kv.setUnit(UOM_dBm);
#  endif

            event->kvWriter->write(kv);
          }
# endif // if FEATURE_SET_WIFI_TX_PWR
          event->kvWriter->write({
                F("Last Disconnect Reason"),
                getWiFi_disconnectReason_str()
              });

# ifdef ESP32
          {
            const int64_t tsf_time = ESPEasy::net::wifi::WiFi_get_TSF_time();

            if (tsf_time > 0) {

              // Split it while printing, so we're not loosing a lot of decimals in the float conversion
              uint32_t tsf_usec{};

              KeyValueStruct kv(
                F("WiFi TSF time"),
                concat(
                  secondsToDayHourMinuteSecond(micros_to_sec_usec(tsf_time, tsf_usec)),
                  strformat(F(".%06u"), tsf_usec))
                );
#  if FEATURE_TASKVALUE_UNIT_OF_MEASURE
              kv.setUnit(UOM_usec);
#  endif
              event->kvWriter->write(kv);
            }
          }
# endif // ifdef ESP32
        }
# ifndef LIMIT_BUILD_SIZE
        event->kvWriter->write({
              F("BSSID"),
              STA_BSSID_STR,
              KeyValueStruct::Format::PreFormatted });

        event->kvWriter->write({
              F("Protocol"),
              ESPEasy::net::wifi::toString(ESPEasy::net::wifi::getConnectionProtocol())
            });

        ESPEasy::net::wifi::NW001_data_struct_WiFi_STA *NW_data =
          static_cast<ESPEasy::net::wifi::NW001_data_struct_WiFi_STA *>(getNWPluginData(event->NetworkIndex));

        if (NW_data) {
          event->kvWriter->write({
                F("Encryption Type"),
                NW_data->getWiFi_encryptionType() });
        }
# endif // ifndef LIMIT_BUILD_SIZE
      }

      break;
    }

# ifdef ESP8266
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_NAME:
    {
      if (event->kvWriter) {
        event->kvWriter->write({ F("Name"), F("sta") });
        success = true;
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HOSTNAME:
    {
      if (event->kvWriter) {
        event->kvWriter->write({ F("Hostname"), WiFi.hostname() });
        success = true;
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HW_ADDRESS:
    {
      if (event->kvWriter) {
        event->kvWriter->write({ F("MAC"), WiFi.macAddress(), KeyValueStruct::Format::PreFormatted });
        success = true;
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_IP:
    {
      if (event->kvWriter) {
        event->kvWriter->write({ F("IP"), WiFi.localIP().toString(), KeyValueStruct::Format::PreFormatted });
        success = true;
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_CLIENT_IP_WEB_ACCESS_ALLOWED:
    {
      IPAddress client_ip;
      client_ip.fromString(string);

      if ((SecuritySettings.IPblockLevel == LOCAL_SUBNET_ALLOWED) &&
          !Settings.getNetworkInterfaceSubnetBlockClientIP(event->NetworkIndex)) {
        success = NWPlugin::ipInRange(client_ip, NetworkID(), NetworkBroadcast());
      } else if (SecuritySettings.IPblockLevel == ONLY_IP_RANGE_ALLOWED) {
        const IPAddress low(SecuritySettings.AllowedIPrangeLow);
        const IPAddress high(SecuritySettings.AllowedIPrangeHigh);
        success = NWPlugin::ipInRange(client_ip, low, high);
      } else {
        success = true;
      }
      break;
    }
# endif // ifdef ESP8266

# ifdef BOARD_HAS_SDIO_ESP_HOSTED

    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_HW_ADDRESS:
    {
      if (event->kvWriter) {
        success = ESPEasy::net::wifi::write_WiFi_Hosted_MCU_info(event->kvWriter);
      }
      break;
    }
#  ifndef LIMIT_BUILD_SIZE
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SHOW_PORT:
    {
      if (event->kvWriter) {
        success = ESPEasy::net::wifi::write_WiFi_Hosted_MCU_pins(event->kvWriter);
      }
      break;
    }
#  endif // ifndef LIMIT_BUILD_SIZE
# endif // ifdef BOARD_HAS_SDIO_ESP_HOSTED
    case NWPlugin::Function::NWPLUGIN_WEBFORM_SAVE:
    {
      // SSID 1
      safe_strncpy(SecuritySettings.WifiSSID, webArg(F("ssid")).c_str(), sizeof(SecuritySettings.WifiSSID));
      copyFormPassword(F("key"), SecuritySettings.WifiKey, sizeof(SecuritySettings.WifiKey));

      // SSID 2
      strncpy_webserver_arg(SecuritySettings.WifiSSID2, F("ssid2"));
      copyFormPassword(F("key2"), SecuritySettings.WifiKey2, sizeof(SecuritySettings.WifiKey2));

# if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE

      for (uint16_t i = 0; i < MAX_EXTRA_WIFI_CREDENTIALS_SEPARATE_FILE; ++i) {
        store_SecurityStruct_deviceSpecific_WebFormItem(
          SecurityStruct_deviceSpecific::KeyType::WiFi_SSID, i);
        store_SecurityStruct_deviceSpecific_WebFormItem(
          SecurityStruct_deviceSpecific::KeyType::WiFi_Password, i);
      }
# endif // if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE

      // Hidden SSID
      Settings.IncludeHiddenSSID(isFormItemChecked(LabelType::CONNECT_HIDDEN_SSID));
      Settings.HiddenSSID_SlowConnectPerBSSID(isFormItemChecked(LabelType::HIDDEN_SSID_SLOW_CONNECT));

# ifdef ESP32
      Settings.PassiveWiFiScan(isFormItemChecked(LabelType::WIFI_PASSIVE_SCAN));
# endif

      webArg2ip(F("espip"),      Settings.IP);
      webArg2ip(F("espgateway"), Settings.Gateway);
      webArg2ip(F("espsubnet"),  Settings.Subnet);
      webArg2ip(F("espdns"),     Settings.DNS);


      Settings.ForceWiFi_bg_mode(isFormItemChecked(LabelType::FORCE_WIFI_BG));
      Settings.WiFiRestart_connection_lost(isFormItemChecked(LabelType::RESTART_WIFI_LOST_CONN));

      Settings.WifiNoneSleep(isFormItemChecked(LabelType::FORCE_WIFI_NOSLEEP));
# ifdef SUPPORT_ARP
      Settings.gratuitousARP(isFormItemChecked(LabelType::PERIODICAL_GRAT_ARP));
# endif // ifdef SUPPORT_ARP
# if FEATURE_SET_WIFI_TX_PWR
      Settings.setWiFi_TX_power(getFormItemFloat(LabelType::WIFI_TX_MAX_PWR));
      Settings.WiFi_sensitivity_margin = getFormItemInt(LabelType::WIFI_SENS_MARGIN);
      Settings.UseMaxTXpowerForSending(isFormItemChecked(LabelType::WIFI_SEND_AT_MAX_TX_PWR));
# endif // if FEATURE_SET_WIFI_TX_PWR
      Settings.ConnectFailRetryCount = getFormItemInt(LabelType::WIFI_NR_RECONNECT_ATTEMPTS);
      Settings.UseLastWiFiFromRTC(isFormItemChecked(LabelType::WIFI_USE_LAST_CONN_FROM_RTC));

# ifndef ESP32
      Settings.WaitWiFiConnect(isFormItemChecked(LabelType::WAIT_WIFI_CONNECT));
# endif
      Settings.SDK_WiFi_autoreconnect(isFormItemChecked(LabelType::SDK_WIFI_AUTORECONNECT));
# if CONFIG_SOC_WIFI_SUPPORT_5G
      Settings.WiFi_band_mode(static_cast<wifi_band_mode_t>(getFormItemInt(getInternalLabel(LabelType::WIFI_BAND_MODE))));
# endif

      /*
       # if FEATURE_USE_IPV6
            Settings.EnableIPv6(isFormItemChecked(LabelType::ENABLE_IPV6));
       # endif
       */

      break;
    }

    case NWPlugin::Function::NWPLUGIN_WEBFORM_LOAD:
    {
      KeyValueWriter_WebForm writer(true);
      # ifdef BOARD_HAS_SDIO_ESP_HOSTED
      ESPEasy::net::wifi::write_WiFi_Hosted_MCU_info(writer.createChild(F("ESP-Hosted-MCU")).get());
      # endif // ifdef BOARD_HAS_SDIO_ESP_HOSTED

      addFormSubHeader(F("Wifi Credentials"));

      // TODO Add pin configuration for ESP32P4.
      // ESP32-C5 may use different SDIO pins.
      // See: https://github.com/espressif/esp-hosted-mcu/blob/main/docs/sdio.md#esp32-p4-function-ev-board-host-pin-mapping

      addFormTextBox(getLabel(LabelType::SSID), F("ssid"), SecuritySettings.getSSID(SecurityStruct::WiFiCredentialsSlot::first), 31);
      addFormPasswordBox(F("WPA Key"), F("key"), SecuritySettings.WifiKey, 63);
      addFormTextBox(F("Fallback SSID"), F("ssid2"), SecuritySettings.getSSID(SecurityStruct::WiFiCredentialsSlot::second), 31);
      addFormPasswordBox(F("Fallback WPA Key"), F("key2"), SecuritySettings.WifiKey2, 63);
      addFormNote(F("WPA Key must be at least 8 characters long"));

# if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE
      addFormSubHeader(F("Wifi Credentials Extra"));

      for (uint16_t i = 0; i < MAX_EXTRA_WIFI_CREDENTIALS_SEPARATE_FILE; ++i) {
        show_SecurityStruct_deviceSpecific_WebFormItem(
          SecurityStruct_deviceSpecific::KeyType::WiFi_SSID, i);
        show_SecurityStruct_deviceSpecific_WebFormItem(
          SecurityStruct_deviceSpecific::KeyType::WiFi_Password, i);
      }
      addFormNote(F("These credentials will be stored in a separate file: <tt>devsecurity.dat</tt>"));
# endif // if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE
      addFormSubHeader(F("Wifi Settings"));
      {
        LabelType::Enum labels[]{
          LabelType::CONNECT_HIDDEN_SSID
          , LabelType::HIDDEN_SSID_SLOW_CONNECT
# ifdef ESP32
          , LabelType::WIFI_PASSIVE_SCAN
# endif
        };
        addFormCheckBoxes(labels, NR_ELEMENTS(labels));
      }

      addFormSubHeader(F("WiFi IP Settings"));

      addFormIPBox(F("ESP WiFi IP"),         F("espip"),      Settings.IP);
      addFormIPBox(F("ESP WiFi Gateway"),    F("espgateway"), Settings.Gateway);
      addFormIPBox(F("ESP WiFi Subnetmask"), F("espsubnet"),  Settings.Subnet);
      addFormIPBox(F("ESP WiFi DNS"),        F("espdns"),     Settings.DNS);
      addFormNote(F("Leave empty for DHCP"));

      /*
       # if FEATURE_USE_IPV6
            addFormCheckBox(LabelType::ENABLE_IPV6);
       # endif
       */

      addFormSubHeader(F("WiFi Mode"));
      addFormCheckBox(LabelType::FORCE_WIFI_BG);
# if CONFIG_SOC_WIFI_SUPPORT_5G
      {
        const __FlashStringHelper *wifiModeNames[] = {
          ESPEasy::net::wifi::getWifiBandModeString(WIFI_BAND_MODE_2G_ONLY),
          ESPEasy::net::wifi::getWifiBandModeString(WIFI_BAND_MODE_5G_ONLY),
          ESPEasy::net::wifi::getWifiBandModeString(WIFI_BAND_MODE_AUTO),
        };
        const int wifiModeOptions[]     = { WIFI_BAND_MODE_2G_ONLY, WIFI_BAND_MODE_5G_ONLY, WIFI_BAND_MODE_AUTO };
        constexpr int nrWifiModeOptions = NR_ELEMENTS(wifiModeNames);
        const FormSelectorOptions selector(
          nrWifiModeOptions,
          wifiModeNames,
          wifiModeOptions);
        selector.addFormSelector(
          LabelType::WIFI_BAND_MODE,
          Settings.WiFi_band_mode());
      }
# endif // if CONFIG_SOC_WIFI_SUPPORT_5G


      addFormSubHeader(F("WiFi Power"));
      addFormCheckBox(LabelType::FORCE_WIFI_NOSLEEP);
# if FEATURE_SET_WIFI_TX_PWR
      addFormFloatNumberBox(LabelType::WIFI_TX_MAX_PWR, 0.0f, MAX_TX_PWR_DBM_11b, 2, 0.25f);
      addFormNumericBox(LabelType::WIFI_SENS_MARGIN, -20, 30);
      addFormCheckBox(LabelType::WIFI_SEND_AT_MAX_TX_PWR);
# endif // if FEATURE_SET_WIFI_TX_PWR

      addFormSubHeader(F("WiFi Tweaks"));

      addFormNumericBox(LabelType::WIFI_NR_RECONNECT_ATTEMPTS, 0, 255);
      {
        LabelType::Enum labels[]{
          LabelType::RESTART_WIFI_LOST_CONN
          , LabelType::WIFI_USE_LAST_CONN_FROM_RTC
# ifndef ESP32
          , LabelType::WAIT_WIFI_CONNECT
# endif
          , LabelType::SDK_WIFI_AUTORECONNECT
# ifdef SUPPORT_ARP
          , LabelType::PERIODICAL_GRAT_ARP
# endif // ifdef SUPPORT_ARP
        };

        addFormCheckBoxes(labels, NR_ELEMENTS(labels));
      }

      break;
    }

    case NWPlugin::Function::NWPLUGIN_INIT:
    {
      initNWPluginData(event->NetworkIndex, new (std::nothrow) ESPEasy::net::wifi::NW001_data_struct_WiFi_STA(event->NetworkIndex));
      auto *NW_data = getNWPluginData(event->NetworkIndex);

      if (NW_data) {
        success = NW_data->init(event);
      }
      break;
    }

    case NWPlugin::Function::NWPLUGIN_EXIT:
    {
      auto *NW_data = getNWPluginData(event->NetworkIndex);

      if (NW_data) {
        NW_data->exit(event);
      }
      success = true;
      break;
    }

    case NWPlugin::Function::NWPLUGIN_FIFTY_PER_SECOND:
    {
      // FIXME TD-er: Also called in ESPEasy_loop()
      // Is it still needed there?
      ESPEasy::net::wifi::loopWiFi();
      break;
    }
# ifdef ESP32
    case NWPlugin::Function::NWPLUGIN_PRIORITY_ROUTE_CHANGED:
    {
      ESPEasy::net::wifi::NW001_data_struct_WiFi_STA *NW_data =
        static_cast<ESPEasy::net::wifi::NW001_data_struct_WiFi_STA *>(getNWPluginData(event->NetworkIndex));

      if (NW_data) {
        success = NW_data->handle_priority_route_changed();
      }
      break;
    }
# endif // ifdef ESP32

    default:
      break;

  }
  return success;
}

} // namespace net
} // namespace ESPEasy

#endif // ifdef USES_NW001
