#ifndef HELPERS__CPLUGIN_HELPER_WEBFORM_H
#define HELPERS__CPLUGIN_HELPER_WEBFORM_H

#include <Arduino.h>

#include "../DataStructs/ControllerSettingsStruct.h"
#include "../Globals/CPlugins.h"


/*********************************************************************************************\
* Functions to load and store controller settings on the web page.
\*********************************************************************************************/
String getControllerParameterName(protocolIndex_t                   ProtocolIndex,
                                  ControllerSettingsStruct::VarType parameterIdx,
                                  bool                              displayName,
                                  bool                            & isAlternative);

String getControllerParameterInternalName(protocolIndex_t ProtocolIndex, ControllerSettingsStruct::VarType parameterIdx);

String getControllerParameterDisplayName(protocolIndex_t ProtocolIndex, ControllerSettingsStruct::VarType parameterIdx, bool& isAlternative);

void addControllerEnabledForm(controllerIndex_t controllerindex);

void addControllerParameterForm(const ControllerSettingsStruct& ControllerSettings, controllerIndex_t controllerindex, ControllerSettingsStruct::VarType varType);

void saveControllerParameterForm(ControllerSettingsStruct        & ControllerSettings,
                                 controllerIndex_t                 controllerindex,
                                 ControllerSettingsStruct::VarType varType);




#endif