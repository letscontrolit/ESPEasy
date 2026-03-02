#include "../DataStructs/NetworkSettingsStruct.h"

#include "../../../ESPEasy_common.h"

#include "../../../src/CustomBuild/ESPEasyLimits.h"
#include "../../../src/ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyNetwork.h"
#include "../../../src/Helpers/Misc.h"
#include "../../../src/Helpers/Networking.h"
#include "../../../src/Helpers/StringConverter.h"


#include <IPAddress.h>
#include <WString.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

namespace ESPEasy {
namespace net {


NetworkSettingsStruct::NetworkSettingsStruct()
{
}

void NetworkSettingsStruct::reset() {
  // Need to make sure every byte between the members is also zero
  // Otherwise the checksum will fail and settings will be saved too often.
  memset(this, 0, sizeof(NetworkSettingsStruct));


}

void NetworkSettingsStruct::validate() {


}

UP_NetworkSettingsStruct MakeNetworkSettings()
{
  void *calloc_ptr = special_calloc(1, sizeof(NetworkSettingsStruct));
  UP_NetworkSettingsStruct T(new (calloc_ptr)  NetworkSettingsStruct());
  return T;
}

} // namespace net
} // namespace ESPEasy
