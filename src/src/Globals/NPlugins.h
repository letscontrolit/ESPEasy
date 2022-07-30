#ifndef GLOBALS_NPLUGIN_H
#define GLOBALS_NPLUGIN_H

#include "../../ESPEasy_common.h"

#if FEATURE_NOTIFIER

#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/NotificationStruct.h"
#include "../DataStructs/NotificationSettingsStruct.h"
#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../DataTypes/NotifierIndex.h"
#include "../DataTypes/NPluginID.h"

#include <map>
#include <vector>
#include <Arduino.h>

typedef uint8_t    nprotocolIndex_t;

extern nprotocolIndex_t INVALID_NPROTOCOL_INDEX;


extern bool (*NPlugin_ptr[NPLUGIN_MAX])(NPlugin::Function,
                                           struct EventStruct *,
                                           String&);
extern npluginID_t NPlugin_id[NPLUGIN_MAX];

extern NotificationStruct Notification[NPLUGIN_MAX];

extern int notificationCount;


bool             NPluginCall(NPlugin::Function   Function,
                             struct EventStruct *event);

bool             validNProtocolIndex(nprotocolIndex_t index);
bool             validNotifierIndex(notifierIndex_t index);
bool             validNPluginID(npluginID_t npluginID);

String           getNPluginNameFromNotifierIndex(notifierIndex_t NotifierIndex);
nprotocolIndex_t getNProtocolIndex(npluginID_t Number);
nprotocolIndex_t getNProtocolIndex_from_NotifierIndex(notifierIndex_t index);
bool             addNPlugin(npluginID_t npluginID, nprotocolIndex_t x);

#endif

#endif // GLOBALS_NPLUGIN_H
