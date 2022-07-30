#ifndef COMMAND_BLYNK_H
#define COMMAND_BLYNK_H

#include "../../ESPEasy_common.h"

#ifdef USES_C012

#include "../Globals/CPlugins.h"


controllerIndex_t firstEnabledBlynk_ControllerIndex();

// FIXME: this should go to PLUGIN_WRITE in _C012.ino
const __FlashStringHelper * Command_Blynk_Get(struct EventStruct *event,
                                    const char         *Line);

bool Blynk_get(const String    & command,
               controllerIndex_t controllerIndex,
               float            *data = nullptr);

#endif // ifdef USES_C012

#endif // COMMAND_BLYNK_H
