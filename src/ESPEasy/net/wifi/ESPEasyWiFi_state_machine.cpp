#include "../wifi/ESPEasyWiFi_state_machine.h"
#include "ESPEasy/net/wifi/WiFi_STA_State.h"

#if FEATURE_WIFI

# include "../../../src/ESPEasyCore/ESPEasy_Log.h"
# include "../../../src/Globals/ESPEasy_Scheduler.h"
# include "../../../src/Globals/RTC.h"
# include "../../../src/Globals/SecuritySettings.h"
# include "../../../src/Globals/Settings.h"

# include "../ESPEasyNetwork.h" // for setNetworkMedium, however this should not be part of the WiFi code
# include "../Globals/WiFi_AP_Candidates.h"
# include "../wifi/ESPEasyWifi.h"
# include "../wifi/ESPEasyWifi_abstracted.h"


namespace ESPEasy {
namespace net {
namespace wifi {

    # define WIFI_STATE_MACHINE_STA_CONNECTING_TIMEOUT   10000
    # define WIFI_STATE_MACHINE_STA_AP_SCANNING_TIMEOUT  10000
    # define WIFI_STATE_MACHINE_STA_SCANNING_TIMEOUT     10000
    # define WIFI_STATE_MACHINE_AP_ONLY_TIMEOUT          60000

    # define WIFI_STATE_MACHINE_STA_CONNECTED_STABLE     300000 // 5 minutes

void ESPEasyWiFi_t::setup() {
  if (!Settings.getNetworkEnabled(NETWORK_INDEX_WIFI_STA)) { return; }

  WiFi_AP_Candidates.clearCache();
  WiFi_AP_Candidates.load_knownCredentials();

  // TODO TD-er: Must maybe also call 'disable()' first?

  // TODO TD-er: Load settings

  // TODO TD-er: Check if settings have changed.

  if (_disabledAtBoot) {
    disable();
    return;
  }

  if (WiFi_pre_setup()) { begin(); }
}

void ESPEasyWiFi_t::enable()  {}

void ESPEasyWiFi_t::disable() { setState(WiFi_STA_State_e::Disabled, 100); }

void ESPEasyWiFi_t::begin()   {
  if (connected()) {
    setState(WiFi_STA_State_e::STA_Connected, WIFI_STATE_MACHINE_STA_CONNECTED_STABLE);
  } else {
    setState(WiFi_STA_State_e::STA_Init, 100);
  }
}

void ESPEasyWiFi_t::loop()
{
  // TODO TD-er: Must inspect WiFiEventData to see if we need to update some state here.
  auto wifi_STA_data = getWiFi_STA_NWPluginData_static_runtime();

  if (!wifi_STA_data) {
    // TODO TD-er: Must set to WiFi_STA_State_e::Disabled ???
    return;
  }

  const bool stateTimeoutReached = _state_timeout.isSet() && _state_timeout.timeReached();

  if (_state != WiFi_STA_State_e::TimeOut) {
    if (_callbackError || stateTimeoutReached)
    {
      // TODO TD-er: Must check what error was given???
      _callbackError = false;

      // TODO TD-er: Must perhaps check what action was pending and act on it?
      if ((_state == WiFi_STA_State_e::Disabled) && stateTimeoutReached) {
        setState(WiFi_STA_State_e::STA_Init, 100);
      } else if ((_state == WiFi_STA_State_e::STA_Connected) && stateTimeoutReached) {
        setState(WiFi_STA_State_e::STA_Connected_Stable);
      } else {
        setState(WiFi_STA_State_e::TimeOut, 100);
      }
      return;
    }
  }

  switch (_state)
  {
    case WiFi_STA_State_e::Disabled:
      // Do nothing here, as the device is disabled.
      break;
    case WiFi_STA_State_e::TimeOut:
      // We always go from TimeOut to Disabled.
      // Called this from the loop() to prevent recursive calls to setState
      setState(WiFi_STA_State_e::Disabled, 100);
      break;
    case WiFi_STA_State_e::STA_Init:
      if (getWiFi_STA_NWPluginData_static_runtime() == nullptr) {
        Scheduler.setNetworkInitTimer(0, NETWORK_INDEX_WIFI_STA);
      }

      if (getWiFi_STA_NWPluginData_static_runtime() != nullptr) {
        if (connected()) {
          setState(WiFi_STA_State_e::STA_Connected, WIFI_STATE_MACHINE_STA_CONNECTED_STABLE);
          break;
        }

        // This is where we decide what to do next:
        // - Reconnect
        // - Scan
        //
        // Do we have candidate to connect to ?
        if (WiFi_AP_Candidates.hasCandidates()) {
          setState(WiFi_STA_State_e::STA_Connecting, WIFI_STATE_MACHINE_STA_CONNECTING_TIMEOUT);
          break;
        }

        // No known candidates, so need to scan first.
        if (WifiIsAP(WiFi.getMode())) {
          // TODO TD-er: Must check if any client is connected.
          // If not, then we can disable AP mode and switch to WiFi_STA_State_e::STA_Scanning
          setState(WiFi_STA_State_e::STA_AP_Scanning, WIFI_STATE_MACHINE_STA_AP_SCANNING_TIMEOUT);
          break;
        }
        setState(WiFi_STA_State_e::STA_Scanning, WIFI_STATE_MACHINE_STA_SCANNING_TIMEOUT);
      }
      break;
    case WiFi_STA_State_e::STA_Scanning:
    case WiFi_STA_State_e::STA_AP_Scanning:
    {
      // -1 if scan not finished
      auto scanCompleteStatus = WiFi_AP_Candidates.scanComplete();

      // Check if scanning is finished
      // When scanning per channel, call for scanning next channel
      if (scanCompleteStatus >= 0) {
        WiFi_AP_Candidates.load_knownCredentials();
        WiFi_AP_Candidates.process_WiFiscan();
# ifndef BUILD_NO_DEBUG

        if (_state == WiFi_STA_State_e::STA_Scanning) {
          addLog(LOG_LEVEL_INFO, strformat(
                   F("WiFi : Scan done, found %d APs"),
                   scanCompleteStatus));
        } else {
          addLog(LOG_LEVEL_INFO, strformat(
                   F("WiFi : Scan channel %d done, found %d APs"),
                   _scan_channel,
                   scanCompleteStatus));
        }
# endif // ifndef BUILD_NO_DEBUG

        if (_state == WiFi_STA_State_e::STA_AP_Scanning) {
          ++_scan_channel;

          // TODO TD-er: What to do with 5 GHz WiFi?
          if (_scan_channel > 14) {
            _scan_channel = 0;

            // "Scan Success"
            setState(WiFi_STA_State_e::Disabled, 100);
          }
          else {
            setState(WiFi_STA_State_e::STA_AP_Scanning, 500);
          }
        } else {
          // "Scan Success"
          setState(WiFi_STA_State_e::Disabled, 100);
        }
      } else if (scanCompleteStatus == -2) { // WIFI_SCAN_FAILED
        addLog(LOG_LEVEL_ERROR, F("WiFi : Scan failed"));

        //        WiFi.scanDelete();
        setState(WiFi_STA_State_e::TimeOut, 1000);
      }
      break;
    }
    case WiFi_STA_State_e::STA_Connecting:
    case WiFi_STA_State_e::STA_Reconnecting:

      // Check if (re)connecting has finished
      if (getSTA_connected_state() == STA_connected_state::Connected) {
        setState(WiFi_STA_State_e::STA_Connected, WIFI_STATE_MACHINE_STA_CONNECTED_STABLE);
      }

      break;

    case WiFi_STA_State_e::STA_Connected:
    case WiFi_STA_State_e::STA_Connected_Stable:

      // Check if still connected
      if (getSTA_connected_state() != STA_connected_state::Connected) {
        // "Disconnect"

        auto wifi_STA_data = getWiFi_STA_NWPluginData_static_runtime();

        if (wifi_STA_data) {
          wifi_STA_data->mark_disconnected();
        }

        if (WiFi.status() == WL_CONNECTED) {
          WiFi.disconnect(true);
        }


        if (WiFi_AP_Candidates.hasCandidates()) {
          setState(WiFi_STA_State_e::STA_Reconnecting, WIFI_STATE_MACHINE_STA_CONNECTING_TIMEOUT);
        } else {
          setState(WiFi_STA_State_e::TimeOut, 100);
        }
      } else {
        // Else mark last timestamp seen as connected
        _last_seen_connected.setNow();

        # if FEATURE_SET_WIFI_TX_PWR
        SetWiFiTXpower();
        # endif
      }
      break;
  }


  {
    // Check if we need to start AP
    // Flag captive portal in webserver and/or whether we might be in setup mode
  }
}

bool ESPEasyWiFi_t::connected() const
{
  return getSTA_connected_state() == STA_connected_state::Connected;
}

void ESPEasyWiFi_t::disconnect() { doWiFiDisconnect(); }

void ESPEasyWiFi_t::setState(WiFi_STA_State_e newState, uint32_t timeout) {
  if (newState == _state) { return; }

  const WiFi_STA_State_e oldState = _state;

  // Need to set the newState first as some of the functions below will call
  // setState, causing a loop, or calling to change state multiple times.


  if (timeout == 0)
  {
    _state_timeout.clear();

  } else {
    _state_timeout.setMillisFromNow(timeout);
  }

  _last_state_change.setNow();
  _state = newState;

  // # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(
      LOG_LEVEL_INFO,
      concat(F("WiFi : Set state from: "), toString(oldState)) +
      concat(F(" to: "),                   toString(newState)) +
      concat(F(" timeout: "),              timeout));
  }

  // # endif // ifndef BUILD_NO_DEBUG


  if (oldState == WiFi_STA_State_e::STA_Connected)
  {}

  if ((oldState == WiFi_STA_State_e::STA_AP_Scanning) ||
      (oldState == WiFi_STA_State_e::STA_Scanning))
  {
    WiFi_AP_Candidates.load_knownCredentials();
    WiFi_AP_Candidates.process_WiFiscan();

  }

  switch (newState)
  {
    case WiFi_STA_State_e::Disabled:
      // TODO TD-er: Maybe call scheduler?
      //  if (WifiIsSTA(WiFi.getMode()))
      //    Scheduler.setNetworkExitTimer(0, NETWORK_INDEX_WIFI_STA);
      WifiDisconnect();
      setSTA(false);
      break;

    case WiFi_STA_State_e::TimeOut:
    {
      auto wifi_STA_data = getWiFi_STA_NWPluginData_static_runtime();

      // From a failed state we always turn off at least WiFi STA.
      // Only when we end up here from STA_Init, we also need to turn off AP.
      if (oldState == WiFi_STA_State_e::STA_Init) {
        // "Init STA Fail"

        // TODO TD-er: Maybe call scheduler?
        //  if (WifiIsAP(WiFi.getMode()))
        //    Scheduler.setNetworkExitTimer(0, NETWORK_INDEX_WIFI_AP);
        setAP(false);
      } else if (oldState == WiFi_STA_State_e::STA_Connecting) {
        // "Connect Fail"
        if (wifi_STA_data) {
          wifi_STA_data->mark_connect_failed();
        }

        //        setState(WiFi_STA_State_e::STA_Reconnecting, WIFI_STATE_MACHINE_STA_CONNECTING_TIMEOUT);
      } else if (oldState == WiFi_STA_State_e::STA_Reconnecting) {
        // "Reconnect Fail"
        if (wifi_STA_data) {
          wifi_STA_data->mark_connect_failed();
        }
      } else if (oldState == WiFi_STA_State_e::STA_Init) {
        // "Init STA Fail"
      } else if ((oldState == WiFi_STA_State_e::STA_Scanning) ||
                 (oldState == WiFi_STA_State_e::STA_AP_Scanning)) {
        // "Scan Fail"
      }

      // TODO TD-er: Handle "Scan Fail" and "Init STA Fail"
      // For "Scan Fail" set state to Disabled, with some timeout?
      // Do we also need to turn off WiFi STA after successful scan?
      break;
    }
    case WiFi_STA_State_e::STA_Init:
/*
      if (getWiFi_STA_NWPluginData_static_runtime() == nullptr) {
        Scheduler.setNetworkInitTimer(0, NETWORK_INDEX_WIFI_STA);
      }
      _state_timeout.setMillisFromNow(Settings.getNetworkInterfaceStartupDelay(NETWORK_INDEX_WIFI_STA) + 100);
*/
      break;
    case WiFi_STA_State_e::STA_AP_Scanning:

      // Start scanning per channel
      if (_scan_channel == 0) { _scan_channel = 1; }

    // fall through
    case WiFi_STA_State_e::STA_Scanning:
      // Start scanning
      startScanning();
      break;
    case WiFi_STA_State_e::STA_Connecting:
    case WiFi_STA_State_e::STA_Reconnecting:

      // Start connecting
      ++_connect_attempt;

      if (!connectSTA()) {
        // TODO TD-er: Must keep track of failed attempts and start AP when either no credentials present or nr. of attempts failed > some
        // threshold.
        addLog(LOG_LEVEL_ERROR, F("WiFi : Connect STA failed"));
        setState(WiFi_STA_State_e::TimeOut, 100);
      }
      break;

    case WiFi_STA_State_e::STA_Connected:
    {
# ifdef ESP32

      // FIXME TD-er: Must move to ESP32-specific cpp file
      // WiFi.STA.setDefault();
# endif // ifdef ESP32

      _connect_attempt = 0;
      _last_seen_connected.setNow();
      _state_timeout.setMillisFromNow(WIFI_STATE_MACHINE_STA_CONNECTED_STABLE);
      auto wifi_STA_data = getWiFi_STA_NWPluginData_static_runtime();

      if (wifi_STA_data) {
        wifi_STA_data->mark_connected();
      }

      /*
         if (Settings.UseRules)
         {
         eventQueue.addDeDup(F("WiFi#Connected"));
         }
         statusLED(true);
       */
      break;
    }
    case WiFi_STA_State_e::STA_Connected_Stable:
      _state_timeout.clear();

      // TODO TD-er: Must mark connection as stable
      break;
  }

  auto wifi_STA_data = getWiFi_STA_NWPluginData_static_runtime();

  if (wifi_STA_data) {
    wifi_STA_data->processEvents();
  }
}

void ESPEasyWiFi_t::checkConnectProgress() {}

void ESPEasyWiFi_t::startScanning()
{
  _state = _scan_channel == 0 ? WiFi_STA_State_e::STA_Scanning : WiFi_STA_State_e::STA_AP_Scanning;
  setSTA(true);
  WifiScan(true, _scan_channel);
  _last_state_change.setNow();
}

bool ESPEasyWiFi_t::connectSTA()
{
  auto wifi_STA_data = getWiFi_STA_NWPluginData_static_runtime();

  if (!wifi_STA_data) { return false; }

  // Make sure the timer is set to off.
  // TODO TD-er: Should we check to see if it is still on and then do what????
  wifi_STA_data->_establishConnectStats.setOff();

  if (!WiFi_AP_Candidates.hasCandidateCredentials())
  {
    /*
        if (!WiFiEventData.warnedNoValidWiFiSettings)
        {
          addLog(LOG_LEVEL_ERROR, F("WIFI : No valid wifi settings"));
          WiFiEventData.warnedNoValidWiFiSettings = true;
        }
        wifi_STA_data->_establishConnectStats.clear();

        //    WiFiEventData.last_wifi_connect_attempt_moment.clear();
        //    _connect_attempt     = 1;
        WiFiEventData.wifiConnectAttemptNeeded = false;
     */

    // No need to wait longer to start AP mode.
    if (!Settings.DoNotStartAPfallback_ConnectFail())
    {
      // Scheduler.setNetworkInitTimer(0, NETWORK_INDEX_WIFI_AP);
    }
    return false;
  }

  /*
     if (WiFiEventData.lastDisconnectReason != WIFI_DISCONNECT_REASON_UNSPECIFIED) {
   # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO, concat(
               F("WiFi : Disconnect reason: "),
               getWiFi_disconnectReason_str()));
   # endif // ifndef BUILD_NO_DEBUG
      WiFiEventData.processedDisconnect = true;
     }

     WiFiEventData.warnedNoValidWiFiSettings = false;
   */
  WiFi_pre_STA_setup();
# if defined(ESP8266)
  WiFi.hostname(NetworkCreateRFCCompliantHostname().c_str());

# endif // if defined(ESP8266)
# if defined(ESP32)

  //  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
# endif // if defined(ESP32)
  doSetConnectionSpeed();

  // Start the process of connecting or starting AP
  if (!WiFi_AP_Candidates.getNext(true))
  {
    return false;
  }

  const WiFi_AP_Candidate candidate = WiFi_AP_Candidates.getCurrent();
# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, strformat(
                 F("WIFI : Connecting %s attempt #%u"),
                 candidate.toString().c_str(),
                 wifi_STA_data->_establishConnectStats.getCycleCount() + 1));
  }
# endif // ifndef BUILD_NO_DEBUG

  // WiFiEventData.markWiFiBegin();

  if (prepareWiFi()) {
    setNetworkMedium(NetworkMedium_t::WIFI);
    RTC.clearLastWiFi();
    RTC.lastWiFiSettingsIndex = candidate.index;

# if FEATURE_SET_WIFI_TX_PWR
    float tx_pwr = 0; // Will be set higher based on RSSI when needed.
    // FIXME TD-er: Must check wifi_STA_data->_establishConnectStats.getCycleCount() to increase TX power

    if (Settings.UseMaxTXpowerForSending()) {
      tx_pwr = Settings.getWiFi_TX_power();
    }
    SetWiFiTXpower(tx_pwr, candidate.rssi);
# endif // if FEATURE_SET_WIFI_TX_PWR

    // Start connect attempt now, so no longer needed to attempt new connection.
    //    WiFiEventData.wifiConnectAttemptNeeded = false;

    //    WiFiEventData.wifiConnectInProgress    = true;
    const String key = WiFi_AP_CandidatesList::get_key(candidate.index);

# if FEATURE_USE_IPV6

    if (Settings.EnableIPv6()) {
      WiFi.enableIPv6(true);
    }
# endif // if FEATURE_USE_IPV6

# ifdef ESP32

    if (Settings.IncludeHiddenSSID()) {
      doSetWiFiCountryPolicyManual();
    }
# endif // ifdef ESP32
    wifi_STA_data->mark_begin_establish_connection();

    if (candidate.bits.isHidden /*&& Settings.HiddenSSID_SlowConnectPerBSSID()*/) {
      //      WiFi.disconnect(false, true);
# ifdef ESP32
      WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
      WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
# endif // ifdef ESP32
      delay(100);
      WiFi.begin(candidate.ssid.c_str(), key.c_str(), candidate.channel, candidate.bssid.mac);

      // If the ssid returned from the scan is empty, it is a hidden SSID
      // it appears that the WiFi.begin() function is asynchronous and takes
      // additional time to connect to a hidden SSID. Therefore a delay of 1000ms
      // is added for hidden SSIDs before calling WiFi.status()
      delay(1000);

      //      WiFi.waitForConnectResult(6000);
    } else {
      if (candidate.allowQuickConnect()) {
# ifdef ESP32
        WiFi.setScanMethod(WIFI_FAST_SCAN);
# endif
        WiFi.begin(candidate.ssid.c_str(), key.c_str(), candidate.channel, candidate.bssid.mac);
      } else {
# ifdef ESP32
        WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
        WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
# endif // ifdef ESP32
        WiFi.begin(candidate.ssid.c_str(), key.c_str());
      }
    }

    // Always wait for a second
    WiFi.waitForConnectResult(1000); // https://github.com/arendst/Tasmota/issues/14985

  }

  return true;
}

bool ESPEasyWiFi_t::shouldStartAP_fallback() const
{
  if (!Settings.getNetworkInterface_isFallback(NETWORK_INDEX_WIFI_AP)) { return false; }


  if (Settings.APfallback_autostart_max_uptime_m() != 0) {
    if ((Settings.APfallback_autostart_max_uptime_m() * 60000) < millis()) {
      return false;
    }
  }

  if (Settings.StartAPfallback_NoCredentials() && !SecuritySettings.hasWiFiCredentials()) {
    return true;
  }

  if (Settings.DoNotStartAPfallback_ConnectFail()) {
    return false;
  }

  return (Settings.ConnectFailRetryCount > 0) &&
         (_connect_attempt > Settings.ConnectFailRetryCount);
}

bool ESPEasyWiFi_t::shouldRedirectTo_setup() const
{
  if (!Settings.ApCaptivePortal()) { return false; }

  if (Settings.StartAPfallback_NoCredentials()
      && !WiFi_AP_Candidates.hasCandidateCredentials()

      //  && !SecuritySettings.hasWiFiCredentials()
      ) {
    return true;
  }

  return !Settings.DoNotStartAPfallback_ConnectFail();
}

} // namespace wifi
} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_WIFI
