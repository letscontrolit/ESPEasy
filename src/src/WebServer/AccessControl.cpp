#include "../WebServer/AccessControl.h"

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
  unsigned long u_ip = ((unsigned long )ip[0] << 24) | ((unsigned long )ip[1] << 16) | ((unsigned long )ip[2] << 8) | ip[3];
  unsigned long u_high = ((unsigned long )high[0] << 24) | ((unsigned long )high[1] << 16) | ((unsigned long )high[2] << 8) | high[3];
  return u_ip <= u_high;
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
      low  = SecuritySettings.AllowedIPrangeLow;
      high = SecuritySettings.AllowedIPrangeHigh;
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
  WiFiClient client(web_server.client());
  return ipInRange(client.remoteIP(), low, high);
}

boolean clientIPallowed()
{
  // TD-er Must implement "safe time after boot"
  IPAddress low, high;

  if (!getIPallowedRange(low, high))
  {
    // No subnet range determined, cannot filter on IP
    return true;
  }
  WiFiClient client(web_server.client());

  if (ipInRange(client.remoteIP(), low, high)) {
    return true;
  }

  if (WifiIsAP(WiFi.getMode())) {
    // @TD-er Fixme: Should match subnet of SoftAP.
    return true;
  }
  String response = F("IP blocked: ");
  response += formatIP(client.remoteIP());
  web_server.send(403, F("text/html"), response);

  if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
    response += F(" Allowed: ");
    response += formatIP(low);
    response += F(" - ");
    response += formatIP(high);
    addLog(LOG_LEVEL_ERROR, response);
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
  String options[3] = { F("Allow All"), F("Allow Local Subnet"), F("Allow IP range") };

  addSelector(name, 3, options, NULL, NULL, choice);
}
