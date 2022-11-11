#include "../DataStructs/ExtendedControllerCredentialsStruct.h"

#include "../Helpers/ESPEasy_Storage.h"

#ifdef ESP32
# include <MD5Builder.h>
#endif // ifdef ESP32

#define EXT_CONTR_CRED_USER_OFFSET 0
#define EXT_CONTR_CRED_PASS_OFFSET 1


uint8_t last_ExtendedControllerCredentialsStruct_md5[16] = { 0 };


ExtendedControllerCredentialsStruct::ExtendedControllerCredentialsStruct() {}


bool ExtendedControllerCredentialsStruct::computeChecksum(uint8_t checksum[16]) const
{
  MD5Builder md5;

  md5.begin();

  for (size_t i = 0; i < CONTROLLER_MAX * 2; ++i) {
    md5.add(_strings[i].c_str());
  }

  md5.calculate();
  uint8_t tmp_md5[16] = { 0 };

  md5.getBytes(tmp_md5);

  if (memcmp(tmp_md5, checksum, 16) != 0) {
    // Data has changed, copy computed checksum
    memcpy(checksum, tmp_md5, 16);
    return false;
  }
  return true;
}

void ExtendedControllerCredentialsStruct::clear() {
  for (size_t i = 0; i < CONTROLLER_MAX * 2; ++i) {
    _strings[i] = String();
  }
}

String ExtendedControllerCredentialsStruct::load()
{
  const String res =
    LoadStringArray(SettingsType::Enum::ExtdControllerCredentials_Type,
                    0,
                    _strings, CONTROLLER_MAX * 2, 0);

  for (int i = 0; i < CONTROLLER_MAX * 2; ++i) {
    _strings[i].trim();
  }

  // Update the checksum after loading.
  computeChecksum(last_ExtendedControllerCredentialsStruct_md5);

  return res;
}

String ExtendedControllerCredentialsStruct::save() const
{
  if (computeChecksum(last_ExtendedControllerCredentialsStruct_md5)) {
    return EMPTY_STRING;
  }
  return SaveStringArray(SettingsType::Enum::ExtdControllerCredentials_Type,
                         0,
                         _strings, CONTROLLER_MAX * 2, 0);
}

String ExtendedControllerCredentialsStruct::getControllerUser(controllerIndex_t controller_idx) const
{
  if (validControllerIndex(controller_idx)) {
    return _strings[controller_idx * 2 + EXT_CONTR_CRED_USER_OFFSET];
  }
  return "";
}

String ExtendedControllerCredentialsStruct::getControllerPass(controllerIndex_t controller_idx) const
{
  if (validControllerIndex(controller_idx)) {
    return _strings[controller_idx * 2 + EXT_CONTR_CRED_PASS_OFFSET];
  }
  return "";
}

void ExtendedControllerCredentialsStruct::setControllerUser(controllerIndex_t controller_idx, const String& user)
{
  if (validControllerIndex(controller_idx)) {
    _strings[controller_idx * 2 + EXT_CONTR_CRED_USER_OFFSET] = user;
  }
}

void ExtendedControllerCredentialsStruct::setControllerPass(controllerIndex_t controller_idx, const String& pass)
{
  if (validControllerIndex(controller_idx)) {
    _strings[controller_idx * 2 + EXT_CONTR_CRED_PASS_OFFSET] = pass;
  }
}
