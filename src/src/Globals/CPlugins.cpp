#include "CPlugins.h"
#include "../../ESPEasy_plugindefs.h"
#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Globals/Protocol.h"
#include "../Globals/Settings.h"
#include "../../ESPEasy_Log.h"


protocolIndex_t   INVALID_PROTOCOL_INDEX   = CPLUGIN_MAX;
controllerIndex_t INVALID_CONTROLLER_INDEX = CONTROLLER_MAX;
cpluginID_t INVALID_C_PLUGIN_ID          = 0;

// FIXME TD-er: Make these private and add functions to access its content.
std::map<cpluginID_t, protocolIndex_t> CPlugin_id_to_ProtocolIndex;
std::vector<cpluginID_t> ProtocolIndex_to_CPlugin_id;

bool (*CPlugin_ptr[CPLUGIN_MAX])(byte,
                                 struct EventStruct *,
                                 String&);

bool validProtocolIndex(protocolIndex_t index)
{
  return (getCPluginID_from_ProtocolIndex(index) != INVALID_C_PLUGIN_ID);
}

bool validControllerIndex(controllerIndex_t index)
{
  return index < CONTROLLER_MAX;
}

bool validCPluginID(cpluginID_t cpluginID)
{
  if (cpluginID == INVALID_C_PLUGIN_ID) {
    return false;
  }
  auto it = CPlugin_id_to_ProtocolIndex.find(cpluginID);
  return (it != CPlugin_id_to_ProtocolIndex.end());
}

bool supportedCPluginID(cpluginID_t cpluginID)
{
  return validProtocolIndex(getProtocolIndex(cpluginID));
}

protocolIndex_t getProtocolIndex_from_ControllerIndex(controllerIndex_t index) {
  if (validControllerIndex(index)) {
    return getProtocolIndex(Settings.Protocol[index]);
  }
  return INVALID_PROTOCOL_INDEX;
}

cpluginID_t getCPluginID_from_ProtocolIndex(protocolIndex_t index) {
  if (index < CPLUGIN_MAX) {
    const cpluginID_t cpluginID = ProtocolIndex_to_CPlugin_id[index];
    return cpluginID;
  }
  return INVALID_C_PLUGIN_ID;
}

cpluginID_t getCPluginID_from_ControllerIndex(controllerIndex_t index) {
  const protocolIndex_t protocolIndex = getProtocolIndex_from_ControllerIndex(index);
  return getCPluginID_from_ProtocolIndex(protocolIndex);
}

protocolIndex_t getProtocolIndex(cpluginID_t cpluginID)
{
  if (cpluginID != INVALID_C_PLUGIN_ID) {
    auto it = CPlugin_id_to_ProtocolIndex.find(cpluginID);

    if (it != CPlugin_id_to_ProtocolIndex.end())
    {
      if (!validProtocolIndex(it->second)) { return INVALID_PROTOCOL_INDEX; }

      if (Protocol[it->second].Number != cpluginID) {
        // FIXME TD-er: Just a check for now, can be removed later when it does not occur.
        String log = F("getProtocolIndex error in Protocol Vector. CPluginID: ");
        log += String(cpluginID);
        log += F(" p_index: ");
        log += String(it->second);
        addLog(LOG_LEVEL_ERROR, log);
      }
      return it->second;
    }
  }
  return INVALID_PROTOCOL_INDEX;
}

String getCPluginNameFromProtocolIndex(protocolIndex_t ProtocolIndex) {
  String controllerName;

  if (validProtocolIndex(ProtocolIndex)) {
    CPlugin_ptr[ProtocolIndex](CPLUGIN_GET_DEVICENAME, nullptr, controllerName);
  }
  return controllerName;
}

String getCPluginNameFromCPluginID(cpluginID_t cpluginID) {
  protocolIndex_t protocolIndex = getProtocolIndex(cpluginID);

  if (!validProtocolIndex(protocolIndex)) {
    String name = F("CPlugin ");
    name += String(static_cast<int>(cpluginID));
    name += F(" not included in build");
    return name;
  }
  return getCPluginNameFromProtocolIndex(protocolIndex);
}
