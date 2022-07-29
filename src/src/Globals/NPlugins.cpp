#include "../Globals/NPlugins.h"

#if FEATURE_NOTIFIER

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataStructs/NotificationStruct.h"
#include "../Globals/Settings.h"


nprotocolIndex_t INVALID_NPROTOCOL_INDEX = NPLUGIN_MAX;


bool (*NPlugin_ptr[NPLUGIN_MAX])(NPlugin::Function,
                                    struct EventStruct *,
                                    String&);
npluginID_t NPlugin_id[NPLUGIN_MAX];

NotificationStruct Notification[NPLUGIN_MAX];

int notificationCount = -1;


bool NPluginCall(NPlugin::Function Function, struct EventStruct *event)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif
  int x;
  struct EventStruct TempEvent;

  if (event == 0) {
    event = &TempEvent;
  }

  switch (Function)
  {
    // Unconditional calls to all plugins
    case NPlugin::Function::NPLUGIN_PROTOCOL_ADD:

      for (x = 0; x < NPLUGIN_MAX; x++) {
        if (validNPluginID(NPlugin_id[x])) {
          String dummy;
          NPlugin_ptr[x](Function, event, dummy);
        }
      }
      return true;
      break;

    case NPlugin::Function::NPLUGIN_GET_DEVICENAME:
    case NPlugin::Function::NPLUGIN_WEBFORM_SAVE:
    case NPlugin::Function::NPLUGIN_WEBFORM_LOAD:
    case NPlugin::Function::NPLUGIN_WRITE:
    case NPlugin::Function::NPLUGIN_NOTIFY:
      break;
  }

  return false;
}


bool validNProtocolIndex(nprotocolIndex_t index) {
  return index != INVALID_NPROTOCOL_INDEX;
}

bool validNotifierIndex(notifierIndex_t index)
{
  return index < NOTIFICATION_MAX;
}

bool validNPluginID(npluginID_t npluginID)
{
  if (npluginID == INVALID_N_PLUGIN_ID) {
    return false;
  }

  // FIXME TD-er: Must search to all included plugins
  return true;
}

String getNPluginNameFromNotifierIndex(notifierIndex_t NotifierIndex) {
  String name;

  if (validNPluginID(NPlugin_id[NotifierIndex])) {
    NPlugin_ptr[NotifierIndex](NPlugin::Function::NPLUGIN_GET_DEVICENAME, nullptr, name);
  }
  return name;
}

/********************************************************************************************\
   Get notificatoin protocol index (plugin index), by NPlugin_id
 \*********************************************************************************************/
nprotocolIndex_t getNProtocolIndex(npluginID_t Number)
{
  for (uint8_t x = 0; x <= notificationCount; x++) {
    if (Notification[x].Number == Number) {
      return x;
    }
  }
  return INVALID_NPROTOCOL_INDEX;
}

nprotocolIndex_t getNProtocolIndex_from_NotifierIndex(notifierIndex_t index) {
  if (validNotifierIndex(index)) {
    return getNProtocolIndex(Settings.Notification[index]);
  }
  return INVALID_NPROTOCOL_INDEX;
}

bool addNPlugin(npluginID_t npluginID, nprotocolIndex_t x) {
  if (x < NPLUGIN_MAX) { 
    // FIXME TD-er: Must add lookup for notification plugins too
//    ProtocolIndex_to_NPlugin_id[x] = npluginID; 
//    NPlugin_id_to_ProtocolIndex[npluginID] = x;

    NPlugin_id[x] = npluginID;
    return true;
  }
  /*
  {
    String log = F("System: Error - Too many N-Plugins. NPLUGIN_MAX = ");
    log += NPLUGIN_MAX;
    addLog(LOG_LEVEL_ERROR, log);
  }
  */
  return false;
}

#endif