#ifndef HELPERS__CPLUGIN_DOMOTICZHELPER_H
#define HELPERS__CPLUGIN_DOMOTICZHELPER_H

#include "../../ESPEasy_common.h"

#ifdef USES_DOMOTICZ

// HUM_STAT can be one of:

// 0=Normal
// 1=Comfortable
// 2=Dry
// 3=Wet
String humStatDomoticz(struct EventStruct *event,
                       byte                rel_index);

int    mapRSSItoDomoticz();

int    mapVccToDomoticz();

// Format including trailing semi colon
String formatUserVarDomoticz(struct EventStruct *event,
                             byte                rel_index);

String formatUserVarDomoticz(int value);

String formatDomoticzSensorType(struct EventStruct *event);
#endif // USES_DOMOTICZ


#endif // ifndef HELPERS__CPLUGIN_DOMOTICZHELPER_H
