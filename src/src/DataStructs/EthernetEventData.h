#ifndef DATASTRUCTS_ETHERNETEVENTDATA_H
#define DATASTRUCTS_ETHERNETEVENTDATA_H

#include "../../ESPEasy_common.h"

#if FEATURE_ETHERNET

#include "../Helpers/LongTermTimer.h"




#ifdef ESP32
# include <esp_event.h>

#endif // ifdef ESP32

#include <IPAddress.h>


// EthStatus
#define ESPEASY_ETH_DISCONNECTED            0


struct EthernetEventData_t {
  bool EthConnectAllowed() const;

  bool unprocessedEthEvents() const;

  void clearAll();
  void markEthBegin();

  bool EthDisconnected() const;
  bool EthGotIP() const;
  bool EthConnected() const;
  bool EthServicesInitialized() const;

  void setEthDisconnected();
  void setEthGotIP();
  void setEthConnected();
  bool setEthServicesInitialized();


  void markGotIP();
  void markLostIP();
  void markDisconnect();
  void markConnected();

  String ESPEasyEthStatusToString() const;


  // Eth related data
  bool          ethSetup        = false;
  uint8_t       ethStatus       = ESPEASY_ETH_DISCONNECTED;
  LongTermTimer last_eth_connect_attempt_moment;
  unsigned int  eth_connect_attempt   = 0;
  bool          eth_considered_stable = false;
  int           eth_reconnects        = -1; // First connection attempt is not a reconnect.

  LongTermTimer           lastConnectMoment;
  LongTermTimer           lastDisconnectMoment;
  LongTermTimer           lastEthResetMoment;
  LongTermTimer           lastGetIPmoment;
  LongTermTimer::Duration lastConnectedDuration_us = 0ll;

  IPAddress dns0_cache{};
  IPAddress dns1_cache{};

  // Semaphore like bools for processing data gathered from Eth events.
  bool processedConnect          = true;
  bool processedDisconnect       = true;
  bool processedGotIP            = true;
  bool processedDHCPTimeout      = true;
  bool ethConnectAttemptNeeded  = true;
  bool ethConnectInProgress     = false;


  bool ethInitSuccess            = false;
  unsigned long connectionFailures = 0;

#ifdef ESP32
  WiFiEventId_t wm_event_id = 0;
#endif // ifdef ESP32

};

#endif

#endif   // ifndef DATASTRUCTS_ETHERNETEVENTDATA_H
