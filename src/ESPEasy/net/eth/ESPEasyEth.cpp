#include "../eth/ESPEasyEth.h"

#if FEATURE_ETHERNET

# include "../../../src/CustomBuild/ESPEasyLimits.h"
# include "../ESPEasyNetwork.h"
# include "../wifi/ESPEasyWifi.h"
# include "../../../src/ESPEasyCore/ESPEasy_Log.h"
# include "../../../src/ESPEasyCore/ESPEasyGPIO.h"
# include "../Globals/NetworkState.h"
# include "../../../src/Globals/Settings.h"
# include "../../../src/Helpers/Hardware_GPIO.h"
# include "../../../src/Helpers/StringConverter.h"
# include "../../../src/Helpers/Networking.h"

# include <ETH.h>
# include <lwip/dns.h>
# include <esp_eth_phy.h>

# include <WiFi.h>

namespace ESPEasy {
namespace net {
namespace eth {

bool ethUseStaticIP() { return Settings.ETH_IP[0] != 0 && Settings.ETH_IP[0] != 255; }

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

MAC_address ETHMacAddress() {
  MAC_address mac;

  /*
     if (!EthEventData.ethInitSuccess) {
      addLog(LOG_LEVEL_ERROR, F("Call NetworkMacAddress() only on connected Ethernet!"));
     } else {
      ETH.macAddress(mac.mac);
     }
   */
  return mac;
}


bool ETHConnected() {
  auto data = getFirst_Enabled_ETH_interface();

  if (data) { return data->connected(); }
  return false;
}

} // namespace eth
} // namespace net
} // namespace ESPEasy

#endif // if FEATURE_ETHERNET
