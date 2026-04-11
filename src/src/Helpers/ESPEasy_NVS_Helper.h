#ifndef HELPERS_ESPEASY_NVS_HELPER_H
#define HELPERS_ESPEASY_NVS_HELPER_H

#include "../../ESPEasy_common.h"

#ifdef ESP32

// Store in NVS partition
# include <Preferences.h>


// Max. 15 char namespace for ESPEasy Factory Default settings
# define FACTORY_DEFAULT_NVS_NAMESPACE      "ESPEasyFacDef"
# define WIFI_CONNECTION_NVS_NAMESPACE      "ESPEasyWiFi"


class ESPEasy_NVS_Helper {
public:
  ESPEasy_NVS_Helper() = default;

  ~ESPEasy_NVS_Helper();

  bool begin(const String& nvs_namespace,
             bool          readOnly = false);

  void end();

  void remove(const String& key);

  bool getPreference(const String& key,
                     String      & value);
  void setPreference(const String& key,
                     const String& value);

  bool getPreference(const String& key,
                     uint32_t    & value);
  void setPreference(const String  & key,
                     const uint32_t& value);

  bool getPreference(const String& key,
                     uint64_t    & value);
  void setPreference(const String  & key,
                     const uint64_t& value);

  bool getPreference(const String& key,
                     uint8_t      *value,
                     size_t        length);
  void setPreference(const String & key,
                     const uint8_t *value,
                     size_t         length);

private:

  Preferences _preferences;
};


#endif // ifdef ESP32
#endif // ifndef HELPERS_ESPEASY_NVS_HELPER_H
