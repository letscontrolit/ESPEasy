#ifndef HELPERS__CPLUGIN_DOMOTICZHELPER_H
#define HELPERS__CPLUGIN_DOMOTICZHELPER_H

#include "../../ESPEasy_common.h"

#if FEATURE_DOMOTICZ

# include "../Helpers/_CPlugin_Helper.h"


int    mapRSSItoDomoticz();

int    mapVccToDomoticz();

String formatDomoticzSensorType(struct EventStruct *event);

# ifdef USES_C002
bool   deserializeDomoticzJson(const String& json,
                               unsigned int& idx,
                               float       & nvalue,
                               int32_t        & nvaluealt,
                               String      & svalue1,
                               String      & switchtype);

String serializeDomoticzJson(struct EventStruct *event);

# endif // ifdef USES_C002
#endif // if FEATURE_DOMOTICZ


#endif // ifndef HELPERS__CPLUGIN_DOMOTICZHELPER_H
