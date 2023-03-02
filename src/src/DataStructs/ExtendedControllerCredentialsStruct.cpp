#include "../DataStructs/ExtendedControllerCredentialsStruct.h"

#include "../Helpers/ESPEasy_Storage.h"

#define EXT_CONTR_CRED_USER_OFFSET 0
#define EXT_CONTR_CRED_PASS_OFFSET 1


ChecksumType last_ExtendedControllerCredentialsStruct_md5;


ExtendedControllerCredentialsStruct::ExtendedControllerCredentialsStruct() {}

bool ExtendedControllerCredentialsStruct::validateChecksum() const
{
  const ChecksumType tmp_checksum(_strings, CONTROLLER_MAX * 2);
  if (tmp_checksum == last_ExtendedControllerCredentialsStruct_md5) {
    return true;
  }
  // Data has changed, copy computed checksum
  last_ExtendedControllerCredentialsStruct_md5 = tmp_checksum;
  return false;
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
  validateChecksum();

  return res;
}

String ExtendedControllerCredentialsStruct::save() const
{
  if (validateChecksum()) {
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
  return EMPTY_STRING;
}

String ExtendedControllerCredentialsStruct::getControllerPass(controllerIndex_t controller_idx) const
{
  if (validControllerIndex(controller_idx)) {
    return _strings[controller_idx * 2 + EXT_CONTR_CRED_PASS_OFFSET];
  }
  return EMPTY_STRING;
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
