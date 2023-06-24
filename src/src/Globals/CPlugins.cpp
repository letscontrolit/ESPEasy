#include "../Globals/CPlugins.h"

#include "../../_Plugin_Helper.h"
#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataStructs/TimingStats.h"
#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/Protocol.h"
#include "../Globals/Settings.h"
#include "../Helpers/_CPlugin_init.h"


/********************************************************************************************\
   Call CPlugin functions
 \*********************************************************************************************/
bool CPluginCall(CPlugin::Function Function, struct EventStruct *event) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  String dummy;

  return CPluginCall(Function, event, dummy);
}

bool CPluginCall(CPlugin::Function Function, struct EventStruct *event, String& str)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  struct EventStruct TempEvent;

  if (event == 0) {
    event = &TempEvent;
  }

  switch (Function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
      // only called from CPluginSetup() directly using protocolIndex
      break;

    // calls to all active controllers
    case CPlugin::Function::CPLUGIN_INIT_ALL:
    case CPlugin::Function::CPLUGIN_UDP_IN:
    case CPlugin::Function::CPLUGIN_INTERVAL:      // calls to send stats information
    case CPlugin::Function::CPLUGIN_GOT_CONNECTED: // calls to send autodetect information
    case CPlugin::Function::CPLUGIN_GOT_INVALID:   // calls to mark unit as invalid
    case CPlugin::Function::CPLUGIN_FLUSH:
    case CPlugin::Function::CPLUGIN_TEN_PER_SECOND:
    case CPlugin::Function::CPLUGIN_FIFTY_PER_SECOND:
    case CPlugin::Function::CPLUGIN_WRITE:
    {
      const bool success = Function != CPlugin::Function::CPLUGIN_WRITE;

      if (Function == CPlugin::Function::CPLUGIN_INIT_ALL) {
        Function = CPlugin::Function::CPLUGIN_INIT;
      }

      for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
        if ((Settings.Protocol[x] != 0) && Settings.ControllerEnabled[x]) {
          event->ControllerIndex = x;
          String command;

          if (Function == CPlugin::Function::CPLUGIN_WRITE) {
            command = str;
          }

          if (CPluginCall(
                getProtocolIndex_from_ControllerIndex(x),
                Function,
                event,
                command)) {
            if (Function == CPlugin::Function::CPLUGIN_WRITE) {
              // Need to stop when write call was handled
              return true;
            }
          }
        }
      }
      return success;
    }

    // calls to specific controller
    case CPlugin::Function::CPLUGIN_INIT:
    case CPlugin::Function::CPLUGIN_EXIT:
    case CPlugin::Function::CPLUGIN_PROTOCOL_TEMPLATE:
    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    case CPlugin::Function::CPLUGIN_PROTOCOL_RECV:
    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    case CPlugin::Function::CPLUGIN_WEBFORM_LOAD:
    case CPlugin::Function::CPLUGIN_WEBFORM_SAVE:
    case CPlugin::Function::CPLUGIN_GET_PROTOCOL_DISPLAY_NAME:
    case CPlugin::Function::CPLUGIN_TASK_CHANGE_NOTIFICATION:
    case CPlugin::Function::CPLUGIN_WEBFORM_SHOW_HOST_CONFIG:
    {
      const controllerIndex_t controllerindex = event->ControllerIndex;
      bool success                            = false;

      if (validControllerIndex(controllerindex)) {
        if (Settings.ControllerEnabled[controllerindex] && supportedCPluginID(Settings.Protocol[controllerindex]))
        {
          if (Function == CPlugin::Function::CPLUGIN_PROTOCOL_SEND) {
            checkDeviceVTypeForTask(event);
          }
          success = CPluginCall(
            getProtocolIndex_from_ControllerIndex(controllerindex),
            Function,
            event,
            str);
        }
        #ifdef ESP32

        if (Function == CPlugin::Function::CPLUGIN_EXIT) {
          Cache.clearControllerSettings(controllerindex);
        }
        #endif // ifdef ESP32
      }
      return success;
    }

    case CPlugin::Function::CPLUGIN_ACKNOWLEDGE: // calls to send acknowledge back to controller

      for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
        if (Settings.ControllerEnabled[x] && supportedCPluginID(Settings.Protocol[x])) {
          CPluginCall(
            getProtocolIndex_from_ControllerIndex(x),
            Function,
            event,
            str);
        }
      }
      return true;
  }

  return false;
}

// Check if there is any controller enabled.
bool anyControllerEnabled() {
  for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
    if (Settings.ControllerEnabled[x] && supportedCPluginID(Settings.Protocol[x])) {
      return true;
    }
  }
  return false;
}

// Find first enabled controller index with this protocol
controllerIndex_t findFirstEnabledControllerWithId(cpluginID_t cpluginid) {
  if (supportedCPluginID(cpluginid)) {
    for (controllerIndex_t i = 0; i < CONTROLLER_MAX; i++) {
      if ((Settings.Protocol[i] == cpluginid) && Settings.ControllerEnabled[i]) {
        return i;
      }
    }
  }
  return INVALID_CONTROLLER_INDEX;
}

bool validProtocolIndex(protocolIndex_t index)
{
  return getCPluginID_from_ProtocolIndex(index) != INVALID_C_PLUGIN_ID;
}

bool validControllerIndex(controllerIndex_t index)
{
  return index < CONTROLLER_MAX;
}

bool validCPluginID(cpluginID_t cpluginID)
{
  return getProtocolIndex_from_CPluginID_(cpluginID) != INVALID_C_PLUGIN_ID;
}

bool supportedCPluginID(cpluginID_t cpluginID)
{
  return validProtocolIndex(getProtocolIndex_from_CPluginID_(cpluginID));
}

protocolIndex_t getProtocolIndex_from_ControllerIndex(controllerIndex_t index) {
  if (validControllerIndex(index)) {
    return getProtocolIndex_from_CPluginID_(Settings.Protocol[index]);
  }
  return INVALID_PROTOCOL_INDEX;
}

protocolIndex_t getProtocolIndex_from_CPluginID(cpluginID_t cpluginID) {
  return getProtocolIndex_from_CPluginID_(cpluginID);
}

cpluginID_t getCPluginID_from_ProtocolIndex(protocolIndex_t index) {
  return getCPluginID_from_ProtocolIndex_(index);
}

cpluginID_t getCPluginID_from_ControllerIndex(controllerIndex_t index) {
  const protocolIndex_t protocolIndex = getProtocolIndex_from_ControllerIndex(index);

  return getCPluginID_from_ProtocolIndex(protocolIndex);
}

String getCPluginNameFromProtocolIndex(protocolIndex_t ProtocolIndex) {
  String controllerName;

  if (validProtocolIndex(ProtocolIndex)) {
    CPluginCall(ProtocolIndex, CPlugin::Function::CPLUGIN_GET_DEVICENAME, nullptr, controllerName);
  }
  return controllerName;
}

String getCPluginNameFromCPluginID(cpluginID_t cpluginID) {
  protocolIndex_t protocolIndex = getProtocolIndex_from_CPluginID_(cpluginID);

  if (!validProtocolIndex(protocolIndex)) {
    String name = F("CPlugin ");
    name += String(static_cast<int>(cpluginID));
    name += F(" not included in build");
    return name;
  }
  return getCPluginNameFromProtocolIndex(protocolIndex);
}
