#include "../DataStructs/SecurityStruct_deviceSpecific.h"

#if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE

# include "../../ESPEasy/net/wifi/ESPEasyWifi.h"

int SecurityStruct_deviceSpecific::maxLength(KeyType keyType)
{
  switch (keyType)
  {
    case KeyType::Controller_User:     return 128;
    case KeyType::Controller_Password: return 128;
    case KeyType::WiFi_SSID:           return 31;
    case KeyType::WiFi_Password:       return 63;
    case KeyType::Pin:                 return 4;
    case KeyType::APN:                 return 64;
  }
  return 0;
}

bool SecurityStruct_deviceSpecific::isPassword(KeyType keyType)
{
  switch (keyType)
  {
    case KeyType::Controller_Password:
    case KeyType::WiFi_Password:
    case KeyType::Pin:
      return true;
    default: break;
  }
  return false;
}

const __FlashStringHelper * SecurityStruct_deviceSpecific::toString(KeyType keyType)
{
  switch (keyType)
  {
    case KeyType::Controller_User:     return F("Controller User");
    case KeyType::Controller_Password: return F("Controller Password");
    case KeyType::WiFi_SSID:           return F("SSID");
    case KeyType::WiFi_Password:       return F("WPA Key");
    case KeyType::Pin:                 return F("Pin");
    case KeyType::APN:                 return F("APN");
  }
  return F("");
}

bool SecurityStruct_deviceSpecific::load()
{
  return _kvs.load(
    SettingsType::Enum::DeviceSpecificCredentials_type,
    0,
    0,
    0);
}

bool SecurityStruct_deviceSpecific::save()
{
  return _kvs.store(
    SettingsType::Enum::DeviceSpecificCredentials_type,
    0,
    0,
    0);
}

void SecurityStruct_deviceSpecific::clear() { _kvs.clear(); }

void SecurityStruct_deviceSpecific::clearWiFiCredentials()
{
  for (uint8_t i = 0; i < MAX_EXTRA_WIFI_CREDENTIALS_SEPARATE_FILE; ++i)
  {
    setWiFiCredentials(i, EMPTY_STRING, EMPTY_STRING);
  }
}

bool SecurityStruct_deviceSpecific::hasWiFiCredentials() const
{
  for (uint8_t i = 0; i < MAX_EXTRA_WIFI_CREDENTIALS_SEPARATE_FILE; ++i)
  {
    if (hasWiFiCredentials(i)) { return true; }
  }
  return false;
}

bool SecurityStruct_deviceSpecific::hasWiFiCredentials(uint8_t index) const
{
  String ssid, passwd;
  return getWiFiCredentials(index, ssid, passwd);
}

bool SecurityStruct_deviceSpecific::getWiFiCredentials(uint8_t index,
                                                       String& ssid,
                                                       String& passwd) const
{
  if (index >= MAX_EXTRA_WIFI_CREDENTIALS_SEPARATE_FILE) { return false; }

  if (!getCredentials(KeyType::WiFi_SSID, KeyType::WiFi_Password, index, ssid, passwd)) { return false; }
  return ESPEasy::net::wifi::validWiFiCredentials(ssid, passwd);
}

void SecurityStruct_deviceSpecific::setWiFiCredentials(
  uint8_t       index,
  const String& ssid,
  const String& passwd)
{
  if (index < MAX_EXTRA_WIFI_CREDENTIALS_SEPARATE_FILE)
  {
    if (ESPEasy::net::wifi::validWiFiCredentials(ssid, passwd)) {
      setCredentials(
        KeyType::WiFi_SSID,
        KeyType::WiFi_Password,
        index,
        ssid,
        passwd);
    }
    else {
      _kvs.clearKey(
        KVS_StorageType::Enum::string_type,
        createKey(KeyType::WiFi_SSID, index));
      _kvs.clearKey(
        KVS_StorageType::Enum::string_type,
        createKey(KeyType::WiFi_Password, index));
    }
  }
}

bool SecurityStruct_deviceSpecific::getControllerCredentials(uint8_t index,
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

void SecurityStruct_deviceSpecific::setControllerCredentials(uint8_t       index,
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

/*
   void SecurityStruct_deviceSpecific::freeUpFirstWiFiCredentials(uint8_t index)
   {

   if (hasWiFiCredentials(index)) freeUpFirstWiFiCredentials(index + 1);
   else {

   }
   }
 */
bool SecurityStruct_deviceSpecific::getValue(
  KeyType keytype, uint8_t index, String& value) const
{
  return _kvs.getValue(createKey(keytype, index), value, KVS_StorageType::Enum::string_type);
}

void SecurityStruct_deviceSpecific::setValue(KeyType keytype, uint8_t index, const String& value)
{
  _kvs.setValue(createKey(keytype, index), value, KVS_StorageType::Enum::string_type);
}

uint32_t SecurityStruct_deviceSpecific::createKey(KeyType keytype, uint16_t index)
{
  return (static_cast<uint32_t>(keytype) & 0xFF) + (static_cast<uint32_t>(index) << 8);
}

bool SecurityStruct_deviceSpecific::getCredentials(KeyType keytype_key,
                                                   KeyType keytype_secret,
                                                   uint8_t index,
                                                   String& key,
                                                   String& secret) const
{
  return
    _kvs.getValue(createKey(keytype_key, index), key, KVS_StorageType::Enum::string_type) &&
    _kvs.getValue(createKey(keytype_secret, index), secret, KVS_StorageType::Enum::string_type);
}

void SecurityStruct_deviceSpecific::setCredentials(KeyType       keytype_key,
                                                   KeyType       keytype_secret,
                                                   uint8_t       index,
                                                   const String& key,
                                                   const String& secret)
{
  _kvs.setValue(createKey(keytype_key, index),    key,    KVS_StorageType::Enum::string_type);
  _kvs.setValue(createKey(keytype_secret, index), secret, KVS_StorageType::Enum::string_type);
}

#endif // if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE
