#include "../ESPEasyCore/ESPEasyEth_ProcessEvent.h"

#if FEATURE_ETHERNET

# include "../../ESPEasy-Globals.h"

# include "../ESPEasyCore/ESPEasyEth.h"
# include "../ESPEasyCore/ESPEasyNetwork.h"
# include "../ESPEasyCore/ESPEasyWifi.h" // LogConnectionStatus

# include "../Globals/ESPEasyEthEvent.h"
# include "../Globals/ESPEasyWiFiEvent.h"
# include "../Globals/ESPEasy_Scheduler.h"
# include "../Globals/ESPEasy_time.h"
# include "../Globals/EventQueue.h"
# include "../Globals/MQTT.h"
# include "../Globals/NetworkState.h"
# include "../Globals/Settings.h"

# include "../Helpers/LongTermTimer.h"
# include "../Helpers/Network.h"
# include "../Helpers/Networking.h"
# include "../Helpers/PeriodicalActions.h"
# include "../Helpers/StringConverter.h"

# include <ETH.h>

void handle_unprocessedEthEvents() {
  if (EthEventData.unprocessedEthEvents()) {
    // Process disconnect events before connect events.
#if FEATURE_USE_IPV6
    if (!EthEventData.processedGotIP6) {
      processEthernetGotIPv6();
    }
#endif

    if (!EthEventData.processedDisconnect) {
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("Eth  : Entering processDisconnect()"));
      # endif // ifndef BUILD_NO_DEBUG
      processEthernetDisconnected();
    }

    // Must process the Ethernet Connected event regardless the active network medium.
    // It may happen by plugging in the cable while WiFi was active.
    if (!EthEventData.processedConnect) {
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("Eth  : Entering processConnect()"));
      # endif // ifndef BUILD_NO_DEBUG
      processEthernetConnected();
    }
  }

  if (active_network_medium == NetworkMedium_t::Ethernet) {
    const bool EthServices_was_initialized = EthEventData.EthServicesInitialized();

    if (!EthEventData.EthServicesInitialized() || EthEventData.unprocessedEthEvents())
    {
      if (!EthEventData.unprocessedEthEvents() && EthEventData.EthConnectAllowed()) {
        NetworkConnectRelaxed();
      }

      if (!EthEventData.processedGotIP) {
        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("Eth  : Entering processGotIP()"));
        # endif // ifndef BUILD_NO_DEBUG
        processEthernetGotIP();
      }

      if (!EthEventData.processedDHCPTimeout) {
        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("Eth  : DHCP timeout, Calling disconnect()"));
        # endif // ifndef BUILD_NO_DEBUG
        EthEventData.processedDHCPTimeout = true;

        // WifiDisconnect();
      }
    }
    EthEventData.setEthServicesInitialized();

    if (!EthServices_was_initialized && EthEventData.setEthServicesInitialized()) {
      registerEthEventHandler();
    }
  }
}

void check_Eth_DNS_valid() {
  // Check if DNS is still valid, as this may have been reset by the WiFi module turned off.
  // DNS records are shared between WiFi and Ethernet IP stack.
  // Turning off WiFi will clead DNS records for Ethernet
  if (EthEventData.EthServicesInitialized() &&
      (active_network_medium == NetworkMedium_t::Ethernet) &&
      EthEventData.ethInitSuccess &&
      !ethUseStaticIP()) {
    const bool has_cache = 
      valid_DNS_address(EthEventData.dns0_cache) || 
      valid_DNS_address(EthEventData.dns1_cache);

    if (has_cache) {
      const IPAddress dns0 = ETH.dnsIP(0);
      const IPAddress dns1 = ETH.dnsIP(1);

      if (!valid_DNS_address(dns0) && !valid_DNS_address(dns1)) {
        static uint32_t lastLog = 0;
        if (timePassedSince(lastLog) > 1000) {
          addLogMove(LOG_LEVEL_ERROR, concat(
            F("ETH  : DNS server was cleared, use cached DNS IP: "), 
            formatIP(EthEventData.dns0_cache)));
          lastLog = millis();
        }
        setDNS(0, EthEventData.dns0_cache);
        setDNS(1, EthEventData.dns1_cache);
      }
    }
  }
}

void processEthernetConnected() {
  if (EthEventData.processedConnect) { return; }

  // FIXME TD-er: Must differentiate among reconnects for WiFi and Ethernet.
  ++WiFiEventData.wifi_reconnects;
  addLog(LOG_LEVEL_INFO, F("processEthernetConnected()"));
  EthEventData.setEthConnected();
  EthEventData.processedConnect = true;

  if (Settings.UseRules)
  {
    eventQueue.add(F("Ethernet#LinkUp"));
  }
  setNetworkMedium(Settings.NetworkMedium);

  if (ethUseStaticIP()) { EthEventData.processedGotIP = false; }
}

void processEthernetDisconnected() {
  if (EthEventData.processedDisconnect) { return; }
  EthEventData.setEthDisconnected();
  EthEventData.processedDisconnect     = true;
  EthEventData.ethConnectAttemptNeeded = true;

  if (Settings.UseRules)
  {
    eventQueue.add(F("Ethernet#Disconnected"));
  }
}

void processEthernetGotIP() {
  if (EthEventData.processedGotIP || !EthEventData.ethInitSuccess) {
    return;
  }

  if (ethUseStaticIP()) {
    ethSetupStaticIPconfig();
  }
  const IPAddress ip = NetworkLocalIP();

  if (!ip) {
    return;
  }

  const IPAddress gw     = NetworkGatewayIP();
  const IPAddress subnet = NetworkSubnetMask();

  IPAddress dns0                              = ETH.dnsIP(0);
  IPAddress dns1                              = ETH.dnsIP(1);
  const LongTermTimer::Duration dhcp_duration = EthEventData.lastConnectMoment.timeDiff(EthEventData.lastGetIPmoment);
  #if ESP_IDF_VERSION_MAJOR >= 5
  const bool mustRequestDHCP = (!dns0 && !dns1) && !ethUseStaticIP();
  #endif

  if (!ethUseStaticIP()) {
    if (!dns0 && !dns1) {
      addLog(LOG_LEVEL_ERROR, F("ETH  : No DNS server received via DHCP, use cached DNS IP"));
      if (EthEventData.dns0_cache) setDNS(0, EthEventData.dns0_cache);
      if (EthEventData.dns1_cache) setDNS(1, EthEventData.dns1_cache);
    } else {
      EthEventData.dns0_cache = dns0;
      EthEventData.dns1_cache = dns1;
    }
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log;

    if (log.reserve(160)) {
      log  = F("ETH MAC: ");
      log += NetworkMacAddress().toString();
      log += ' ';

      if (ethUseStaticIP()) {
        log += F("Static");
      } else {
        log += F("DHCP");
      }
      log += F(" IP: ");
      log += formatIP(ip);
      log += ' ';
      log += wrap_braces(NetworkGetHostname());
      log += F(" GW: ");
      log += formatIP(gw);
      log += F(" SN: ");
      log += formatIP(subnet);
      log += F(" DNS: ");
      log += formatIP(dns0);
      log += '/';
      log += formatIP(dns1);

      if (EthLinkUp()) {
        if (EthFullDuplex()) {
          log += F(" FULL_DUPLEX");
        }
        log += ' ';
        log += EthLinkSpeed();
        log += F("Mbps");
      } else {
        log += F(" Link Down");
      }

      if ((dhcp_duration > 0ll) && (dhcp_duration < 30000000ll)) {
        // Just log times when they make sense.
        log += F("   duration: ");
        log += static_cast<int32_t>(dhcp_duration / 1000);
        log += F(" ms");
      }

      addLogMove(LOG_LEVEL_INFO, log);
    }
  }

  // First try to get the time, since that may be used in logs
  if (node_time.systemTimePresent()) {
    node_time.initTime();
  }
# if FEATURE_MQTT
  mqtt_reconnect_count        = 0;
  MQTTclient_should_reconnect = true;
  timermqtt_interval          = 100;
  Scheduler.setIntervalTimer(SchedulerIntervalTimer_e::TIMER_MQTT);
  scheduleNextMQTTdelayQueue();
# endif // if FEATURE_MQTT
  Scheduler.sendGratuitousARP_now();

  if (Settings.UseRules)
  {
    eventQueue.add(F("Ethernet#Connected"));
  }
  statusLED(true);
  logConnectionStatus();

  EthEventData.processedGotIP = true;
#if ESP_IDF_VERSION_MAJOR >= 5
  if (mustRequestDHCP /*&& EthEventData.lastConnectMoment.millisPassedSince() < 10000*/) {
    // FIXME TD-er: Must add some check other than fixed timeout here to not constantly make DHCP requests.
    // Force new DHCP request.
    ETH.config();
  }
#endif

  EthEventData.setEthGotIP();
  CheckRunningServices();
}

#if FEATURE_USE_IPV6
void processEthernetGotIPv6() {
  if (!EthEventData.processedGotIP6) {
    if (loglevelActiveFor(LOG_LEVEL_INFO))
      addLog(LOG_LEVEL_INFO, String(F("ETH event: Got IP6 ")) + EthEventData.unprocessed_IP6.toString(true));
    EthEventData.processedGotIP6 = true;
#if FEATURE_ESPEASY_P2P
//    updateUDPport(true);
#endif

  }
}
#endif

#endif // if FEATURE_ETHERNET
