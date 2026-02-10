#include "../DataStructs/SettingsStruct.h"

#include "../../ESPEasy_common.h"

#ifndef DATASTRUCTS_SETTINGSSTRUCT_CPP
# define DATASTRUCTS_SETTINGSSTRUCT_CPP


# include "../CustomBuild/CompiletimeDefines.h"
# include "../CustomBuild/ESPEasyLimits.h"
# include "../DataStructs/DeviceStruct.h"
# include "../ESPEasy/net/DataStructs/NetworkDriverStruct.h"
# include "../DataTypes/NPluginID.h"
# include "../ESPEasy/net/DataTypes/NWPluginID.h"
# include "../ESPEasy/net/DataTypes/NWPluginID.h"
# include "../DataTypes/PluginID.h"
# include "../DataTypes/SPI_options.h"
# include "../Globals/CPlugins.h"
# include "../Globals/Plugins.h"
# include "../Helpers/Misc.h"
# include "../Helpers/StringParser.h"
# include "../ESPEasy/net/Helpers/_NWPlugin_init.h"

# include "../../ESPEasy/net/Globals/NWPlugins.h"

# if FEATURE_I2C_MULTIPLE
#  include "../Helpers/Hardware_device_info.h"
# endif

# if ESP_IDF_VERSION_MAJOR >= 5
#  include <driver/gpio.h>
#  include "include/esp32x_fixes.h"
# endif // if ESP_IDF_VERSION_MAJOR >= 5

# if FEATURE_ETHERNET
#  include "../../ESPEasy/net/DataTypes/EthernetParameters.h"
# endif

# if CONFIG_ETH_USE_ESP32_EMAC && FEATURE_ETHERNET
#  include <pins_arduino.h>
# endif

/*
   // VariousBits1 defaults to 0, keep in mind when adding bit lookups.
template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::appendUnitToHostname()  const {
  return !bitRead(VariousBits1, 1);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::appendUnitToHostname(bool value) {
  bitWrite(VariousBits1, 1, !value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::uniqueMQTTclientIdReconnect_unused() const {
  return bitRead(VariousBits1, 2);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::uniqueMQTTclientIdReconnect_unused(bool value) {
  bitWrite(VariousBits1, 2, value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::OldRulesEngine() const {
  #ifdef WEBSERVER_NEW_RULES
  return !bitRead(VariousBits1, 3);
  #else
  return true;
  #endif
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::OldRulesEngine(bool value) {
  bitWrite(VariousBits1, 3, !value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::ForceWiFi_bg_mode() const {
  return bitRead(VariousBits1, 4);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::ForceWiFi_bg_mode(bool value) {
  bitWrite(VariousBits1, 4, value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::WiFiRestart_connection_lost() const {
  return bitRead(VariousBits1, 5);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::WiFiRestart_connection_lost(bool value) {
  bitWrite(VariousBits1, 5, value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::EcoPowerMode() const {
  return bitRead(VariousBits1, 6);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::EcoPowerMode(bool value) {
  bitWrite(VariousBits1, 6, value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::WifiNoneSleep() const {
  return bitRead(VariousBits1, 7);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::WifiNoneSleep(bool value) {
  bitWrite(VariousBits1, 7, value);
}

// Enable send gratuitous ARP by default, so invert the values (default = 0)
template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::gratuitousARP() const {
  return !bitRead(VariousBits1, 8);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::gratuitousARP(bool value) {
  bitWrite(VariousBits1, 8, !value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::TolerantLastArgParse() const {
  return bitRead(VariousBits1, 9);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::TolerantLastArgParse(bool value) {
  bitWrite(VariousBits1, 9, value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::SendToHttp_ack() const {
  return bitRead(VariousBits1, 10);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::SendToHttp_ack(bool value) {
  bitWrite(VariousBits1, 10, value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::UseESPEasyNow() const {
#ifdef USES_ESPEASY_NOW
  return bitRead(VariousBits1, 11);
#else
  return false;
#endif
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::UseESPEasyNow(bool value) {
#ifdef USES_ESPEASY_NOW
  bitWrite(VariousBits1, 11, value);
#endif
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::IncludeHiddenSSID() const {
  return bitRead(VariousBits1, 12);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::IncludeHiddenSSID(bool value) {
  bitWrite(VariousBits1, 12, value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::UseMaxTXpowerForSending() const {
  return bitRead(VariousBits1, 13);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::UseMaxTXpowerForSending(bool value) {
  bitWrite(VariousBits1, 13, value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::ApCaptivePortal() const {
  return bitRead(VariousBits1, 14);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::ApCaptivePortal(bool value) {
  bitWrite(VariousBits1, 14, value);
}

// VariousBits1 bit 15 was used by PeriodicalScanWiFi
// Now removed, is reset to 0, can be used for some other setting.

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::JSONBoolWithoutQuotes() const {
  return bitRead(VariousBits1, 16);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::JSONBoolWithoutQuotes(bool value) {
  bitWrite(VariousBits1, 16, value);
}
*/

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::CombineTaskValues_SingleEvent(taskIndex_t taskIndex) const {
  if (validTaskIndex(taskIndex)) {
    return bitRead(TaskDeviceSendDataFlags[taskIndex], 0);
  }
  return false;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::CombineTaskValues_SingleEvent(taskIndex_t taskIndex, bool value) {
  if (validTaskIndex(taskIndex)) {
    bitWrite(TaskDeviceSendDataFlags[taskIndex], 0, value);
  }
}

#if FEATURE_STRING_VARIABLES
template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::ShowDerivedTaskValues(taskIndex_t taskIndex) const {
  if (validTaskIndex(taskIndex)) {
    return bitRead(TaskDeviceSendDataFlags[taskIndex], 1);
  }
  return false;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::ShowDerivedTaskValues(taskIndex_t taskIndex, bool value) {
  if (validTaskIndex(taskIndex)) {
    bitWrite(TaskDeviceSendDataFlags[taskIndex], 1, value);
  }
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::EventAndLogDerivedTaskValues(taskIndex_t taskIndex) const {
  if (validTaskIndex(taskIndex)) {
    return bitRead(TaskDeviceSendDataFlags[taskIndex], 2);
  }
  return false;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::EventAndLogDerivedTaskValues(taskIndex_t taskIndex, bool value) {
  if (validTaskIndex(taskIndex)) {
    bitWrite(TaskDeviceSendDataFlags[taskIndex], 2, value);
  }
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::SendDerivedTaskValues(taskIndex_t taskIndex, controllerIndex_t controllerIndex) const {
  if (validTaskIndex(taskIndex) && validControllerIndex(controllerIndex)) {
    return bitRead(TaskDeviceSendDataFlags[taskIndex], 3 + controllerIndex); // ATTENTION: uses bits 3..6!!!
  }
  return false;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::SendDerivedTaskValues(taskIndex_t taskIndex, controllerIndex_t controllerIndex, bool value) {
  if (validTaskIndex(taskIndex) && validControllerIndex(controllerIndex)) {
    bitWrite(TaskDeviceSendDataFlags[taskIndex], 3 + controllerIndex, value); // ATTENTION: uses bits 3..6!!!
  }
}
#endif // if FEATURE_STRING_VARIABLES
/*
template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::DoNotStartAPfallback_ConnectFail() const {
  return bitRead(VariousBits1, 17);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::DoNotStartAPfallback_ConnectFail(bool value) {
  bitWrite(VariousBits1, 17, value);
}


template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::UseAlternativeDeepSleep() const {
  return bitRead(VariousBits1, 18);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::UseAlternativeDeepSleep(bool value) {
  bitWrite(VariousBits1, 18, value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::UseLastWiFiFromRTC() const {
  return bitRead(VariousBits1, 19);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::UseLastWiFiFromRTC(bool value) {
  bitWrite(VariousBits1, 19, value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::EnableTimingStats() const {
  return bitRead(VariousBits1, 20);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::EnableTimingStats(bool value) {
  bitWrite(VariousBits1, 20, value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::AllowTaskValueSetAllPlugins() const {
  return bitRead(VariousBits1, 21);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::AllowTaskValueSetAllPlugins(bool value) {
  bitWrite(VariousBits1, 21, value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::EnableClearHangingI2Cbus() const {
  return bitRead(VariousBits1, 22);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::EnableClearHangingI2Cbus(bool value) {
  bitWrite(VariousBits1, 22, value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::EnableRAMTracking() const {
  return bitRead(VariousBits1, 23);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::EnableRAMTracking(bool value) {
  bitWrite(VariousBits1, 23, value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::EnableRulesCaching() const {
  return !bitRead(VariousBits1, 24);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::EnableRulesCaching(bool value) {
  bitWrite(VariousBits1, 24, !value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::EnableRulesEventReorder() const {
  return !bitRead(VariousBits1, 25);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::EnableRulesEventReorder(bool value) {
  bitWrite(VariousBits1, 25, !value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::AllowOTAUnlimited() const {
  return bitRead(VariousBits1, 26);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::AllowOTAUnlimited(bool value) {
  bitWrite(VariousBits1, 26, value);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::SendToHTTP_follow_redirects() const {
  return bitRead(VariousBits1, 27);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::SendToHTTP_follow_redirects(bool value) {
  bitWrite(VariousBits1, 27, value);
}

#if FEATURE_AUTO_DARK_MODE
template<uint32_t N_TASKS>
uint8_t SettingsStruct_tmpl<N_TASKS>::getCssMode() const {
  return get2BitFromUL(VariousBits1, 28); // Also occupies bit 29!
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::setCssMode(uint8_t value) {
  set2BitToUL(VariousBits1, 28, value); // Also occupies bit 29!
}
#endif // FEATURE_AUTO_DARK_MODE

#if FEATURE_I2C_DEVICE_CHECK
template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::CheckI2Cdevice() const { // Inverted
  return !bitRead(VariousBits1, 30);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::CheckI2Cdevice(bool value) { // Inverted
  bitWrite(VariousBits1, 30, !value);
}
#endif // if FEATURE_I2C_DEVICE_CHECK
*/
/*
template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::WaitWiFiConnect() const { 
  return bitRead(VariousBits2, 0);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::WaitWiFiConnect(bool value) { 
  bitWrite(VariousBits2, 0, value);
}


template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::SDK_WiFi_autoreconnect() const { 
  return bitRead(VariousBits2, 1);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::SDK_WiFi_autoreconnect(bool value) { 
  bitWrite(VariousBits2, 1, value);
}


#if FEATURE_RULES_EASY_COLOR_CODE
template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::DisableRulesCodeCompletion() const { 
  return bitRead(VariousBits2, 2);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::DisableRulesCodeCompletion(bool value) { 
  bitWrite(VariousBits2, 2, value);
}
#endif // if FEATURE_RULES_EASY_COLOR_CODE

#if FEATURE_TARSTREAM_SUPPORT
template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::DisableSaveConfigAsTar() const { 
  return bitRead(VariousBits2, 3); // Using bit 4 now...
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::DisableSaveConfigAsTar(bool value) { 
  bitWrite(VariousBits2, 3, value); // Using bit 4 now...
}
#endif // if FEATURE_TARSTREAM_SUPPORT
*/


template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::isTaskEnableReadonly(taskIndex_t taskIndex) const {
  if (validTaskIndex(taskIndex)) {
    return bitRead(VariousTaskBits[taskIndex], 0);
  }
  return false;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::setTaskEnableReadonly(taskIndex_t taskIndex, bool value) {
  if (validTaskIndex(taskIndex)) {
    bitWrite(VariousTaskBits[taskIndex], 0, value);
  }
}

#if FEATURE_PLUGIN_PRIORITY
template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::isPowerManagerTask(taskIndex_t taskIndex) const {
  if (validTaskIndex(taskIndex)) {
    return bitRead(VariousTaskBits[taskIndex], 1);
  }
  return false;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::setPowerManagerTask(taskIndex_t taskIndex, bool value) {
  if (validTaskIndex(taskIndex)) {
    bitWrite(VariousTaskBits[taskIndex], 1, value);
  }
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::isPriorityTask(taskIndex_t taskIndex) const {
  if (validTaskIndex(taskIndex)) {
    return isPowerManagerTask(taskIndex); // Add more?
  }
  return false;
}
#endif // if FEATURE_PLUGIN_PRIORITY

#if FEATURE_MQTT
template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::SendRetainedTaskValues(taskIndex_t taskIndex, controllerIndex_t controllerIndex) const {
  if (validTaskIndex(taskIndex) && validControllerIndex(controllerIndex)) {
    return bitRead(VariousTaskBits[taskIndex], 2 + controllerIndex); // ATTENTION: uses bits 2..5!!!
  }
  return false;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::SendRetainedTaskValues(taskIndex_t taskIndex, controllerIndex_t controllerIndex, bool value) {
  if (validTaskIndex(taskIndex) && validControllerIndex(controllerIndex)) {
    bitWrite(VariousTaskBits[taskIndex], 2 + controllerIndex, value); // ATTENTION: uses bits 2..5!!!
  }
}
#endif // if FEATURE_MQTT

template<uint32_t N_TASKS>
ExtTimeSource_e SettingsStruct_tmpl<N_TASKS>::ExtTimeSource() const {
  return static_cast<ExtTimeSource_e>(ExternalTimeSource >> 1);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::ExtTimeSource(ExtTimeSource_e value) {
  uint8_t newValue = static_cast<uint8_t>(value) << 1;
  if (UseNTP()) {
    newValue += 1;
  }
  ExternalTimeSource = newValue;
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::UseNTP() const {
  return bitRead(ExternalTimeSource, 0);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::UseNTP(bool value) {
  bitWrite(ExternalTimeSource, 0, value);
}


template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::validate() {
  if (UDPPort > 65535) { UDPPort = 0; }

  if ((Latitude  < -90.0f) || (Latitude > 90.0f)) { Latitude = 0.0f; }

  if ((Longitude < -180.0f) || (Longitude > 180.0f)) { Longitude = 0.0f; }

  if (getVariousBits1() > (1u << 31)) { setVariousBits1(0); } // FIXME: Check really needed/useful?
  ZERO_TERMINATE(Name);
  ZERO_TERMINATE(NTPHost);

  if ((I2C_clockSpeed == 0) || (I2C_clockSpeed > 3400000)) { I2C_clockSpeed = DEFAULT_I2C_CLOCK_SPEED; }
  if (WebserverPort == 0) { WebserverPort = 80;}
  if (SyslogPort == 0) { SyslogPort = 514; }

  #if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  if (console_serial_port == 0 && UseSerial) {
    console_serial_port = DEFAULT_CONSOLE_PORT;
    // Set default RX/TX pins for Serial0
    console_serial_rxpin = DEFAULT_CONSOLE_PORT_RXPIN;
    console_serial_txpin = DEFAULT_CONSOLE_PORT_TXPIN;
  }
#ifdef ESP8266
  if (console_serial_port == 2) {
    // Set default RX/TX pins for Serial0
    console_serial_rxpin = DEFAULT_CONSOLE_PORT_RXPIN;
    console_serial_txpin = DEFAULT_CONSOLE_PORT_TXPIN;
  } else if (console_serial_port == 3) {
    // Set default RX/TX pins for Serial0_swapped
    console_serial_rxpin = 13;
    console_serial_txpin = 15;
  }
#  endif // ifdef ESP8266
  # endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  // Make sure the WiFi and AP drivers are always added and set enabled when loading older settings,
  // or factory default settings
# if FEATURE_ETHERNET
  bool fixedNetworkIndex_drivers_added = false;
# endif

  for (ESPEasy::net::networkDriverIndex_t index; ESPEasy::net::validNetworkDriverIndex(index); ++index)
  {
    const ESPEasy::net::NetworkDriverStruct& nw = ESPEasy::net::getNetworkDriverStruct(index);

    if (nw.alwaysPresent) {
      if (validNetworkIndex(nw.fixedNetworkIndex)) {
        const ESPEasy::net::nwpluginID_t nwpluginID = ESPEasy::net::getNWPluginID_from_NetworkDriverIndex(index);

        if (getNWPluginID_for_network(nw.fixedNetworkIndex) != nwpluginID) {
          setNWPluginID_for_network(nw.fixedNetworkIndex, nwpluginID);
# if FEATURE_ETHERNET
          fixedNetworkIndex_drivers_added = true;
# endif
          setNetworkEnabled(nw.fixedNetworkIndex, nw.enabledOnFactoryReset);
        }
      }
    }
# if FEATURE_ETHERNET
    else if (fixedNetworkIndex_drivers_added) {
      // Check to see if there are other network adapters configured in the settings and not setup yet
      if (ETH_Phy_Type != ESPEasy::net::EthPhyType_t::notSet) {
        const ESPEasy::net::nwpluginID_t nwpluginID = ESPEasy::net::getNWPluginID_from_NetworkDriverIndex(index);

        if ((nwpluginID.value == 3 && ESPEasy::net::isRMII_EthernetType(ETH_Phy_Type)) ||
            (nwpluginID.value == 4 && ESPEasy::net::isSPI_EthernetType(ETH_Phy_Type)))
        {
          const ESPEasy::net::networkIndex_t nw_index = nwpluginID.value - 1;

          if (getNWPluginID_for_network(nw_index) != nwpluginID) {
            setNWPluginID_for_network(nw_index, nwpluginID);
            setNetworkEnabled(nw_index, nw.enabledOnFactoryReset);
          }
        }
      }
    }
#endif
  }
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::networkSettingsEmpty() const {
  return IP[0] == 0 && Gateway[0] == 0 && Subnet[0] == 0 && DNS[0] == 0;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearNetworkSettings() {
  for (uint8_t i = 0; i < 4; ++i) {
    IP[i]      = 0;
    Gateway[i] = 0;
    Subnet[i]  = 0;
    DNS[i]     = 0;
    ETH_IP[i]       = 0;
    ETH_Gateway[i]  = 0;
    ETH_Subnet[i]   = 0;
    ETH_DNS[i]      = 0;
  }
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearTimeSettings() {
  ExternalTimeSource = 0;
  ZERO_FILL(NTPHost);
  TimeZone  = 0;
  DST       = false;
  DST_Start = 0;
  DST_End   = 0;
  Latitude  = 0.0f;
  Longitude = 0.0f;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearNotifications() {
  for (uint8_t i = 0; i < NOTIFICATION_MAX; ++i) {
    Notification[i]        = 0u;// .setInvalid();
    NotificationEnabled[i] = false;
  }
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearControllers() {
  for (controllerIndex_t i = 0; i < CONTROLLER_MAX; ++i) {
    Protocol[i]          = 0;
    ControllerEnabled[i] = false;
  }
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearTasks() {
  for (taskIndex_t task = 0; task < N_TASKS; ++task) {
    clearTask(task);
  }
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearLogSettings() {
  SyslogLevel    = 0;
  SerialLogLevel = 0;
  WebLogLevel    = 0;
  SDLogLevel     = 0;
  SyslogFacility = DEFAULT_SYSLOG_FACILITY;
  ZERO_FILL(Syslog_IP);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearUnitNameSettings() {
  Unit = 0;
  appendUnitToHostname(false);
  ZERO_FILL(Name);
  UDPPort = 0;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearMisc() {
  PID                      = ESP_PROJECT_PID;
  Version                  = VERSION;
  Build                    = get_build_nr();
  IP_Octet                 = 0;
  Delay                    = DEFAULT_DELAY;
  Pin_i2c_sda              = DEFAULT_PIN_I2C_SDA;
  Pin_i2c_scl              = DEFAULT_PIN_I2C_SCL;
  Pin_status_led           = DEFAULT_PIN_STATUS_LED;
  Pin_status_led_Inversed  = DEFAULT_PIN_STATUS_LED_INVERSED;
  Pin_sd_cs                = -1;
#ifdef ESP32
  #if FEATURE_I2C_MULTIPLE
  Pin_i2c2_sda             = DEFAULT_PIN_I2C2_SDA;
  Pin_i2c2_scl             = DEFAULT_PIN_I2C2_SCL;
  Pin_i2c3_sda             = DEFAULT_PIN_I2C3_SDA;
  Pin_i2c3_scl             = DEFAULT_PIN_I2C3_SCL;
  #endif
  // Ethernet related settings are never used on ESP8266
  ETH_Phy_Addr             = DEFAULT_ETH_PHY_ADDR;
  ETH_Pin_mdc_cs           = DEFAULT_ETH_PIN_MDC;
  ETH_Pin_mdio_irq         = DEFAULT_ETH_PIN_MDIO;
  ETH_Pin_power_rst        = DEFAULT_ETH_PIN_POWER;
  ETH_Phy_Type             = DEFAULT_ETH_PHY_TYPE;
  ETH_Clock_Mode           = DEFAULT_ETH_CLOCK_MODE;
#endif
  NetworkMedium            = DEFAULT_NETWORK_MEDIUM;

  I2C_clockSpeed_Slow      = DEFAULT_I2C_CLOCK_SPEED_SLOW;
  I2C_Multiplexer_Type     = I2C_MULTIPLEXER_NONE;
  I2C_Multiplexer_Addr     = -1;
  memset(I2C_Multiplexer_Channel, -1, sizeof(I2C_Multiplexer_Channel));
  I2C_Multiplexer_ResetPin = -1;

  {
    // Here we initialize all data to 0, so this is the ONLY reason why PinBootStates 
    // can now be directly accessed.
    // In all other use cases, use the get and set functions for it.

    ZERO_FILL(PinBootStates);
    # ifdef ESP32
    ZERO_FILL(PinBootStates_ESP32);
    # endif // ifdef ESP32
  }
  BaudRate                         = DEFAULT_SERIAL_BAUD;
  MessageDelay_unused              = 0;
  deepSleep_wakeTime               = 0;
  CustomCSS                        = false;
  WDI2CAddress                     = 0;
  UseRules                         = DEFAULT_USE_RULES;
  UseSerial                        = DEFAULT_USE_SERIAL;
  UseSSDP                          = false;
  WireClockStretchLimit            = 0;
  I2C_clockSpeed                   = DEFAULT_I2C_CLOCK_SPEED;
  WebserverPort                    = 80;
  SyslogPort                       = 514;
  VariousBits_3._all_bits          = 0;
  ConnectionFailuresThreshold      = 0;
  MQTTRetainFlag_unused            = false;
  InitSPI                          = DEFAULT_SPI;
  deepSleepOnFail                  = false;
  UseValueLogger                   = false;
  ArduinoOTAEnable                 = false;
  UseRTOSMultitasking              = false;
  Pin_Reset                        = -1;
  StructSize                       = sizeof(SettingsStruct_tmpl<N_TASKS>);
  MQTTUseUnitNameAsClientId_unused = 0;
  setVariousBits1(0);
  setVariousBits2(0);

  console_serial_port              = DEFAULT_CONSOLE_PORT; 
  console_serial_rxpin             = DEFAULT_CONSOLE_PORT_RXPIN;
  console_serial_txpin             = DEFAULT_CONSOLE_PORT_TXPIN;
  console_serial0_fallback         = DEFAULT_CONSOLE_SER0_FALLBACK;

  OldRulesEngine(DEFAULT_RULES_OLDENGINE);
  ForceWiFi_bg_mode(DEFAULT_WIFI_FORCE_BG_MODE);
  WiFiRestart_connection_lost(DEFAULT_WIFI_RESTART_WIFI_CONN_LOST);
  EcoPowerMode(DEFAULT_ECO_MODE);
  WifiNoneSleep(DEFAULT_WIFI_NONE_SLEEP);
  gratuitousARP(DEFAULT_GRATUITOUS_ARP);
  TolerantLastArgParse(DEFAULT_TOLERANT_LAST_ARG_PARSE);
  SendToHttp_ack(DEFAULT_SEND_TO_HTTP_ACK);
  #ifdef USES_ESPEASY_NOW
  UseESPEasyNow(DEFAULT_USE_ESPEASYNOW);
  #else
  UseESPEasyNow(false);
  #endif
  ApCaptivePortal(DEFAULT_AP_FORCE_SETUP);
  DoNotStartAPfallback_ConnectFail(DEFAULT_DONT_ALLOW_START_AP);
}


template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearTask(taskIndex_t task) {
  if (task >= N_TASKS) { return; }

  for (controllerIndex_t i = 0; i < CONTROLLER_MAX; ++i) {
    TaskDeviceID[i][task]       = 0u;
    TaskDeviceSendData[i][task] = false;
  }
  TaskDeviceNumber[task]     = 0u; //.setInvalid();
  OLD_TaskDeviceID[task]     = 0u; // UNUSED: this can be removed
  TaskDevicePin1[task]       = -1;
  TaskDevicePin2[task]       = -1;
  TaskDevicePin3[task]       = -1;
  TaskDevicePort[task]       = 0u;
  TaskDevicePin1PullUp[task] = false;

  for (uint8_t cv = 0; cv < PLUGIN_CONFIGVAR_MAX; ++cv) {
    TaskDevicePluginConfig[task][cv] = 0;
  }
  TaskDevicePin1Inversed[task] = false;

  for (uint8_t cv = 0; cv < PLUGIN_CONFIGFLOATVAR_MAX; ++cv) {
    TaskDevicePluginConfigFloat[task][cv] = 0.0f;
  }

  for (uint8_t cv = 0; cv < PLUGIN_CONFIGLONGVAR_MAX; ++cv) {
    TaskDevicePluginConfigLong[task][cv] = 0;
  }
  TaskDeviceSendDataFlags[task] = 0u;
  VariousTaskBits[task]         = 0;
  TaskDeviceDataFeed[task]      = 0u;
  TaskDeviceTimer[task]         = 0u;
//  TaskDeviceEnabled[task].value = 0u; // Should also clear any temporary flags.
  TaskDeviceEnabled[task]       = false;
  I2C_Multiplexer_Channel[task] = -1;
}

template<uint32_t N_TASKS>
String SettingsStruct_tmpl<N_TASKS>::getHostname() const {
  return this->getHostname(this->appendUnitToHostname());
}

template<uint32_t N_TASKS>
String SettingsStruct_tmpl<N_TASKS>::getHostname(bool appendUnit) const {
  String hostname = this->getName();

  if ((this->Unit != 0) && appendUnit) { // only append non-zero unit number
    hostname += '_';
    hostname += this->Unit;
  }
  return hostname;
}

template<uint32_t N_TASKS>
String SettingsStruct_tmpl<N_TASKS>::getName() const {
  String unitname = this->Name;

  return parseTemplate(unitname);
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::getPinBootStateIndex(
  int8_t  gpio_pin,
  int8_t& index_low
    # ifdef ESP32
  , int8_t& index_high
    # endif // ifdef ESP32
  ) const {
  index_low = -1;
# ifdef ESP32
  index_high = -1;
  if ((gpio_pin < 0) || !(GPIO_IS_VALID_GPIO(gpio_pin))) { return false; }
# endif // ifdef ESP32
  constexpr int maxStates = NR_ELEMENTS(PinBootStates);

  if (gpio_pin < maxStates) {
    index_low = gpio_pin;
    return true;
  }
# ifdef ESP32
  constexpr int maxStatesesp32 = NR_ELEMENTS(PinBootStates_ESP32);

  index_high = gpio_pin - maxStates;

#  if defined(ESP32_CLASSIC) || defined(ESP32C2) || defined(ESP32C3) || defined(ESP32C5) || defined(ESP32C5) || defined(ESP32C6) || defined(ESP32C61)

  // These can all store in the PinBootStates_ESP32 array
  return (index_high < maxStatesesp32);

#  elif defined(ESP32S2)

  // First make sure we're not dealing with flash/PSRAM connected pins
  if (!((gpio_pin > 21) && (gpio_pin < 33) && (gpio_pin != 26))) {
    // Previously used index, to maintain compatibility with previous settings.

    if (index_high >= maxStatesesp32) {
      // Now try to fix the bug by inserting the missing ones into unused spots
      // This way we don't need to convert existing settings
      index_high -= maxStatesesp32;
      constexpr int8_t offsetFlashPin = 22 - maxStates;
      index_high += offsetFlashPin;

      if (gpio_pin >= 26) {
        // Skip index for GPIO 26
        ++index_high;
      }
    }
    return (index_high < maxStatesesp32);
  }

#  elif defined(ESP32S3)

  // GPIO 22 ... 32 should never be used.
  // Thus:
  //  - map <maxStates> ... <21> to the beginning of PinBootStates_ESP32
  //  - map <33> ... <48> to the end of PinBootStates_ESP32
  if (gpio_pin < 22) {
    return true;
  }

  if (gpio_pin >= 33) {
    index_high = gpio_pin - maxStates + 22 - 33;
    return true;
  }

#  elif defined(ESP32P4)

  // GPIO 27 ... 39 should never be used.
  // Thus:
  //  - map <maxStates> ... <27> to the beginning of PinBootStates_ESP32
  //  - map <40> ... <54> to the end of PinBootStates_ESP32
  if (gpio_pin < 27) {
    return true;
  }

  if (gpio_pin >= 40) {
    index_high = gpio_pin - maxStates + 27 - 39;
    return true;
  }

#  else // if defined(ESP32_CLASSIC) || defined(ESP32C3)

  static_assert(false, "Implement processor architecture");
  
#  endif // if defined(ESP32_CLASSIC) || defined(ESP32C3)
# endif // ifdef ESP32

  return false;
}

template<uint32_t N_TASKS>
PinBootState SettingsStruct_tmpl<N_TASKS>::getPinBootState(int8_t gpio_pin) const {
  if (gpio_pin < 0) return PinBootState::Default_state;
# ifdef ESP8266
  int8_t index_low{};

  if (getPinBootStateIndex(gpio_pin, index_low)) {
    return static_cast<PinBootState>(PinBootStates[index_low]);
  }

# endif // ifdef ESP8266
# ifdef ESP32
  int8_t index_low{};
  int8_t index_high{};

  if (getPinBootStateIndex(gpio_pin, index_low, index_high)) {
    if (index_low >= 0) {
      return static_cast<PinBootState>(PinBootStates[index_low]);
    }

    if (index_high >= 0) {
      return static_cast<PinBootState>(PinBootStates_ESP32[index_high]);
    }
  }
# endif // ifdef ESP32
  return PinBootState::Default_state;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::setPinBootState(int8_t gpio_pin, PinBootState state) {
  if (gpio_pin < 0) return;
# ifdef ESP8266
  int8_t index_low{};

  if (getPinBootStateIndex(gpio_pin, index_low)) {
    PinBootStates[index_low] = static_cast<int8_t>(state);
  }
# endif // ifdef ESP8266

# ifdef ESP32
  int8_t index_low{};
  int8_t index_high{};

  if (getPinBootStateIndex(gpio_pin, index_low, index_high)) {
    if (index_low >= 0) {
      PinBootStates[index_low] = static_cast<int8_t>(state);
    }

    if (index_high >= 0) {
      PinBootStates_ESP32[index_high] = static_cast<int8_t>(state);
    }
  }
# endif // ifdef ESP32
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::isSPI_enabled(uint8_t spi_bus) const {
  const SPI_Options_e SPI_selection = static_cast<SPI_Options_e>(0 == spi_bus ? InitSPI : InitSPI1);
  return SPI_Options_e::None != SPI_selection;
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::getSPI_pins(int8_t spi_gpios[3], uint8_t spi_bus, bool noCheck) const {
  spi_gpios[0] = -1;
  spi_gpios[1] = -1;
  spi_gpios[2] = -1;

  if (noCheck || isSPI_valid(spi_bus)) {
    # ifdef ESP32
    const SPI_Options_e SPI_selection = static_cast<SPI_Options_e>(0 == spi_bus ? InitSPI : InitSPI1);

    switch (SPI_selection) {
      case SPI_Options_e::Vspi_Fspi:
      {
        spi_gpios[0] = VSPI_FSPI_SCK; 
        spi_gpios[1] = VSPI_FSPI_MISO; 
        spi_gpios[2] = VSPI_FSPI_MOSI;
        break;
      }
#ifdef ESP32_CLASSIC
      case SPI_Options_e::Hspi:
      {
        spi_gpios[0] = HSPI_SCLK;
        spi_gpios[1] = HSPI_MISO;
        spi_gpios[2] = HSPI_MOSI;
        break;
      }
#endif
      case SPI_Options_e::UserDefined:
      {
        #ifdef ESP32
        if (0 == spi_bus)
        #endif // ifdef ESP32
        {
          spi_gpios[0] = SPI_SCLK_pin;
          spi_gpios[1] = SPI_MISO_pin;
          spi_gpios[2] = SPI_MOSI_pin;
        }
        #ifdef ESP32
        else if (1 == spi_bus) {
          spi_gpios[0] = SPI1_SCLK_pin;
          spi_gpios[1] = SPI1_MISO_pin;
          spi_gpios[2] = SPI1_MOSI_pin;
        }
        #endif // ifdef ESP32
        break;
      }
      case SPI_Options_e::None:
        return false;
    }
    # endif // ifdef ESP32
    # ifdef ESP8266
    spi_gpios[0] = 14; spi_gpios[1] = 12; spi_gpios[2] = 13;
    # endif // ifdef ESP8266
    return true;
  }
  return false;
}

#ifdef ESP32
template<uint32_t N_TASKS>
spi_host_device_t SettingsStruct_tmpl<N_TASKS>::getSPI_host(uint8_t spi_bus) const
{
  if (isSPI_valid(spi_bus)) {
    const SPI_Options_e SPI_selection = static_cast<SPI_Options_e>(0 == spi_bus ? InitSPI : InitSPI1);
    switch (SPI_selection) {
      case SPI_Options_e::Vspi_Fspi:
      {
        #if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
        return static_cast<spi_host_device_t>(FSPI_HOST);
        #else
        return static_cast<spi_host_device_t>(VSPI_HOST);
        #endif
      }
#ifdef ESP32_CLASSIC
      case SPI_Options_e::Hspi:
      {
        return static_cast<spi_host_device_t>(HSPI_HOST);
      }
#endif
      case SPI_Options_e::UserDefined:
      {
        #if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
        return static_cast<spi_host_device_t>(FSPI_HOST);
        #else
        return static_cast<spi_host_device_t>(VSPI_HOST);
        #endif
      }
      case SPI_Options_e::None:
        break;
    }

  }
  #if ESP_IDF_VERSION_MAJOR < 5
  #if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
  return static_cast<spi_host_device_t>(FSPI_HOST);
  #else
  return static_cast<spi_host_device_t>(VSPI_HOST);
  #endif
  #else
  return spi_host_device_t::SPI_HOST_MAX;
  #endif
}
#endif


template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::isSPI_pin(int8_t pin, uint8_t spi_bus) const {
  if (pin < 0) { return false; }
  int8_t spi_gpios[3];

  if (getSPI_pins(spi_gpios, 0u) && ((0xFF == spi_bus) || (0u == spi_bus))) {
    for (uint8_t i = 0; i < 3; ++i) {
      if (spi_gpios[i] == pin) { return true; }
    }
  }
  #ifdef ESP32
  if ((getSPIBusCount() > 1u) && getSPI_pins(spi_gpios, 1u) && ((0xFF == spi_bus) || (1u == spi_bus))) {
    for (uint8_t i = 0; i < 3; ++i) {
      if (spi_gpios[i] == pin) { return true; }
    }
  }
  #endif // ifdef ESP32
  return false;
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::isSPI_valid(uint8_t spi_bus) const {
  int8_t spi0_pins[3];
  bool result = false;
  getSPI_pins(spi0_pins, 0u, true);

  #ifdef ESP32
  if (0 == spi_bus)
  #endif // ifdef ESP32
  {
    if (InitSPI == static_cast<uint8_t>(SPI_Options_e::None)) { return false; }

    result = !((spi0_pins[0] == -1) ||
               (spi0_pins[1] == -1) ||
               (spi0_pins[2] == -1) ||
               (spi0_pins[0] == spi0_pins[1]) ||
               (spi0_pins[1] == spi0_pins[2]) ||
               (spi0_pins[2] == spi0_pins[0]));
    #ifdef ESP32
    if (result && (getSPIBusCount() > 1)) { // Cross-check pins with other bus
      int8_t spi1_pins[3];
      getSPI_pins(spi1_pins, 1u, true);

      for (uint8_t i = 0; (i < 3) && result; ++i) {
        for (uint8_t j = 0; (j < 3) && result; ++j) {
          if (spi0_pins[i] == spi1_pins[j]) {
            result = false;
          }
        }
      }
    }
    #endif // ifdef ESP32
  }
  #ifdef ESP32
  else if ((1 == spi_bus) && (getSPIBusCount() > 1)) {
    if (InitSPI1 == static_cast<uint8_t>(SPI_Options_e::None)) { return false; }
    int8_t spi1_pins[3];
    getSPI_pins(spi1_pins, 1u, true);

    result = !((spi1_pins[0] == -1) ||
               (spi1_pins[1] == -1) ||
               (spi1_pins[2] == -1) ||
               (spi1_pins[0] == spi1_pins[1]) ||
               (spi1_pins[1] == spi1_pins[2]) ||
               (spi1_pins[2] == spi1_pins[0]));

    if (result) { // Cross-check pins
      for (uint8_t i = 0; (i < 3) && result; ++i) {
        for (uint8_t j = 0; (j < 3) && result; ++j) {
          if (spi0_pins[i] == spi1_pins[j]) {
            result = false;
          }
        }
      }
    }
  }
  #endif // ifdef ESP32
  return result;
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::isI2C_pin(int8_t pin) const {
  if (pin < 0) { return false; }
  #if !FEATURE_I2C_MULTIPLE
  const uint8_t i2cBus = 0;
  #else // if !FEATURE_I2C_MULTIPLE
  for (uint8_t i2cBus = 0; i2cBus < getI2CBusCount(); ++i2cBus)
  #endif // if !FEATURE_I2C_MULTIPLE
  {
    if ((getI2CSdaPin(i2cBus) == pin) || (getI2CSclPin(i2cBus) == pin)) {
      return true;
    }
  }
  return false;
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::isI2CEnabled(uint8_t i2cBus) const {
  return (getI2CSdaPin(i2cBus) != -1) &&
        (getI2CSclPin(i2cBus) != -1) &&
        (getI2CClockSpeed(i2cBus) > 0) &&
        (getI2CClockSpeedSlow(i2cBus) > 0);
}

// stored in I2C_SPI_bus_Flags per Task
template<uint32_t N_TASKS>
uint8_t SettingsStruct_tmpl<N_TASKS>::getSPIBusForTask(taskIndex_t TaskIndex) const {
  return get2BitFromUL(I2C_SPI_bus_Flags[TaskIndex], SPI_FLAGS_TASK_BUS_NUMBER);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::setSPIBusForTask(taskIndex_t TaskIndex, uint8_t spi_bus) {
  set2BitToUL(I2C_SPI_bus_Flags[TaskIndex], SPI_FLAGS_TASK_BUS_NUMBER, spi_bus);
}

#if FEATURE_SD
// stored in I2C_SPI_bus_Flags for Task 1 (index 0)
template<uint32_t N_TASKS>
uint8_t SettingsStruct_tmpl<N_TASKS>::getSPIBusForSDCard() const {
  return get2BitFromUL(I2C_SPI_bus_Flags[0], SPI_FLAGS_SDCARD_BUS_NUMBER);
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::setSPIBusForSDCard(uint8_t spi_bus) {
  set2BitToUL(I2C_SPI_bus_Flags[0], SPI_FLAGS_SDCARD_BUS_NUMBER, spi_bus);
}
#endif // if FEATURE_SD

template<uint32_t N_TASKS>
uint8_t SettingsStruct_tmpl<N_TASKS>::getI2CInterface(taskIndex_t TaskIndex) const {
  return get3BitFromUL(I2C_SPI_bus_Flags[TaskIndex], I2C_FLAGS_BUS_NUMBER);
}

template<uint32_t N_TASKS>
int8_t SettingsStruct_tmpl<N_TASKS>::getI2CSdaPin(uint8_t i2cBus) const {
  if (0 == i2cBus) {
    return Pin_i2c_sda;
  #if FEATURE_I2C_MULTIPLE
  } else if (1 == i2cBus) {
    return Pin_i2c2_sda;
  #if FEATURE_I2C_INTERFACE_3
  } else {
    return Pin_i2c3_sda;
  #endif // if FEATURE_I2C_INTERFACE_3
  #endif // if FEATURE_I2C_MULTIPLE
  }
  return -1;
}

template<uint32_t N_TASKS>
int8_t SettingsStruct_tmpl<N_TASKS>::getI2CSclPin(uint8_t i2cBus) const {
  if (0 == i2cBus) {
    return Pin_i2c_scl;
  #if FEATURE_I2C_MULTIPLE
  } else if (1 == i2cBus) {
    return Pin_i2c2_scl;
  #if FEATURE_I2C_INTERFACE_3
  } else {
    return Pin_i2c3_scl;
  #endif // if FEATURE_I2C_INTERFACE_3
  #endif // if FEATURE_I2C_MULTIPLE
  }
  return -1;
}

template<uint32_t N_TASKS>
uint32_t SettingsStruct_tmpl<N_TASKS>::getI2CClockSpeed(uint8_t i2cBus) const {
  if (0 == i2cBus) {
    return I2C_clockSpeed;
  #if FEATURE_I2C_MULTIPLE
  } else if (1 == i2cBus) {
    return I2C2_clockSpeed;
  #if FEATURE_I2C_INTERFACE_3
  } else {
    return I2C3_clockSpeed;
  #endif // if FEATURE_I2C_INTERFACE_3
  #endif // if FEATURE_I2C_MULTIPLE
  }
  return 0u;
}

template<uint32_t N_TASKS>
uint32_t SettingsStruct_tmpl<N_TASKS>::getI2CClockSpeedSlow(uint8_t i2cBus) const {
  if (0 == i2cBus) {
    return I2C_clockSpeed_Slow;
  #if FEATURE_I2C_MULTIPLE
  } else if (1 == i2cBus) {
    return I2C2_clockSpeed_Slow;
  #if FEATURE_I2C_INTERFACE_3
  } else {
    return I2C3_clockSpeed_Slow;
  #endif // if FEATURE_I2C_INTERFACE_3
  #endif // if FEATURE_I2C_MULTIPLE
  }
  return 0u;
}

template<uint32_t N_TASKS>
uint32_t SettingsStruct_tmpl<N_TASKS>::getI2CClockStretch(uint8_t i2cBus) const {
  if (0 == i2cBus) {
    return WireClockStretchLimit;
  }
  return 0u;
}

#if FEATURE_I2C_MULTIPLE
template<uint32_t N_TASKS>
uint8_t SettingsStruct_tmpl<N_TASKS>::getI2CInterfaceRTC() const {
  return get3BitFromUL(I2C_peripheral_bus, I2C_PERIPHERAL_BUS_CLOCK);
}

template<uint32_t N_TASKS>
uint8_t SettingsStruct_tmpl<N_TASKS>::getI2CInterfaceWDT() const {
  return get3BitFromUL(I2C_peripheral_bus, I2C_PERIPHERAL_BUS_WDT);
}

template<uint32_t N_TASKS>
uint8_t SettingsStruct_tmpl<N_TASKS>::getI2CInterfacePCFMCP() const {
  return get3BitFromUL(I2C_peripheral_bus, I2C_PERIPHERAL_BUS_PCFMCP);
}
#endif // if FEATURE_I2C_MULTIPLE

#if FEATURE_I2CMULTIPLEXER
template<uint32_t N_TASKS>
int8_t SettingsStruct_tmpl<N_TASKS>::getI2CMultiplexerType(uint8_t i2cBus) const {
  if (0 == i2cBus) {
    return I2C_Multiplexer_Type;
  #if FEATURE_I2C_MULTIPLE
  } else if (1 == i2cBus) {
    return I2C2_Multiplexer_Type;
  #if FEATURE_I2C_INTERFACE_3
  } else {
    return I2C3_Multiplexer_Type;
  #endif // if FEATURE_I2C_INTERFACE_3
  #endif // if FEATURE_I2C_MULTIPLE
  }
  return -1;
}

template<uint32_t N_TASKS>
int8_t SettingsStruct_tmpl<N_TASKS>::getI2CMultiplexerAddr(uint8_t i2cBus) const {
  if (0 == i2cBus) {
    return I2C_Multiplexer_Addr;
  #if FEATURE_I2C_MULTIPLE
  } else if (1 == i2cBus) {
    return I2C2_Multiplexer_Addr;
  #if FEATURE_I2C_INTERFACE_3
  } else {
    return I2C3_Multiplexer_Addr;
  #endif // if FEATURE_I2C_INTERFACE_3
  #endif // if FEATURE_I2C_MULTIPLE
  }
  return -1;
}

template<uint32_t N_TASKS>
int8_t SettingsStruct_tmpl<N_TASKS>::getI2CMultiplexerResetPin(uint8_t i2cBus) const {
  if (0 == i2cBus) {
    return I2C_Multiplexer_ResetPin;
  #if FEATURE_I2C_MULTIPLE
  } else if (1 == i2cBus) {
    return I2C2_Multiplexer_ResetPin;
  #if FEATURE_I2C_INTERFACE_3
  } else {
    return I2C3_Multiplexer_ResetPin;
  #endif // if FEATURE_I2C_INTERFACE_3
  #endif // if FEATURE_I2C_MULTIPLE
  }
  return -1;
}
#endif // if FEATURE_I2CMULTIPLEXER

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::isEthernetPin(int8_t pin) const {
  #if FEATURE_ETHERNET
  if (pin < 0) return false;
  if (NetworkMedium == ESPEasy::net::NetworkMedium_t::Ethernet &&
      !ESPEasy::net::isSPI_EthernetType(ETH_Phy_Type)) {
  #if CONFIG_ETH_USE_ESP32_EMAC && FEATURE_ETHERNET
  #ifdef ESP32P4
  switch (pin) {
    // MDC/MDIO/Power can be configured by user
    //case ETH_PHY_MDC     :
    //case ETH_PHY_MDIO    :
    //case ETH_PHY_POWER   :
    case ETH_RMII_TX_EN  :
    case ETH_RMII_TX0    :
    case ETH_RMII_TX1    :
    case ETH_RMII_RX0    :
    case ETH_RMII_RX1_EN :
    case ETH_RMII_CRS_DV :
    case ETH_RMII_CLK    :
      return true;
  }
  #else
    if (19 == pin) return true; // ETH TXD0
    if (21 == pin) return true; // ETH TX EN
    if (22 == pin) return true; // ETH TXD1
    if (25 == pin) return true; // ETH RXD0
    if (26 == pin) return true; // ETH RXD1
    if (27 == pin) return true; // ETH CRS_DV
  #endif
  #endif
  }
  #endif // if FEATURE_ETHERNET
  return false;
}


template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::isEthernetPinOptional(int8_t pin) const {
  #if FEATURE_ETHERNET
  if (pin < 0) return false;
  if (NetworkMedium == ESPEasy::net::NetworkMedium_t::Ethernet) {
    if (!ESPEasy::net::isSPI_EthernetType(ETH_Phy_Type) 
# if CONFIG_ETH_USE_ESP32_EMAC && FEATURE_ETHERNET
    && ESPEasy::net::isGpioUsedInETHClockMode(ETH_Clock_Mode, pin)
  #endif
  ) return true;
    if (ETH_Pin_mdc_cs == pin) return true;
    if (ETH_Pin_mdio_irq == pin) return true;
    if (ETH_Pin_power_rst == pin) return true;
  }
  #endif // if FEATURE_ETHERNET
  return false;
}

template<uint32_t N_TASKS>
int8_t SettingsStruct_tmpl<N_TASKS>::getTaskDevicePin(taskIndex_t taskIndex, uint8_t pinnr) const {
  if (validTaskIndex(taskIndex)) {
    switch(pinnr) {
      case 1: return TaskDevicePin1[taskIndex];
      case 2: return TaskDevicePin2[taskIndex];
      case 3: return TaskDevicePin3[taskIndex];
    }
  }
  return -1;
}

template<uint32_t N_TASKS>
float SettingsStruct_tmpl<N_TASKS>::getWiFi_TX_power() const {
  return WiFi_TX_power / 4.0f;
}
  
template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::setWiFi_TX_power(float dBm) {
  WiFi_TX_power = dBm * 4.0f;
}

template<uint32_t N_TASKS>
pluginID_t SettingsStruct_tmpl<N_TASKS>::getPluginID_for_task(taskIndex_t taskIndex) const {
  if (validTaskIndex(taskIndex)) {
    const uint8_t tdn = TaskDeviceNumber[taskIndex];
    if (tdn > 0) {
      return pluginID_t::toPluginID(tdn);
    }
  }
  return INVALID_PLUGIN_ID;
}

template<uint32_t N_TASKS>
ESPEasy::net::nwpluginID_t SettingsStruct_tmpl<N_TASKS>::getNWPluginID_for_network(ESPEasy::net::networkIndex_t index) const
{
  if (validNetworkIndex(index)) {
    const uint8_t nwa = NWPluginID[index];
    if (nwa > 0) {
      return ESPEasy::net::nwpluginID_t::toPluginID(nwa);
    }
  }
  return ESPEasy::net::INVALID_NW_PLUGIN_ID;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::setNWPluginID_for_network(ESPEasy::net::networkIndex_t index, ESPEasy::net::nwpluginID_t id)
{
  if (validNetworkIndex(index)) {
    NWPluginID[index] = id.value;
    if (id.isValid()) {
      ESPEasy::net::networkDriverIndex_t NetworkDriverIndex = 
        ESPEasy::net::getNetworkDriverIndex_from_NetworkIndex(index);

      if (ESPEasy::net::validNetworkDriverIndex(NetworkDriverIndex)) {
        struct EventStruct TempEvent;
        TempEvent.NetworkIndex = index;

        String dummy;
        ESPEasy::net::NWPluginCall(NWPlugin::Function::NWPLUGIN_LOAD_DEFAULTS, &TempEvent, dummy);
      }
    }
  }
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::getNetworkEnabled(ESPEasy::net::networkIndex_t index) const {
  //if (index == 1) return true;
  if (validNetworkIndex(index)) return bitRead(NetworkEnabled_bits, index);
  return false;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::setNetworkEnabled(ESPEasy::net::networkIndex_t index, bool enabled) {
  if (validNetworkIndex(index)) {
    bitWrite(NetworkEnabled_bits, index, enabled);
  }
}

template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::getNetworkInterfaceSubnetBlockClientIP(ESPEasy::net::networkIndex_t index) const {
  if (validNetworkIndex(index)) return bitRead(NetworkInterfaceSubnetBlockClientIP_bits, index);
  return false;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::setNetworkInterfaceSubnetBlockClientIP(ESPEasy::net::networkIndex_t index, bool enabled) {
  if (validNetworkIndex(index)) {
    bitWrite(NetworkInterfaceSubnetBlockClientIP_bits, index, enabled);
  }
}

#if FEATURE_USE_IPV6
template<uint32_t N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::getNetworkEnabled_IPv6(ESPEasy::net::networkIndex_t index) const {
  //if (index == 1) return true;
  if (validNetworkIndex(index)) return bitRead(NetworkEnabled_ipv6_bits, index);
  return false;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::setNetworkEnabled_IPv6(ESPEasy::net::networkIndex_t index, bool enabled) {
  if (validNetworkIndex(index)) {
    bitWrite(NetworkEnabled_ipv6_bits, index, enabled);
  }
}
#endif


#ifdef ESP32
template<uint32_t N_TASKS>
uint8_t SettingsStruct_tmpl<N_TASKS>::getRoutePrio_for_network(ESPEasy::net::networkIndex_t index) const
{
  if (validNetworkIndex(index)) {
    return NetworkRoutePrio[index];
  }
  return 0;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::setRoutePrio_for_network(ESPEasy::net::networkIndex_t index, uint8_t prio)
{
  if (validNetworkIndex(index)) {
    NetworkRoutePrio[index] = prio;
  }
}
#endif

template<uint32_t N_TASKS>
uint32_t SettingsStruct_tmpl<N_TASKS>::getNetworkInterfaceStartupDelayAtBoot(ESPEasy::net::networkIndex_t index) const
{
  if (validNetworkIndex(index)) {
    return static_cast<uint32_t>(NetworkInterfaceStartupDelayAtBoot[index]) * 10ul;
  }
  return 0;
}

template<uint32_t N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::setNetworkInterfaceStartupDelayAtBoot(ESPEasy::net::networkIndex_t index, uint32_t delay_ms)
{
  if (validNetworkIndex(index)) {
    NetworkInterfaceStartupDelayAtBoot[index] = delay_ms/10ul;
  }
}


#endif // ifndef DATASTRUCTS_SETTINGSSTRUCT_CPP
