#include "../DataStructs/FactoryDefault_CDN_customurl_NVS.h"

#ifdef ESP32

# if FEATURE_ALTERNATIVE_CDN_URL
#  include "../Helpers/ESPEasy_Storage.h"

#  define FACTORY_DEFAULT_NVS_CDN_CUSTOMURL_KEY "CDN_customurl"

bool FactoryDefault_CDN_customurl_NVS::applyToSettings_from_NVS(ESPEasy_NVS_Helper& preferences)
{
  String _url;

  if (preferences.getPreference(F(FACTORY_DEFAULT_NVS_CDN_CUSTOMURL_KEY), _url)) {
    set_CDN_url_custom(_url);
    return true;
  }
  return false;
}

void FactoryDefault_CDN_customurl_NVS::fromSettings_to_NVS(ESPEasy_NVS_Helper& preferences)
{
  preferences.setPreference(F(FACTORY_DEFAULT_NVS_CDN_CUSTOMURL_KEY), get_CDN_url_custom());
}

void FactoryDefault_CDN_customurl_NVS::clear_from_NVS(ESPEasy_NVS_Helper& preferences)
{
  preferences.remove(F(FACTORY_DEFAULT_NVS_CDN_CUSTOMURL_KEY));
}

# endif // if FEATURE_ALTERNATIVE_CDN_URL
#endif  // ifdef ESP32
