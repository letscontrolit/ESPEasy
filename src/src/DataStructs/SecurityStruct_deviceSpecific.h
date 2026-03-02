#pragma once

#include "../../ESPEasy_common.h"

#if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE


# define MAX_EXTRA_WIFI_CREDENTIALS_SEPARATE_FILE   3

# include "../Helpers/_ESPEasy_key_value_store.h"

class SecurityStruct_deviceSpecific
{
public:

  // Indices used to create key in storage, so do not change values
  enum class KeyType {
    Controller_User     = 0,
    Controller_Password = 1,

    WiFi_SSID     = 8,
    WiFi_Password = 9,

    // Left some gap in numbers, so we can later add other WiFi specifics like for enterprise wifi
    // Having a block, makes it easier later to implement a faster check using a bitmask


    Pin = 32,
    APN = 33

  };

  static int                        maxLength(KeyType keyType);

  static bool                       isPassword(KeyType keyType);

  static const __FlashStringHelper* toString(KeyType keyType);


  SecurityStruct_deviceSpecific() = default;

  bool load();

  bool save();

  void clear();

  void clearWiFiCredentials();

  bool hasWiFiCredentials() const;

  bool hasWiFiCredentials(uint8_t index) const;

  // Move all set WiFi credentials to free up first entry
  // Typically used for setup page.
//  void freeUpFirstWiFiCredentials(uint8_t index = 0);

  bool getWiFiCredentials(uint8_t index,
                          String& ssid,
                          String& passwd) const
  {
    return
      index < MAX_EXTRA_WIFI_CREDENTIALS_SEPARATE_FILE &&
      getCredentials(KeyType::WiFi_SSID, KeyType::WiFi_Password, index, ssid, passwd);
  }

  void setWiFiCredentials(
    uint8_t       index,
    const String& ssid,
    const String& passwd)
  {
    if (index < MAX_EXTRA_WIFI_CREDENTIALS_SEPARATE_FILE)
    {
      setCredentials(
        KeyType::WiFi_SSID,
        KeyType::WiFi_Password,
        index,
        ssid,
        passwd);
    }
  }

  bool getControllerCredentials(uint8_t index,
                                String& user,
                                String& passwd) const
  {
    return
      (index < CONTROLLER_MAX) && getCredentials(
      KeyType::Controller_User,
      KeyType::Controller_Password,
      index,
      user,
      passwd);
  }

  void setControllerCredentials(uint8_t       index,
                                const String& user,
                                const String& passwd)
  {
    if (index < CONTROLLER_MAX) {
      setCredentials(
        KeyType::Controller_User,
        KeyType::Controller_Password,
        index,
        user,
        passwd);
    }
  }

  bool getValue(KeyType keytype,
                uint8_t index,
                String& value) const;
  void setValue(KeyType       keytype,
                uint8_t       index,
                const String& value);

  // Used for ESPEasy_key_value_store_webform
  ESPEasy_key_value_store* _getKVS() { return &_kvs; }

  static uint32_t          createKey(KeyType  keytype,
                                     uint16_t index);

private:

  bool getCredentials(KeyType keytype_key,
                      KeyType keytype_secret,
                      uint8_t index,
                      String& key,
                      String& secret) const;
  void setCredentials(KeyType       keytype_key,
                      KeyType       keytype_secret,
                      uint8_t       index,
                      const String& key,
                      const String& secret);


  ESPEasy_key_value_store _kvs;

}; // class SecurityStruct_deviceSpecific

#endif // if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE
