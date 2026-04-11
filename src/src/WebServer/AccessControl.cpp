#include "../WebServer/AccessControl.h"

#include "../DataTypes/FormSelectorOptions.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../../ESPEasy/net/ESPEasyNetwork.h"
#include "../../ESPEasy/net/wifi/ESPEasyWifi.h"


#include "../Globals/SecuritySettings.h"
#include "../Globals/Services.h"

#include "../Helpers/Networking.h"
#include "../Helpers/StringConverter.h"

#include "../WebServer/Markup.h"

#include "../../ESPEasy/net/Helpers/NWAccessControl.h"



bool clientIPinSubnetDefaultNetwork() {
  return NWPlugin::ipInRange(
    web_server.client().remoteIP(), 
    ESPEasy::net::NetworkID(), 
    ESPEasy::net::NetworkBroadcast());
}

bool clientIPallowed()
{
  #if ESP_IDF_VERSION_MAJOR>=5
  // FIXME TD-er: remoteIP() is reporting incorrect value
//  return true;
  #endif
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
  if (ESPEasy::net::ipInAllowedSubnet(remoteIP)) {
    return true;
  }

  if ( ESPEasy::net::wifi::WifiIsAP(WiFi.getMode())) {
    // @TD-er Fixme: Should match subnet of SoftAP.
    return true;
  }
  String response = concat(F("IP blocked: "), formatIP(remoteIP));
  web_server.send(403, F("text/html"), response);

  if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
    response += F(" Allowed: ");
    response += ESPEasy::net::describeAllowedIPrange();
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
