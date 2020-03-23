
#ifdef HAS_ETHERNET

#include "ETH.h"

String ethGetHostname() {
  String hostnameToReturn(Settings.getHostname());
  hostnameToReturn.replace(" ", "-");
  hostnameToReturn.replace("_", "-"); // See RFC952
  return hostnameToReturn;
}

bool ethCheckSettings() {
  bool result = true;
  if (Settings.ETH_Phy_Type != 0 && Settings.ETH_Phy_Type != 1)
    result = false;
  if (Settings.ETH_Clock_Mode > 3)
    result = false;
  if (Settings.ETH_Pin_mdc > MAX_GPIO)
    result = false;
  if (Settings.ETH_Pin_mdio > MAX_GPIO)
    result = false;
  if (Settings.ETH_Pin_power > MAX_GPIO)
    result = false;
  return result;
}

bool ethPrepare() {
  if (!ethCheckSettings())
  {
    addLog(LOG_LEVEL_ERROR, F("ETH: Settings not correct!!!"));
    return false;
  }
  ETH.setHostname(ethGetHostname().c_str());
  return true;
}

String ethGetDebugClockModeStr() {
  switch (Settings.ETH_Clock_Mode)
  {
    case 0: return F("ETH_CLOCK_GPIO0_IN");
    case 1: return F("ETH_CLOCK_GPIO0_OUT");
    case 2: return F("ETH_CLOCK_GPIO16_OUT");
    case 3: return F("ETH_CLOCK_GPIO17_OUT");
    default: return F("ETH_CLOCK_ERR");
  }
}

void ethPrintSettings() {
  String settingsDebugLog;
  settingsDebugLog.reserve(115);
  settingsDebugLog += F("ETH: PHY Type: ");
  settingsDebugLog += Settings.ETH_Phy_Type == 0 ? F("ETH_PHY_LAN8720") : F("ETH_PHY_TLK110");
  settingsDebugLog += F(" PHY Addr: ");
  settingsDebugLog += Settings.ETH_Phy_Addr;
  settingsDebugLog += F(" Eth Clock mode: ");
  settingsDebugLog += ethGetDebugClockModeStr();
  settingsDebugLog += F(" MDC Pin: ");
  settingsDebugLog += String(Settings.ETH_Pin_mdc);
  settingsDebugLog += F(" MIO Pin: ");
  settingsDebugLog += String(Settings.ETH_Pin_mdio);
  settingsDebugLog += F(" Power Pin: ");
  settingsDebugLog += String(Settings.ETH_Pin_power);
  addLog(LOG_LEVEL_INFO, settingsDebugLog);
}

void ETHConnectRelaxed() {
  ethPrintSettings();
  if (!ethPrepare()) {
    // Dead code for now...
    addLog(LOG_LEVEL_ERROR, F("ETH : Could not prepare ETH!"));
    return;
  }
  ETH.begin(Settings.ETH_Phy_Addr,
            Settings.ETH_Pin_power,
            Settings.ETH_Pin_mdc,
            Settings.ETH_Pin_mdio,
            (eth_phy_type_t)Settings.ETH_Phy_Type,
            (eth_clock_mode_t)Settings.ETH_Clock_Mode);
}

#endif