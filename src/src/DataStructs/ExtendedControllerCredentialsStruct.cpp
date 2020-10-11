#include "ExtendedControllerCredentialsStruct.h"

#include "../../ESPEasy_fdwdecl.h"

#include "../Helpers/ESPEasy_Storage.h"

#define EXT_CONTR_CRED_USER_OFFSET 0
#define EXT_CONTR_CRED_PASS_OFFSET 1


ExtendedControllerCredentialsStruct::ExtendedControllerCredentialsStruct() {}


String ExtendedControllerCredentialsStruct::load()
{
  return LoadStringArray(SettingsType::Enum::ExtdControllerCredentials_Type,
                           0,
                           _strings, CONTROLLER_MAX * 2, 0);
}

String ExtendedControllerCredentialsStruct::save() const
{
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
