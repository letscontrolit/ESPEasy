#include "NPlugins.h"

#include "../DataStructs/NotificationStruct.h"
#include "Settings.h"


nprotocolIndex_t INVALID_NPROTOCOL_INDEX = NPLUGIN_MAX;
notifierIndex_t  INVALID_NOTIFIER_INDEX  = NOTIFICATION_MAX;
npluginID_t      INVALID_N_PLUGIN_ID     = 0;


boolean (*NPlugin_ptr[NPLUGIN_MAX])(NPlugin::Function,
                                    struct EventStruct *,
                                    String&);
npluginID_t NPlugin_id[NPLUGIN_MAX];

NotificationStruct Notification[NPLUGIN_MAX];

int notificationCount = -1;


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
  for (byte x = 0; x <= notificationCount; x++) {
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
