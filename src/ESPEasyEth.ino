
#ifdef HAS_ETHERNET
  #define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
  #define ETH_PHY_POWER 12
  #include <ETH.h>


String EthGetHostname()
{
  String hostnameToReturn(Settings.getHostname());
  hostnameToReturn.replace(" ", "-");
  hostnameToReturn.replace("_", "-"); // See RFC952
  return hostnameToReturn;
}

bool prepareEth() {
  char hostname[40];
  safe_strncpy(hostname, EthGetHostname(), sizeof(hostname));
  ETH.setHostname(hostname);
  return true;
}

void ETHConnectRelaxed() {
  // if (!ethConnectAttemptNeeded) {
  //   return; // already connected or connect attempt in progress need to disconnect first
  // }
  if (!prepareEth()) {
    // Dead code for now...
    addLog(LOG_LEVEL_ERROR, F("ETH : Could not prepare ETH!"));
    return;
  }
  ETH.begin();
}

#endif