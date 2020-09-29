#include "ESPEasy_common.h"

#include "src/Globals/CPlugins.h"
#include "src/Globals/Protocol.h"
#include "src/Globals/Settings.h"

#include "src/DataStructs/ESPEasy_EventStruct.h"
#include "src/DataStructs/ESPEasy_plugin_functions.h"
#include "src/DataStructs/TimingStats.h"


// ********************************************************************************
// Initialize all Controller CPlugins that where defined earlier
// and initialize the function call pointer into the CCPlugin array
// ********************************************************************************

static const char ADDCPLUGIN_ERROR[] PROGMEM = "System: Error - Too many C-Plugins";

// Because of compiler-bug (multiline defines gives an error if file ending is CRLF) the define is striped to a single line

/*
 #define ADDCPLUGIN(NNN) \
   if (x < CPLUGIN_MAX) \
   { \
    ProtocolIndex_to_CPlugin_id[x] = CPLUGIN_ID_##NNN; \
    CPlugin_id_to_ProtocolIndex[CPLUGIN_ID_##NNN] = x; \
    CPlugin_ptr[x++] = &CPlugin_##NNN; \
   } \
  else \
    addLog(LOG_LEVEL_ERROR, FPSTR(ADDCPLUGIN_ERROR));
*/
#define ADDCPLUGIN(NNN) if (x < CPLUGIN_MAX) {     ProtocolIndex_to_CPlugin_id[x] = CPLUGIN_ID_##NNN; CPlugin_id_to_ProtocolIndex[CPLUGIN_ID_##NNN] = x; CPlugin_ptr[x++] = &CPlugin_##NNN; } else addLog(LOG_LEVEL_ERROR, FPSTR(ADDCPLUGIN_ERROR));

void CPluginInit(void)
{
  ProtocolIndex_to_CPlugin_id.resize(CPLUGIN_MAX + 1); // INVALID_CONTROLLER_INDEX may be used as index for this array.
  ProtocolIndex_to_CPlugin_id[CPLUGIN_MAX] = INVALID_C_PLUGIN_ID;
  byte x;

  // Clear pointer table for all plugins
  for (x = 0; x < CPLUGIN_MAX; x++)
  {
    CPlugin_ptr[x]                 = nullptr;
    ProtocolIndex_to_CPlugin_id[x] = INVALID_C_PLUGIN_ID;
    // Do not initialize CPlugin_id_to_ProtocolIndex[x] to an invalid value. (it is map)
  }

  x = 0;

#ifdef CPLUGIN_001
  ADDCPLUGIN(001)
#endif

#ifdef CPLUGIN_002
  ADDCPLUGIN(002)
#endif

#ifdef CPLUGIN_003
  ADDCPLUGIN(003)
#endif

#ifdef CPLUGIN_004
  ADDCPLUGIN(004)
#endif

#ifdef CPLUGIN_005
  ADDCPLUGIN(005)
#endif

#ifdef CPLUGIN_006
  ADDCPLUGIN(006)
#endif

#ifdef CPLUGIN_007
  ADDCPLUGIN(007)
#endif

#ifdef CPLUGIN_008
  ADDCPLUGIN(008)
#endif

#ifdef CPLUGIN_009
  ADDCPLUGIN(009)
#endif

#ifdef CPLUGIN_010
  ADDCPLUGIN(010)
#endif

#ifdef CPLUGIN_011
  ADDCPLUGIN(011)
#endif

#ifdef CPLUGIN_012
  ADDCPLUGIN(012)
#endif

#ifdef CPLUGIN_013
  ADDCPLUGIN(013)
#endif

#ifdef CPLUGIN_014
  ADDCPLUGIN(014)
#endif

#ifdef CPLUGIN_015
  ADDCPLUGIN(015)
#endif

#ifdef CPLUGIN_016
  ADDCPLUGIN(016)
#endif

#ifdef CPLUGIN_017
  ADDCPLUGIN(017)
#endif

#ifdef CPLUGIN_018
  ADDCPLUGIN(018)
#endif

#ifdef CPLUGIN_019
  ADDCPLUGIN(019)
#endif

#ifdef CPLUGIN_020
  ADDCPLUGIN(020)
#endif

#ifdef CPLUGIN_021
  ADDCPLUGIN(021)
#endif

#ifdef CPLUGIN_022
  ADDCPLUGIN(022)
#endif

#ifdef CPLUGIN_023
  ADDCPLUGIN(023)
#endif

#ifdef CPLUGIN_024
  ADDCPLUGIN(024)
#endif

#ifdef CPLUGIN_025
  ADDCPLUGIN(025)
#endif

// When extending this, search for EXTEND_CONTROLLER_IDS 
// in the code to find all places that need to be updated too.


  CPluginCall(CPlugin::Function::CPLUGIN_PROTOCOL_ADD, 0);

  // Set all not supported cplugins to disabled.
  for (controllerIndex_t controller = 0; controller < CONTROLLER_MAX; ++controller) {
    if (!supportedCPluginID(Settings.Protocol[controller])) {
      Settings.ControllerEnabled[controller] = false;
    }
  }
  CPluginCall(CPlugin::Function::CPLUGIN_INIT_ALL, 0);
}
