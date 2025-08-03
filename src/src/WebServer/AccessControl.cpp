#include "../WebServer/AccessControl.h"

#include "../DataTypes/FormSelectorOptions.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"

#include "../Globals/SecuritySettings.h"
#include "../Globals/Services.h"

#include "../Helpers/Networking.h"
#include "../Helpers/StringConverter.h"

#include "../WebServer/Markup.h"

// ********************************************************************************
// Allowed IP range check
// ********************************************************************************

boolean ipLessEqual(const IPAddress& ip, const IPAddress& high)
{
  // FIXME TD-er: Must check whether both are of same type and check full range IPv6
  for (int i = 0; i < 4; ++i) {
    if (ip[i] != high[i]) {
      return ip[i] < high[i];
    }
  }
  // Is equal
  return true;
}

boolean ipInRange(const IPAddress& ip, const IPAddress& low, const IPAddress& high)
{
  return ipLessEqual(low, ip) && ipLessEqual(ip, high);
}

String describeAllowedIPrange() {
  String reply;

  switch (SecuritySettings.IPblockLevel) {
    case ALL_ALLOWED:
      reply +=  F("All Allowed");
      break;
    default:
    {
      IPAddress low, high;
      getIPallowedRange(low, high);
      reply +=  formatIP(low);
      reply +=  F(" - ");
      reply +=  formatIP(high);
    }
  }
  return reply;
}

bool getIPallowedRange(IPAddress& low, IPAddress& high)
{
  switch (SecuritySettings.IPblockLevel) {
    case LOCAL_SUBNET_ALLOWED:

      if (WifiIsAP(WiFi.getMode())) {
        // WiFi is active as accesspoint, do not check.
        return false;
      }
      return getSubnetRange(low, high);
    case ONLY_IP_RANGE_ALLOWED:
      low  = IPAddress(SecuritySettings.AllowedIPrangeLow);
      high = IPAddress(SecuritySettings.AllowedIPrangeHigh);
      break;
    default:
      low  = IPAddress(0, 0, 0, 0);
      high = IPAddress(255, 255, 255, 255);
      return false;
  }
  return true;
}

bool clientIPinSubnet() {
  IPAddress low, high;

  if (!getSubnetRange(low, high)) {
    // Could not determine subnet.
    return false;
  }
  return ipInRange(web_server.client().remoteIP(), low, high);
}

boolean clientIPallowed()
{
  #if ESP_IDF_VERSION_MAJOR>=5
  // FIXME TD-er: remoteIP() is reporting incorrect value
//  return true;
  #endif
  // TD-er Must implement "safe time after boot"
  IPAddress low, high;

  if (!getIPallowedRange(low, high))
  {
    // No subnet range determined, cannot filter on IP
    return true;
  }
  const IPAddress remoteIP = web_server.client().remoteIP();
  if (remoteIP == IPAddress(0, 0, 0, 0) 
  #if ESP_IDF_VERSION_MAJOR>=5
  || remoteIP.type() == IPv6
  #else
  || !remoteIP.isV4()
  #endif
  ) {
    // FIXME TD-er: Must see what's going on here, why the client doesn't send remote IP for some reason
    return true;
  }

  if (ipInRange(remoteIP, low, high)) {
    return true;
  }

  if (WifiIsAP(WiFi.getMode())) {
    // @TD-er Fixme: Should match subnet of SoftAP.
    return true;
  }
  String response = F("IP blocked: ");
  response += formatIP(remoteIP);
  web_server.send(403, F("text/html"), response);

  if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
    response += F(" Allowed: ");
    response += formatIP(low);
    response += F(" - ");
    response += formatIP(high);
    addLogMove(LOG_LEVEL_ERROR, response);
  }
  return false;
}

void clearAccessBlock()
{
  SecuritySettings.IPblockLevel = ALL_ALLOWED;
}

// ********************************************************************************
// Add a IP Access Control select dropdown list
// ********************************************************************************
void addIPaccessControlSelect(const String& name, int choice)
{
  const __FlashStringHelper *  options[] = { F("Allow All"), F("Allow Local Subnet"), F("Allow IP range") };

  const FormSelectorOptions selector(NR_ELEMENTS(options), options);
  selector.addSelector(name, choice);
}
