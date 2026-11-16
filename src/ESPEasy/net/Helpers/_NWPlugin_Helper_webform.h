#pragma once

#include "../../../ESPEasy_common.h"

#include "../DataStructs/NetworkSettingsStruct.h"
#include "../DataTypes/NetworkIndex.h"
#include "../DataTypes/NetworkDriverIndex.h"


namespace ESPEasy {
namespace net {


/*********************************************************************************************\
* Functions to load and store network settings on the web page.
\*********************************************************************************************/
String getNetworkParameterName(networkDriverIndex_t           NetworkDriverIndex,
                               NetworkSettingsStruct::VarType parameterIdx,
                               bool                           displayName,
                               bool                         & isAlternative);



void addNetworkEnabledForm(networkIndex_t networkindex);

/*
void addNetworkParameterForm(const NetworkSettingsStruct  & NetworkSettings,
                             networkIndex_t                 networkindex,
                             NetworkSettingsStruct::VarType varType);
*/
void saveNetworkParameterForm(NetworkSettingsStruct        & NetworkSettings,
                              networkIndex_t                 networkindex,
                              NetworkSettingsStruct::VarType varType);

} // namespace net
} // namespace ESPEasy
