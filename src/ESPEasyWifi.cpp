#include "ESPEasyWifi.h"

#include "ESPEasy-Globals.h"
#include "ESPEasyNetwork.h"
#include "ESPEasyWiFi_credentials.h"
#include "ESPEasyWifi_ProcessEvent.h"
#include "src/DataStructs/TimingStats.h"
#include "src/Globals/ESPEasyWiFiEvent.h"
#include "src/Globals/EventQueue.h"
#include "src/Globals/NetworkState.h"
#include "src/Globals/RTC.h"
#include "src/Globals/SecuritySettings.h"
#include "src/Helpers/ESPEasy_time_calc.h"
#include "src/Helpers/StringConverter.h"

// ********************************************************************************
// WiFi state
// ********************************************************************************

/*
   WiFi STA states:
   1 STA off                 => ESPEASY_WIFI_DISCONNECTED
   2 STA connecting
   3 STA connected           => ESPEASY_WIFI_CONNECTED
   4 STA got IP              => ESPEASY_WIFI_GOT_IP
   5 STA connected && got IP => ESPEASY_WIFI_SERVICES_INITIALIZED

   N.B. the states are flags, meaning both "connected" and "got IP" must be set
        to be considered ESPEASY_WIFI_SERVICES_INITIALIZED

   The flag wifiConnectAttemptNeeded indicates whether a new connect attempt is needed.
   This is set to true when:
   - Security settings have been saved with AP mode enabled. FIXME TD-er, this may not be the best check.
   - WiFi connect timeout reached  &  No client is connected to the AP mode of the node.
   - Wifi is reset
   - WiFi setup page has been loaded with SSID/pass values.


   WiFi AP mode states:
   1 AP on                        => reset AP disable timer
   2 AP client connect/disconnect => reset AP disable timer
   3 AP off                       => AP disable timer = 0;

   AP mode will be disabled when both apply:
   - AP disable timer (timerAPoff) expired
   - No client is connected to the AP.

   AP mode will be enabled when at least one applies:
   - No valid WiFi settings
   - Start AP timer (timerAPstart) expired

   Start AP timer is set or cleared at:
   - Set timerAPstart when "valid WiFi connection" state is observed.
   - Disable timerAPstart when ESPEASY_WIFI_SERVICES_INITIALIZED wifi state is reached.

   For the first attempt to connect after a cold boot (RTC values are 0), a WiFi scan will be 
   performed to find the strongest known SSID.
   This will set RTC.lastBSSID and RTC.lastWiFiChannel
   
   Quick reconnect (using BSSID/channel of last connection) when both apply:
   - If wifi_connect_attempt < 3
   - RTC.lastBSSID is known
   - RTC.lastWiFiChannel != 0

   Change of wifi settings when both apply:
   - "other" settings valid
   - (wifi_connect_attempt % 2) == 0

   Reset of wifi_connect_attempt to 0 when both apply:
   - connection successful
   - Connection stable (connected for > 5 minutes)

 */


// ********************************************************************************
// Check WiFi connected status
// This is basically the state machine to switch between states:
// - Initiate WiFi reconnect
// - Start/stop of AP mode
// ********************************************************************************
bool WiFiConnected() {
  START_TIMER;

  if (unprocessedWifiEvents()) { return false; }

  bool wifi_isconnected = false;
  #ifdef ESP8266
  // Perform check on SDK function, see: https://github.com/esp8266/Arduino/issues/7432
  station_status_t status = wifi_station_get_connect_status();
  switch(status) {
    case STATION_GOT_IP:
      wifi_isconnected = true;
      break;
    case STATION_NO_AP_FOUND:
    case STATION_CONNECT_FAIL:
    case STATION_WRONG_PASSWORD:
      wifi_isconnected = false;
      break;
    case STATION_IDLE:
    case STATION_CONNECTING:
      wifi_isconnected = bitRead(wifiStatus, ESPEASY_WIFI_SERVICES_INITIALIZED);
      break;

    default:
      wifi_isconnected = false;
      break;
  }


  #endif
  #ifdef ESP32
  if (WiFi.isConnected()) {
    wifi_isconnected = true;
  }

  #endif


  // For ESP82xx, do not rely on WiFi.status() with event based wifi.
  bool validWiFi = (WiFi.RSSI() < 0) && wifi_isconnected && hasIPaddr();
  if (validWiFi != bitRead(wifiStatus, ESPEASY_WIFI_SERVICES_INITIALIZED)) {
    // else wifiStatus is no longer in sync.
    if (checkAndResetWiFi()) {
      // Wifi has been reset, so no longer valid WiFi
      validWiFi = false;
    }
  }

  if (validWiFi) {
    // Connected, thus disable any timer to start AP mode. (except when in WiFi setup mode)
    if (!wifiSetupConnect) {
      timerAPstart.clear();
    }
    STOP_TIMER(WIFI_ISCONNECTED_STATS);
    // Only return true after some time since it got connected.
    return wifi_considered_stable || lastConnectMoment.timeoutReached(100);
  }

  if ((timerAPstart.isSet()) && timerAPstart.timeoutReached(WIFI_RECONNECT_WAIT)) {
    // Timer reached, so enable AP mode.
    if (!WifiIsAP(WiFi.getMode())) {
      setAP(true);
    }
    timerAPstart.clear();
  }


  // When made this far in the code, we apparently do not have valid WiFi connection.
  if (!timerAPstart.isSet() && !WifiIsAP(WiFi.getMode())) {
    // First run we do not have WiFi connection any more, set timer to start AP mode
    // Only allow the automatic AP mode in the first N minutes after boot.
    if ((wdcounter / 2) < WIFI_ALLOW_AP_AFTERBOOT_PERIOD) {
      timerAPstart.setNow();
      // Fixme TD-er: Make this more elegant as it now needs to know about the extra time needed for the AP start timer.
      timerAPoff.set(timerAPstart.get() + (WIFI_RECONNECT_WAIT * 1000ll));
    }
  }

  if (wifiConnectTimeoutReached() && !wifiSetup) {
    // It took too long to make a connection, set flag we need to try again
    if (!wifiAPmodeActivelyUsed()) {
      wifiConnectAttemptNeeded = true;
    }
    wifiConnectInProgress = false;
  }
  delay(1);
  STOP_TIMER(WIFI_NOTCONNECTED_STATS);
  return false;
}

void WiFiConnectRelaxed() {
  if (!wifiConnectAttemptNeeded || wifiConnectInProgress) {
    return; // already connected or connect attempt in progress need to disconnect first
  }
  if (!processedScanDone) {
    // Scan is still active, so do not yet connect.
    return;
  }

  // Start connect attempt now, so no longer needed to attempt new connection.
  wifiConnectAttemptNeeded = false;

  if (!prepareWiFi()) {
    addLog(LOG_LEVEL_ERROR, F("WIFI : Could not prepare WiFi!"));
    last_wifi_connect_attempt_moment.clear();
    wifi_connect_attempt             = 1;
    return;
  }

  if (wifiSetupConnect) {
    // wifiSetupConnect is when run from the setup page.
    RTC.lastWiFiSettingsIndex     = 0; // Force to load the first settings.
    RTC.lastWiFiChannel = 0; // Force slow connect
    wifi_connect_attempt = 0;
    wifiSetupConnect     = false;
  }

  // Switch between WiFi credentials
  if ((wifi_connect_attempt != 0) && ((wifi_connect_attempt % 2) == 0)) {
    if (selectNextWiFiSettings()) {
      // Switch WiFi settings, so the last known BSSID cannot be used for a quick reconnect.
      RTC.lastBSSID[0] = 0;
    }
  }
  const char *ssid       = getLastWiFiSettingsSSID();
  const char *passphrase = getLastWiFiSettingsPassphrase();

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("WIFI : Connecting ");
    log += ssid;
    log += F(" attempt #");
    log += wifi_connect_attempt + 1;
    addLog(LOG_LEVEL_INFO, log);
  }
  lastDisconnectMoment.clear();
  last_wifi_connect_attempt_moment.setNow();
  wifiConnectInProgress            = true;

  // First try quick reconnect using last known BSSID and channel.
  bool useQuickConnect = RTC.lastBSSID[0] != 0 && RTC.lastWiFiChannel != 0 && wifi_connect_attempt < 3;

  if (useQuickConnect) {
    WiFi.begin(ssid, passphrase, RTC.lastWiFiChannel, &RTC.lastBSSID[0]);
  } else {
    WiFi.begin(ssid, passphrase);
  }
  ++wifi_connect_attempt;
  logConnectionStatus();
}

// ********************************************************************************
// Set Wifi config
// ********************************************************************************
bool prepareWiFi() {
  if (!selectValidWiFiSettings()) {
    addLog(LOG_LEVEL_ERROR, F("WIFI : No valid wifi settings"));

    // No need to wait longer to start AP mode.
    setAP(true);
    return false;
  }
  setSTA(true);
  char hostname[40];
  safe_strncpy(hostname, NetworkCreateRFCCompliantHostname().c_str(), sizeof(hostname));
  #if defined(ESP8266)
  wifi_station_set_hostname(hostname);

  if (Settings.WifiNoneSleep()) {
    // Only set this mode during setup.
    // Reset to default power mode requires a reboot since setting it to WIFI_LIGHT_SLEEP will cause a crash.
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
  }

  #endif // if defined(ESP8266)
  #if defined(ESP32)
  WiFi.setHostname(hostname);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  #endif // if defined(ESP32)

  if (RTC.lastWiFiChannel == 0 && wifi_connect_attempt <= 1) {
    WifiScan(false, true);
  }
  setConnectionSpeed();
  setupStaticIPconfig();
  return true;
}

bool checkAndResetWiFi() {
  #ifdef ESP8266
  station_status_t status = wifi_station_get_connect_status();

  switch(status) {
    case STATION_GOT_IP:
      // This is a valid status, no need to reset
      return false;
    case STATION_NO_AP_FOUND:
    case STATION_CONNECT_FAIL:
    case STATION_WRONG_PASSWORD:
      // Reason to reset WiFi
      break;
    case STATION_IDLE:
    case STATION_CONNECTING:
      if (!last_wifi_connect_attempt_moment.timeoutReached(15000)) {
        return false;
      }
      break;
  }
  String log = F("WIFI  : WiFiConnected() out of sync: ");
  log += ESPeasyWifiStatusToString();
  log += F(" RSSI: ");
  log += String(WiFi.RSSI());
  log += F(" status: ");
  log += SDKwifiStatusToString(status);
  // Call for reset first, to make sure a syslog call will not try to send.
  resetWiFi();
  addLog(LOG_LEVEL_INFO, log);

  #endif
  #ifdef ESP32
  if (WiFi.isConnected()) {
    return false;
  } else {
    if (!last_wifi_connect_attempt_moment.timeoutReached(15000)) {
      return false;
    }
    String log = F("WIFI  : WiFiConnected() out of sync: ");
    log += ESPeasyWifiStatusToString();
    log += F(" RSSI: ");
    log += String(WiFi.RSSI());
    // Call for reset first, to make sure a syslog call will not try to send.
    resetWiFi();
    addLog(LOG_LEVEL_INFO, log);
  }
  #endif
  return true;
}


void resetWiFi() {
  if (lastWiFiResetMoment.isSet() && !lastWiFiResetMoment.timeoutReached(1000)) {
    // Don't reset WiFi too often
    return;
  }
  lastDisconnectMoment.setNow();
  lastWiFiResetMoment.setNow();
  wifiStatus            = ESPEASY_WIFI_DISCONNECTED;
  lastConnectMoment.clear();
  wifi_considered_stable = false;
  lastGetIPmoment.clear();
  lastGetScanMoment.clear();
  last_wifi_connect_attempt_moment.clear();
  timerAPstart.clear();

  // Mark all flags to default to prevent handling old events.
  processedConnect          = true;
  processedDisconnect       = true;
  processedGotIP            = true;
  processedDHCPTimeout      = true;
  processedConnectAPmode    = true;
  processedDisconnectAPmode = true;
  processedScanDone         = true;
  wifiConnectAttemptNeeded  = true;
  WifiDisconnect();

  // Send this log only after WifiDisconnect() or else sending to syslog may cause issues
  addLog(LOG_LEVEL_INFO, String(F("Reset WiFi.")));

  //  setWifiMode(WIFI_OFF);

  initWiFi();
}

void initWiFi()
{
#ifdef ESP8266

  // See https://github.com/esp8266/Arduino/issues/5527#issuecomment-460537616
  // FIXME TD-er: Do not destruct WiFi object, it may cause crashes with queued UDP traffic.
  WiFi.~ESP8266WiFiClass();
  WiFi = ESP8266WiFiClass();
#endif // ifdef ESP8266

  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  WiFi.setAutoReconnect(true);
  // The WiFi.disconnect() ensures that the WiFi is working correctly. If this is not done before receiving WiFi connections,
  // those WiFi connections will take a long time to make or sometimes will not work at all.
  WiFi.disconnect(true);
  setWifiMode(WIFI_OFF);

#if defined(ESP32)
  wm_event_id = WiFi.onEvent(WiFiEvent);
#endif
#ifdef ESP8266
  // WiFi event handlers
  stationConnectedHandler = WiFi.onStationModeConnected(onConnected);
	stationDisconnectedHandler = WiFi.onStationModeDisconnected(onDisconnect);
	stationGotIpHandler = WiFi.onStationModeGotIP(onGotIP);
  stationModeDHCPTimeoutHandler = WiFi.onStationModeDHCPTimeout(onDHCPTimeout);
  APModeStationConnectedHandler = WiFi.onSoftAPModeStationConnected(onConnectedAPmode);
  APModeStationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(onDisconnectedAPmode);
#endif
}

// ********************************************************************************
// Disconnect from Wifi AP
// ********************************************************************************
void WifiDisconnect()
{
  #if defined(ESP32)
  WiFi.disconnect();
  WiFi.removeEvent(wm_event_id);
  #else // if defined(ESP32)
  ETS_UART_INTR_DISABLE();
  wifi_station_disconnect();
  ETS_UART_INTR_ENABLE();
  #endif // if defined(ESP32)
  wifiStatus          = ESPEASY_WIFI_DISCONNECTED;
  processedDisconnect = false;
  wifiConnectAttemptNeeded = true;
}

// ********************************************************************************
// Scan WiFi network
// ********************************************************************************
void WifiScan(bool async, bool quick) {
  if (WiFi.scanComplete() == -1) { 
    // Scan still busy
    return;
  }
  addLog(LOG_LEVEL_INFO, F("WIFI  : Start network scan"));
  bool show_hidden         = true;
  processedScanDone = false;
  lastGetScanMoment.setNow();
  if (quick) {
    #ifdef ESP8266
    // Only scan a single channel if the RTC.lastWiFiChannel is known to speed up connection time.
    WiFi.scanNetworks(async, show_hidden, RTC.lastWiFiChannel);
    #else
    WiFi.scanNetworks(async, show_hidden);
    #endif
  } else {
    WiFi.scanNetworks(async, show_hidden);
  }
}

// ********************************************************************************
// Scan all Wifi Access Points
// ********************************************************************************
void WifiScan()
{
  // Direct Serial is allowed here, since this function will only be called from serial input.
  serialPrintln(F("WIFI : SSID Scan start"));
  WifiScan(false, false);
  const int8_t scanCompleteStatus = WiFi.scanComplete();
  if (scanCompleteStatus <= 0) {
    serialPrintln(F("WIFI : No networks found"));
  }
  else
  {
    serialPrint(F("WIFI : "));
    serialPrint(String(scanCompleteStatus));
    serialPrintln(F(" networks found"));

    for (int i = 0; i < scanCompleteStatus; ++i)
    {
      // Print SSID and RSSI for each network found
      serialPrint(F("WIFI : "));
      serialPrint(String(i + 1));
      serialPrint(": ");
      serialPrintln(formatScanResult(i, " "));
      delay(10);
    }
  }
  serialPrintln("");
}

// ********************************************************************************
// Manage Wifi Modes
// ********************************************************************************
void setSTA(bool enable) {
  switch (WiFi.getMode()) {
    case WIFI_OFF:

      if (enable) { setWifiMode(WIFI_STA); }
      break;
    case WIFI_STA:

      if (!enable) { setWifiMode(WIFI_OFF); }
      break;
    case WIFI_AP:

      if (enable) { setWifiMode(WIFI_AP_STA); }
      break;
    case WIFI_AP_STA:

      if (!enable) { setWifiMode(WIFI_AP); }
      break;
    default:
      break;
  }
}

void setAP(bool enable) {
  WiFiMode_t wifimode = WiFi.getMode();

  switch (wifimode) {
    case WIFI_OFF:

      if (enable) { setWifiMode(WIFI_AP); }
      break;
    case WIFI_STA:

      if (enable) { setWifiMode(WIFI_AP_STA); }
      break;
    case WIFI_AP:

      if (!enable) { setWifiMode(WIFI_OFF); }
      break;
    case WIFI_AP_STA:

      if (!enable) { setWifiMode(WIFI_STA); }
      break;
    default:
      break;
  }
}

// Only internal scope
void setAPinternal(bool enable)
{
  if (enable) {
    // create and store unique AP SSID/PW to prevent ESP from starting AP mode with default SSID and No password!
    // setup ssid for AP Mode when needed
    String softAPSSID = NetworkCreateRFCCompliantHostname();
    String pwd        = SecuritySettings.WifiAPKey;
    IPAddress subnet(DEFAULT_AP_SUBNET);

    if (!WiFi.softAPConfig(apIP, apIP, subnet)) {
      addLog(LOG_LEVEL_ERROR, F("WIFI : [AP] softAPConfig failed!"));
    }

    if (WiFi.softAP(softAPSSID.c_str(), pwd.c_str())) {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        eventQueue.add(F("WiFi#APmodeEnabled"));
        String log(F("WIFI : AP Mode ssid will be "));
        log += softAPSSID;
        log += F(" with address ");
        log += WiFi.softAPIP().toString();
        addLog(LOG_LEVEL_INFO, log);
      }
    } else {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String log(F("WIFI : Error while starting AP Mode with SSID: "));
        log += softAPSSID;
        log += F(" IP: ");
        log += apIP.toString();
        addLog(LOG_LEVEL_ERROR, log);
      }
    }
    #ifdef ESP32

    #else // ifdef ESP32

    if (wifi_softap_dhcps_status() != DHCP_STARTED) {
      if (!wifi_softap_dhcps_start()) {
        addLog(LOG_LEVEL_ERROR, F("WIFI : [AP] wifi_softap_dhcps_start failed!"));
      }
    }
    #endif // ifdef ESP32
    timerAPoff.setNow();
  } else {
    if (dnsServerActive) {
      dnsServerActive = false;
      dnsServer.stop();
    }
  }
}

String getWifiModeString(WiFiMode_t wifimode)
{
  switch (wifimode) {
    case WIFI_OFF:   return F("OFF");
    case WIFI_STA:   return F("STA");
    case WIFI_AP:    return F("AP");
    case WIFI_AP_STA: return F("AP+STA");
    default:
      break;
  }
  return F("Unknown");
}

void setWifiMode(WiFiMode_t wifimode) {
  const WiFiMode_t cur_mode = WiFi.getMode();

  if (cur_mode == wifimode) {
    return;
  }

  if (wifimode != WIFI_OFF) {
    #ifdef ESP8266

    // See: https://github.com/esp8266/Arduino/issues/6172#issuecomment-500457407
    WiFi.forceSleepWake(); // Make sure WiFi is really active.
    #endif // ifdef ESP8266
    delay(100);
  }

  addLog(LOG_LEVEL_INFO, String(F("WIFI : Set WiFi to ")) + getWifiModeString(wifimode));

  int retry = 2;
  while (!WiFi.mode(wifimode) && retry > 0) {
    addLog(LOG_LEVEL_INFO, F("WIFI : Cannot set mode!!!!!"));
    delay(100);
    --retry;
  }

  if (wifimode == WIFI_OFF) {
    delay(1000);
    #ifdef ESP8266
    WiFi.forceSleepBegin();
    #endif // ifdef ESP8266
    delay(1);
  } else {
    delay(100); // Must allow for some time to init.
  }
  bool new_mode_AP_enabled = WifiIsAP(wifimode);

  if (WifiIsAP(cur_mode) && !new_mode_AP_enabled) {
    eventQueue.add(F("WiFi#APmodeDisabled"));
  }

  if (WifiIsAP(cur_mode) != new_mode_AP_enabled) {
    // Mode has changed
    setAPinternal(new_mode_AP_enabled);
  }
  #ifdef FEATURE_MDNS
  MDNS.notifyAPChange();
  #endif
}

bool WifiIsAP(WiFiMode_t wifimode)
{
  #if defined(ESP32)
  return (wifimode == WIFI_MODE_AP) || (wifimode == WIFI_MODE_APSTA);
  #else // if defined(ESP32)
  return (wifimode == WIFI_AP) || (wifimode == WIFI_AP_STA);
  #endif // if defined(ESP32)
}

bool WifiIsSTA(WiFiMode_t wifimode)
{
  #if defined(ESP32)
  return (wifimode & WIFI_MODE_STA) != 0;
  #else // if defined(ESP32)
  return (wifimode & WIFI_STA) != 0;
  #endif // if defined(ESP32)
}

bool useStaticIP() {
  return Settings.IP[0] != 0 && Settings.IP[0] != 255;
}

bool wifiConnectTimeoutReached() {
  // For the first attempt, do not wait to start connecting.
  if (wifi_connect_attempt == 0) { return true; }

  if (lastDisconnectMoment.isSet() && (last_wifi_connect_attempt_moment > lastDisconnectMoment)) {
    // Connection attempt was already ended.
    return true;
  }

  if (WifiIsAP(WiFi.getMode())) {
    // Initial setup of WiFi, may take much longer since accesspoint is still active.
    return last_wifi_connect_attempt_moment.timeoutReached(20000);
  }

  // wait until it connects + add some device specific random offset to prevent
  // all nodes overloading the accesspoint when turning on at the same time.
  #if defined(ESP8266)
  const unsigned int randomOffset_in_msec = (wifi_connect_attempt == 1) ? 0 : 1000 * ((ESP.getChipId() & 0xF));
  #endif // if defined(ESP8266)
  #if defined(ESP32)
  const unsigned int randomOffset_in_msec = (wifi_connect_attempt == 1) ? 0 : 1000 * ((ESP.getEfuseMac() & 0xF));
  #endif // if defined(ESP32)
  return last_wifi_connect_attempt_moment.timeoutReached(DEFAULT_WIFI_CONNECTION_TIMEOUT + randomOffset_in_msec);
}

bool wifiAPmodeActivelyUsed()
{
  if (!WifiIsAP(WiFi.getMode()) || (!timerAPoff.isSet())) {
    // AP not active or soon to be disabled in processDisableAPmode()
    return false;
  }
  return WiFi.softAPgetStationNum() != 0;

  // FIXME TD-er: is effectively checking for AP active enough or must really check for connected clients to prevent automatic wifi
  // reconnect?
}

void setConnectionSpeed() {
  #ifdef ESP8266

  if (!Settings.ForceWiFi_bg_mode() || (wifi_connect_attempt > 10)) {
    WiFi.setPhyMode(WIFI_PHY_MODE_11N);
  } else {
    WiFi.setPhyMode(WIFI_PHY_MODE_11G);
  }
  #endif // ifdef ESP8266

  // Does not (yet) work, so commented out.
  #ifdef ESP32
  /*
  uint8_t protocol = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G; // Default to BG

  if (!Settings.ForceWiFi_bg_mode() || (wifi_connect_attempt > 10)) {
    // Set to use BGN
    protocol |= WIFI_PROTOCOL_11N;
  }

  if (WifiIsSTA(WiFi.getMode())) {
    esp_wifi_set_protocol(WIFI_IF_STA, protocol);
  }

  if (WifiIsAP(WiFi.getMode())) {
    esp_wifi_set_protocol(WIFI_IF_AP, protocol);
  }
  */
  #endif // ifdef ESP32
}

void setupStaticIPconfig() {
  setUseStaticIP(useStaticIP());

  if (!useStaticIP()) { return; }
  const IPAddress ip     = Settings.IP;
  const IPAddress gw     = Settings.Gateway;
  const IPAddress subnet = Settings.Subnet;
  const IPAddress dns    = Settings.DNS;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("IP   : Static IP : ");
    log += formatIP(ip);
    log += F(" GW: ");
    log += formatIP(gw);
    log += F(" SN: ");
    log += formatIP(subnet);
    log += F(" DNS: ");
    log += formatIP(dns);
    addLog(LOG_LEVEL_INFO, log);
  }
  WiFi.config(ip, gw, subnet, dns);
}

// ********************************************************************************
// Formatting WiFi related strings
// ********************************************************************************
String formatScanResult(int i, const String& separator) {
  int32_t rssi = 0;

  return formatScanResult(i, separator, rssi);
}

String formatScanResult(int i, const String& separator, int32_t& rssi) {
  String result = WiFi.SSID(i);

  htmlEscape(result);
  #ifndef ESP32

  if (WiFi.isHidden(i)) {
    result += F("#Hidden#");
  }
  #endif // ifndef ESP32
  rssi    = WiFi.RSSI(i);
  result += separator;
  result += WiFi.BSSIDstr(i);
  result += separator;
  result += F("Ch:");
  result += WiFi.channel(i);
  result += " (";
  result += rssi;
  result += F("dBm) ");

  switch (WiFi.encryptionType(i)) {
  #ifdef ESP32
    case WIFI_AUTH_OPEN: result            += F("open"); break;
    case WIFI_AUTH_WEP:  result            += F("WEP"); break;
    case WIFI_AUTH_WPA_PSK: result         += F("WPA/PSK"); break;
    case WIFI_AUTH_WPA2_PSK: result        += F("WPA2/PSK"); break;
    case WIFI_AUTH_WPA_WPA2_PSK: result    += F("WPA/WPA2/PSK"); break;
    case WIFI_AUTH_WPA2_ENTERPRISE: result += F("WPA2 Enterprise"); break;
  #else // ifdef ESP32
    case ENC_TYPE_WEP: result  += F("WEP"); break;
    case ENC_TYPE_TKIP: result += F("WPA/PSK"); break;
    case ENC_TYPE_CCMP: result += F("WPA2/PSK"); break;
    case ENC_TYPE_NONE: result += F("open"); break;
    case ENC_TYPE_AUTO: result += F("WPA/WPA2/PSK"); break;
  #endif // ifdef ESP32
    default:
      break;
  }
  return result;
}

#ifndef ESP32
String SDKwifiStatusToString(uint8_t sdk_wifistatus) {
  switch (sdk_wifistatus) {
    case STATION_IDLE:           return F("STATION_IDLE");
    case STATION_CONNECTING:     return F("STATION_CONNECTING");
    case STATION_WRONG_PASSWORD: return F("STATION_WRONG_PASSWORD");
    case STATION_NO_AP_FOUND:    return F("STATION_NO_AP_FOUND");
    case STATION_CONNECT_FAIL:   return F("STATION_CONNECT_FAIL");
    case STATION_GOT_IP:         return F("STATION_GOT_IP");
  }
  return getUnknownString();
}

#endif // ifndef ESP32

String ArduinoWifiStatusToString(uint8_t arduino_corelib_wifistatus) {
  String log;

  switch (arduino_corelib_wifistatus) {
    case WL_NO_SHIELD:       log += F("WL_NO_SHIELD"); break;
    case WL_IDLE_STATUS:     log += F("WL_IDLE_STATUS"); break;
    case WL_NO_SSID_AVAIL:   log += F("WL_NO_SSID_AVAIL"); break;
    case WL_SCAN_COMPLETED:  log += F("WL_SCAN_COMPLETED"); break;
    case WL_CONNECTED:       log += F("WL_CONNECTED"); break;
    case WL_CONNECT_FAILED:  log += F("WL_CONNECT_FAILED"); break;
    case WL_CONNECTION_LOST: log += F("WL_CONNECTION_LOST"); break;
    case WL_DISCONNECTED:    log += F("WL_DISCONNECTED"); break;
    default:  log                += arduino_corelib_wifistatus; break;
  }
  return log;
}

String ESPeasyWifiStatusToString() {
  String log;
  if (wifiStatus == ESPEASY_WIFI_DISCONNECTED) {
    log = F("DISCONNECTED");
  } else {
    if (bitRead(wifiStatus, ESPEASY_WIFI_CONNECTED)) {
      log += F("Conn. ");
    }
    if (bitRead(wifiStatus, ESPEASY_WIFI_GOT_IP)) {
      log += F("IP ");
    }
    if (bitRead(wifiStatus, ESPEASY_WIFI_SERVICES_INITIALIZED)) {
      log += F("Init");
    }
  }
  return log;
}

void logConnectionStatus() {
  #ifdef esp8266
  const uint8_t arduino_corelib_wifistatus = WiFi.status();
  const uint8_t sdk_wifistatus             = wifi_station_get_connect_status();

  if ((arduino_corelib_wifistatus == WL_CONNECTED) != (sdk_wifistatus == STATION_GOT_IP)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("WIFI  : SDK station status differs from Arduino status. SDK-status: ");
      log += SDKwifiStatusToString(sdk_wifistatus);
      log += F(" Arduino status: ");
      log += ArduinoWifiStatusToString(arduino_corelib_wifistatus);
      addLog(LOG_LEVEL_ERROR, log);
    }
  }
  #endif
#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("WIFI  : Arduino wifi status: ");
    log += ArduinoWifiStatusToString(WiFi.status());
    log += F(" ESPeasy internal wifi status: ");
    log += ESPeasyWifiStatusToString();
    addLog(LOG_LEVEL_INFO, log);
  }
/*
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;

    switch (WiFi.status()) {
      case WL_NO_SSID_AVAIL: {
        log = F("WIFI : No SSID found matching: ");
        break;
      }
      case WL_CONNECT_FAILED: {
        log = F("WIFI : Connection failed to: ");
        break;
      }
      case WL_DISCONNECTED: {
        log = F("WIFI : WiFi.status() = WL_DISCONNECTED  SSID: ");
        break;
      }
      case WL_IDLE_STATUS: {
        log = F("WIFI : Connection in IDLE state: ");
        break;
      }
      case WL_CONNECTED: {
        break;
      }
      default:
        break;
    }

    if (log.length() > 0) {
      const char *ssid = getLastWiFiSettingsSSID();
      log += ssid;
      addLog(LOG_LEVEL_INFO, log);
    }
  }
  */
#endif // ifndef BUILD_NO_DEBUG
}

String getLastDisconnectReason() {
  String reason = "(";

  reason += lastDisconnectReason;
  reason += F(") ");

  switch (lastDisconnectReason) {
    case WIFI_DISCONNECT_REASON_UNSPECIFIED:                reason += F("Unspecified");              break;
    case WIFI_DISCONNECT_REASON_AUTH_EXPIRE:                reason += F("Auth expire");              break;
    case WIFI_DISCONNECT_REASON_AUTH_LEAVE:                 reason += F("Auth leave");               break;
    case WIFI_DISCONNECT_REASON_ASSOC_EXPIRE:               reason += F("Assoc expire");             break;
    case WIFI_DISCONNECT_REASON_ASSOC_TOOMANY:              reason += F("Assoc toomany");            break;
    case WIFI_DISCONNECT_REASON_NOT_AUTHED:                 reason += F("Not authed");               break;
    case WIFI_DISCONNECT_REASON_NOT_ASSOCED:                reason += F("Not assoced");              break;
    case WIFI_DISCONNECT_REASON_ASSOC_LEAVE:                reason += F("Assoc leave");              break;
    case WIFI_DISCONNECT_REASON_ASSOC_NOT_AUTHED:           reason += F("Assoc not authed");         break;
    case WIFI_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD:        reason += F("Disassoc pwrcap bad");      break;
    case WIFI_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD:       reason += F("Disassoc supchan bad");     break;
    case WIFI_DISCONNECT_REASON_IE_INVALID:                 reason += F("IE invalid");               break;
    case WIFI_DISCONNECT_REASON_MIC_FAILURE:                reason += F("Mic failure");              break;
    case WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT:     reason += F("4way handshake timeout");   break;
    case WIFI_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT:   reason += F("Group key update timeout"); break;
    case WIFI_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS:         reason += F("IE in 4way differs");       break;
    case WIFI_DISCONNECT_REASON_GROUP_CIPHER_INVALID:       reason += F("Group cipher invalid");     break;
    case WIFI_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID:    reason += F("Pairwise cipher invalid");  break;
    case WIFI_DISCONNECT_REASON_AKMP_INVALID:               reason += F("AKMP invalid");             break;
    case WIFI_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION:      reason += F("Unsupp RSN IE version");    break;
    case WIFI_DISCONNECT_REASON_INVALID_RSN_IE_CAP:         reason += F("Invalid RSN IE cap");       break;
    case WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED:         reason += F("802 1X auth failed");       break;
    case WIFI_DISCONNECT_REASON_CIPHER_SUITE_REJECTED:      reason += F("Cipher suite rejected");    break;
    case WIFI_DISCONNECT_REASON_BEACON_TIMEOUT:             reason += F("Beacon timeout");           break;
    case WIFI_DISCONNECT_REASON_NO_AP_FOUND:                reason += F("No AP found");              break;
    case WIFI_DISCONNECT_REASON_AUTH_FAIL:                  reason += F("Auth fail");                break;
    case WIFI_DISCONNECT_REASON_ASSOC_FAIL:                 reason += F("Assoc fail");               break;
    case WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT:          reason += F("Handshake timeout");        break;
    default:  reason                                               += getUnknownString();       break;
  }
  return reason;
}
