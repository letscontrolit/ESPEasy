#ifdef HAS_ETHERNET
void ETHEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      addLog(LOG_LEVEL_INFO, F("ETH Started"));
      char hostname[40];
      safe_strncpy(hostname, createRFCCompliantHostname(WifiGetAPssid()).c_str(), sizeof(hostname));
      ETH.setHostname(hostname);
      {
        String log = F("ETH Hostname: ");
        log += String(hostname);
        addLog(LOG_LEVEL_INFO, log);
      }
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      addLog(LOG_LEVEL_INFO, F("ETH Connected"));
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      {
        String log = F("ETH MAC: ");
        log += ETH.macAddress();
        log += F(", IPv4: ");
        log += ETH.localIP().toString();
        if (ETH.fullDuplex()) {
          log += F(", FULL_DUPLEX");
        }
        log += F(", ");
        log += ETH.linkSpeed();
        log += F("Mbps");
        addLog(LOG_LEVEL_INFO, log);
      }
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      addLog(LOG_LEVEL_ERROR, F("ETH Disconnected"));
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      addLog(LOG_LEVEL_INFO, F("ETH Stopped"));
      eth_connected = false;
      break;
    case SYSTEM_EVENT_GOT_IP6:
      addLog(LOG_LEVEL_INFO, F("ETH Got IP6"));
      break;
    default:
      break;
  }
}
#endif