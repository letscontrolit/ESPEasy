#ifndef DATASTRUCTS_FACTORYDEFAULT_CDN_CUSTOMURL_NVS_H
#define DATASTRUCTS_FACTORYDEFAULT_CDN_CUSTOMURL_NVS_H


#include "../../ESPEasy_common.h"

#ifdef ESP32
# if FEATURE_ALTERNATIVE_CDN_URL

#  include "../Helpers/ESPEasy_NVS_Helper.h"

class FactoryDefault_CDN_customurl_NVS {
public:

  static bool applyToSettings_from_NVS(ESPEasy_NVS_Helper& preferences);

  static void fromSettings_to_NVS(ESPEasy_NVS_Helper& preferences);

  static void clear_from_NVS(ESPEasy_NVS_Helper& preferences);
};

# endif // if FEATURE_ALTERNATIVE_CDN_URL
#endif // ifdef ESP32

#endif // ifndef DATASTRUCTS_FACTORYDEFAULT_CDN_CUSTOMURL_NVS_H
