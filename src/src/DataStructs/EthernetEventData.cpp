#include "../DataStructs/EthernetEventData.h"

#include "../ESPEasyCore/ESPEasy_Log.h"

// Bit numbers for Eth status
#define ESPEASY_ETH_CONNECTED               0
#define ESPEASY_ETH_GOT_IP                  1
#define ESPEASY_ETH_SERVICES_INITIALIZED    2

bool EthernetEventData_t::EthConnectAllowed() const {
  if (!ethConnectAttemptNeeded) return false;
  if (last_eth_connect_attempt_moment.isSet()) {
    // TODO TD-er: Make this time more dynamic.
    if (!last_eth_connect_attempt_moment.timeoutReached(10000)) {
      return false;
    }
  }
  return true;
}

bool EthernetEventData_t::unprocessedEthEvents() const {
  if (processedConnect && processedDisconnect && processedGotIP && processedDHCPTimeout)
  {
    return false;
  }
  return true;
}

void EthernetEventData_t::clearAll() {
  lastDisconnectMoment.clear();
  lastConnectMoment.clear();
  lastGetIPmoment.clear();
  last_eth_connect_attempt_moment.clear();

  setEthDisconnected();
  lastEthResetMoment.setNow();
  eth_considered_stable = false;

  // Mark all flags to default to prevent handling old events.
  processedConnect          = true;
  processedDisconnect       = true;
  processedGotIP            = true;
  processedDHCPTimeout      = true;
  ethConnectAttemptNeeded  = true;
}

void EthernetEventData_t::markEthBegin() {
  setEthDisconnected();
  lastDisconnectMoment.clear();
  lastConnectMoment.clear();
  lastGetIPmoment.clear();
  last_eth_connect_attempt_moment.setNow();
  eth_considered_stable = false;
  ethConnectInProgress  = true;
  ++eth_connect_attempt;
}

bool EthernetEventData_t::EthDisconnected() const {
  return ethStatus == ESPEASY_ETH_DISCONNECTED;
}

bool EthernetEventData_t::EthGotIP() const {
  return bitRead(ethStatus, ESPEASY_ETH_GOT_IP);
}

bool EthernetEventData_t::EthConnected() const {
  return bitRead(ethStatus, ESPEASY_ETH_CONNECTED);
}

bool EthernetEventData_t::EthServicesInitialized() const {
  return bitRead(ethStatus, ESPEASY_ETH_SERVICES_INITIALIZED);
}

void EthernetEventData_t::setEthDisconnected() {
  ethStatus = ESPEASY_ETH_DISCONNECTED;
}

void EthernetEventData_t::setEthGotIP() {
  bitSet(ethStatus, ESPEASY_ETH_GOT_IP);
  setEthServicesInitialized();
}

void EthernetEventData_t::setEthConnected() {
  bitSet(ethStatus, ESPEASY_ETH_CONNECTED);
  setEthServicesInitialized();
}

void EthernetEventData_t::setEthServicesInitialized() {
  if (!unprocessedEthEvents() && !EthServicesInitialized()) {
    if (EthGotIP() && EthConnected()) {
      addLog(LOG_LEVEL_DEBUG, F("Eth : Eth services initialized"));
      bitSet(ethStatus, ESPEASY_ETH_SERVICES_INITIALIZED);
      ethConnectInProgress = false;
    }
  }
}

void EthernetEventData_t::markGotIP() {
  lastGetIPmoment.setNow();

  // Create the 'got IP event' so mark the ethStatus to not have the got IP flag set
  // This also implies the services are not fully initialized.
  bitClear(ethStatus, ESPEASY_ETH_GOT_IP);
  bitClear(ethStatus, ESPEASY_ETH_SERVICES_INITIALIZED);
  processedGotIP = false;
}

void EthernetEventData_t::markLostIP() {
  bitClear(ethStatus, ESPEASY_ETH_GOT_IP);
  bitClear(ethStatus, ESPEASY_ETH_SERVICES_INITIALIZED);
}

void EthernetEventData_t::markDisconnect() {
  lastDisconnectMoment.setNow();

  if (last_eth_connect_attempt_moment.isSet() && !lastConnectMoment.isSet()) {
    // There was an unsuccessful connection attempt
    lastConnectedDuration_us = last_eth_connect_attempt_moment.timeDiff(lastDisconnectMoment);
  } else {
    lastConnectedDuration_us = lastConnectMoment.timeDiff(lastDisconnectMoment);
  }
  setEthDisconnected();
  processedDisconnect  = false;
}

void EthernetEventData_t::markConnected() {
  lastConnectMoment.setNow();
  processedConnect    = false;
}

