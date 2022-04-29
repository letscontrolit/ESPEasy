#include "../ESPEasyCore/ESPEasyEth.h"

#ifdef HAS_ETHERNET

#include "../CustomBuild/ESPEasyLimits.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Settings.h"
#include "../Helpers/StringConverter.h"

#include <ETH.h>
#include <lwip/dns.h>
#if ESP_IDF_VERSION_MAJOR > 3
 #include <esp_eth_phy.h>
#else
 #include <eth_phy/phy.h>
#endif

bool ethUseStaticIP() {
  return Settings.ETH_IP[0] != 0 && Settings.ETH_IP[0] != 255;
}

void ethSetupStaticIPconfig() {
  if (!ethUseStaticIP()) { 
    const IPAddress IP_zero(0, 0, 0, 0); 
    if (!ETH.config(IP_zero, IP_zero, IP_zero, IP_zero)) {
      addLog(LOG_LEVEL_ERROR, F("ETH  : Cannot set IP config"));
    }
    return; 
  }
  const IPAddress ip     = Settings.ETH_IP;
  const IPAddress gw     = Settings.ETH_Gateway;
  const IPAddress subnet = Settings.ETH_Subnet;
  const IPAddress dns    = Settings.ETH_DNS;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("ETH IP   : Static IP : ");
    log += formatIP(ip);
    log += F(" GW: ");
    log += formatIP(gw);
    log += F(" SN: ");
    log += formatIP(subnet);
    log += F(" DNS: ");
    log += formatIP(dns);
    addLogMove(LOG_LEVEL_INFO, log);
  }
  ETH.config(ip, gw, subnet, dns);
}

void ethSetDNS(const IPAddress dns0, const IPAddress dns1) 
{
  ip_addr_t d;
  d.type = IPADDR_TYPE_V4;

  if(dns0 != (uint32_t)0x00000000 && dns0 != INADDR_NONE) {
    // Set DNS0-Server
    d.u_addr.ip4.addr = static_cast<uint32_t>(dns0);
    dns_setserver(0, &d);
  }

  if(dns1 != (uint32_t)0x00000000 && dns1 != INADDR_NONE) {
    // Set DNS1-Server
    d.u_addr.ip4.addr = static_cast<uint32_t>(dns1);
    dns_setserver(1, &d);
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("ETH IP   : Set DNS: ");
    log += formatIP(dns0);
    log += '/';
    log += formatIP(dns1);
    addLogMove(LOG_LEVEL_INFO, log);
  }
}

bool ethCheckSettings() {
  return isValid(Settings.ETH_Phy_Type) 
      && isValid(Settings.ETH_Clock_Mode)
      && isValid(Settings.NetworkMedium)
      && (Settings.ETH_Pin_mdc   <= MAX_GPIO)
      && (Settings.ETH_Pin_mdio  <= MAX_GPIO)
      && (Settings.ETH_Pin_power <= MAX_GPIO);
}

bool ethPrepare() {
  {
    char hostname[40];
    safe_strncpy(hostname, NetworkCreateRFCCompliantHostname().c_str(), sizeof(hostname));
    ETH.setHostname(hostname);
  }
  ethSetupStaticIPconfig();
  return true;
}

void ethPrintSettings() {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    if (log.reserve(115)) {
//    log += F("ETH/Wifi mode: ");
//    log += toString(active_network_medium);
      log += F("ETH PHY Type: ");
      log += toString(Settings.ETH_Phy_Type);
      log += F(" PHY Addr: ");
      log += Settings.ETH_Phy_Addr;
      log += F(" Eth Clock mode: ");
      log += toString(Settings.ETH_Clock_Mode);
      log += F(" MDC Pin: ");
      log += String(Settings.ETH_Pin_mdc);
      log += F(" MIO Pin: ");
      log += String(Settings.ETH_Pin_mdio);
      log += F(" Power Pin: ");
      log += String(Settings.ETH_Pin_power);
      addLogMove(LOG_LEVEL_INFO, log);
    }
  }
}

MAC_address ETHMacAddress() {
  MAC_address mac;
  if(!EthEventData.ethInitSuccess) {
    addLog(LOG_LEVEL_ERROR, F("Call NetworkMacAddress() only on connected Ethernet!"));
  } else {
    #if ESP_IDF_VERSION_MAJOR > 3
    ETH.macAddress(mac.mac);
    #else
    esp_eth_get_mac(mac.mac);
    #endif
  }
  return mac;
}

bool ETHConnectRelaxed() {
  if (EthEventData.ethInitSuccess) {
    return EthLinkUp();
  }
  ethPrintSettings();
  if (!ethCheckSettings())
  {
    addLog(LOG_LEVEL_ERROR, F("ETH: Settings not correct!!!"));
    EthEventData.ethInitSuccess = false;
    return false;
  }
  // Re-register event listener
  #if defined(ESP32)
  removeWiFiEventHandler();
  #endif

  ethPower(true);
  EthEventData.markEthBegin();

  // Re-register event listener
  #if defined(ESP32)
  registerWiFiEventHandler();
  #endif

  if (!EthEventData.ethInitSuccess) {
    EthEventData.ethInitSuccess = ETH.begin( 
      Settings.ETH_Phy_Addr,
      Settings.ETH_Pin_power,
      Settings.ETH_Pin_mdc,
      Settings.ETH_Pin_mdio,
      (eth_phy_type_t)Settings.ETH_Phy_Type,
      (eth_clock_mode_t)Settings.ETH_Clock_Mode);
  }
  if (EthEventData.ethInitSuccess) {
    // FIXME TD-er: Not sure if this is correctly set to false
    //EthEventData.ethConnectAttemptNeeded = false;

    if (EthLinkUp()) {
      // We might miss the connected event, since we are already connected.
      EthEventData.markConnected();
    }
  }
  return EthEventData.ethInitSuccess;
}

void ethPower(bool enable) {
  if (Settings.ETH_Pin_power != -1) {
    if (GPIO_Internal_Read(Settings.ETH_Pin_power) == enable) {
      // Already the desired state
      return;
    }
    EthEventData.ethInitSuccess = false;
    EthEventData.clearAll();
    if (!enable) {
      #ifdef ESP_IDF_VERSION_MAJOR
      // FIXME TD-er: See: https://github.com/espressif/arduino-esp32/issues/6105
      // Need to store the last link state, as it will be cleared after destructing the object.
      EthEventData.setEthDisconnected();
      if (ETH.linkUp()) {
        EthEventData.setEthConnected();
      }
      #endif
      ETH = ETHClass();
    }

    GPIO_Write(1, Settings.ETH_Pin_power, enable ? 1 : 0);
    if (!enable) {
      if (Settings.ETH_Clock_Mode == EthClockMode_t::Ext_crystal_osc) {
        delay(600); // Give some time to discharge any capacitors
        // Delay is needed to make sure no clock signal remains present which may cause the ESP to boot into flash mode.
      }
    } else {
      delay(400); // LAN chip needs to initialize before calling Eth.begin()
    }
  }
}

bool ETHConnected() {
  if (EthEventData.EthServicesInitialized()) {
    if (EthLinkUp()) {
      stop_eth_dhcps();
      return true;
    }
    // Apparently we missed an event
    EthEventData.processedDisconnect = false;
  } else if (EthEventData.ethInitSuccess) {
    if (EthLinkUp()) {
      stop_eth_dhcps();
      EthEventData.setEthConnected();
      if (NetworkLocalIP() != IPAddress(0, 0, 0, 0) && 
          !EthEventData.EthGotIP()) {
        EthEventData.processedGotIP = false;
      }
      if (EthEventData.lastConnectMoment.isSet()) {
        if (!EthEventData.EthServicesInitialized()) {
          if (EthEventData.lastConnectMoment.millisPassedSince() > 10000 &&
              EthEventData.lastGetIPmoment.isSet()) {
            EthEventData.processedGotIP = false;
            EthEventData.markLostIP();
          }
        }
      }
      return EthEventData.EthServicesInitialized();
    } else {
      if (EthEventData.last_eth_connect_attempt_moment.isSet() && 
          EthEventData.last_eth_connect_attempt_moment.millisPassedSince() < 5000) {
        return false;
      }
      setNetworkMedium(NetworkMedium_t::WIFI);
    }
  }
  return false;
}

#endif