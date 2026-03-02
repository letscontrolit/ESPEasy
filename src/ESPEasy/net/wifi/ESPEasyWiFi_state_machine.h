#pragma once

#include "../../../ESPEasy_common.h"

#if FEATURE_WIFI

# include "../DataStructs/WiFi_AP_Candidate.h"
# include "../../../src/Helpers/LongTermTimer.h"
# include "../../../src/Helpers/LongTermOnOffTimer.h"

# include "../wifi/WiFi_STA_connected_state.h"
# include "../wifi/WiFi_State.h"

# include <IPAddress.h>


namespace ESPEasy {
namespace net {
namespace wifi {

class ESPEasyWiFi_t
{
public:

  // Called after settings have changed or at boot
  void setup();

  // Start the process of connecting or start AP, depending on the existing configuration.
  void enable();

  // Terminate WiFi activity
  void disable();

private:

  void begin();

public:

  // Process the state machine for managing WiFi connection
  void        loop();

  WiFiState_e getState() const
  {
    return _state;
  }

  bool connected() const;

  // Get the IP-address in this order:
  // - STA interface if connected,
  // - AP interface if active
  // - 0.0.0.0 if neither connected nor active.
  IPAddress getIP() const;

  void      disconnect();

private:

  void setState(WiFiState_e newState,
                uint32_t    timeout = 0);

  // Handle timeouts + start of AP mode
  void                checkConnectProgress();

  // Check to see if we already have some AP to connect to.
  void                checkScanningProgress();

  void                startScanning();

  bool                connectSTA();


  STA_connected_state getSTA_connected_state() const;

  bool shouldStartAP_fallback() const;

  //  WiFi_AP_Candidate _active_sta;
  //  WiFi_AP_Candidate _AP_conf;

  //  String _last_ssid;
  //  MAC_address _last_bssid;
  //  uint8_t _last_channel = 0;
  WiFiState_e _state = WiFiState_e::Disabled;

  LongTermTimer _last_state_change;
  LongTermTimer _state_timeout;
  LongTermTimer _last_seen_connected;

  uint8_t _scan_channel{};

  //  LongTermTimer _AP_start_timestamp;

  uint32_t _connect_attempt = 0;
  


  //  uint32_t _nrReconnects = 0;

  bool _callbackError = false;

  // Special modes for initial setup or re-configuration of a node via serial port/IMPROV
  //  bool _improvActive = false;
  //  bool _setupActive  = false;


  // WiFi settings => Move to separate class/struct
  #ifdef ESP32P4
  bool _disabledAtBoot = false;
  #else
  bool _disabledAtBoot = false;
  #endif

  //  bool _passiveScan       = false;
  //  bool _includeHiddenSSID = false;
  //  float _TX_power         = -1;

  // Manual IP settings
  // IP
  // Gateway
  // Subnet
  // DNS


  // soft AP configuration => TODO TD-er: Implement
  //  bool _must_Start_AP;  // "Do Not Start AP"
  //  bool _has_ap{false};  // Maybe just check for active mode?
  //  WiFiAP _ap_config;
  //   WPA AP mode key
  //   Don't force /setup in AP-Mode


}; // class ESPEasyWiFi_t

} // namespace wifi
} // namespace net
} // namespace ESPEasy
#endif // if FEATURE_WIFI
