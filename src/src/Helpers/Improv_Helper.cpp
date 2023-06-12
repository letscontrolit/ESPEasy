#include "../Helpers/Improv_Helper.h"

#if FEATURE_IMPROV
# include "../CustomBuild/CompiletimeDefines.h"
# include "../ESPEasyCore/ESPEasyNetwork.h"
# include "../Helpers/StringGenerator_System.h"

void OnImprovError(ImprovTypes::Error error)
{}

void OnImprovConnected(const char *ssid, const char *password)
{}

bool OnImprovESPEasyConnectWiFi(const char *ssid, const char *password)
{
  return false;
}

Improv_Helper_t::Improv_Helper_t() {}

void Improv_Helper_t::init()
{
  // Set callback functions
  _improv.onImprovError(OnImprovError);
  _improv.onImprovConnected(OnImprovConnected);
  _improv.setCustomConnectWiFi(OnImprovESPEasyConnectWiFi);

  String firmwareName      = get_binary_filename();
  const String buildString = getSystemBuildString();

  firmwareName.replace(F("ESP_Easy_mega_"), F(""));
  firmwareName.replace(buildString,         F(""));
  firmwareName.replace('_',                 ' ');

  // Remove chip variant from the name
  const int pos = firmwareName.indexOf(F("ESP"));
  if (pos != -1) {
    const int endpos = firmwareName.indexOf(' ', pos);
    if (endpos != -1) {
      firmwareName.replace(firmwareName.substring(pos, endpos), F(""));
    }
  }
  firmwareName.replace(F("  "), F(" "));
  firmwareName.trim();

  _improv.setDeviceInfo(
    firmwareName.c_str(),
    buildString.c_str(),
    NetworkGetHostNameFromSettings().c_str());

  String chipModel = getChipModel();

//  chipModel += F(" rev ");
//  chipModel += getChipRevision();
  chipModel += '/';
  chipModel += (getFlashRealSizeInBytes() >> 20);
  chipModel += 'M';
  chipModel += ' ';
  chipModel += getChipFeaturesString();

  _improv.setDeviceChipInfo(chipModel.c_str());
}

bool Improv_Helper_t::handle(uint8_t b, Stream *serialForWrite)
{
  return _improv.handleSerial(b, serialForWrite);
}

#endif // if FEATURE_IMPROV
