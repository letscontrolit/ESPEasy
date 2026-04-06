#include "../Helpers/_NWPlugin_Helper_webform.h"

#include "../../../ESPEasy_common.h"

#include "../../../src/DataStructs/ESPEasy_EventStruct.h"
#include "../../../src/DataTypes/ESPEasy_plugin_functions.h"
#include "../../../src/Globals/SecuritySettings.h"
#include "../../../src/Globals/Settings.h"
#include "../../../src/Helpers/Networking.h"
#include "../../../src/WebServer/ESPEasy_WebServer.h"
#include "../../../src/WebServer/Markup.h"
#include "../../../src/WebServer/Markup_Forms.h"
#include "../Globals/NWPlugins.h"
#include "../Helpers/_NWPlugin_init.h"

namespace ESPEasy {
namespace net {

/*********************************************************************************************\
* Functions to load and store network settings on the web page.
\*********************************************************************************************/
const __FlashStringHelper* toString(NetworkSettingsStruct::VarType parameterIdx, bool displayName)
{
  switch (parameterIdx)
  {
    case NetworkSettingsStruct::NETWORK_IP:                       return F("IP");
    case NetworkSettingsStruct::NETWORK_PORT:                     return F("Port");
    case NetworkSettingsStruct::NETWORK_USER:                     return F("User");
    case NetworkSettingsStruct::NETWORK_PASS:                     return F("Password");
    case NetworkSettingsStruct::NETWORK_ENABLED:

      if (displayName) { return F("Enabled"); }
      else {             return F("networkenabled"); }


    default:
      return F("Undefined");
  }
}

String getNetworkParameterName(networkDriverIndex_t           NetworkDriverIndex,
                               NetworkSettingsStruct::VarType parameterIdx,
                               bool                           displayName,
                               bool                         & isAlternative) {
  String name;

  if (displayName) {
    EventStruct tmpEvent;
    tmpEvent.idx = parameterIdx;

    // Only Network Driver specific call, so may call do_NWPluginCall directly 
    if (do_NWPluginCall(NetworkDriverIndex, NWPlugin::Function::NWPLUGIN_GET_PARAMETER_DISPLAY_NAME, &tmpEvent, name)) {
      // Found an alternative name for it.
      isAlternative = true;
      return name;
    }
  }
  isAlternative = false;

  name = toString(parameterIdx, displayName);

  if (!displayName) {
    // Change name to lower case and remove spaces to make it an internal name.
    name.toLowerCase();
    removeChar(name, ' ');
  }
  return name;
}

String getNetworkParameterInternalName(networkDriverIndex_t NetworkDriverIndex, NetworkSettingsStruct::VarType parameterIdx) {
  bool isAlternative; // Dummy, not needed for internal name
  bool displayName = false;

  return getNetworkParameterName(NetworkDriverIndex, parameterIdx, displayName, isAlternative);
}

String getNetworkParameterDisplayName(networkDriverIndex_t           NetworkDriverIndex,
                                      NetworkSettingsStruct::VarType parameterIdx,
                                      bool                         & isAlternative) {
  bool displayName = true;

  return getNetworkParameterName(NetworkDriverIndex, parameterIdx, displayName, isAlternative);
}

void addNetworkEnabledForm(networkIndex_t networkindex) {
  networkDriverIndex_t NetworkDriverIndex = getNetworkDriverIndex_from_NetworkIndex(networkindex);

  if (!validNetworkDriverIndex(NetworkDriverIndex)) {
    return;
  }

  NetworkSettingsStruct::VarType varType = NetworkSettingsStruct::NETWORK_ENABLED;

  bool isAlternativeDisplayName = false;
  const String displayName      = getNetworkParameterDisplayName(NetworkDriverIndex, varType, isAlternativeDisplayName);
  const String internalName     = getNetworkParameterInternalName(NetworkDriverIndex, varType);
  addFormCheckBox(displayName, internalName, Settings.getNetworkEnabled(networkindex));
}

void addNetworkParameterForm(const NetworkSettingsStruct  & NetworkSettings,
                             networkIndex_t                 networkindex,
                             NetworkSettingsStruct::VarType varType) {
  networkDriverIndex_t NetworkDriverIndex = getNetworkDriverIndex_from_NetworkIndex(networkindex);

  if (!validNetworkDriverIndex(NetworkDriverIndex)) {
    return;
  }

  bool isAlternativeDisplayName = false;
  const String displayName      = getNetworkParameterDisplayName(NetworkDriverIndex, varType, isAlternativeDisplayName);
  const String internalName     = getNetworkParameterInternalName(NetworkDriverIndex, varType);

  switch (varType)
  {
    case NetworkSettingsStruct::NETWORK_IP:
    {
      addFormIPBox(displayName, internalName, NetworkSettings.IP);
      break;
    }
    case NetworkSettingsStruct::NETWORK_PORT:
    {
      addFormNumericBox(displayName, internalName, NetworkSettings.Port, 1, 65535);
      break;
    }

    case NetworkSettingsStruct::NETWORK_USER:
    {
      /*
            const size_t fieldMaxLength =
              NetworkSettings.useExtendedCredentials() ? EXT_SECURITY_MAX_USER_LENGTH : sizeof(SecuritySettings.NetworkUser[0]) - 1;
            addFormTextBox(displayName,
                           internalName,
                           getNetworkUser(networkindex, NetworkSettings, false),
                           fieldMaxLength);
       */
      break;
    }
    case NetworkSettingsStruct::NETWORK_PASS:
    {
      /*
            const size_t fieldMaxLength =
              NetworkSettings.useExtendedCredentials() ? EXT_SECURITY_MAX_PASS_LENGTH : sizeof(SecuritySettings.NetworkPassword[0]) - 1;

            if (isAlternativeDisplayName) {
              // It is not a regular password, thus use normal text field.
              addFormTextBox(displayName, internalName,
                             getNetworkPass(networkindex, NetworkSettings),
                             fieldMaxLength);
            } else {
              addFormPasswordBox(displayName, internalName,
                                 getNetworkPass(networkindex, NetworkSettings),
                                 fieldMaxLength);
            }
       */
      break;
    }

    case NetworkSettingsStruct::NETWORK_TIMEOUT:
    {
      break;
    }

    case NetworkSettingsStruct::NETWORK_ENABLED:
      addFormCheckBox(displayName, internalName, Settings.getNetworkEnabled(networkindex));
      break;
  }
}

void saveNetworkParameterForm(NetworkSettingsStruct        & NetworkSettings,
                              networkIndex_t                 networkindex,
                              NetworkSettingsStruct::VarType varType) {
  const networkDriverIndex_t NetworkDriverIndex =
    getNetworkDriverIndex_from_NetworkIndex(networkindex);

  if (!validNetworkDriverIndex(NetworkDriverIndex)) {
    return;
  }
  const String internalName = getNetworkParameterInternalName(NetworkDriverIndex, varType);

  switch (varType)
  {

    case NetworkSettingsStruct::NETWORK_IP:
      /*
            if (!NetworkSettings.UseDNS)
            {
              str2ip(webArg(internalName), NetworkSettings.IP);
            }
       */
      break;
    case NetworkSettingsStruct::NETWORK_PORT:
      NetworkSettings.Port = getFormItemInt(internalName, NetworkSettings.Port);
      break;

    case NetworkSettingsStruct::NETWORK_USER:
      //      setNetworkUser(networkindex, NetworkSettings, webArg(internalName));
      break;
    case NetworkSettingsStruct::NETWORK_PASS:
    {
      String password;

      if (getFormPassword(internalName, password)) {
        //        setNetworkPass(networkindex, NetworkSettings, password);
      }
      break;
    }

    case NetworkSettingsStruct::NETWORK_TIMEOUT:
    {
      break;
    }

    case NetworkSettingsStruct::NETWORK_ENABLED:
      Settings.setNetworkEnabled(networkindex, isFormItemChecked(internalName));
#ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO,
             concat(F("Save NW_Enabled: "),
                    internalName) + ' ' + isFormItemChecked(internalName) + ' ' + Settings.getNetworkEnabled(networkindex));
#endif
      break;
  }
}

} // namespace net
} // namespace ESPEasy
