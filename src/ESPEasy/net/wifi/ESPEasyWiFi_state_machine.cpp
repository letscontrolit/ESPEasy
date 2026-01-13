#include "../wifi/ESPEasyWiFi_state_machine.h"

#if FEATURE_WIFI

# include "../../../src/ESPEasyCore/ESPEasy_Log.h"
# include "../../../src/Globals/EventQueue.h"
# include "../../../src/Globals/RTC.h"
# include "../../../src/Globals/SecuritySettings.h"
# include "../../../src/Globals/Settings.h"
# include "../../../src/Helpers/NetworkStatusLED.h"
# include "../../../src/Helpers/StringConverter.h"
# include "../../../src/Helpers/StringGenerator_WiFi.h"


# include "../ESPEasyNetwork.h" // for setNetworkMedium, however this should not be part of the WiFi code
# include "../Globals/ESPEasyWiFiEvent.h"
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

void ESPEasyWiFi_t::setup() {
  if (!Settings.getNetworkEnabled(NETWORK_INDEX_WIFI_STA)) return;

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

void ESPEasyWiFi_t::disable() { setState(WiFiState_e::Disabled, 100); }

void ESPEasyWiFi_t::begin()   {
  if (WiFi_AP_Candidates.hasCandidates()) {
    setState(WiFiState_e::IdleWaiting, 100);
  } else {
//    if (!Settings.DoNotStartAPfallback_ConnectFail()) {
//      setState(WiFiState_e::AP_only, WIFI_STATE_MACHINE_AP_ONLY_TIMEOUT);
//    } else 
    {
      if (WifiIsAP(WiFi.getMode())) {
        // TODO TD-er: Must check if any client is connected.
        // If not, then we can disable AP mode and switch to WiFiState_e::STA_Scanning
        setState(WiFiState_e::STA_AP_Scanning, WIFI_STATE_MACHINE_STA_AP_SCANNING_TIMEOUT);
      } else {
        //            setState(WiFiState_e::STA_AP_Scanning, WIFI_STATE_MACHINE_STA_AP_SCANNING_TIMEOUT);
        setState(WiFiState_e::STA_Scanning, WIFI_STATE_MACHINE_STA_SCANNING_TIMEOUT);
      }
    }
  }
}

void ESPEasyWiFi_t::loop()
{
  // TODO TD-er: Must inspect WiFiEventData to see if we need to update some state here.
  auto wifi_STA_data = getWiFi_STA_NWPluginData_static_runtime();

  if (!wifi_STA_data) { return; }

  if (_state != WiFiState_e::IdleWaiting) {
    if (_callbackError ||
        (_state_timeout.isSet() && _state_timeout.timeReached()))
    {
      // TODO TD-er: Must check what error was given???
      _callbackError = false;
      setState(WiFiState_e::WiFiOFF);
    }
  }

  switch (_state)
  {
    case WiFiState_e::Disabled:
      // Do nothing here, as the device is disabled.
      break;
    case WiFiState_e::WiFiOFF:
      begin();

      //      setState(WiFiState_e::IdleWaiting, 100);
      break;
    case WiFiState_e::AP_only:
    case WiFiState_e::AP_Fallback:

      if (WiFi_AP_Candidates.hasCandidates() ||
          (_state_timeout.timeReached() &&
           !ESPEasy::net::wifi::wifiAPmodeActivelyUsed())) {
        setState(WiFiState_e::IdleWaiting, 100);
      }
      break;
    case WiFiState_e::IdleWaiting:

      if (connected()) {
        setState(WiFiState_e::STA_Connected, 100);
        break;
      }

      if (_state_timeout.timeReached() || (getSTA_connected_state() == STA_connected_state::Idle)) {
        // This is where we decide what to do next:
        // - Reconnect
        // - Scan
        //


        // Do we have candidate to connect to ?
        if (WiFi_AP_Candidates.hasCandidates()) {
          setState(WiFiState_e::STA_Connecting, WIFI_STATE_MACHINE_STA_CONNECTING_TIMEOUT);
        } else if ((WiFi_AP_Candidates.scanComplete() == 0)
                   || (WiFi_AP_Candidates.scanComplete() == -3)) {
          if (WifiIsAP(WiFi.getMode())) {
            // TODO TD-er: Must check if any client is connected.
            // If not, then we can disable AP mode and switch to WiFiState_e::STA_Scanning
            setState(WiFiState_e::STA_AP_Scanning, WIFI_STATE_MACHINE_STA_AP_SCANNING_TIMEOUT);
          } else {
            //            setState(WiFiState_e::STA_AP_Scanning, WIFI_STATE_MACHINE_STA_AP_SCANNING_TIMEOUT);
            setState(WiFiState_e::STA_Scanning, WIFI_STATE_MACHINE_STA_SCANNING_TIMEOUT);
          }

          // Move up?
        } else if (!WiFi_AP_Candidates.hasCandidateCredentials() ||
                   !Settings.DoNotStartAPfallback_ConnectFail()) {
          if (!WiFi_AP_Candidates.hasCandidateCredentials()

              //  && !WiFiEventData.warnedNoValidWiFiSettings
              )
          {
            addLog(LOG_LEVEL_ERROR, F("WIFI : No valid wifi settings"));

            //            WiFiEventData.warnedNoValidWiFiSettings = true;
          }

          wifi_STA_data->_establishConnectStats.clear();

          //          WiFiEventData.wifiConnectAttemptNeeded = false;
          // end move up??
        }
      }
      break;
    case WiFiState_e::STA_Scanning:
    {
      // -1 if scan not finished
      auto scanCompleteStatus = WiFi_AP_Candidates.scanComplete();

      if (scanCompleteStatus >= 0) {
        WiFi_AP_Candidates.load_knownCredentials();
        WiFi_AP_Candidates.process_WiFiscan();
# ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_INFO, strformat(
                 F("WiFi : Scan done, found %d APs"),
                 WiFi_AP_Candidates.scanComplete()));
# endif // ifndef BUILD_NO_DEBUG
      } else if (scanCompleteStatus == -2) { // WIFI_SCAN_FAILED
        addLog(LOG_LEVEL_ERROR, F("WiFi : Scan failed"));

        //        WiFi.scanDelete();
        setState(WiFiState_e::WiFiOFF, 1000);
      }

      if (_state_timeout.timeReached() || (scanCompleteStatus >= 0)) {
        //        WiFi.scanDelete();

        if (_state_timeout.timeReached()) {
# ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_ERROR, F("WiFi : Scan Running Timeout"));
# endif
        }

        if (WiFi_AP_Candidates.hasCandidates()) {
          setState(WiFiState_e::WiFiOFF, 100);
        } else {
          if (shouldStartAP_fallback()) {
            setState(WiFiState_e::AP_Fallback, Settings.APfallback_minimal_on_time_sec() * 1000);
            // TODO TD-er: Must keep track of whether the user has forced AP to be autostarted.
          } else {
            setState(WiFiState_e::WiFiOFF, 1000);
          }
        }
      }
      break;
    }
    case WiFiState_e::STA_AP_Scanning:
    {
      // -1 if scan not finished
      auto scanCompleteStatus = WiFi_AP_Candidates.scanComplete();

      if (scanCompleteStatus >= 0) {
        WiFi_AP_Candidates.load_knownCredentials();
        WiFi_AP_Candidates.process_WiFiscan();
# ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_INFO, strformat(
                 F("WiFi : Scan channel %d done, found %d APs"),
                 _scan_channel,
                 WiFi_AP_Candidates.scanComplete()));
# endif // ifndef BUILD_NO_DEBUG
      } else if (scanCompleteStatus == -2) { // WIFI_SCAN_FAILED
        addLog(LOG_LEVEL_ERROR, F("WiFi : Scan failed"));

        //        WiFi.scanDelete();
        setState(WiFiState_e::WiFiOFF, 1000);
      }

      if (_state_timeout.timeReached() || (scanCompleteStatus >= 0)) {
        //        WiFi.scanDelete();

        if (_state_timeout.timeReached()) {
# ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_ERROR, F("WiFi : Scan Running Timeout"));
# endif
        }
        ++_scan_channel;

        if (_scan_channel > 14) {
          _scan_channel = 0;

          if (!WiFi_AP_Candidates.hasCandidateCredentials() &&
              !Settings.DoNotStartAPfallback_ConnectFail()) {
            setState(WiFiState_e::AP_only, WIFI_STATE_MACHINE_AP_ONLY_TIMEOUT);
          } else {
            setState(WiFiState_e::WiFiOFF, 100);
          }
        }
        else {
          setState(WiFiState_e::STA_AP_Scanning, 500);
        }
      }
      break;

      // Check if scanning is finished
      // When scanning per channel, call for scanning next channel
    }
    case WiFiState_e::STA_Connecting:
    case WiFiState_e::STA_Reconnecting:

      // Check if (re)connecting has finished
    {
      const STA_connected_state sta_connected_state = getSTA_connected_state();

      if (sta_connected_state == STA_connected_state::Connected) {
        setState(WiFiState_e::STA_Connected);
      } else if (_state_timeout.timeReached()) {
        if (_state == WiFiState_e::STA_Connecting) {
          setState(WiFiState_e::STA_Reconnecting, WIFI_STATE_MACHINE_STA_CONNECTING_TIMEOUT);
        } else {
          setState(WiFiState_e::WiFiOFF);
        }
      }

      break;
    }
    case WiFiState_e::STA_Connected:

      // Check if still connected
      if (getSTA_connected_state() != STA_connected_state::Connected) {
        //        setState(WiFiState_e::WiFiOFF);
        if (WiFi_AP_Candidates.hasCandidates()) {
          setState(WiFiState_e::STA_Connecting, WIFI_STATE_MACHINE_STA_CONNECTING_TIMEOUT);
        } else {
          setState(WiFiState_e::STA_Scanning, WIFI_STATE_MACHINE_STA_SCANNING_TIMEOUT);
        }

        /*
           if (Settings.UseRules)
           {
           eventQueue.add(F("WiFi#Disconnected"));
           }
           statusLED(false);
         */

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

void ESPEasyWiFi_t::setState(WiFiState_e newState, uint32_t timeout) {
  if (newState == _state) { return; }
# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(
      LOG_LEVEL_INFO,
      concat(F("WiFi : Set state from: "), toString(_state)) +
      concat(F(" to: "),                   toString(newState)) +
      concat(F(" timeout: "),              timeout));
  }
# endif // ifndef BUILD_NO_DEBUG

  if (_state == WiFiState_e::AP_only ||
      _state == WiFiState_e::AP_Fallback) {
    setAPinternal(false);
    setAP(false);
  }

  if (_state == WiFiState_e::STA_Connected)
  {
    auto wifi_STA_data = getWiFi_STA_NWPluginData_static_runtime();

    if (wifi_STA_data) {
      wifi_STA_data->mark_disconnected();

      if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect();
      }
    }
  }

  if ((_state == WiFiState_e::STA_AP_Scanning) ||
      (_state == WiFiState_e::STA_Scanning))
  {
    WiFi_AP_Candidates.process_WiFiscan();
  }

  if (timeout == 0)
  {
    _state_timeout.clear();

  } else {
    _state_timeout.setMillisFromNow(timeout);
  }

  _last_state_change.setNow();
  _state = newState;


  switch (newState)
  {
    case WiFiState_e::Disabled:
      // Do nothing here, as the device is disabled.
      setSTA(false);
      break;
    case WiFiState_e::WiFiOFF:
      // TODO TD-er: Must cancel all and turn off WiFi.
      setSTA_AP(false, false);
      break;
    case WiFiState_e::AP_only:
    case WiFiState_e::AP_Fallback:
      setAPinternal(true);
      break;
    case WiFiState_e::IdleWaiting:
      // Do nothing here as we're waiting till the timeout is over
      break;
    case WiFiState_e::STA_AP_Scanning:

      // Start scanning per channel
      if (_scan_channel == 0) { _scan_channel = 1; }

    //      break;
    case WiFiState_e::STA_Scanning:
      // Start scanning
      startScanning();
      break;
    case WiFiState_e::STA_Connecting:
    case WiFiState_e::STA_Reconnecting:

      // Start connecting
      ++_connect_attempt;
      if (!connectSTA()) {
        // TODO TD-er: Must keep track of failed attempts and start AP when either no credentials present or nr. of attempts failed > some
        // threshold.
        if (!WiFi_AP_Candidates.hasCandidates()) {
          setState(WiFiState_e::STA_Scanning, WIFI_STATE_MACHINE_STA_CONNECTING_TIMEOUT);
        } else {
          setState(WiFiState_e::IdleWaiting, 100);
        }
      }
      break;
    case WiFiState_e::STA_Connected:
    {
# ifdef ESP32

      // FIXME TD-er: Must move to ESP32-specific cpp file
      // WiFi.STA.setDefault();
# endif // ifdef ESP32
      _connect_attempt = 0;
      _last_seen_connected.setNow();
      _state_timeout.clear();
      auto wifi_STA_data = getWiFi_STA_NWPluginData_static_runtime();

      if (wifi_STA_data) {
        wifi_STA_data->mark_connected();
      }


      /*
         if (Settings.UseRules)
         {
         eventQueue.add(F("WiFi#Connected"));
         }
         statusLED(true);
       */
      break;

    }
  }
}

void ESPEasyWiFi_t::checkConnectProgress() {}

void ESPEasyWiFi_t::startScanning()
{
  _state = _scan_channel == 0 ? WiFiState_e::STA_Scanning : WiFiState_e::STA_AP_Scanning;
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
      //      setAPinternal(true);
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
  wifi_station_set_hostname(NetworkCreateRFCCompliantHostname().c_str());

# endif // if defined(ESP8266)
# if defined(ESP32)

  //  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
# endif // if defined(ESP32)
  doSetConnectionSpeed();
  setupStaticIPconfig();

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
  if ((Settings.APfallback_autostart_max_uptime_m() * 1000) > millis()) {
    return false;
  }

  if (Settings.StartAPfallback_NoCredentials() && !SecuritySettings.hasWiFiCredentials()) {
    return true;
  }

  if (Settings.DoNotStartAPfallback_ConnectFail()) {
    return false;
  }

  return _connect_attempt > Settings.ConnectFailRetryCount;
}



} // namespace wifi
} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_WIFI
