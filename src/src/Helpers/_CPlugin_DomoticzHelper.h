#ifndef HELPERS__CPLUGIN_DOMOTICZHELPER_H
#define HELPERS__CPLUGIN_DOMOTICZHELPER_H

#include "../../ESPEasy_common.h"

#ifdef USES_DOMOTICZ

# include "../Helpers/_CPlugin_Helper.h"

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

# ifdef USES_C002
bool   deserializeDomoticzJson(const String& json,
                               unsigned int& idx,
                               float       & nvalue,
                               long        & nvaluealt,
                               String      & svalue1,
                               String      & switchtype);

String serializeDomoticzJson(struct EventStruct *event);

# endif // ifdef USES_C002
#endif // ifdef USES_DOMOTICZ


#endif // ifndef HELPERS__CPLUGIN_DOMOTICZHELPER_H
