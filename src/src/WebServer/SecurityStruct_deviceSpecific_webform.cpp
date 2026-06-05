#include "../WebServer/SecurityStruct_deviceSpecific_webform.h"

#if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE

# include "../Globals/SecuritySettings.h"
# include "../Helpers/StringConverter.h"
# include "../WebServer/Markup_Forms.h"

String DevSpecific_Security_getLabelString(SecurityStruct_deviceSpecific::KeyType keytype,
                                           uint16_t                               index,
                                           bool                                   displayString,
                                           KVS_StorageType::Enum & storageType,
                                           bool                                   suppressIndex_displayString = false)
{
  storageType = KVS_StorageType::Enum::string_type;
  String res = SecurityStruct_deviceSpecific::toString(keytype);

  if ((keytype == SecurityStruct_deviceSpecific::KeyType::WiFi_SSID) ||
      (keytype == SecurityStruct_deviceSpecific::KeyType::WiFi_Password)) {
    if (displayString) {
      if (!suppressIndex_displayString) {
        res += ' ';
        res += index + 1;
      }
    } else {
      res += index;
    }
  }

  if (!displayString) {
    res.toLowerCase();
    res.replace(' ', '_');
    return concat(F("KVS_ID_"), res);
  }
  return res;
}

WebFormItemParams make_DevSpecific_Security_WebFormItemParams(
  SecurityStruct_deviceSpecific::KeyType keytype,
  uint16_t                               index,
  bool                                   suppressIndex_displayString)
{
  KVS_StorageType::Enum storageType;

  auto res = WebFormItemParams(
    DevSpecific_Security_getLabelString(
      keytype, index, true,  storageType, suppressIndex_displayString),
    DevSpecific_Security_getLabelString(
      keytype, index, false, storageType, suppressIndex_displayString),
    storageType,
    SecurityStruct_deviceSpecific::createKey(keytype, index));

  res._password  = SecurityStruct_deviceSpecific::isPassword(keytype);
  res._maxLength = SecurityStruct_deviceSpecific::maxLength(keytype);
  return res;
}

void show_SecurityStruct_deviceSpecific_WebFormItem(
  SecurityStruct_deviceSpecific::KeyType keytype,
  uint16_t                               index,
  bool                                   suppressIndex_displayString)
{
  showWebformItem(
    *SecuritySettings_deviceSpecific._getKVS(),
    make_DevSpecific_Security_WebFormItemParams(keytype, index, suppressIndex_displayString));
}

void store_SecurityStruct_deviceSpecific_WebFormItem(
  SecurityStruct_deviceSpecific::KeyType keytype,
  uint16_t                               index)
{
  KVS_StorageType::Enum storageType;

  auto id =
    DevSpecific_Security_getLabelString(keytype, index, false, storageType);

  if (SecurityStruct_deviceSpecific::isPassword(keytype)) {
    String passwd;

    if (!getFormPassword(id, passwd)) {
      // Not changed, so do not try to store.
      return;
    }
  }

  storeWebformItem(
    *SecuritySettings_deviceSpecific._getKVS(),
    SecurityStruct_deviceSpecific::createKey(keytype, index),
    storageType,
    id);
}

#endif // if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE
