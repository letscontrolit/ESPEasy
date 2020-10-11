#ifndef GLOBALS_NPLUGIN_H
#define GLOBALS_NPLUGIN_H

#include <map>
#include <vector>
#include "../DataStructs/ESPEasyLimits.h"
#include "../DataStructs/ESPEasy_plugin_functions.h"

struct NotificationStruct;

typedef byte    nprotocolIndex_t;
typedef byte    notifierIndex_t;
typedef uint8_t npluginID_t;

extern nprotocolIndex_t INVALID_NPROTOCOL_INDEX;
extern notifierIndex_t  INVALID_NOTIFIER_INDEX;
extern npluginID_t      INVALID_N_PLUGIN_ID;


extern boolean (*NPlugin_ptr[NPLUGIN_MAX])(NPlugin::Function,
                                           struct EventStruct *,
                                           String&);
extern npluginID_t NPlugin_id[NPLUGIN_MAX];

extern NotificationStruct Notification[NPLUGIN_MAX];

extern int notificationCount;


byte             NPluginCall(NPlugin::Function   Function,
                             struct EventStruct *event);

bool             validNProtocolIndex(nprotocolIndex_t index);
bool             validNotifierIndex(notifierIndex_t index);
bool             validNPluginID(npluginID_t npluginID);

String           getNPluginNameFromNotifierIndex(notifierIndex_t NotifierIndex);
nprotocolIndex_t getNProtocolIndex(npluginID_t Number);
nprotocolIndex_t getNProtocolIndex_from_NotifierIndex(notifierIndex_t index);


#endif // GLOBALS_NPLUGIN_H
