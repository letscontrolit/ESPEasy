#include "../ESPEasyCore/ESPEasyEth.h"

#if FEATURE_ETHERNET

#include "../CustomBuild/ESPEasyLimits.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../ESPEasyCore/ESPEasyEthEvent.h"
#include "../Globals/ESPEasyEthEvent.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Settings.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/Networking.h"

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
  const IPAddress IP_zero(0, 0, 0, 0); 
  if (!ethUseStaticIP()) { 
    if (!ETH.config(IP_zero, IP_zero, IP_zero, IP_zero)) {
      addLog(LOG_LEVEL_ERROR, F("ETH  : Cannot set IP config"));
    }
    return; 
  }
  const IPAddress ip     = Settings.ETH_IP;
  const IPAddress gw     = Settings.ETH_Gateway;
  const IPAddress subnet = Settings.ETH_Subnet;
  const IPAddress dns    = Settings.ETH_DNS;

  EthEventData.dns0_cache = dns;
  EthEventData.dns1_cache = IP_zero;


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
  setDNS(0, EthEventData.dns0_cache);
  setDNS(1, EthEventData.dns1_cache);
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
  char hostname[40];
  safe_strncpy(hostname, NetworkCreateRFCCompliantHostname().c_str(), sizeof(hostname));
  ETH.setHostname(hostname);
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

void removeEthEventHandler()
{
  WiFi.removeEvent(EthEventData.wm_event_id);
  EthEventData.wm_event_id = 0;
}

void registerEthEventHandler()
{
  if (EthEventData.wm_event_id != 0) {
    removeEthEventHandler();
  }
  EthEventData.wm_event_id = WiFi.onEvent(EthEvent);
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
  removeEthEventHandler();

  ethPower(true);
  EthEventData.markEthBegin();

  // Re-register event listener
  registerEthEventHandler();

  if (!EthEventData.ethInitSuccess) {
    ethResetGPIOpins();
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
    addLog(LOG_LEVEL_INFO, enable ? F("ETH power ON") : F("ETH power OFF"));
    if (!enable) {
      EthEventData.ethInitSuccess = false;
      EthEventData.clearAll();
      #ifdef ESP_IDF_VERSION_MAJOR
      // FIXME TD-er: See: https://github.com/espressif/arduino-esp32/issues/6105
      // Need to store the last link state, as it will be cleared after destructing the object.
      EthEventData.setEthDisconnected();
      if (ETH.linkUp()) {
        EthEventData.setEthConnected();
      }
      #endif
//      ETH = ETHClass();
    }
    if (enable) {
//      ethResetGPIOpins();
    }
//    gpio_reset_pin((gpio_num_t)Settings.ETH_Pin_power);

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

void ethResetGPIOpins() {
  // fix an disconnection issue after rebooting Olimex POE - this forces a clean state for all GPIO involved in RMII
  // Thanks to @s-hadinger and @Jason2866
  // Resetting state of power pin is done in ethPower()
  addLog(LOG_LEVEL_INFO, F("ethResetGPIOpins()"));
  gpio_reset_pin((gpio_num_t)Settings.ETH_Pin_mdc);
  gpio_reset_pin((gpio_num_t)Settings.ETH_Pin_mdio);
  gpio_reset_pin(GPIO_NUM_19);    // EMAC_TXD0 - hardcoded
  gpio_reset_pin(GPIO_NUM_21);    // EMAC_TX_EN - hardcoded
  gpio_reset_pin(GPIO_NUM_22);    // EMAC_TXD1 - hardcoded
  gpio_reset_pin(GPIO_NUM_25);    // EMAC_RXD0 - hardcoded
  gpio_reset_pin(GPIO_NUM_26);    // EMAC_RXD1 - hardcoded
  gpio_reset_pin(GPIO_NUM_27);    // EMAC_RX_CRS_DV - hardcoded
  /*
  switch (Settings.ETH_Clock_Mode) {
    case EthClockMode_t::Ext_crystal_osc:       // ETH_CLOCK_GPIO0_IN
    case EthClockMode_t::Int_50MHz_GPIO_0:      // ETH_CLOCK_GPIO0_OUT
      gpio_reset_pin(GPIO_NUM_0);
      break;
    case EthClockMode_t::Int_50MHz_GPIO_16:     // ETH_CLOCK_GPIO16_OUT
      gpio_reset_pin(GPIO_NUM_16);
      break;
    case EthClockMode_t::Int_50MHz_GPIO_17_inv: // ETH_CLOCK_GPIO17_OUT
      gpio_reset_pin(GPIO_NUM_17);
      break;
  }
  */
  delay(1);
}

bool ETHConnected() {
  if (EthEventData.EthServicesInitialized()) {
    if (EthLinkUp()) {
      return true;
    }
    // Apparently we missed an event
    EthEventData.processedDisconnect = false;
  } else if (EthEventData.ethInitSuccess) {
    if (EthLinkUp()) {
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

#endif // if FEATURE_ETHERNET