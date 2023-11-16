#include "../Helpers/Improv_Helper.h"

#if FEATURE_IMPROV
# include "../CustomBuild/CompiletimeDefines.h"
# include "../ESPEasyCore/ESPEasyNetwork.h"
# include "../Globals/SecuritySettings.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Hardware_device_info.h"
# include "../Helpers/StringConverter.h"
# include "../Helpers/StringGenerator_System.h"


void OnImprovError(ImprovTypes::Error error)
{
  if ((error != ImprovTypes::Error::ERROR_NONE) &&
      loglevelActiveFor(LOG_LEVEL_ERROR)) {
    String log = F("IMPROV : ");

    switch (error) {
      case ImprovTypes::Error::ERROR_INVALID_RPC:       log += F("Invalid RPC"); break;
      case ImprovTypes::Error::ERROR_UNKNOWN_RPC:       log += F("Unkown RPC"); break;
      case ImprovTypes::Error::ERROR_UNABLE_TO_CONNECT: log += F("Unable to connect"); break;
      case ImprovTypes::Error::ERROR_NOT_AUTHORIZED:    log += F("Not Authorized"); break;
      case ImprovTypes::Error::ERROR_UNKNOWN:           log += F("Unknown"); break;
      default:
        return;
    }
    addLogMove(LOG_LEVEL_ERROR, log);
  }
}

void OnImprovConnected(const char *ssid, const char *password)
{
  safe_strncpy(
    SecuritySettings.WifiSSID,
    ssid,
    sizeof(SecuritySettings.WifiSSID2));
  safe_strncpy(
    SecuritySettings.WifiKey,
    password,
    sizeof(SecuritySettings.WifiKey));
  SaveSettings();
}

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

  // FIXME TD-er: Implement callback to use ESPEasy functions to connect to WiFi
  //  _improv.setCustomTryConnectToWiFi(OnImprovESPEasyConnectWiFi);

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

  // TD-er: Removed chip features description as it is just too much info.

  /*
   # ifdef ESP32
     chipModel += ' ';
     chipModel += getChipFeaturesString();
   # endif // ifdef ESP32
   */
  _improv.setDeviceChipInfo(chipModel.c_str());
}

bool Improv_Helper_t::handle(uint8_t b, Stream *serialForWrite)
{
  #ifdef USE_SECOND_HEAP
  // Do not store in 2nd heap, std::list cannot handle 2nd heap well
  HeapSelectDram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  _tmpbuffer.push_back(b);

  switch (_improv.handleSerial(b, serialForWrite)) {
    case ImprovTypes::ParseState::VALID_INCOMPLETE:
      _mustDumpBuffer = false;
      return true;
    case ImprovTypes::ParseState::VALID_COMPLETE:
      _tmpbuffer.clear();
      _mustDumpBuffer = false;
      return true;
    case ImprovTypes::ParseState::INVALID:
      //   _mustDumpBuffer = true;
      break;
  }
  _mustDumpBuffer = true;
  return false;
}

bool Improv_Helper_t::getFromBuffer(uint8_t& b)
{
  if (available() == 0) { return false; }
  b = _tmpbuffer.front();
  _tmpbuffer.pop_front();

  if (_tmpbuffer.size() == 0) {
    _mustDumpBuffer = false;
  }
  return true;
}

size_t Improv_Helper_t::available() const
{
  if (_mustDumpBuffer) { return _tmpbuffer.size(); }
  return 0u;
}

#endif // if FEATURE_IMPROV
